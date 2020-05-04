#include "bench_framework.cpp"

int main() {
    BenchFramework framework;
    framework.RunAllBenchmarks("../experiments/", BenchFramework::Range(1, 100001, 10, true));
    return 0;
}