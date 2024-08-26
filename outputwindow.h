#ifndef OUTPUTWINDOW_H
#define OUTPUTWINDOW_H

#include <QMainWindow>
#include <QPixmap>

namespace Ui {
class OutputWindow;
}

class OutputWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit OutputWindow(const QPixmap& pixmap, const std::string& glsl, QWidget *parent = nullptr);
    ~OutputWindow();

private:
    Ui::OutputWindow *ui;
};

#endif // OUTPUTWINDOW_H
