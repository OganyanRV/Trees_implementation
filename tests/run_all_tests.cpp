#include <string>

#include "test_framework.cpp"

/**
 * Runs all tests for all trees in the project
 */
TEST_CASE("Test All") {
    TestFramework framework;
    framework.RunAllForAll();
}