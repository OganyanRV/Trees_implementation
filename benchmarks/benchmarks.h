#pragma once
#include <algorithm>
#include <chrono>
#include <fstream>
#include <ostream>
#include <random>
#include <vector>

#include "../trees/abstract_tree.h"
#include "../trees/avl_tree.h"
#include "../trees/cartesian_tree.h"
#include "../trees/rb_tree.h"
#include "../trees/skip_list.h"
#include "../trees/splay_tree.h"
#include "../trees/stdlib_set.h"

/// Nanoseconds to milliseconds
#define nanoMultiplier 1e-6

/// All types of trees
enum class ImplType { kAVL, kCartesian, kRB, kSkipList, kSplay, kSet };

/**
 * Makes a tree of given type and returns a shared pointer on it
 * @tparam T Tree value type
 * @tparam Types Types of constructor parameters
 * @param type Type of tree to make
 * @param params Parameters for tree constructor
 * @return shared pointer on a new tree
 */
template <class T, class... Types>
std::shared_ptr<ITree<T>> MakeTree(ImplType type, Types... params) {
    if (type == ImplType::kAVL) {
        return std::make_shared<AVLTree<T>>(params...);
    } else if (type == ImplType::kCartesian) {
        return std::make_shared<CartesianTree<T>>(params...);
    } else if (type == ImplType::kRB) {
        return std::make_shared<RBTree<T>>(params...);
    } else if (type == ImplType::kSkipList) {
        return std::make_shared<SkipList<T>>(params...);
    } else if (type == ImplType::kSplay) {
        return std::make_shared<SplayTree<T>>(params...);
    } else if (type == ImplType::kSet) {
        return std::make_shared<StdlibSet<T>>(params...);
    } else {
        throw std::runtime_error("Impossible behaviour");
    }
}

/**
 * All functions below are same
 *
 * Benchmark
 * @param type Type of tree to check
 * @param gen Mersenne Twister generator
 * @param op_count Number of operations to do
 * @return Operating time in milliseconds
 */
double IncreasingIntSeriesInsert(ImplType type, std::mt19937& gen, uint64_t op_count) {
    auto tree = MakeTree<int>(type);
    auto begin = std::chrono::high_resolution_clock::now();
    for (uint64_t i = 0; i < op_count; ++i) {
        tree->insert(i);
    }
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() *
           nanoMultiplier;
}

double DecreasingIntSeriesInsert(ImplType type, std::mt19937& gen, uint64_t op_count) {
    auto tree = MakeTree<int>(type);
    auto begin = std::chrono::high_resolution_clock::now();
    for (uint64_t i = 0; i < op_count; ++i) {
        tree->insert(-i);
    }
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() *
           nanoMultiplier;
}

double ConvergingIntSeriesInsert(ImplType type, std::mt19937& gen, uint64_t op_count) {
    auto tree = MakeTree<int>(type);
    auto begin = std::chrono::high_resolution_clock::now();
    for (uint64_t i = 0; i<op_count>> 1u; ++i) {
        tree->insert(i);
        tree->insert(op_count - i - 1);
    }
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() *
           nanoMultiplier;
}

double DivergingIntSeriesInsert(ImplType type, std::mt19937& gen, uint64_t op_count) {
    auto tree = MakeTree<int>(type);
    auto begin = std::chrono::high_resolution_clock::now();
    for (uint64_t i = op_count >> 1u; i < op_count; ++i) {
        tree->insert(i);
        tree->insert(op_count - i - 1);
    }
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() *
           nanoMultiplier;
}

double RandomSparseIntSeriesInsert(ImplType type, std::mt19937& gen, uint64_t op_count) {
    auto tree = MakeTree<int>(type);
    std::uniform_int_distribution dist(std::numeric_limits<int>::min(),
                                       std::numeric_limits<int>::max());
    auto begin = std::chrono::high_resolution_clock::now();
    for (uint64_t i = 0; i < op_count; ++i) {
        tree->insert(dist(gen));
    }
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() *
           nanoMultiplier;
}

double RandomDenseIntSeriesInsert(ImplType type, std::mt19937& gen, uint64_t op_count) {
    auto tree = MakeTree<int>(type);
    std::uniform_int_distribution dist(0ul, op_count / 5);
    auto begin = std::chrono::high_resolution_clock::now();
    for (uint64_t i = 0; i < op_count; ++i) {
        tree->insert(dist(gen));
    }
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() *
           nanoMultiplier;
}

