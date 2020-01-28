#include <iostream>
#include <string>

#include "test_framework.cpp"

using std::cout;

int main() {
    TestFramework framework;
    framework.RunAll(Substr("Cartesian"));
    return 0;
}