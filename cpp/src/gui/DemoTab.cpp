#include "DemoTab.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <sstream>
#include <iostream>
#include <iomanip>
#include "../core/PeruMap.h"
#include "../core/Algorithms.h"
#include "../core/CoreUtils.h"

using namespace std;

DemoTab::DemoTab(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);
    
    QHBoxLayout *top = new QHBoxLayout();
    btnRun = new QPushButton("Ejecutar demo (Perú)");
    top->addWidget(btnRun);
    top->addStretch();
    layout->addLayout(top);
    
    txtOutput = new QTextEdit();
    txtOutput->setReadOnly(true);
    QFont font("Consolas");
    font.setStyleHint(QFont::Monospace);
    txtOutput->setFont(font);
    layout->addWidget(txtOutput);
    
    connect(btnRun, &QPushButton::clicked, this, &DemoTab::runDemo);
}

void DemoTab::runDemo() {
    txtOutput->clear();
    txtOutput->append("Ejecutando…\n");
    
    // Simplification for the port: running synchronously in GUI thread for DemoTab
    // In a real Qt app, this would use QtConcurrent::run
    
    stringstream ss;
    
    PeruMap::instance().load("data/peru_map.json");
    Graph g = PeruMap::instance().build_graph();
    
    ss << "Mapa del Peru como grafo de interferencia (FAP): Graph(n=" << g.n() << ", m=" << g.m() << ")\n";
    ss << "  nodo = departamento (antena) | arista = frontera (interferencia)\n";
    ss << "  color = frecuencia | objetivo: minimizar frecuencias = chi(G)\n\n";
    
    auto linea = [&](const string& nombre, const unordered_map<int, int>& coloring, double ms, const string& extra = "") {
        bool ok = is_proper(g, coloring).valid;
        ss << "  " << left << setw(24) << nombre << " colores=" << num_colors(coloring) << "  "
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
    
    ss << "\nchi(Peru) = " << res.k << ". Cota inferior: la clique K4 {Cusco, Apurimac, Ayacucho, Arequipa} obliga a 4 frecuencias.\n";
    ss << "\nAsignacion optima de frecuencias:\n";
    
    auto table = PeruMap::instance().frequency_table(g, res.coloring);
    for (const auto& pair : table) {
        ss << "  f" << pair.first << ": ";
        for (size_t i = 0; i < pair.second.size(); ++i) {
            ss << pair.second[i] << (i + 1 < pair.second.size() ? ", " : "");
        }
        ss << "\n";
    }
    
    txtOutput->setPlainText(QString::fromStdString(ss.str()));
}
