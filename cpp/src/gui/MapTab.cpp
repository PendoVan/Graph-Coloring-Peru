#define _USE_MATH_DEFINES
#include "MapTab.h"
#include "../core/PeruMap.h"
#include "../core/Algorithms.h"
#include "../core/CoreUtils.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QGroupBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QGraphicsTextItem>
#include <QPen>
#include <QBrush>
#include <chrono>

static const QList<QColor> PALETTE = {
    QColor("#4e79a7"), QColor("#f28e2b"), QColor("#59a14f"), QColor("#e15759"), QColor("#b07aa1"),
    QColor("#edc948"), QColor("#76b7b2"), QColor("#ff9da7"), QColor("#9c755f"), QColor("#bab0ac")
};

QColor freqColor(int c) {
    if (c == 0) return QColor("#dddddd");
    return PALETTE[(c - 1) % PALETTE.size()];
}

QColor distColor(double t) {
    QColor c;
    c.setHsvF((1.0 - t) * 0.33, 0.75, 0.95);
    return c;
}

MapTab::MapTab(QWidget *parent) : QWidget(parent), animTimer(new QTimer(this)), animIndex(0) {
    graph = PeruMap::instance().build_graph();
    sourceId = 15; // Lima default id (varies depending on json, we will find it dynamically)
    
    // Find Lima
    for(const auto& cap : PeruMap::instance().capitals()) {
        if(cap.second.first == "Lima") {
            sourceId = cap.first;
            break;
        }
    }
    
    buildUI();
    
    connect(animTimer, &QTimer::timeout, [this]() {
        if (animIndex >= animOrder.size()) {
            animTimer->stop();
            if (!isDistanceMode) updateTable();
            return;
        }
        
        int dep = animOrder[animIndex];
        QColor targetColor;
        if (isDistanceMode) {
            int max_lvl = 1;
            for(const auto& pair : bfsLevels) max_lvl = std::max(max_lvl, pair.second);
            targetColor = distColor(static_cast<double>(bfsLevels[dep]) / std::max(1, max_lvl));
        } else {
            targetColor = freqColor(coloring[dep]);
        }
        
        fillPolygon(dep, targetColor);
        animIndex++;
    });

    QTimer::singleShot(120, this, &MapTab::drawMap);
}

MapTab::~MapTab() {}

void MapTab::buildUI() {
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    
    // Left Panel
    QVBoxLayout *leftLayout = new QVBoxLayout();
    
    QGroupBox *grpMode = new QGroupBox("Modo");
    QVBoxLayout *modeLayout = new QVBoxLayout();
    btnModeFreq = new QRadioButton("Frecuencias (coloreo FAP)");
    btnModeDist = new QRadioButton("Distancia (desde depto elegido)");
    btnModeFreq->setChecked(true);
    modeLayout->addWidget(btnModeFreq);
    modeLayout->addWidget(btnModeDist);
    grpMode->setLayout(modeLayout);
    leftLayout->addWidget(grpMode);
    
    comboAlgo = new QComboBox();
    comboAlgo->addItems({"Greedy (natural)", "Greedy (mayor grado)", "Greedy (smallest-last)", "Greedy (por distancia)", "DSATUR", "Backtracking (exacto)"});
    comboAlgo->setCurrentText("DSATUR");
    leftLayout->addWidget(new QLabel("Algoritmo (modo frecuencias)"));
    leftLayout->addWidget(comboAlgo);
    
    comboSource = new QComboBox();
    std::vector<int> verts = graph.vertices();
    std::sort(verts.begin(), verts.end(), [&](int a, int b){ return graph.label(a) < graph.label(b); });
    for (int v : verts) {
        comboSource->addItem(QString::fromStdString(graph.label(v)), v);
    }
    comboSource->setCurrentText(QString::fromStdString(graph.label(sourceId)));
    leftLayout->addWidget(new QLabel("Departamento base:"));
    leftLayout->addWidget(comboSource);
    
    QHBoxLayout *timeoutLayout = new QHBoxLayout();
    timeoutLayout->addWidget(new QLabel("timeout BT (s):"));
    spinTimeout = new QSpinBox();
    spinTimeout->setRange(1, 120);
    spinTimeout->setValue(10);
    timeoutLayout->addWidget(spinTimeout);
    leftLayout->addLayout(timeoutLayout);
    
    chkRoads = new QCheckBox("Mostrar carreteras (grafo)");
    chkRoads->setChecked(true);
    chkLabels = new QCheckBox("Mostrar nombres");
    chkLabels->setChecked(true);
    leftLayout->addWidget(chkRoads);
    leftLayout->addWidget(chkLabels);
    
    QPushButton *btnColorear = new QPushButton("▶ Colorear");
    QPushButton *btnAnimar = new QPushButton("⏱ Animar por distancia");
    QPushButton *btnReset = new QPushButton("↺ Reiniciar");
    leftLayout->addWidget(btnColorear);
    leftLayout->addWidget(btnAnimar);
    leftLayout->addWidget(btnReset);
    leftLayout->addStretch();
    
    mainLayout->addLayout(leftLayout);
    
    // Center Canvas
    scene = new QGraphicsScene(this);
    view = new QGraphicsView(scene);
    view->setRenderHint(QPainter::Antialiasing);
    mainLayout->addWidget(view, 1); // stretch 1
    
    // Right Panel
    QVBoxLayout *rightLayout = new QVBoxLayout();
    
    QGroupBox *grpResult = new QGroupBox("Resultado");
    QFormLayout *formResult = new QFormLayout();
    lblResultBase = new QLabel("—");
    lblResultAlgo = new QLabel("—");
    lblResultColors = new QLabel("—");
    lblResultTime = new QLabel("—");
    lblResultProper = new QLabel("—");
    formResult->addRow("Base:", lblResultBase);
    formResult->addRow("Algoritmo:", lblResultAlgo);
    formResult->addRow("Colores:", lblResultColors);
    formResult->addRow("Tiempo:", lblResultTime);
    formResult->addRow("Propio:", lblResultProper);
    grpResult->setLayout(formResult);
    rightLayout->addWidget(grpResult);
    
    QGroupBox *grpTable = new QGroupBox("Departamentos por frecuencia");
    QVBoxLayout *tableLayout = new QVBoxLayout();
    tableFreq = new QTableWidget(0, 2);
    tableFreq->setHorizontalHeaderLabels({"frec.", "departamentos"});
    tableFreq->horizontalHeader()->resizeSection(0, 50);
    tableFreq->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    tableFreq->setWordWrap(true);
    tableFreq->setTextElideMode(Qt::ElideNone);
    tableFreq->setAlternatingRowColors(true);
    tableFreq->verticalHeader()->setVisible(false); // Hide row numbers
    tableFreq->setSelectionMode(QAbstractItemView::NoSelection);
    tableLayout->addWidget(tableFreq);
    grpTable->setLayout(tableLayout);
    rightLayout->addWidget(grpTable);
    
    mainLayout->addLayout(rightLayout);
    
    lblHint = new QLabel("Clic en un departamento para elegirlo como base.");
    // We would place lblHint at the bottom if we had a vertical layout wrapping everything.
    
    // Connections
    connect(btnModeFreq, &QRadioButton::toggled, this, &MapTab::modeChanged);
    connect(chkRoads, &QCheckBox::stateChanged, this, &MapTab::drawMap);
    connect(chkLabels, &QCheckBox::stateChanged, this, &MapTab::drawMap);
    connect(comboSource, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MapTab::sourceChanged);
    connect(btnColorear, &QPushButton::clicked, this, &MapTab::colorize);
    connect(btnAnimar, &QPushButton::clicked, this, &MapTab::animateDistance);
    connect(btnReset, &QPushButton::clicked, this, &MapTab::resetMap);
}

