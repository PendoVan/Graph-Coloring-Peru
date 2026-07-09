#ifndef RECURSOS_TAB_H
#define RECURSOS_TAB_H

#include <QWidget>
#include <QPushButton>
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>

class RecursosTab : public QWidget {
    Q_OBJECT
public:
    explicit RecursosTab(QWidget *parent = nullptr);
    ~RecursosTab();

private slots:
    void measureResources();
    void handleMeasurementFinished(const std::vector<double>& timesPeru, const std::vector<double>& memsPeru,
                                   const std::vector<double>& timesGeom, const std::vector<double>& memsGeom);

private:
    QPushButton *btnMeasure;
    QChartView *chartTime;
    QChartView *chartMem;
    QBarCategoryAxis *axisXTime;
    QBarCategoryAxis *axisXMem;
};

#endif // RECURSOS_TAB_H
