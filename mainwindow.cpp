#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "outputwindow.h"
#include <QFileDialog>
#include <QMouseEvent>
#include <QColorDialog>
#include <iostream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    runPanel_ = new RunPanel(this);
    addDockWidget(Qt::LeftDockWidgetArea, runPanel_);
    connect(runPanel_, &RunPanel::runBegin, this, &MainWindow::runBegin);
    connect(runPanel_, &RunPanel::runEnd, this, &MainWindow::runEnd);

    colourPanel_ = new ColourPanel(this);
    addDockWidget(Qt::LeftDockWidgetArea, colourPanel_);

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"), "/home/jana", tr("Image Files (*.png *.jpg *.bmp)"));

    image_.load(fileName);

    ui->label->setStyleSheet("QLabel { background-color : white; }");

    ui->label->setPixmap(QPixmap::fromImage(image_));

    ui->label->installEventFilter(this);

    timer_ = new QTimer(this);
    connect(timer_, &QTimer::timeout, this, &MainWindow::timerPoll);
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::eventFilter(QObject *o, QEvent *e) {
    if (thread_) {
        return false; /* still running */
    }
    if(o == ui->label && e->type() == QMouseEvent::MouseButtonPress) {
        QMouseEvent* me = dynamic_cast<QMouseEvent*>(e);
        src_colour_ = image_.pixel(me->pos());
        QColorDialog* cd = new QColorDialog(src_colour_, this);
        connect(cd, &QColorDialog::colorSelected, this, &MainWindow::colorSelected);
        cd->show();
    }
    return false;
}

void MainWindow::colorSelected(const QColor &color) {
    colourPanel_->addColourMapping(src_colour_, color);
}

void MainWindow::runBegin(const double learningRate) {
    colourPanel_->setInputEnabled(false);
    runPanel_->setState(RunPanel::StopEnabled);
    runPanel_->resetGraph();
    thread_ = std::make_shared<run_thread>(image_, colourPanel_->getColours(), learningRate);
    timer_->start(500);
}

void MainWindow::runEnd() {
    thread_->stop();
    runPanel_->setState(RunPanel::StopDisabled);
}

void MainWindow::timerPoll() {
    if (thread_) {
        if (thread_->is_done()) {
            OutputWindow* output = new OutputWindow(QPixmap::fromImage(thread_->result()), thread_->result_string(), this);
            output->show();
            thread_.reset();
            runPanel_->setState(RunPanel::RunEnabled);
            colourPanel_->setInputEnabled(true);
        } else if (thread_->is_running()){
            runPanel_->setError(thread_->get_last_error(), thread_->get_best_error());
        }
    }
}
