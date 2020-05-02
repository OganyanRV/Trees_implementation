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

#define nanoMultiplier 1e-6

enum class ImplType { kAVL, kCartesian, kRB, kSkipList, kSplay };

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
    } else {
        throw std::runtime_error("Impossible behaviour");
    }
}

double SimpleBench(ImplType type, std::mt19937& gen, uint64_t op_count) {
    auto tree = MakeTree<int>(type);
    auto begin = std::chrono::high_resolution_clock::now();
    for (uint64_t i = 0; i < op_count; ++i) {
        tree->insert(i);
    }
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() * nanoMultiplier;
}