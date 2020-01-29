#define CATCH_CONFIG_MAIN
#include "../catch/catch.hpp"
#include <iostream>
#include <string>

#include "test_framework.cpp"

using std::cout;

TEST_CASE("Test AVL") {
    TestFramework framework;
    //framework.RunAll(Substr("AVL"));
}

TEST_CASE("Test Cartesian") {
    TestFramework framework;
    framework.RunAll(Substr("Cartesian"));
}

TEST_CASE("Test Red-Black") {
    TestFramework framework;
    //framework.RunAll(Substr("Red-Black"));
}

TEST_CASE("Test Splay") {
    TestFramework framework;
    //framework.RunAll(Substr("Splay"));
}
