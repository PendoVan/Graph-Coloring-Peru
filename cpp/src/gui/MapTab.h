#ifndef MAINTAB_H
#define MAINTAB_H

#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QComboBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QSpinBox>
#include <QTableWidget>
#include <QLabel>
#include <QTimer>
#include "../core/Graph.h"

class MapTab : public QWidget {
    Q_OBJECT
public:
    explicit MapTab(QWidget *parent = nullptr);
    ~MapTab();

private slots:
    void modeChanged();
    void sourceChanged();
    void colorize();
    void animateDistance();
    void resetMap();
    void drawMap();

private:
    void buildUI();
    void updateResults(const QString& algo, const QString& colors, const QString& time, const QString& proper);
    void updateTable();
    void fillPolygon(int dep, const QColor& color);

    Graph graph;
    int sourceId;
    std::vector<int> coloring;
    std::vector<int> bfsLevels;

    QGraphicsScene *scene;
    QGraphicsView *view;

    QRadioButton *btnModeFreq;
    QRadioButton *btnModeDist;
    QComboBox *comboAlgo;
    QComboBox *comboSource;
    QSpinBox *spinTimeout;
    QCheckBox *chkRoads;
    QCheckBox *chkLabels;

    QLabel *lblResultBase;
    QLabel *lblResultAlgo;
    QLabel *lblResultColors;
    QLabel *lblResultTime;
    QLabel *lblResultProper;
    
    QTableWidget *tableFreq;
    QLabel *lblHint;

    std::unordered_map<int, std::vector<QGraphicsPolygonItem*>> polyItems;
    QTimer *animTimer;
    std::vector<int> animOrder;
    int animIndex;
    bool isDistanceMode;
};

#endif // MAINTAB_H
