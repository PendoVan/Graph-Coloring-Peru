#include "DocsTab.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDesktopServices>
#include <QUrl>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>

DocsTab::DocsTab(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);
    
    QLabel *lblTitle = new QLabel("<h2>Reportes y Documentación del Proyecto</h2>");
    lblTitle->setAlignment(Qt::AlignCenter);
    layout->addWidget(lblTitle);
    
    QLabel *lblDesc = new QLabel("Aquí puedes consultar los reportes teóricos, el análisis de complejidad algorítmica y los archivos estáticos generados durante la investigación.");
    lblDesc->setWordWrap(true);
    lblDesc->setAlignment(Qt::AlignCenter);
    layout->addWidget(lblDesc);
    
    layout->addSpacing(30);
    
    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    buttonsLayout->addStretch();
    
    QPushButton *btnPdf = new QPushButton("📄 Abrir Análisis de Complejidad (PDF)");
    btnPdf->setMinimumHeight(50);
    btnPdf->setMinimumWidth(300);
    buttonsLayout->addWidget(btnPdf);
    
    QPushButton *btnFolder = new QPushButton("📁 Explorar carpeta de Documentos");
    btnFolder->setMinimumHeight(50);
    btnFolder->setMinimumWidth(300);
    buttonsLayout->addWidget(btnFolder);
    
    buttonsLayout->addStretch();
    layout->addLayout(buttonsLayout);
    
    layout->addStretch();
    
    connect(btnPdf, &QPushButton::clicked, this, &DocsTab::openPdf);
    connect(btnFolder, &QPushButton::clicked, this, &DocsTab::openDocsFolder);
}

DocsTab::~DocsTab() {}

void DocsTab::openPdf() {
    QFileInfo fileInfo("docs/analisis_complejidad.pdf");
    if (fileInfo.exists()) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(fileInfo.absoluteFilePath()));
    } else {
        QMessageBox::warning(this, "Archivo no encontrado", "No se encontró 'docs/analisis_complejidad.pdf'.");
    }
}

void DocsTab::openDocsFolder() {
    QFileInfo dirInfo("docs/");
    if (dirInfo.exists() && dirInfo.isDir()) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(dirInfo.absoluteFilePath()));
    } else {
        QMessageBox::warning(this, "Carpeta no encontrada", "No se encontró la carpeta 'docs/'.");
    }
}
