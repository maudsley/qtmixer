#ifndef RUNPANEL_H
#define RUNPANEL_H

#include <QDockWidget>
#include <deque>

namespace Ui {
class RunPanel;
}

class RunPanel : public QDockWidget
{
    Q_OBJECT

public:
    explicit RunPanel(QWidget *parent = nullptr);
    ~RunPanel();

    enum States {
        RunEnabled,
        StopEnabled,
        StopDisabled
    };

    void setState(const States state);

    void setError(const double error, const double best_error);

    void resetGraph();

signals:
    void runBegin(const double learningRate);
    void runEnd();

private:
    void runButtonClick();
    void stopButtonClick();

    Ui::RunPanel *ui;
    std::deque<double> errors_;
    std::deque<double> best_errors_;
};

#endif // RUNPANEL_H
