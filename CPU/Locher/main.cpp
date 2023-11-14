#include "benchmark.h"

int main(int argc, char *argv[]) {
    std::ios::sync_with_stdio(false);
    if(argc < 2){
        std::cerr << "File name needed" << std::endl;
        return -1;
    }
    Benchmark bench(argv[1]);
    bench.Bench();
    return 0;
}
