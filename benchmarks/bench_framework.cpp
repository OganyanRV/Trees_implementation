#include <functional>
#include <iostream>
#include <fstream>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "benchmarks.h"

using std::cout;
std::mutex stdout_mutex_;

class BenchFramework {
public:
    BenchFramework() {
        // All types of trees are listed below.
        types_.emplace("AVL_tree", ImplType::kAVL);
        types_.emplace("Cartesian_tree", ImplType::kCartesian);
        types_.emplace("Red-Black_tree", ImplType::kRB);
        types_.emplace("Skip_list", ImplType::kSkipList);
        types_.emplace("Splay_tree", ImplType::kSplay);
        types_.emplace("Stdlib_set", ImplType::kSet);

        /* All benchmarks are listed below.
         * We'll use '!' for good benchmarks that we need,
         * '%' for useless and demonstrative benchmarks.
         * You can also use your own symbol for your benchmarks.
         */
        benchmarks_.emplace("!_increasing_int_series_insert_bench", IncreasingIntSeriesInsert);
        benchmarks_.emplace("!_decreasing_int_series_insert_bench", DecreasingIntSeriesInsert);
        benchmarks_.emplace("!_converging_int_series_insert_bench", ConvergingIntSeriesInsert);
        benchmarks_.emplace("!_diverging_int_series_insert_bench", DivergingIntSeriesInsert);
        benchmarks_.emplace("!_random_sparse_int_series_insert_bench", RandomSparseIntSeriesInsert);
        benchmarks_.emplace("!_random_dense_int_series_insert_bench", RandomDenseIntSeriesInsert);
    }

    struct Range {
        Range() = delete;
        Range(uint64_t begin, uint64_t end, uint64_t step = 1, bool log_scale = false,
              uint64_t num_folds = 5) {
            begin_ = begin;
            end_ = end;
            step_ = step;
            log_scale_ = log_scale;
            num_folds_ = num_folds;
        }

        uint64_t begin_;
        uint64_t end_;
        uint64_t step_;
        uint64_t num_folds_;
        bool log_scale_;
    };

private:
    static void RunBench(
        std::pair<const std::string, std::function<double(ImplType, std::mt19937 &, uint64_t)>>
            &bench,
        const std::string &path, const Range &range, const std::map<std::string, ImplType> &types,
        std::mt19937 gen) {
        auto begin = std::chrono::high_resolution_clock::now();
        std::ofstream out(path + bench.first + ".csv");
        out.precision(3);
        out << "op_count";
        for (auto &type : types) {
            for (uint64_t i = 0; i < range.num_folds_; ++i) {
                out << ", " << type.first << "_split_" << i;
            }
        }
        try {
            if (range.log_scale_) {
                assert(range.step_ > 1);
                long double step =
                    std::pow((long double)(range.end_) / range.begin_, 1.0l / (range.step_ - 1));
                long double cur_approx = range.begin_;
                uint64_t prev = 0;
                for (uint64_t i = 1; i < range.step_; ++i) {
                    uint64_t cur = std::floor(cur_approx);
                    if (cur == prev) {
                        cur_approx *= step;
                        continue;
                    }
                    out << '\n' << std::to_string(cur);
                    for (auto &type : types) {
                        for (uint64_t fold = 0; fold < range.num_folds_; ++fold) {
                            out << ", " << std::fixed << bench.second(type.second, gen, cur);
                        }
                    }
                    cur_approx *= step;
                    prev = cur;
                }
                out << '\n' << std::to_string(range.end_);
                for (auto &type : types) {
                    for (uint64_t fold = 0; fold < range.num_folds_; ++fold) {
                        out << ", " << std::fixed << bench.second(type.second, gen, range.end_);
                    }
                }
            } else {
                for (uint64_t i = range.begin_; i <= range.end_; i += range.step_) {
                    out << '\n' << std::to_string(i);
                    for (auto &type : types) {
                        for (uint64_t fold = 0; fold < range.num_folds_; ++fold) {
                            out << ", " << std::fixed << bench.second(type.second, gen, i);
                        }
                    }
                }
            }
        } catch (std::exception &ex) {
            std::lock_guard<std::mutex> lockGuard(stdout_mutex_);
            cout << bench.first << " failure: " << ex.what() << '\n';
        } catch (...) {
            std::lock_guard<std::mutex> lockGuard(stdout_mutex_);
            cout << bench.first
                 << " failure: Unknown exception. PLEASE THROW \"std::exception\" BASED "
                    "EXCEPTIONS\n";
        }
        out.close();
        auto end = std::chrono::high_resolution_clock::now();
        double time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() *
                      nanoMultiplier;
        std::lock_guard<std::mutex> lockGuard(stdout_mutex_);
        cout << bench.first << ": OK. Time spent: " << time << "ms\n";
    }

public:
    template <class BenchPredicate>
    void RunBenchmarks(const std::string &path, const Range &range,
                       BenchPredicate bench_predicate) {
        std::random_device device;
        std::vector<std::thread> threads;
        for (auto &bench : benchmarks_) {
            if (bench_predicate(bench.first)) {
                stdout_mutex_.lock();
                cout << "Running " << bench.first << '\n';
                stdout_mutex_.unlock();
                threads.emplace_back(RunBench, std::ref(bench), std::ref(path), std::ref(range),
                                     std::ref(types_), std::mt19937(device()));
            }
        }
        for (auto &thread : threads) {
            thread.join();
        }
    }

    void RunAllBenchmarks(const std::string &path, const Range &range) {
        RunBenchmarks(path, range, Every());
    }

private:
    std::map<std::string, ImplType> types_;
    std::map<std::string, std::function<double(ImplType, std::mt19937 &, uint64_t)>> benchmarks_;

    class Every {
    public:
        bool operator()(const std::string &arg) {
            return true;
        }
    };
};

class Substr {
public:
    explicit Substr(const char *str) : str_(str) {
    }

    bool operator()(const std::string &arg) {
        return arg.find(str_) != std::string::npos;
    }

private:
    std::string str_;
};

class FullMatch {
public:
    explicit FullMatch(const char *str) : str_(str) {
    }

    bool operator()(const std::string &arg) {
        return arg == str_;
    }

private:
    std::string str_;
};
