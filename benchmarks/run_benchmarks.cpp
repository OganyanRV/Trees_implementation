#include "bench_framework.cpp"
int main() {
    BenchFramework framework;
    framework.RunAllBenchmarks("../experiments/", BenchFramework::Range(1, 1'000'000, 20, true));
    return 0;
}