double RandomSparseStringsInsert(ImplType type, std::mt19937& gen, uint64_t op_count) {
    auto tree = MakeTree<std::string>(type);
    std::uniform_int_distribution dist(std::numeric_limits<int>::min(),
                                       std::numeric_limits<int>::max());
    std::ifstream fin("../experiments/some_text.txt");
    std::string text;
    getline(fin, text);
    fin.close();
    std::vector<std::string> elements;
    for (uint64_t i = 0; i < op_count; ++i) {
        elements.emplace_back(text + std::to_string(dist(gen)));
    }
    auto begin = std::chrono::high_resolution_clock::now();
    for (uint64_t i = 0; i < op_count; ++i) {
        tree->insert(elements[i]);
    }
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() *
           nanoMultiplier;
}

double RandomDenseStringsInsert(ImplType type, std::mt19937& gen, uint64_t op_count) {
    auto tree = MakeTree<std::string>(type);
    std::uniform_int_distribution dist(0ul, op_count / 5);
    std::ifstream fin("../experiments/some_text.txt");
    std::string text;
    getline(fin, text);
    fin.close();
    std::vector<std::string> elements;
    for (uint64_t i = 0; i < op_count; ++i) {
        elements.emplace_back(text + std::to_string(dist(gen)));
    }
    auto begin = std::chrono::high_resolution_clock::now();
    for (uint64_t i = 0; i < op_count; ++i) {
        tree->insert(elements[i]);
    }
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() *
           nanoMultiplier;
}

double IncreasingIntSeriesEraseAfterIncreasingSeriesInsert(ImplType type, std::mt19937& gen,
                                                           uint64_t op_count) {
    auto tree = MakeTree<int>(type);
    for (uint64_t i = 0; i < op_count; ++i) {
        tree->insert(i);
    }
    auto begin = std::chrono::high_resolution_clock::now();
    for (uint64_t i = 0; i < op_count; ++i) {
        tree->erase(i);
    }
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() *
           nanoMultiplier;
}

double DecreasingIntSeriesEraseAfterIncreasingSeriesInsert(ImplType type, std::mt19937& gen,
                                                           uint64_t op_count) {
    auto tree = MakeTree<int>(type);
    for (uint64_t i = 0; i < op_count; ++i) {
        tree->insert(i);
    }
    auto begin = std::chrono::high_resolution_clock::now();
    for (uint64_t i = op_count - 1; i > 0; --i) {
        tree->erase(i);
    }
    tree->erase(0);
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() *
           nanoMultiplier;
}

double ConvergingIntSeriesEraseAfterIncreasingSeriesInsert(ImplType type, std::mt19937& gen,
                                                           uint64_t op_count) {
    auto tree = MakeTree<int>(type);
    for (uint64_t i = 0; i < op_count; ++i) {
        tree->insert(i);
    }
    auto begin = std::chrono::high_resolution_clock::now();
    for (uint64_t i = 0; i<op_count>> 1u; ++i) {
        tree->erase(i);
        tree->erase(op_count - i - 1);
    }
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() *
           nanoMultiplier;
}

double DivergingIntSeriesEraseAfterIncreasingSeriesInsert(ImplType type, std::mt19937& gen,
                                                          uint64_t op_count) {
    auto tree = MakeTree<int>(type);
    for (uint64_t i = 0; i < op_count; ++i) {
        tree->insert(i);
    }
    auto begin = std::chrono::high_resolution_clock::now();
    for (uint64_t i = op_count >> 1u; i < op_count; ++i) {
        tree->erase(i);
        tree->erase(op_count - i - 1);
    }
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() *
           nanoMultiplier;
}

double NonexistentIntSeriesEraseAfterIncreasingSeriesInsert(ImplType type, std::mt19937& gen,
                                                            uint64_t op_count) {
    auto tree = MakeTree<int>(type);
    for (uint64_t i = 0; i < op_count << 1u; i += 2) {
        tree->insert(i);
    }
    auto begin = std::chrono::high_resolution_clock::now();
    for (uint64_t i = 1; i < op_count << 1u; i += 2) {
        tree->erase(i);
    }
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() *
           nanoMultiplier;
}

double RandomIntSeriesEraseAfterIncreasingSeriesInsert(ImplType type, std::mt19937& gen,
                                                       uint64_t op_count) {
    auto tree = MakeTree<int>(type);
    std::vector<int> elements;
    for (uint64_t i = 0; i < op_count; ++i) {
        tree->insert(i);
        elements.emplace_back(i);
    }
    std::shuffle(elements.begin(), elements.end(), gen);
    auto begin = std::chrono::high_resolution_clock::now();
    for (uint64_t i = 0; i < op_count; ++i) {
        tree->erase(elements[i]);
    }
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() *
           nanoMultiplier;
}

