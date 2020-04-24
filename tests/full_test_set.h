#pragma once
#include "../catch/catch.hpp"
#include <exception>
#include <iostream>
#include <random>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "../trees/abstract_tree.h"
#include "../trees/avl_tree.h"
#include "../trees/cartesian_tree.h"
#include "../trees/rb_tree.h"
#include "../trees/skip_list.h"
#include "../trees/splay_tree.h"

// #define RELEASE_BUILD

/* Here we're going to write tests for our trees.
 * To add a test you are to write it as a function (e. g. SomeTest())
 * Then you are to add it to TestFramework default constructor.
 * Don't forget to name your test!
 */

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
        throw std::runtime_error("Tree is not implemented yet");
        // return std::make_shared<SkipList<T>>(params...);
    } else if (type == ImplType::kSplay) {
        return std::make_shared<SplayTree<T>>(params...);
    } else {
        throw std::runtime_error("Impossible behaviour");
    }
}

/* This function returns a copy of given tree (rhs) of type 'type'
 * as a shared pointer to base class (lhs).
 * It uses copy assignment operator.
 */
template <class T>
void MakeCopyAssignment(ImplType type, std::shared_ptr<ITree<T>>& lhs,
                        std::shared_ptr<ITree<T>> rhs) {
    if (type == ImplType::kAVL) {
        *dynamic_cast<AVLTree<T>*>(lhs.get()) = *dynamic_cast<AVLTree<T>*>(rhs.get());
    } else if (type == ImplType::kCartesian) {
        *dynamic_cast<CartesianTree<T>*>(lhs.get()) = *dynamic_cast<CartesianTree<T>*>(rhs.get());
    } else if (type == ImplType::kRB) {
        *dynamic_cast<RBTree<T>*>(lhs.get()) = *dynamic_cast<RBTree<T>*>(rhs.get());
    } else if (type == ImplType::kSkipList) {
        throw std::runtime_error("Tree is not implemented yet");
        // *dynamic_cast<SkipList<T>*>(lhs.get()) = *dynamic_cast<SkipList<T>*>(rhs.get());
    } else if (type == ImplType::kSplay) {
        *dynamic_cast<SplayTree<T>*>(lhs.get()) = *dynamic_cast<SplayTree<T>*>(rhs.get());
    } else {
        throw std::runtime_error("Impossible behaviour");
    }
}

template <class T>
bool operator==(std::set<T> set, std::shared_ptr<ITree<T>> tree) {
    if (set.size() != tree->size()) {
        return false;
    }
    auto tree_it = tree->begin();
    for (const T& elem : set) {
        REQUIRE_NOTHROW(*tree_it);
        if (elem != *tree_it) {
            return false;
        }
        REQUIRE_NOTHROW(++tree_it);
    }
    REQUIRE(tree_it == tree->end());
    return true;
}

template <class T>
void CheckFindAndLB(const std::set<T>& set, std::shared_ptr<ITree<T>> tree, const T& value) {
    auto set_it = set.find(value);
    if (set_it == set.end()) {
        REQUIRE(tree->find(value) == tree->end());
    } else {
        auto it = tree->find(value);
        REQUIRE(*it == *set_it);
        if (set_it != set.begin()) {
            REQUIRE(it != tree->begin());
            REQUIRE(*(--it) == *(--set_it));
            ++it, ++set_it;
        }
        ++set_it, ++it;
        if (set_it != set.end()) {
            REQUIRE(it != tree->end());
            REQUIRE(*it == *set_it);
        }
    }

    set_it = set.lower_bound(value);
    if (set_it == set.end()) {
        auto it = tree->lower_bound(value);
        REQUIRE(it == tree->end());
        if (set_it != set.begin()) {
            REQUIRE(it != tree->begin());
            REQUIRE(*(--set_it) == *(--it));
        }
    } else {
        auto it = tree->lower_bound(value);
        REQUIRE(*it == *set_it);
        if (set_it != set.begin()) {
            REQUIRE(it != tree->begin());
            REQUIRE(*(--set_it) == *(--it));
            ++set_it, ++it;
        }
        ++set_it, ++it;
        if (set_it != set.end()) {
            REQUIRE(it != tree->end());
            REQUIRE(*set_it == *it);
        }
    }
}

class Random {
public:
    template <class T>
    static uint32_t Next(const T& from, const T& to) {
        static Random rand = Random();
        std::uniform_int_distribution<T> dist(from, to);
        return dist(rand.gen_);
    }

private:
    Random() {
#ifdef RELEASE_BUILD
        // GOOD INIT
        std::random_device device;
        gen_ = std::mt19937(device());
#else
        // BAD INIT
        gen_ = std::mt19937(0u);
#endif
    }

