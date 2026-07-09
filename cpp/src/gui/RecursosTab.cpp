#include "RecursosTab.h"
#include "../core/PeruMap.h"
#include "../core/Generators.h"
#include "../core/Algorithms.h"
#include "../benchmark/Benchmark.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <thread>
#include <chrono>

RecursosTab::RecursosTab(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    btnMeasure = new QPushButton("Medir recursos (Perú + geométrico 5000)");
    mainLayout->addWidget(btnMeasure);
    
    QHBoxLayout *chartsLayout = new QHBoxLayout();
    
    // Time Chart
    QChart *chartT = new QChart();
    chartT->setTitle("Tiempo");
    chartTime = new QChartView(chartT);
    chartTime->setRenderHint(QPainter::Antialiasing);
    chartsLayout->addWidget(chartTime);
    
    // Mem Chart
    QChart *chartM = new QChart();
    chartM->setTitle("Memoria pico");
    chartMem = new QChartView(chartM);
    chartMem->setRenderHint(QPainter::Antialiasing);
    chartsLayout->addWidget(chartMem);
    
    mainLayout->addLayout(chartsLayout);
    
    connect(btnMeasure, &QPushButton::clicked, this, &RecursosTab::measureResources);
}

RecursosTab::~RecursosTab() {}

void RecursosTab::measureResources() {
    btnMeasure->setEnabled(false);
    btnMeasure->setText("Midiendo (espere un momento)...");
    
    std::thread([this]() {
        std::vector<QString> algos = {"Greedy (natural)", "Greedy (mayor grado)", "Greedy (smallest-last)", "DSATUR"};
        std::vector<double> tP(4, 0), mP(4, 0), tG(4, 0), mG(4, 0);
        
        Graph peru = PeruMap::instance().build_graph();
        Graph geom = geometric(5000, 6.0).g;
        
        auto run_algo = [](const Graph& g, const QString& algo_name, double& time_ms, double& mem_mb) {
            double mem1 = Benchmark::get_current_memory_mb();
            auto start = std::chrono::high_resolution_clock::now();
            
            std::vector<int> result;
            if (algo_name == "Greedy (natural)") result = greedy(g, order_natural(g));
            else if (algo_name == "Greedy (mayor grado)") result = greedy(g, order_largest_first(g));
            else if (algo_name == "Greedy (smallest-last)") result = greedy(g, order_smallest_last(g));
            else if (algo_name == "DSATUR") result = dsatur(g);
            
            auto end = std::chrono::high_resolution_clock::now();
            double mem2 = Benchmark::get_current_memory_mb();
            
            time_ms = std::chrono::duration<double, std::milli>(end - start).count();
            mem_mb = std::max(0.0, mem2 - mem1);
            
            // To prevent compiler optimizing out the result
            volatile size_t dummy = result.size();
        };
        
        for (size_t i = 0; i < algos.size(); ++i) {
            run_algo(peru, algos[i], tP[i], mP[i]);
            run_algo(geom, algos[i], tG[i], mG[i]);
        }
        
        QMetaObject::invokeMethod(this, [this, tP, mP, tG, mG]() {
            handleMeasurementFinished(tP, mP, tG, mG);
        }, Qt::QueuedConnection);
    }).detach();
}

void RecursosTab::handleMeasurementFinished(const std::vector<double>& tP, const std::vector<double>& mP,
                                           const std::vector<double>& tG, const std::vector<double>& mG) {
    btnMeasure->setEnabled(true);
    btnMeasure->setText("Medir recursos (Perú + geométrico 5000)");
    
    QStringList categories = {"g.natural", "g.largest_first", "g.smallest_last", "dsatur"};
    
    // Setup Time Chart
    QBarSet *setPeruT = new QBarSet("Perú (n=25)");
    QBarSet *setGeomT = new QBarSet("geométrico (n=5000)");
    for(int i=0; i<4; ++i) { *setPeruT << tP[i]; *setGeomT << tG[i]; }
    
    QBarSeries *seriesT = new QBarSeries();
    seriesT->append(setPeruT);
    seriesT->append(setGeomT);
    
    QChart *chartT = chartTime->chart();
    chartT->removeAllSeries();
    chartT->addSeries(seriesT);
    chartT->createDefaultAxes();
    chartT->axes(Qt::Horizontal).first()->deleteLater();
    
    QBarCategoryAxis *axisXT = new QBarCategoryAxis();
    axisXT->append(categories);
    chartT->addAxis(axisXT, Qt::AlignBottom);
    seriesT->attachAxis(axisXT);
    chartT->axes(Qt::Vertical).first()->setTitleText("ms");
    
    // Setup Mem Chart
    QBarSet *setPeruM = new QBarSet("Perú (n=25)");
    QBarSet *setGeomM = new QBarSet("geométrico (n=5000)");
    for(int i=0; i<4; ++i) { *setPeruM << mP[i]; *setGeomM << mG[i]; }
    
    QBarSeries *seriesM = new QBarSeries();
    seriesM->append(setPeruM);
    seriesM->append(setGeomM);
    
    QChart *chartM = chartMem->chart();
    chartM->removeAllSeries();
    chartM->addSeries(seriesM);
    chartM->createDefaultAxes();
    chartM->axes(Qt::Horizontal).first()->deleteLater();
    
    QBarCategoryAxis *axisXM = new QBarCategoryAxis();
    axisXM->append(categories);
    chartM->addAxis(axisXM, Qt::AlignBottom);
    seriesM->attachAxis(axisXM);
    chartM->axes(Qt::Vertical).first()->setTitleText("MB");
}
