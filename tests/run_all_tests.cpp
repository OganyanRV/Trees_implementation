#include <string>

#include "test_framework.cpp"

TEST_CASE("Test All") {
    TestFramework framework;
    framework.RunAllForAll();
}