    std::mt19937 gen_;
};

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
    REQUIRE_NOTHROW(tree->erase(5));
    REQUIRE(tree->empty());
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
    {
        auto it = tree->end();
        REQUIRE_THROWS_AS(++it, std::exception);
    }
    REQUIRE(tree->empty());
}

void EmptyCopyingTest(ImplType type) {
    auto tree1 = MakeTree<int>(type);
    auto tree2 = MakeTree<int, std::shared_ptr<ITree<int>>>(type, tree1);
    REQUIRE(tree1->empty());
    REQUIRE(tree2->empty());
    REQUIRE(tree1->begin() != tree2->begin());
    REQUIRE(tree1->end() != tree2->end());
    auto tree3 = MakeTree<int, std::shared_ptr<ITree<int>>>(type, tree1);
    MakeCopyAssignment(type, tree3, tree2);
    REQUIRE(tree2->empty());
    REQUIRE(tree3->empty());
    REQUIRE(tree3->begin() != tree2->begin());
    REQUIRE(tree3->end() != tree2->end());
    REQUIRE_NOTHROW(MakeCopyAssignment(type, tree3, tree3));
}

void FewElementsTest(ImplType type) {
    std::vector<int> fill = {1, 0};
    auto tree = MakeTree<int>(type);
    std::set<int> set;
    for (int& value : fill) {
        tree->insert(value);
        set.insert(value);
        REQUIRE(!tree->empty());
    }
    REQUIRE(set == tree);
    set.clear();
    REQUIRE_NOTHROW(tree->clear());
    REQUIRE(set == tree);
    REQUIRE(tree->empty());
}

void FewElementsIteratorTest(ImplType type) {
    {
        std::vector<int> fill = {3, 4, 2, 5, 1};
        std::set<int> set(fill.begin(), fill.end());
        auto tree = MakeTree<int>(type, fill.begin(), fill.end());
        REQUIRE(set == tree);
        REQUIRE(tree->find(10) == tree->end());
        REQUIRE(tree->lower_bound(0) == tree->begin());
    }
    {
        std::vector<int> fill = {3, 4, 2, 5, 1};
        std::set<int> set(fill.begin(), fill.end());
        auto tree = MakeTree<int>(type, fill.begin(), fill.end());
        auto it = tree->end();
        REQUIRE_THROWS_AS(*it, std::exception);
        REQUIRE_THROWS_AS(it++, std::exception);
        it = tree->begin();
        REQUIRE_THROWS_AS(--it, std::exception);
    }
    {
        std::vector<std::pair<std::string, int>> fill = {
            {"one", 1}, {"two", 2}, {"three", 3}, {"four", 4}};
        std::set<std::pair<std::string, int>> set(fill.begin(), fill.end());
        auto tree = MakeTree<std::pair<std::string, int>>(type, fill.begin(), fill.end());
        auto it = tree->begin();
        REQUIRE(it->first == "four");
        REQUIRE(it->second == 4);
        ++it, ++it;
        REQUIRE(it->first == "three");
        REQUIRE(it->second == 3);
        it = tree->begin();
        REQUIRE_THROWS_AS(it--, std::exception);
        it = tree->end();
        REQUIRE_THROWS_AS(*it, std::exception);
        REQUIRE_THROWS_AS(it->first, std::exception);
        REQUIRE_THROWS_AS(it->second, std::exception);
        REQUIRE_THROWS_AS(++it, std::exception);
    }
    {
        auto tree = MakeTree<std::pair<int, int>, std::initializer_list<std::pair<int, int>>>(
            type, {{0, 1},
                   {-5, 0},
                   {3, 11},
                   {std::numeric_limits<int>::max(), std::numeric_limits<int>::min()}});
        REQUIRE(tree->begin()->first == -5);
        REQUIRE((--tree->end())->second == std::numeric_limits<int>::min());
        tree->clear();
        tree->insert(std::make_pair(1, 1));
        tree->insert(std::make_pair(-1, 1));
        REQUIRE(tree->size() == 2);
        REQUIRE(tree->begin()->first == -1);
    }
}

