#include <string>

#include "test_framework.cpp"

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

TEST_CASE("Test Skip list") {
    TestFramework framework;
    framework.RunAll(Substr("Skip list"));
}

TEST_CASE("Test Splay") {
    TestFramework framework;
    framework.RunAll(Substr("Splay"));
}
