#ifndef BENCHMARK_H
#define BENCHMARK_H

#include "../core/Graph.h"
#include <string>
#include <vector>
#include <functional>

struct Row {
    std::string instance;
    int n;
    int m;
    std::string algorithm;
    double time_s;
    double time_sd;
    int colors;
    bool valid;
    double mem_mb;
    std::string note;
};

class Benchmark {
public:
    static std::vector<Row> run_all(
        const std::vector<int>& geo_sizes = {1000, 10000, 100000, 1000000},
        const std::vector<int>& gnp_sizes = {1000},
        int reps = 5,
        int reps_big = 3,
        double bt_timeout = 30.0,
        int seed = 42,
        std::function<void(const std::string&, double)> on_progress = nullptr
    );
    
    static void to_csv(const std::vector<Row>& rows, const std::string& path);
    static double get_peak_memory_mb();
    static double get_current_memory_mb();
};

#endif // BENCHMARK_H
