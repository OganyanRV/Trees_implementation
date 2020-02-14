#define CATCH_CONFIG_MAIN
#include "../catch/catch.hpp"
#include <iostream>
#include <string>

#include "test_framework.cpp"

using std::cout;

TEST_CASE("Test AVL") {
    TestFramework framework;
    framework.RunAll(Substr("AVL"));
}

TEST_CASE("Test Cartesian") {
    TestFramework framework;
    framework.RunAll(Substr("Cartesian"));
}

TEST_CASE("Test Red-Black") {
    TestFramework framework;
    framework.RunAll(Substr("Red-Black"));
}

TEST_CASE("Test Splay") {
    TestFramework framework;
    // framework.RunAll(Substr("Splay"));
}

TEST_CASE("Test All") {
    TestFramework framework;
    // framework.RunAll(Substr("AVL"));
    // framework.RunAll(Substr("Cartesian"));
    // framework.RunAll(Substr("Red-Black"));
    // framework.RunAll(Substr("Splay"));
}

TEST_CASE("Test RB") {
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