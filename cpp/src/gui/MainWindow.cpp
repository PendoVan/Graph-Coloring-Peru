#include "MainWindow.h"
#include "MapTab.h"
#include "BenchTab.h"
#include "DemoTab.h"
#include "DocsTab.h"
#include "RecursosTab.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Coloreo de Grafos — Asignación de Frecuencias (FAP) · Mapa del Perú");
    resize(1260, 800);
    setMinimumSize(1040, 640);
    
    tabs = new QTabWidget(this);
    setCentralWidget(tabs);
    
    tabs->addTab(new MapTab(), "Mapa del Perú");
    tabs->addTab(new BenchTab(), "Benchmark y Gráficos");
    tabs->addTab(new RecursosTab(), "Recursos");
    tabs->addTab(new DemoTab(), "Consola / Log");
    tabs->addTab(new DocsTab(), "Reportes y Documentación");
}

MainWindow::~MainWindow() {}
