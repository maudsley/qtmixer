#include "runpanel.h"
#include "ui_runpanel.h"
#include <sstream>
#include <QPainter>
#include <QImage>

RunPanel::RunPanel(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::RunPanel)
{
    QWidget* container = new QWidget(this);
    ui->setupUi(container);
    setWidget(container);

    connect(ui->runButton, &QPushButton::clicked, this, &RunPanel::runButtonClick);
    connect(ui->stopButton, &QPushButton::clicked, this, &RunPanel::stopButtonClick);

    setState(RunEnabled);

    ui->graph->setVisible(false);
    ui->error->setText("Idle");
}

RunPanel::~RunPanel()
{
    delete ui;
}

void RunPanel::setState(const States state) {
    if (state == RunEnabled) {
        ui->rate->setEnabled(true);
        ui->runButton->setEnabled(true);
        ui->stopButton->setEnabled(false);
    } else if (state == StopEnabled) {
        ui->rate->setEnabled(false);
        ui->runButton->setEnabled(false);
        ui->stopButton->setEnabled(true);
    } else if (state == StopDisabled) {
        ui->stopButton->setEnabled(false);
    }
}

void RunPanel::setError(const double error, const double best_error) {
    if (best_errors_.empty() || (best_error < errors_[best_errors_.back()])) {
        best_errors_.push_back(errors_.size());
        errors_.push_back(best_error);
    }

    errors_.push_back(error);

    std::stringstream ss;
    ss << "Best: " << errors_[best_errors_.back()] << ", Last: " << errors_.back() << std::endl;
    ui->error->setText(ss.str().c_str());

    QPixmap image(300, 100);
    QPainter painter(&image);

    const int width = image.width() - 1;
    const int height = image.height() - 1;

    painter.fillRect(0, 0, image.width(), image.height(), Qt::white);

    double max_error = -std::numeric_limits<double>::max();
    double min_error = std::numeric_limits<double>::max();

    for (size_t i = 0; i < errors_.size(); ++i) {
        max_error = std::max(max_error, errors_[i]);
        min_error = std::min(min_error, errors_[i]);
    }

    double scale = 0.8;

    painter.setPen(Qt::black);
    for (size_t i = 1; i < errors_.size(); ++i) {
        double x_begin = (i - 1) / double(errors_.size() - 1) * width;
        double y_begin = (errors_[i - 1] - min_error) / (max_error - min_error);
        y_begin = ((y_begin - 0.5) * scale + 0.5) * height;
        double x_end = i / double(errors_.size() - 1) * width;
        double y_end = (errors_[i] - min_error) / (max_error - min_error);
        y_end = ((y_end - 0.5) * scale + 0.5) * height;
        painter.drawLine(int(x_begin), int(height - y_begin), int(x_end), int(height - y_end));
    }

    painter.setPen(Qt::red);
    for (size_t i = 1; i < best_errors_.size(); ++i) {
        double x_begin = (best_errors_[i] - 1) / double(errors_.size() - 1) * width;
        double y_begin = (errors_[best_errors_[i - 1]] - min_error) / (max_error - min_error);
        y_begin = ((y_begin - 0.5) * scale + 0.5) * height;
        double x_end = best_errors_[i] / double(errors_.size() - 1) * width;
        double y_end = (errors_[best_errors_[i]] - min_error) / (max_error - min_error);
        y_end = ((y_end - 0.5) * scale + 0.5) * height;
        painter.drawLine(int(x_begin), int(height - y_begin), int(x_end), int(height - y_end));
    }

    ui->graph->setPixmap(image);
    ui->graph->setVisible(true);
}

void RunPanel::resetGraph() {
    errors_.clear();
    best_errors_.clear();
}

void RunPanel::runButtonClick() {
    ui->rate->setEnabled(false);
    emit runBegin(ui->rate->value());
}

void RunPanel::stopButtonClick() {
    emit runEnd();
}
