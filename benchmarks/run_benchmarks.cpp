#include "bench_framework.cpp"
#include <set>
#include <iostream>
#include <exception>

int main() {
    BenchFramework framework;
    framework.RunAllBenchmarks("../experiments/", BenchFramework::Range(1, 1000001, 10, true));
    return 0;
}