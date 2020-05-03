#include <functional>
#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <vector>

#include "benchmarks.h"

using std::cout;

class BenchFramework {
public:
    BenchFramework() {
        // All types of trees are listed below.
        //types_.emplace("AVL tree", ImplType::kAVL);
        //types_.emplace("Cartesian tree", ImplType::kCartesian);
        //types_.emplace("Red-Black tree", ImplType::kRB);
        types_.emplace("Skip list", ImplType::kSkipList);
        //types_.emplace("Splay tree", ImplType::kSplay);

        /* All benchmarks are listed below.
         * We'll use '!' for good benchmarks that we need,
         * '%' for useless and demonstrative benchmarks.
         * You can also use your own symbol for your benchmarks.
         */
        benchmarks_.emplace("%_simple_bench", SimpleBench);
    }

    struct Range {
        Range() = delete;
        Range(uint64_t begin, uint64_t end, uint64_t step = 1, uint64_t num_folds = 5) {
            begin_ = begin;
            end_ = end;
            step_ = step;
            num_folds_ = num_folds;
        }

        uint64_t begin_;
        uint64_t end_;
        uint64_t step_;
        uint64_t num_folds_;
    };

    template <class BenchPredicate>
    void RunBenchmarks(const std::string &path, const Range &range,
                       BenchPredicate bench_predicate) {
        std::random_device device;
        std::mt19937 gen(device());
        for (auto &bench : benchmarks_) {
            if (bench_predicate(bench.first)) {
                cout << "Running " << bench.first << ": ";
                std::ofstream out(path + bench.first + ".csv");
                out.precision(3);
                out << "op_count";
                for (auto &type : types_) {
                    for (uint64_t i = 0; i < range.num_folds_; ++i) {
                        out << ", " << type.first << "_split_" << i;
                    }
                }
                try {
                    for (uint64_t i = range.begin_; i < range.end_; i += range.step_) {
                        out << '\n' << std::to_string(i);
                        for (auto &type : types_) {
                            for (uint64_t fold = 0; fold < range.num_folds_; ++fold) {
                                out << ", " << std::fixed << bench.second(type.second, gen, i);
                            }
                        }
                    }
                } catch (std::exception &ex) {
                    cout << "Failure: " << ex.what() << "\n";
                } catch (...) {
                    cout << "Failure: Unknown exception. PLEASE THROW \"std::exception\" BASED "
                            "EXCEPTIONS\n";
                }
                out.close();
                cout << "OK\n\n";
            }
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
