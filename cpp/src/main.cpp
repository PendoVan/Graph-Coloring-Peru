#include <QApplication>
#include <QCommandLineParser>
#include <iostream>
#include <iomanip>
#include "core/PeruMap.h"
#include "core/Algorithms.h"
#include "core/CoreUtils.h"
#include "benchmark/Benchmark.h"
#include "gui/MainWindow.h"

using namespace std;

void run_demo() {
    PeruMap::instance().load("data/peru_map.json");
    Graph g = PeruMap::instance().build_graph();
    
    cout << "Mapa del Peru como grafo de interferencia (FAP): " << "Graph(n=" << g.n() << ", m=" << g.m() << ")\n";
    cout << "  nodo = departamento (antena) | arista = frontera (interferencia)\n";
    cout << "  color = frecuencia | objetivo: minimizar frecuencias = chi(G)\n\n";
    
    auto linea = [&](const string& nombre, const vector<int>& coloring, double ms, const string& extra = "") {
        bool ok = is_proper(g, coloring).valid;
        cout << "  " << left << setw(24) << nombre << " colores=" << num_colors(coloring) << "  "
             << (ok ? "valida  " : "INVALIDA") << right << setw(7) << fixed << setprecision(3) << ms << " ms  " << extra << "\n";
    };
    
    auto t0 = chrono::high_resolution_clock::now();
    auto c1 = greedy(g, order_largest_first(g));
    linea("Greedy (mayor grado)", c1, chrono::duration<double, milli>(chrono::high_resolution_clock::now() - t0).count());
    
    t0 = chrono::high_resolution_clock::now();
    auto c2 = greedy(g, order_smallest_last(g));
    linea("Greedy (smallest-last)", c2, chrono::duration<double, milli>(chrono::high_resolution_clock::now() - t0).count());
    
    t0 = chrono::high_resolution_clock::now();
    auto c3 = dsatur(g);
    linea("DSATUR", c3, chrono::duration<double, milli>(chrono::high_resolution_clock::now() - t0).count());
    
    ExactResult res = chromatic_number(g, 200, 60.0);
    string extra = "chi(G)=" + to_string(res.k) + ", cotas=[" + to_string(res.lower) + "," + to_string(res.upper) + "], nodos=" + to_string(res.nodes);
    linea("Backtracking (exacto)", res.coloring, res.elapsed * 1000.0, extra);
    
    cout << "\nchi(Peru) = " << res.k << ". Cota inferior: la clique K4 {Cusco, Apurimac, Ayacucho, Arequipa} obliga a 4 frecuencias.\n";
    cout << "\nAsignacion optima de frecuencias:\n";
    
    auto table = PeruMap::instance().frequency_table(g, res.coloring);
    for (const auto& pair : table) {
        cout << "  f" << pair.first << ": ";
        for (size_t i = 0; i < pair.second.size(); ++i) {
            cout << pair.second[i] << (i + 1 < pair.second.size() ? ", " : "");
        }
        cout << "\n";
    }
}

void run_bench() {
    auto rows = Benchmark::run_all(
        {1000, 10000, 100000, 1000000}, {1000}, 5, 3, 30.0, 42,
        [](const string& msg, double f) {
            cout << "[" << fixed << setprecision(1) << setw(5) << (f * 100.0) << "%] " << msg << "\n";
        }
    );
    
    for (const auto& r : rows) {
        cout << "  " << left << setw(10) << r.instance << " " << setw(22) << r.algorithm
             << " n=" << setw(8) << r.n << " colores=" << setw(3) << r.colors
             << " t=" << fixed << setprecision(3) << (r.time_s * 1000.0) << "ms mem=" 
             << setprecision(2) << r.mem_mb << "MB " << r.note << "\n";
    }
    
    string out_path = "../data/resultados_cpp.csv";
    Benchmark::to_csv(rows, out_path);
    cout << "CSV -> " << out_path << "\n";
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName("GraphColoringPeru");
    
    QCommandLineParser parser;
    parser.setApplicationDescription("Coloreo de grafos para asignacion de frecuencias (FAP) — mapa del Peru.");
    parser.addHelpOption();
    
    QCommandLineOption demoOption("demo", "ejecucion por consola");
    parser.addOption(demoOption);
    
    QCommandLineOption benchOption("bench", "benchmark por consola");
    parser.addOption(benchOption);
    
    QCommandLineOption guiOption("gui", "abrir la aplicacion (por defecto)");
    parser.addOption(guiOption);
    
    parser.process(app);
    
    if (parser.isSet(demoOption)) {
        run_demo();
        return 0;
    } else if (parser.isSet(benchOption)) {
        run_bench();
        return 0;
    }
    
    // Fallback or explicit GUI
    if (!PeruMap::instance().load("data/peru_map.json")) {
        cout << "Error: No se pudo cargar data/peru_map.json. Asegurate de ejecutar desde la carpeta 'cpp'." << endl;
    }
    
    MainWindow w;
    w.show();
    
    return app.exec();
}