double IncreasingIntSeriesEraseAfterRandomSparseSeriesInsert(ImplType type, std::mt19937& gen,
                                                             uint64_t op_count) {
    auto tree = MakeTree<int>(type);
    std::uniform_int_distribution dist(std::numeric_limits<int>::min(),
                                       std::numeric_limits<int>::max());
    std::vector<int> elements;
    for (uint64_t i = 0; i < op_count; ++i) {
        elements.emplace_back(dist(gen));
        tree->insert(elements.back());
    }
    std::sort(elements.begin(), elements.end());
    auto begin = std::chrono::high_resolution_clock::now();
    for (uint64_t i = 0; i < op_count; ++i) {
        tree->erase(elements[i]);
    }
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() *
           nanoMultiplier;
}

double DecreasingIntSeriesEraseAfterRandomSparseSeriesInsert(ImplType type, std::mt19937& gen,
                                                             uint64_t op_count) {
    auto tree = MakeTree<int>(type);
    std::uniform_int_distribution dist(std::numeric_limits<int>::min(),
                                       std::numeric_limits<int>::max());
    std::vector<int> elements;
    for (uint64_t i = 0; i < op_count; ++i) {
        elements.emplace_back(dist(gen));
        tree->insert(elements.back());
    }
    std::sort(elements.rbegin(), elements.rend());
    auto begin = std::chrono::high_resolution_clock::now();
    for (uint64_t i = 0; i < op_count; ++i) {
        tree->erase(elements[i]);
    }
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() *
           nanoMultiplier;
}

double ConvergingIntSeriesEraseAfterRandomSparseSeriesInsert(ImplType type, std::mt19937& gen,
                                                             uint64_t op_count) {
    auto tree = MakeTree<int>(type);
    std::uniform_int_distribution dist(std::numeric_limits<int>::min(),
                                       std::numeric_limits<int>::max());
    std::vector<int> elements;
    for (uint64_t i = 0; i < op_count; ++i) {
        elements.emplace_back(dist(gen));
        tree->insert(elements.back());
    }
    std::sort(elements.begin(), elements.end());
    auto begin = std::chrono::high_resolution_clock::now();
    for (uint64_t i = 0; i<op_count>> 1u; ++i) {
        tree->erase(elements[i]);
        tree->erase(elements[op_count - i - 1]);
    }
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() *
           nanoMultiplier;
}

double DivergingIntSeriesEraseAfterRandomSparseSeriesInsert(ImplType type, std::mt19937& gen,
                                                            uint64_t op_count) {
    auto tree = MakeTree<int>(type);
    std::uniform_int_distribution dist(std::numeric_limits<int>::min(),
                                       std::numeric_limits<int>::max());
    std::vector<int> elements;
    for (uint64_t i = 0; i < op_count; ++i) {
        elements.emplace_back(dist(gen));
        tree->insert(elements.back());
    }
    std::sort(elements.begin(), elements.end());
    auto begin = std::chrono::high_resolution_clock::now();
    for (uint64_t i = op_count >> 1u; i < op_count; ++i) {
        tree->erase(elements[i]);
        tree->erase(elements[op_count - i - 1]);
    }
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() *
           nanoMultiplier;
}

double NonexistentIntSeriesEraseAfterRandomSparseSeriesInsert(ImplType type, std::mt19937& gen,
                                                              uint64_t op_count) {
    auto tree = MakeTree<int>(type);
    std::uniform_int_distribution dist(std::numeric_limits<int>::min(),
                                       std::numeric_limits<int>::max());
    for (uint64_t i = 0; i < op_count; ++i) {
        tree->insert(dist(gen));
    }
    auto begin = std::chrono::high_resolution_clock::now();
    for (uint64_t i = 0; i < op_count; ++i) {
        tree->erase(dist(gen));
    }
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() *
           nanoMultiplier;
}

double RandomIntSeriesEraseAfterRandomSparseSeriesInsert(ImplType type, std::mt19937& gen,
                                                         uint64_t op_count) {
    auto tree = MakeTree<int>(type);
    std::uniform_int_distribution dist(std::numeric_limits<int>::min(),
                                       std::numeric_limits<int>::max());
    std::vector<int> elements;
    for (uint64_t i = 0; i < op_count; ++i) {
        elements.emplace_back(dist(gen));
        tree->insert(elements.back());
    }
    std::shuffle(elements.begin(), elements.end(), gen);
    auto begin = std::chrono::high_resolution_clock::now();
    for (uint64_t i = 0; i < op_count; ++i) {
        tree->erase(elements[i]);
    }
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() *
           nanoMultiplier;
}

double RandomStringsEraseAfterRandomInsert(ImplType type, std::mt19937& gen, uint64_t op_count) {
    auto tree = MakeTree<std::string>(type);
    std::uniform_int_distribution dist(std::numeric_limits<int>::min(),
                                       std::numeric_limits<int>::max());
    std::ifstream fin("../experiments/some_text.txt");
    std::string text;
    getline(fin, text);
    fin.close();
    std::vector<std::string> elements;
    for (uint64_t i = 0; i < op_count; ++i) {
        elements.emplace_back(text + std::to_string(dist(gen)));
        tree->insert(elements.back());
    }
    std::shuffle(elements.begin(), elements.end(), gen);
    auto begin = std::chrono::high_resolution_clock::now();
    for (uint64_t i = 0; i < op_count; ++i) {
        tree->erase(elements[i]);
    }
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() *
           nanoMultiplier;
}