void FewElementsCopyingTest(ImplType type) {
    {
        std::set<int> fill = {123, 532, 635, 13, 256, 986};
        auto tree = MakeTree<int>(type, fill.begin(), fill.end());
        auto tree2 = MakeTree<int>(type, tree);
        REQUIRE(tree2->size() == tree->size());
        tree2->erase(532);
        REQUIRE(tree2->size() == 5);
        REQUIRE(tree->size() == 6);
        tree->insert(1);
        tree2->insert(100);
        REQUIRE(tree2->size() == 6);
        REQUIRE(tree->size() == 7);
        REQUIRE(tree->find(1) != tree->end());
        REQUIRE(*tree2->lower_bound(99) == 100);
        tree->clear();
        REQUIRE(tree2->size() == 6);
    }
    {
        std::set<int> fill = {123, 532, 635, 13, 256, 986};
        auto tree = MakeTree<int>(type, fill.begin(), fill.end());
        auto tree2 = MakeTree<int>(type);
        MakeCopyAssignment(type, tree2, tree);
        REQUIRE(tree2->size() == tree->size());
        tree2->erase(532);
        REQUIRE(tree2->size() == 5);
        REQUIRE(tree->size() == 6);
        tree->insert(1);
        tree2->insert(100);
        REQUIRE(tree2->size() == 6);
        REQUIRE(tree->size() == 7);
        REQUIRE(tree->find(1) != tree->end());
        REQUIRE(*tree2->lower_bound(99) == 100);
        tree->clear();
        REQUIRE(tree2->size() == 6);
    }
    {
        auto tree = MakeTree<int>(type);
        auto tree2 = MakeTree<int>(type);
        MakeCopyAssignment(type, tree2, tree);
        REQUIRE(tree->empty());
        REQUIRE(tree2->empty());
        tree->insert(10);
        REQUIRE(tree2->empty());
        tree->erase(10);
        REQUIRE(tree2->empty());
        tree2->insert(15);
        tree2->insert(20);
        auto tree3 = MakeTree<int>(type);
        MakeCopyAssignment(type, tree3, tree2);
        tree2->clear();
        REQUIRE(tree3->size() == 2);
    }
    {
        std::vector<int> fill = {3, 3, -1, 6, 0, 0, 17, -5, 4, 2};
        std::set<int> set(fill.begin(), fill.end());
        auto tree1 = MakeTree<int>(type, fill.begin(), fill.end());
        auto tree2 = MakeTree<int>(type);
        MakeCopyAssignment(type, tree2, tree1);
        tree2->insert(5);
        tree2->insert(18);
        tree2->insert(-2);
        auto tree1_it = tree1->begin(), tree2_it = tree2->begin();
        auto it = set.begin();
        while (tree1_it != tree1->end() || tree2_it != tree2->end() || it != set.end()) {
            if (*tree2_it == 5 || *tree2_it == 18 || *tree2_it == -2) {
                ++tree2_it;
                continue;
            }
            if (tree1_it == tree1->end() || tree2_it == tree2->end() || it == set.end()) {
                REQUIRE(tree1_it == tree1->end());
                REQUIRE(tree2_it == tree2->end());
                REQUIRE(it == set.end());
            } else {
                REQUIRE(*tree1_it == *tree2_it);
                REQUIRE(*tree1_it == *it);
                ++tree1_it, ++tree2_it, ++it;
            }
        }
    }
}

class StrangeInt {
public:
    static int counter;

    StrangeInt() : value_() {
        ++counter;
    }
    StrangeInt(int value) : value_(value) {
        ++counter;
    }
    StrangeInt(const StrangeInt& other) : value_(other.value_) {
        ++counter;
    }
    StrangeInt(StrangeInt&& other) noexcept : value_(other.value_) {
        ++counter;
    }

    bool operator<(const StrangeInt& other) const {
        return value_ < other.value_;
    }

    static void init() {
        counter = 0;
    }

    ~StrangeInt() {
        --counter;
    }

    friend std::ostream& operator<<(std::ostream& out, const StrangeInt& value) {
        out << value.value_;
        return out;
    }

private:
    int value_;
};
int StrangeInt::counter;

void StrangeTest(ImplType type) {
    {
        int count = StrangeInt::counter;
        auto tree = MakeTree<StrangeInt>(type);
        tree->insert(2);
        tree->insert(42);
        tree->clear();
        REQUIRE(count == StrangeInt::counter);
    }
    {
        int count = StrangeInt::counter;
        std::set<int> fill = {123, 532, 635, 13, 256, 986};
        auto tree = MakeTree<StrangeInt>(type, fill.begin(), fill.end());
        for (auto& value : *tree) {
            tree->erase(value);
        }
        REQUIRE(count == StrangeInt::counter);
    }
    int count = StrangeInt::counter;
    {
        std::set<int> fill = {123, 532, 635, 13, 256, 986};
        auto tree = MakeTree<StrangeInt>(type, fill.begin(), fill.end());
    }
    REQUIRE(count == StrangeInt::counter);
}

