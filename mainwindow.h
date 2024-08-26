#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "colourpanel.h"
#include "runpanel.h"
#include "runthread.h"
#include <QMainWindow>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    bool eventFilter(QObject *o, QEvent *e) override;

    void colorSelected(const QColor &color);

    void runClick();

    void runBegin(const double learningRate);
    void runEnd();

    void timerPoll();

private:
    Ui::MainWindow *ui;
    ColourPanel* colourPanel_;
    RunPanel* runPanel_;
    QImage image_;
    QColor src_colour_;
    QTimer* timer_;
    std::shared_ptr<run_thread> thread_;
};
#endif // MAINWINDOW_H