double NonexistentStringsEraseAfterRandomInsert(ImplType type, std::mt19937& gen,
                                                uint64_t op_count) {
    auto tree = MakeTree<std::string>(type);
    std::uniform_int_distribution dist(std::numeric_limits<int>::min(),
                                       std::numeric_limits<int>::max());
    std::ifstream fin("../experiments/some_text.txt");
    std::string text;
    getline(fin, text);
    fin.close();
    for (uint64_t i = 0; i < op_count; ++i) {
        tree->insert(text + std::to_string(dist(gen)));
    }
    auto begin = std::chrono::high_resolution_clock::now();
    for (uint64_t i = 0; i < op_count; ++i) {
        tree->erase(text + std::to_string(dist(gen)));
    }
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() *
           nanoMultiplier;
}

double RandomInsertAndEraseIntAlternation(ImplType type, std::mt19937& gen, uint64_t op_count) {
    if (op_count < 10) {
        return 0.0;
    }
    uint64_t step = op_count / 10;
    auto tree = MakeTree<int>(type);
    std::uniform_int_distribution dist(0, static_cast<int>(3 * step));
    auto begin = std::chrono::high_resolution_clock::now();
    for (uint64_t i = 0; i < step << 1u; ++i) {
        tree->insert(dist(gen));
    }
    for (uint64_t i = 0; i < step; ++i) {
        tree->erase(dist(gen));
    }
    for (uint64_t i = 0; i < step << 1u; ++i) {
        tree->insert(dist(gen));
    }
    for (uint64_t i = 0; i < step << 1u; ++i) {
        tree->erase(dist(gen));
    }
    for (uint64_t i = 0; i < step; ++i) {
        tree->insert(dist(gen));
    }
    for (uint64_t i = 0; i < step << 1u; ++i) {
        tree->erase(dist(gen));
    }
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() *
           nanoMultiplier;
}

double FindIntAfterRandomSparseInsert(ImplType type, std::mt19937& gen, uint64_t op_count) {
    auto tree = MakeTree<int>(type);
    std::uniform_int_distribution dist(std::numeric_limits<int>::min(),
                                       std::numeric_limits<int>::max());
    std::vector<int> elements;
    for (uint64_t i = 0; i < op_count; ++i) {
        elements.emplace_back(dist(gen));
        tree->insert(elements.back());
    }
    std::shuffle(elements.begin(), elements.end(), gen);
    // We use counter, so that compiler doesn't apply optimizations
    int counter = 0;
    auto begin = std::chrono::high_resolution_clock::now();
    for (uint64_t i = 0; i < op_count; ++i) {
        auto it = tree->find(elements[i]);
        if (it != tree->end()) {
            counter += *it;
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    elements.emplace_back(counter);
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() *
           nanoMultiplier;
}

double FindRandomSparseIntAfterRandomSparseInsert(ImplType type, std::mt19937& gen,
                                                  uint64_t op_count) {
    auto tree = MakeTree<int>(type);
    std::uniform_int_distribution dist(std::numeric_limits<int>::min(),
                                       std::numeric_limits<int>::max());
    for (uint64_t i = 0; i < op_count; ++i) {
        tree->insert(dist(gen));
    }
    // We use counter, so that compiler doesn't apply optimizations
    int counter = 0;
    auto begin = std::chrono::high_resolution_clock::now();
    for (uint64_t i = 0; i < op_count; ++i) {
        auto it = tree->find(dist(gen));
        if (it != tree->end()) {
            counter += *it;
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::vector<int> useless(1, counter);
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() *
           nanoMultiplier;
}

double LowerBoundRandomSparseIntAfterRandomSparseInsert(ImplType type, std::mt19937& gen,
                                                        uint64_t op_count) {
    auto tree = MakeTree<int>(type);
    std::uniform_int_distribution dist(std::numeric_limits<int>::min(),
                                       std::numeric_limits<int>::max());
    for (uint64_t i = 0; i < op_count; ++i) {
        tree->insert(dist(gen));
    }
    // We use counter, so that compiler doesn't apply optimizations
    int counter = 0;
    auto begin = std::chrono::high_resolution_clock::now();
    for (uint64_t i = 0; i < op_count; ++i) {
        auto it = tree->lower_bound(dist(gen));
        if (it != tree->end()) {
            counter += *it;
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::vector<int> useless(1, counter);
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() *
           nanoMultiplier;
}