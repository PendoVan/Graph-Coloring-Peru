#include "Benchmark.h"
#include "../core/Algorithms.h"
#include "../core/Generators.h"
#include "../core/PeruMap.h"
#include "../core/CoreUtils.h"
#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#else
#include <sys/resource.h>
#endif

using namespace std;

double Benchmark::get_peak_memory_mb() {
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
        return static_cast<double>(pmc.PeakWorkingSetSize) / (1024.0 * 1024.0);
    }
#else
    struct rusage r_usage;
    if (getrusage(RUSAGE_SELF, &r_usage) == 0) {
#ifdef __APPLE__
        return static_cast<double>(r_usage.ru_maxrss) / (1024.0 * 1024.0);
#else
        return static_cast<double>(r_usage.ru_maxrss) / 1024.0;
#endif
    }
#endif
    return 0.0;
}

double Benchmark::get_current_memory_mb() {
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
        return static_cast<double>(pmc.PrivateUsage) / (1024.0 * 1024.0);
    }
#endif
    return 0.0;
}

template<typename Func>
static tuple<double, double, vector<int>> time_func(Func f, int reps) {
    vector<double> samples;
    vector<int> coloring;
    
    for (int i = 0; i < reps; ++i) {
        auto t0 = chrono::high_resolution_clock::now();
        coloring = f();
        auto t1 = chrono::high_resolution_clock::now();
        double dt = chrono::duration<double>(t1 - t0).count();
        samples.push_back(dt);
    }
    
    double sum = 0.0;
    for (double x : samples) sum += x;
    double mean = sum / reps;
    
    double sd = 0.0;
    if (reps > 1) {
        double sq_sum = 0.0;
        for (double x : samples) sq_sum += (x - mean) * (x - mean);
        sd = sqrt(sq_sum / (reps - 1));
    }
    
    return {mean, sd, coloring};
}

static vector<Row> bench_greedy_dsatur(const string& instance, const Graph& g, int reps) {
    vector<Row> rows;
    
    auto jobs = {
        make_pair(string("greedy_natural"), function<vector<int>()>([&](){ return greedy(g, order_natural(g)); })),
        make_pair(string("greedy_largest_first"), function<vector<int>()>([&](){ return greedy(g, order_largest_first(g)); })),
        make_pair(string("greedy_smallest_last"), function<vector<int>()>([&](){ return greedy(g, order_smallest_last(g)); })),
        make_pair(string("dsatur"), function<vector<int>()>([&](){ return dsatur(g); }))
    };
    
    for (const auto& job : jobs) {
        double mem_before = Benchmark::get_peak_memory_mb();
        auto res = time_func(job.second, reps);
        double mem_after = Benchmark::get_peak_memory_mb();
        double mem = mem_after; // Peak memory
        
        double mean = get<0>(res);
        double sd = get<1>(res);
        auto col = get<2>(res);
        
        rows.push_back({instance, g.n(), g.m(), job.first, mean, sd, num_colors(col), is_proper(g, col).valid, mem, ""});
    }
    return rows;
}

static Row bench_backtracking(const string& instance, const Graph& g, double timeout_s) {
    try {
        ExactResult res = chromatic_number(g, max(g.n(), 200), timeout_s);
        string note = res.exact ? "chi=" + to_string(res.k) + " exacto, " + to_string(res.nodes) + " nodos"
                                : "chi en [" + to_string(res.lower) + "," + to_string(res.upper) + "] (timeout), " + to_string(res.nodes) + " nodos";
        return {instance, g.n(), g.m(), "backtracking", res.elapsed, 0.0, res.k, is_proper(g, res.coloring).valid, 0.0, note};
    } catch(const exception& e) {
        return {instance, g.n(), g.m(), "backtracking", 0.0, 0.0, 0, false, 0.0, string("Error: ") + e.what()};
    }
}

vector<Row> Benchmark::run_all(const vector<int>& geo_sizes, const vector<int>& gnp_sizes, int reps, int reps_big, double bt_timeout, int seed, function<void(const string&, double)> on_progress) {
    vector<Row> rows;
    int steps = 1 + gnp_sizes.size() + geo_sizes.size();
    int done = 0;
    
    auto tick = [&](const string& msg) {
        done++;
        if (on_progress) {
            on_progress(msg, static_cast<double>(done) / steps);
        }
    };
    
    PeruMap::instance().load("data/peru_map.json");
    Graph peru = PeruMap::instance().build_graph();
    
    auto r1 = bench_greedy_dsatur("Peru", peru, reps);
    rows.insert(rows.end(), r1.begin(), r1.end());
    rows.push_back(bench_backtracking("Peru", peru, bt_timeout));
    tick("Peru (n=25)");
    
    for (int n : gnp_sizes) {
        Graph g = gnp(n, p_for_avg_degree(n, 10.0), seed);
        auto r = bench_greedy_dsatur("gnp_" + to_string(n), g, reps);
        rows.insert(rows.end(), r.begin(), r.end());
        if (n <= 2000) {
            rows.push_back(bench_backtracking("gnp_" + to_string(n), g, bt_timeout));
        }
        tick("G(n,p) n=" + to_string(n));
    }
    
    for (int n : geo_sizes) {
        Graph g = geometric(n, 6.0, seed).g;
        auto r = bench_greedy_dsatur("geo_" + to_string(n), g, n < 100000 ? reps : reps_big);
        rows.insert(rows.end(), r.begin(), r.end());
        tick("geometrico n=" + to_string(n));
    }
    
    return rows;
}

void Benchmark::to_csv(const vector<Row>& rows, const string& path) {
    ofstream f(path);
    if (!f.is_open()) return;
    
    f << "instancia,n,m,algoritmo,t_medio_s,t_sd_s,colores,valida,mem_mb,nota\n";
    for (const auto& r : rows) {
        f << r.instance << "," << r.n << "," << r.m << "," << r.algorithm << ","
          << setprecision(6) << r.time_s << "," << setprecision(3) << r.time_sd << ","
          << r.colors << "," << (r.valid ? 1 : 0) << "," << setprecision(3) << r.mem_mb << ",\""
          << r.note << "\"\n";
    }
}
