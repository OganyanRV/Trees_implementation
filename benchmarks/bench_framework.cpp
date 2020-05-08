#include <functional>
#include <iostream>
#include <map>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

#include "benchmarks.h"

using std::cout;

/// stdout access blocking
std::mutex stdout_mutex_;

/**
 * Framework for speed testing ITree based structures
 */
class BenchFramework {
public:
    BenchFramework() {
        /// All types of trees are listed below.
        types_.emplace("AVL_tree", ImplType::kAVL);
        types_.emplace("Cartesian_tree", ImplType::kCartesian);
        types_.emplace("Red-Black_tree", ImplType::kRB);
        types_.emplace("Skip_list", ImplType::kSkipList);
        types_.emplace("Splay_tree", ImplType::kSplay);
        types_.emplace("Stdlib_set", ImplType::kSet);

        /**
         * All benchmarks are listed below.
         * To determine it's ours, we put '!' in the beginning
         */
        benchmarks_.emplace("!_increasing_int_series_insert_bench", IncreasingIntSeriesInsert);
        benchmarks_.emplace("!_decreasing_int_series_insert_bench", DecreasingIntSeriesInsert);
        benchmarks_.emplace("!_converging_int_series_insert_bench", ConvergingIntSeriesInsert);
        benchmarks_.emplace("!_diverging_int_series_insert_bench", DivergingIntSeriesInsert);
        benchmarks_.emplace("!_random_sparse_int_series_insert_bench", RandomSparseIntSeriesInsert);
        benchmarks_.emplace("!_random_dense_int_series_insert_bench", RandomDenseIntSeriesInsert);
        benchmarks_.emplace("!_random_sparse_strings_insert_bench", RandomSparseStringsInsert);
        benchmarks_.emplace("!_random_dense_strings_insert_bench", RandomDenseStringsInsert);

        benchmarks_.emplace("!_increasing_int_series_erase_after_increasing_series_insert_bench",
                            IncreasingIntSeriesEraseAfterIncreasingSeriesInsert);
        benchmarks_.emplace("!_decreasing_int_series_erase_after_increasing_series_insert_bench",
                            DecreasingIntSeriesEraseAfterIncreasingSeriesInsert);
        benchmarks_.emplace("!_converging_int_series_erase_after_increasing_series_insert_bench",
                            ConvergingIntSeriesEraseAfterIncreasingSeriesInsert);
        benchmarks_.emplace("!_diverging_int_series_erase_after_increasing_series_insert_bench",
                            DivergingIntSeriesEraseAfterIncreasingSeriesInsert);
        benchmarks_.emplace("!_nonexistent_int_series_erase_after_increasing_series_insert_bench",
                            NonexistentIntSeriesEraseAfterIncreasingSeriesInsert);
        benchmarks_.emplace("!_random_int_series_erase_after_increasing_series_insert_bench",
                            RandomIntSeriesEraseAfterIncreasingSeriesInsert);

        benchmarks_.emplace("!_increasing_int_series_erase_after_random_sparse_series_insert_bench",
                            IncreasingIntSeriesEraseAfterRandomSparseSeriesInsert);
        benchmarks_.emplace("!_decreasing_int_series_erase_after_random_sparse_series_insert_bench",
                            DecreasingIntSeriesEraseAfterRandomSparseSeriesInsert);
        benchmarks_.emplace("!_converging_int_series_erase_after_random_sparse_series_insert_bench",
                            ConvergingIntSeriesEraseAfterRandomSparseSeriesInsert);
        benchmarks_.emplace("!_diverging_int_series_erase_after_random_sparse_series_insert_bench",
                            DivergingIntSeriesEraseAfterRandomSparseSeriesInsert);
        benchmarks_.emplace(
            "!_nonexistent_int_series_erase_after_random_sparse_series_insert_bench",
            NonexistentIntSeriesEraseAfterRandomSparseSeriesInsert);
        benchmarks_.emplace("!_random_int_series_erase_after_random_sparse_series_insert_bench",
                            RandomIntSeriesEraseAfterRandomSparseSeriesInsert);

        benchmarks_.emplace("!_random_strings_erase_after_random_insert_bench",
                            RandomStringsEraseAfterRandomInsert);
        benchmarks_.emplace("!_nonexistent_strings_erase_after_random_insert_bench",
                            NonexistentStringsEraseAfterRandomInsert);

        benchmarks_.emplace("!_random_insert_and_erase_int_alternation_bench",
                            RandomInsertAndEraseIntAlternation);

        benchmarks_.emplace("!_find_int_after_random_sparse_insert_bench",
                            FindIntAfterRandomSparseInsert);
        benchmarks_.emplace("!_find_random_sparse_int_after_random_sparse_insert_bench",
                            FindRandomSparseIntAfterRandomSparseInsert);
        benchmarks_.emplace("!_lower_bound_random_sparse_int_after_random_sparse_insert_bench",
                            LowerBoundRandomSparseIntAfterRandomSparseInsert);
    }

