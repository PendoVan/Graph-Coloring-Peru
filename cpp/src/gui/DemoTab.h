#ifndef DEMOTAB_H
#define DEMOTAB_H

#include <QWidget>
#include <QTextEdit>
#include <QPushButton>

class DemoTab : public QWidget {
    Q_OBJECT
public:
    explicit DemoTab(QWidget *parent = nullptr);

private slots:
    void runDemo();

private:
    QTextEdit *txtOutput;
    QPushButton *btnRun;
};

#endif // DEMOTAB_H
