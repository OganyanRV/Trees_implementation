#pragma once
#include <vector>
#include <ostream>
#include <random>
#include <chrono>

#include "../trees/abstract_tree.h"
#include "../trees/avl_tree.h"
#include "../trees/cartesian_tree.h"
#include "../trees/rb_tree.h"
#include "../trees/skip_list.h"
#include "../trees/splay_tree.h"
#include "../trees/stdlib_set.h"

#define nanoMultiplier 1e-6

enum class ImplType { kAVL, kCartesian, kRB, kSkipList, kSplay, kSet };

/* This function returns a new tree of given type 'type'
 * as a shared pointer to base class.
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
        tree->insert(op_count - i);
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
        tree->insert(op_count - i);
    }
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() *
           nanoMultiplier;
}

double RandomSparseIntSeriesInsert(ImplType type, std::mt19937& gen, uint64_t op_count) {
    auto tree = MakeTree<int>(type);
    auto begin = std::chrono::high_resolution_clock::now();
    std::uniform_int_distribution dist(std::numeric_limits<int>::min(),
                                       std::numeric_limits<int>::max());
    for (uint64_t i = 0; i < op_count; ++i) {
        tree->insert(dist(gen));
    }
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() *
           nanoMultiplier;
}

double RandomDenseIntSeriesInsert(ImplType type, std::mt19937& gen, uint64_t op_count) {
    auto tree = MakeTree<int>(type);
    auto begin = std::chrono::high_resolution_clock::now();
    std::uniform_int_distribution dist(0ul, op_count / 5);
    for (uint64_t i = 0; i < op_count; ++i) {
        tree->insert(dist(gen));
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
    for (uint64_t i = op_count - 1; i >= 0; --i) {
        tree->erase(i);
    }
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() *
           nanoMultiplier;
}