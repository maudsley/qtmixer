#include "outputwindow.h"
#include "ui_outputwindow.h"

OutputWindow::OutputWindow(const QPixmap& pixmap, const std::string& glsl, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::OutputWindow)
{
    ui->setupUi(this);

    setWindowTitle("Output Viewer");

    ui->label->setStyleSheet("QLabel { background-color : white; }");

    ui->label->setPixmap(pixmap);

    ui->code->setPlainText(glsl.c_str());
}

OutputWindow::~OutputWindow()
{
    delete ui;
}
