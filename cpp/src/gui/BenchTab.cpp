#include "BenchTab.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QtCharts/QChart>

BenchTab::BenchTab(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);
    
    QHBoxLayout *top = new QHBoxLayout();
    top->addWidget(new QLabel("Tamaño:"));
    
    comboPreset = new QComboBox();
    comboPreset->addItems({"Rápido (hasta 20 000)", "Medio (hasta 100 000)", "Completo (hasta 1 000 000)"});
    top->addWidget(comboPreset);
    
    btnRun = new QPushButton("Ejecutar benchmark");
    btnSave = new QPushButton("Guardar CSV");
    top->addWidget(btnRun);
    top->addWidget(btnSave);
    
    progressBar = new QProgressBar();
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    top->addWidget(progressBar);
    
    lblStatus = new QLabel("");
    top->addWidget(lblStatus);
    top->addStretch();
    
    layout->addLayout(top);
    
    QHBoxLayout *body = new QHBoxLayout();
    
    table = new QTableWidget(0, 7);
    table->setHorizontalHeaderLabels({"instancia", "n", "m", "algoritmo", "colores", "tiempo", "mem"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    body->addWidget(table, 1);
    
    QChart *chart = new QChart();
    chart->setTitle("Escalabilidad (grafos geométricos)");
    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    body->addWidget(chartView, 1);
    
    layout->addLayout(body);
    
    // Dummy connection for now (requires QtConcurrent for real async)
    connect(btnRun, &QPushButton::clicked, this, &BenchTab::runBenchmark);
    connect(btnSave, &QPushButton::clicked, this, &BenchTab::saveCsv);
}

#include <thread>
#include <QMessageBox>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QLogValueAxis>
#include <set>
#include <algorithm>

void BenchTab::runBenchmark() {
    btnRun->setEnabled(false);
    btnSave->setEnabled(false);
    table->setRowCount(0);
    progressBar->setValue(0);
    lblStatus->setText("Iniciando benchmark...");
    
    QString preset = comboPreset->currentText();
    std::vector<int> geo, gnp;
    int reps = 3;
    
    if (preset.startsWith("Rápido")) {
        geo = {500, 2000, 20000};
        gnp = {1000};
    } else if (preset.startsWith("Medio")) {
        geo = {1000, 10000, 100000};
        gnp = {1000};
    } else {
        geo = {1000, 10000, 100000, 1000000};
        gnp = {1000};
    }
    
    std::thread([this, geo, gnp, reps]() {
        auto rows = Benchmark::run_all(geo, gnp, reps, 3, 8.0, 42, [this](const std::string& msg, double frac) {
            QMetaObject::invokeMethod(this, [this, msg, frac]() {
                handleProgress(QString::fromStdString(msg), frac);
            });
        });
        
        QMetaObject::invokeMethod(this, [this, rows]() {
            currentRows = rows;
            handleFinished();
        });
    }).detach();
}

void BenchTab::handleProgress(const QString& msg, double frac) {
    progressBar->setValue(static_cast<int>(frac * 100));
    lblStatus->setText(msg);
}

void BenchTab::handleFinished() {
    btnRun->setEnabled(true);
    btnSave->setEnabled(true);
    lblStatus->setText("Benchmark finalizado.");
    fillTable(currentRows);
    drawChart(currentRows);
}

void BenchTab::fillTable(const std::vector<Row>& rows) {
    table->setRowCount(0);
    for (const auto& r : rows) {
        int row = table->rowCount();
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(r.instance)));
        table->setItem(row, 1, new QTableWidgetItem(QString::number(r.n)));
        table->setItem(row, 2, new QTableWidgetItem(QString::number(r.m)));
        table->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(r.algorithm)));
        table->setItem(row, 4, new QTableWidgetItem(QString::number(r.colors)));
        
        QString t = r.time_s < 1.0 ? QString::number(r.time_s * 1000.0, 'f', 3) + " ms"
                                   : QString::number(r.time_s, 'f', 3) + " s";
        table->setItem(row, 5, new QTableWidgetItem(t));
        table->setItem(row, 6, new QTableWidgetItem(r.mem_mb > 0 ? QString::number(r.mem_mb, 'f', 2) + " MB" : "—"));
    }
}

void BenchTab::drawChart(const std::vector<Row>& rows) {
    QChart *chart = chartView->chart();
    chart->removeAllSeries();
    for(auto ax : chart->axes()) {
        chart->removeAxis(ax);
    }
    
    std::set<std::string> algos;
    for (const auto& r : rows) {
        if (r.instance.rfind("geo_", 0) == 0) {
            algos.insert(r.algorithm);
        }
    }
    
    QLogValueAxis *axisX = new QLogValueAxis();
    axisX->setTitleText("n (vértices)");
    axisX->setLabelFormat("%g");
    axisX->setBase(10.0);
    
    QLogValueAxis *axisY = new QLogValueAxis();
    axisY->setTitleText("tiempo (s)");
    axisY->setLabelFormat("%g");
    axisY->setBase(10.0);
    
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    
    double minX = 1e9, maxX = 0;
    double minY = 1e9, maxY = 0;
    
    for (const auto& algo : algos) {
        QLineSeries *series = new QLineSeries();
        series->setName(QString::fromStdString(algo));
        
        std::vector<std::pair<int, double>> pts;
        for (const auto& r : rows) {
            if (r.instance.rfind("geo_", 0) == 0 && r.algorithm == algo && r.time_s > 0) {
                pts.push_back({r.n, std::max(0.0001, r.time_s)}); // prevent log(0)
            }
        }
        
        std::sort(pts.begin(), pts.end());
        
        for (const auto& pt : pts) {
            series->append(pt.first, pt.second);
            minX = std::min(minX, (double)pt.first);
            maxX = std::max(maxX, (double)pt.first);
            minY = std::min(minY, pt.second);
            maxY = std::max(maxY, pt.second);
        }
        
        if (!pts.empty()) {
            chart->addSeries(series);
            series->attachAxis(axisX);
            series->attachAxis(axisY);
        }
    }
    
    if (minX <= maxX) {
        axisX->setRange(std::max(10.0, minX / 2), maxX * 2);
        axisY->setRange(std::max(0.0001, minY / 2), maxY * 2);
    }
}

void BenchTab::saveCsv() {
    if (currentRows.empty()) {
        lblStatus->setText("Ejecute el benchmark primero.");
        return;
    }
    Benchmark::to_csv(currentRows, "data/resultados.csv");
    lblStatus->setText("Guardado en data/resultados.csv");
}