void MapTab::modeChanged() {
    drawMap();
    if (btnModeDist->isChecked()) {
        updateResults("Distancia (BFS)", "—", "—", "verde=cerca");
    }
}

void MapTab::sourceChanged() {
    sourceId = comboSource->currentData().toInt();
    lblResultBase->setText(QString::fromStdString(graph.label(sourceId)));
    drawMap();
}

void MapTab::resetMap() {
    animTimer->stop();
    coloring.clear();
    updateTable();
    updateResults("—", "—", "—", "—");
    drawMap();
}

void MapTab::colorize() {
    animTimer->stop();
    if (btnModeDist->isChecked()) {
        drawMap();
        return;
    }
    
    QString algo = comboAlgo->currentText();
    auto t0 = std::chrono::high_resolution_clock::now();
    QString note = "";
    
    if (algo == "Greedy (natural)") coloring = greedy(graph, order_natural(graph));
    else if (algo == "Greedy (mayor grado)") coloring = greedy(graph, order_largest_first(graph));
    else if (algo == "Greedy (smallest-last)") coloring = greedy(graph, order_smallest_last(graph));
    else if (algo == "Greedy (por distancia)") coloring = greedy(graph, order_distance(graph, sourceId));
    else if (algo == "DSATUR") coloring = dsatur(graph);
    else if (algo == "Backtracking (exacto)") {
        ExactResult res = chromatic_number(graph, 300, spinTimeout->value());
        coloring = res.coloring;
        note = res.exact ? QString("χ=%1 (%2 nodos)").arg(res.k).arg(res.nodes)
                         : QString("χ∈[%1,%2] timeout").arg(res.lower).arg(res.upper);
    }
    
    auto t1 = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    
    bool proper = is_proper(graph, coloring).valid;
    updateResults(algo, QString::number(num_colors(coloring)), QString::number(ms, 'f', 3) + " ms", proper ? "sí" : "—" + (!note.isEmpty() ? " · " + note : ""));
    updateTable();
    drawMap();
}

void MapTab::animateDistance() {
    animTimer->stop();
    animOrder = order_distance(graph, sourceId);
    isDistanceMode = btnModeDist->isChecked();
    
    if (isDistanceMode) {
        bfsLevels = bfs_levels(graph, sourceId);
        updateResults("Animación distancia", "—", "—", "verde=cerca");
    } else {
        coloring = greedy(graph, animOrder);
        updateResults("Greedy por distancia", QString::number(num_colors(coloring)), "0.0 ms", "coloreo en oleadas");
    }
    
    drawMap();
    for(const auto& pair : polyItems) {
        for(auto poly : pair.second) {
            poly->setBrush(QColor("#eeeeee"));
        }
    }
    
    animIndex = 0;
    animTimer->start(110);
}

