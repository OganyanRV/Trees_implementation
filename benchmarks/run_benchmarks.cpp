#include "bench_framework.cpp"
/**
 * Here you can run the benchmarks
 * @return 0
 */
int main() {
    BenchFramework framework;
    framework.RunAllBenchmarks("../experiments/", BenchFramework::Range(1, 1'000'000, 35, true));
    // framework.RunAllBenchmarks("../experiments/", BenchFramework::Range(0, 1'000'000, 200'000));
    // framework.RunAllBenchmarks("../experiments/", BenchFramework::Range(1, 100'000, 10, true));
    return 0;
}