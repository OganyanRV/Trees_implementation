#include "bench_framework.cpp"

int main() {
    BenchFramework framework;
    framework.RunAllBenchmarks("../experiments/", BenchFramework::Range(50000, 50001));
    return 0;
}