void MapTab::updateResults(const QString& algo, const QString& colors, const QString& time, const QString& proper) {
    lblResultAlgo->setText(algo);
    lblResultColors->setText(colors);
    lblResultTime->setText(time);
    lblResultProper->setText(proper);
}

void MapTab::updateTable() {
    tableFreq->setRowCount(0);
    if (coloring.empty()) return;
    
    auto table = PeruMap::instance().frequency_table(graph, coloring);
    for (const auto& pair : table) {
        QStringList deps;
        for (const auto& d : pair.second) deps << QString::fromStdString(d);
        
        int row = tableFreq->rowCount();
        tableFreq->insertRow(row);
        
        QTableWidgetItem *item1 = new QTableWidgetItem("f" + QString::number(pair.first));
        QTableWidgetItem *item2 = new QTableWidgetItem(deps.join(", "));
        
        tableFreq->setItem(row, 0, item1);
        tableFreq->setItem(row, 1, item2);
    }
    tableFreq->resizeRowsToContents();
}

void MapTab::fillPolygon(int dep, const QColor& color) {
    auto it = polyItems.find(dep);
    if (it != polyItems.end()) {
        for(auto item : it->second) {
            item->setBrush(color);
        }
    }
}

void MapTab::drawMap() {
    scene->clear();
    polyItems.clear();
    
    auto rings = PeruMap::instance().rings();
    
    double minLon = 1e9, maxLon = -1e9, minLat = 1e9, maxLat = -1e9;
    for(const auto& r : rings) {
        for(const auto& poly : r.second) {
            for(const auto& pt : poly) {
                minLon = std::min(minLon, pt.x());
                maxLon = std::max(maxLon, pt.x());
                minLat = std::min(minLat, pt.y());
                maxLat = std::max(maxLat, pt.y());
            }
        }
    }
    
    double kx = std::cos((minLat + maxLat) / 2.0 * M_PI / 180.0);
    double s = 1.0;
    
    if (btnModeDist->isChecked()) {
        bfsLevels = bfs_levels(graph, sourceId);
    }
    
    int max_lvl = 1;
    for(const auto& pair : bfsLevels) max_lvl = std::max(max_lvl, pair.second);

    for (const auto& r : rings) {
        int dep = r.first;
        QColor color = QColor("#dddddd");
        if (btnModeDist->isChecked()) {
            color = distColor(static_cast<double>(bfsLevels[dep]) / std::max(1, max_lvl));
        } else if (coloring.count(dep)) {
            color = freqColor(coloring[dep]);
        }
        
        QPen pen(QColor("#555555"));
        pen.setWidth(0); // Cosmetic pen for normal departments
        if (dep == sourceId) {
            pen.setColor(QColor("#111111"));
            pen.setWidth(2);
            pen.setCosmetic(true); // Prevent border from becoming giant on zoom
        }
        
        for (const auto& poly : r.second) {
            QPolygonF tfPoly;
            for(const auto& pt : poly) {
                tfPoly << QPointF((pt.x() - minLon) * kx * s, -(pt.y() - minLat) * s);
            }
            QGraphicsPolygonItem *item = scene->addPolygon(tfPoly, pen, QBrush(color));
            polyItems[dep].push_back(item);
        }
    }
    
    if (chkRoads->isChecked()) {
        auto routes = PeruMap::instance().routes();
        for (const auto& pair : routes) {
            const Route& r = pair.second;
            QPainterPath path;
            if (!r.path.empty()) {
                path.moveTo((r.path[0].x() - minLon) * kx * s, -(r.path[0].y() - minLat) * s);
                for (size_t i = 1; i < r.path.size(); ++i) {
                    path.lineTo((r.path[i].x() - minLon) * kx * s, -(r.path[i].y() - minLat) * s);
                }
            }
            QPen rPen(QColor("#c0392b"), 0); // width 0 is a cosmetic pen (always 1 pixel wide)
            if (r.straight) {
                rPen.setColor(QColor("#2980b9"));
                rPen.setStyle(Qt::DashLine);
            }
            scene->addPath(path, rPen);
        }
    }
    
    if (chkLabels->isChecked()) {
        auto capitals = PeruMap::instance().capitals();
        for (const auto& cap : capitals) {
            QGraphicsTextItem *text = scene->addText(QString::fromStdString(cap.second.first.substr(0, 4)));
            QFont f = text->font();
            f.setPointSize(6);
            f.setBold(false);
            text->setFont(f);
            text->setDefaultTextColor(QColor("#000000"));
            text->setFlag(QGraphicsItem::ItemIgnoresTransformations);
            text->setPos((cap.second.second.x() - minLon) * kx * s, -(cap.second.second.y() - minLat) * s);
        }
    }
    
    view->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
    view->scale(1.05, 1.05); // Ligeramente más grande (zoom in)
}
