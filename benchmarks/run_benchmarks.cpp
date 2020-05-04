#include "bench_framework.cpp"

int main() {
    BenchFramework framework;
    framework.RunAllBenchmarks("../experiments/", BenchFramework::Range(100000, 100001));
    return 0;
}