    /**
     * This structure specifies how to test our trees
     * begin and end are the boundaries of the value interval
     * if log_scale is set, then step specifies how many values (in log scale) will be tested
     * if not, then step is just step, which is added to the current value
     * num_folds specifies the number of identical tests for averaging
     */
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
    /**
     * This function runs given bench with given range and tree types
     * Results are written to the file, the name of which matches the name of the bench
     * @param bench Benchmark to run
     * @param path Path to the results folder
     * @param range Range for benchmarking
     * @param types Tree types for benchmarking
     * @param gen Mersenne Twister generator
     */
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
            cout << bench.first << "\tfailure: " << ex.what() << '\n';
        } catch (...) {
            std::lock_guard<std::mutex> lockGuard(stdout_mutex_);
            cout << bench.first
                 << "\tfailure: Unknown exception. PLEASE THROW \"std::exception\" BASED "
                    "EXCEPTIONS\n";
        }
        out.close();
        auto end = std::chrono::high_resolution_clock::now();
        double time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() *
                      nanoMultiplier;
        std::lock_guard<std::mutex> lockGuard(stdout_mutex_);
        cout << bench.first << ":\tOK. Time spent: " << time << "ms\n";
    }

public:
    /**
     * This function runs all benchmarks, which satisfy the given predicate.
     * It takes common range and path to the folder with results.
     * The number of used threads always doesn't exceed 4.
     * @tparam BenchPredicate Template parameter functor
     * @param path Path to the results folder
     * @param range Range for benchmarking
     * @param bench_predicate Functor for benchmarking
     */
    template <class BenchPredicate>
    void RunBenchmarks(const std::string &path, const Range &range,
                       BenchPredicate bench_predicate) {
        std::random_device device;
        std::queue<std::thread> threads;
        auto it = benchmarks_.begin();
        while (it != benchmarks_.end() && threads.size() < 3) {
            if (bench_predicate(it->first)) {
                stdout_mutex_.lock();
                cout << "Running " << it->first << '\n';
                stdout_mutex_.unlock();
                threads.emplace(RunBench, std::ref(*it), std::ref(path), std::ref(range),
                                std::ref(types_), std::mt19937(device()));
                ++it;
            }
        }
        while (it != benchmarks_.end()) {
            if (bench_predicate(it->first)) {
                stdout_mutex_.lock();
                cout << "Running " << it->first << '\n';
                stdout_mutex_.unlock();
                threads.emplace(RunBench, std::ref(*it), std::ref(path), std::ref(range),
                                std::ref(types_), std::mt19937(device()));
                ++it;
                threads.front().join();
                threads.pop();
            }
        }
        while (!threads.empty()) {
            threads.front().join();
            threads.pop();
        }
    }

    /**
     * This function generalizes 'RunBenchmarks()' to use all available benchmarks
     * @param path Path to the results folder
     * @param range Range for benchmarking
     */
    void RunAllBenchmarks(const std::string &path, const Range &range) {
        RunBenchmarks(path, range, Every());
    }

private:
    /// Types of trees (name + type)
    std::map<std::string, ImplType> types_;
    /// Benchmarks (name + function)
    std::map<std::string, std::function<double(ImplType, std::mt19937 &, uint64_t)>> benchmarks_;

    /// Functor for every object in a list
    class Every {
    public:
        /**
         * Functor, which always returns true
         * @param arg String to compare with
         * @return True
         */
        bool operator()(const std::string &arg) {
            return true;
        }
    };
};

/// Functor for objects, which has a substring in their name matching to @param str_
class Substr {
public:
    /**
     * Constructor for the functor
     * @param str Substring, which we are going to find
     */
    explicit Substr(const char *str) : str_(str) {
    }

    /**
     * Functor
     * @param arg String to compare with
     * @return True if 'str_' is contained in @param arg
     */
    bool operator()(const std::string &arg) {
        return arg.find(str_) != std::string::npos;
    }

private:
    std::string str_;
};

/// Functor for objects, which name matches to the given string
class FullMatch {
public:
    /**
     * Constructor for the functor
     * @param str String, which we are going to compare with
     */
    explicit FullMatch(const char *str) : str_(str) {
    }

    /**
     * Functor
     * @param arg String to compare with
     * @return true if 'str_' is matched with @param arg
     */
    bool operator()(const std::string &arg) {
        return arg == str_;
    }

private:
    std::string str_;
};
