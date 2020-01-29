#pragma once
#include "../catch/catch.hpp"
#include <exception>
#include <iostream>

#include "../trees/abstract_tree.h"

/* Here we're gonna write tests for our trees.
 * To add a test you are to write it as a function (e. g. SomeTest())
 * Then you are to add it to TestFramework default constructor.
 * Don't forget to name your test!
 */

using std::cout;

enum class ImplType { kAVL, kCartesian, kRB, kSplay };

/* This function returns a new tree of given type 'type'
 * as a shared pointer to base class.
 */
template <class T>
std::shared_ptr<ITree<T>> MakeTree(ImplType type = ImplType::kRB) {
    if (type == ImplType::kAVL) {
        throw std::runtime_error("Tree is not implemented yet");
        // return std::make_shared<AVLTree<T>>();
    } else if (type == ImplType::kCartesian) {
        return std::make_shared<CartesianTree<T>>();
    } else if (type == ImplType::kRB) {
        throw std::runtime_error("Tree is not implemented yet");
        // return std::make_shared<RBTree<T>>();
    } else if (type == ImplType::kSplay) {
        throw std::runtime_error("Tree is not implemented yet");
        // return std::make_shared<SplayTree<T>>();
    } else {
        throw std::runtime_error("Impossible behaviour");
    }
}

/* This function returns a copy of given tree of type 'type'
 * as a shared pointer to base class.
 * It uses copy constructor.
 */
template <class T>
std::shared_ptr<ITree<T>> MakeCopyCons(ImplType type, std::shared_ptr<ITree<T>> arg) {
    if (type == ImplType::kAVL) {
        throw std::runtime_error("Tree is not implemented yet");
        // return std::make_shared<AVLTree<T>>(*dynamic_cast<AVLTree<T>*>(arg.get()));
    } else if (type == ImplType::kCartesian) {
        return std::make_shared<CartesianTree<T>>(*dynamic_cast<CartesianTree<T>*>(arg.get()));
    } else if (type == ImplType::kRB) {
        throw std::runtime_error("Tree is not implemented yet");
        // return std::make_shared<RBTree<T>>(*dynamic_cast<RBTree<T>*>(arg.get()));
    } else if (type == ImplType::kSplay) {
        throw std::runtime_error("Tree is not implemented yet");
        // return std::make_shared<SplayTree<T>>(*dynamic_cast<SplayTree<T>*>(arg.get()));
    } else {
        throw std::runtime_error("Impossible behaviour");
    }
}

/* This function returns a copy of given tree (rhs) of type 'type'
 * as a shared pointer to base class (lhs).
 * It uses copy assignment operator.
 */
template <class T>
void MakeCopyAss(ImplType type, std::shared_ptr<ITree<T>>& lhs, std::shared_ptr<ITree<T>> rhs) {
    if (type == ImplType::kAVL) {
        throw std::runtime_error("Tree is not implemented yet");
        // *dynamic_cast<AVLTree<T>*>(lhs.get()) = *dynamic_cast<AVLTree<T>*>(rhs.get());
    } else if (type == ImplType::kCartesian) {
        *dynamic_cast<CartesianTree<T>*>(lhs.get()) = *dynamic_cast<CartesianTree<T>*>(rhs.get());
    } else if (type == ImplType::kRB) {
        throw std::runtime_error("Tree is not implemented yet");
        // *dynamic_cast<RBTree<T>*>(lhs.get()) = *dynamic_cast<RBTree<T>*>(rhs.get());
    } else if (type == ImplType::kSplay) {
        throw std::runtime_error("Tree is not implemented yet");
        // *dynamic_cast<SplayTree<T>*>(lhs.get()) = *dynamic_cast<SplayTree<T>*>(rhs.get());
    } else {
        throw std::runtime_error("Impossible behaviour");
    }
}

void SomeTest(ImplType type) {
    auto tree = MakeTree<int>(type);
    tree->insert(1);
    REQUIRE(*tree->begin() == 1);
}

void EmptinessTest(ImplType type) {
    auto tree = MakeTree<int>(type);
    REQUIRE(tree->size() == 0);
    REQUIRE(tree->empty());
    REQUIRE_NOTHROW(tree->clear());
    REQUIRE(tree->size() == 0);
}

void EmptyIteratorsTest(ImplType type) {
    auto tree = MakeTree<int>(type);
    REQUIRE(tree->begin() == tree->end());
    REQUIRE(tree->find(10) == tree->end());
    REQUIRE(tree->lower_bound(0) == tree->end());
    {
        auto it = tree->begin();
        REQUIRE_THROWS_AS(--it, std::exception);
    }
    {
        auto it = tree->begin();
        REQUIRE_THROWS_AS(++it, std::exception);
    }
    {
        auto it = tree->end();
        REQUIRE_THROWS_AS(it--, std::exception);
    }
    {
        auto it = tree->begin();
        REQUIRE_THROWS_AS(it++, std::exception);
    }
    REQUIRE(tree->empty());
}

void EmptyCopyingTest(ImplType type) {
    auto tree1 = MakeTree<int>(type);
    auto tree2 = MakeCopyCons<int>(type, tree1);
    REQUIRE(tree1->empty());
    REQUIRE(tree2->empty());
    REQUIRE(tree1->begin() != tree2->begin());
    REQUIRE(tree1->end() != tree2->end());
    auto tree3 = MakeCopyCons<int>(type, tree1);
    MakeCopyAss(type, tree3, tree2);
    REQUIRE(tree2->empty());
    REQUIRE(tree3->empty());
    REQUIRE(tree3->begin() != tree2->begin());
    REQUIRE(tree3->end() != tree2->end());
    REQUIRE_NOTHROW(MakeCopyAss(type, tree3, tree3));
}