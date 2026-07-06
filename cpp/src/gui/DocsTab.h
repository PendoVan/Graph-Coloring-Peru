#ifndef DOCS_TAB_H
#define DOCS_TAB_H

#include <QWidget>
#include <QLabel>

class DocsTab : public QWidget {
    Q_OBJECT
public:
    explicit DocsTab(QWidget *parent = nullptr);
    ~DocsTab();

private:
    void openPdf();
    void openDocsFolder();
};

#endif // DOCS_TAB_H
