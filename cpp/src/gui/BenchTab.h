#ifndef BENCHTAB_H
#define BENCHTAB_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QComboBox>
#include <QProgressBar>
#include <QLabel>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include "../benchmark/Benchmark.h"

class BenchTab : public QWidget {
    Q_OBJECT
public:
    explicit BenchTab(QWidget *parent = nullptr);

private slots:
    void runBenchmark();
    void saveCsv();
    void handleProgress(const QString& msg, double frac);
    void handleFinished();

private:
    QComboBox *comboPreset;
    QPushButton *btnRun;
    QPushButton *btnSave;
    QProgressBar *progressBar;
    QLabel *lblStatus;
    QTableWidget *table;
    QChartView *chartView;
    
    std::vector<Row> currentRows;
    
    void fillTable(const std::vector<Row>& rows);
    void drawChart(const std::vector<Row>& rows);
};

#endif // BENCHTAB_H
