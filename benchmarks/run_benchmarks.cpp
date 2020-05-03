#include "bench_framework.cpp"

int main(){
    BenchFramework framework;
    uint64_t m = 100000;
    framework.RunAllBenchmarks("../experiments/", BenchFramework::Range(m, m+1, 1, 1));
    return 0;
}