#define CATCH_CONFIG_MAIN
#include "../catch/catch.hpp"
#include <iostream>
#include <string>

#include "test_framework.cpp"

using std::cout;

TEST_CASE("Test All") {
    TestFramework framework;
    framework.RunAllForAll();
}