void StrangeCopyTest(ImplType type) {
    {
        int count = StrangeInt::counter;
        std::set<int> fill = {123, 532, 635, 13, 256, 986};
        auto tree = MakeTree<StrangeInt>(type, fill.begin(), fill.end());
        auto tree2 = MakeTree<StrangeInt>(type, tree);
        tree2->insert(1000);
        auto tree3 = MakeTree<StrangeInt>(type);
        MakeCopyAssignment(type, tree3, tree2);
        tree3->erase(1000);
        REQUIRE(tree3->size() == tree->size());
        REQUIRE(tree->size() + 1 == tree2->size());
        tree->clear();
        tree2->clear();
        tree3->clear();
        REQUIRE(count == StrangeInt::counter);
    }
    int count = StrangeInt::counter;
    {
        std::set<int> fill = {123, 532, 635, 13, 256, 986};
        auto tree = MakeTree<StrangeInt>(type, fill.begin(), fill.end());
        auto tree2 = MakeTree<StrangeInt>(type, tree);
        tree2->insert(1000);
        auto tree3 = MakeTree<StrangeInt>(type);
        MakeCopyAssignment(type, tree3, tree2);
        tree3->erase(1000);
        REQUIRE(tree3->size() == tree->size());
        REQUIRE(tree->size() + 1 == tree2->size());
    }
    REQUIRE(count == StrangeInt::counter);
}

void FindAndLBTest(ImplType type) {
    for (int count = 0; count < 100; ++count) {
        std::vector<int> fill;
        for (int i = 0; i < 10; ++i) {
            fill.emplace_back(Random::Next(-10, 10));
        }
        std::set<int> set(fill.begin(), fill.end());
        auto tree = MakeTree<int>(type, fill.begin(), fill.end());
        for (int i = 0; i < 40; ++i) {
            CheckFindAndLB<int>(set, tree, Random::Next(-10, 10));
        }
    }
}

void InsertAndEraseTest(ImplType type) {
    for (int count = 0; count < 100; ++count) {
        std::vector<int> fill;
        for (int i = 0; i < 10; ++i) {
            fill.emplace_back(Random::Next(-10, 10));
        }
        std::set<int> set(fill.begin(), fill.end());
        auto tree = MakeTree<int>(type);
        for (const int& value : fill) {
            tree->insert(value);
        }
        for (int i = 0; i < 10; ++i) {
            int value = Random::Next(-10, 10);
            if (Random::Next(0, 1)) {
                set.insert(value);
                tree->insert(value);
            } else {
                set.erase(value);
                tree->erase(value);
            }
            CheckFindAndLB(set, tree, value);
        }
        auto it = set.begin();
        while (!set.empty()) {
            if (Random::Next(0, 5)) {
                ++it;
            } else {
                int value = *it;
                tree->erase(value);
                it = set.erase(it);
                CheckFindAndLB(set, tree, value);
            }
            if (it == set.end()) {
                it = set.begin();
            }
        }
    }
}

void RBBlackHeightTest(ImplType type) {
    if (type != ImplType::kRB) {
        std::cout << "Test is only designed for RB trees. ";
        return;
    }
    for (int count = 0; count < 100; ++count) {
        std::vector<int> fill;
        for (int i = 0; i < 10; ++i) {
            fill.emplace_back(Random::Next(-100, 100));
        }
        std::set<int> set(fill.begin(), fill.end());
        auto tree = std::make_shared<RBTree<int>>();
        for (const int& value : fill) {
            tree->insert(value);
            REQUIRE_NOTHROW(tree->CheckRB());
        }

        for (int i = 0; i < 10; ++i) {
            int value = Random::Next(-100, 100);
            if (Random::Next(0, 1)) {
                set.insert(value);
                tree->insert(value);
            } else {
                set.erase(value);
                tree->erase(value);
            }
            REQUIRE_NOTHROW(tree->CheckRB());
        }
        auto it = set.begin();
        while (!set.empty()) {
            if (Random::Next(0, 5)) {
                ++it;
            } else {
                int value = *it;
                tree->erase(value);
                it = set.erase(it);
                REQUIRE_NOTHROW(tree->CheckRB());
            }
            if (it == set.end()) {
                it = set.begin();
            }
        }
    }
}
