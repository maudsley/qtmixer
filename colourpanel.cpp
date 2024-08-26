#include "colourpanel.h"
#include "ui_colourpanel.h"
#include <QLabel>
#include <QPushButton>

ColourPanel::ColourPanel(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::ColourPanel)
{
    QWidget* container = new QWidget(this);
    ui->setupUi(container);
    setWidget(container);
}

ColourPanel::~ColourPanel()
{
    delete ui;
}

std::vector<std::pair<QColor, QColor>> ColourPanel::getColours() const {
    return map_;
}

void ColourPanel::setInputEnabled(const bool state) {
    for (auto map_item : row_) {
        map_item.first->setEnabled(state);
    }
}

void ColourPanel::addColourMapping(const QColor& src, const QColor& dest) {
    map_.push_back(std::make_pair(src, dest));
    refreshColourList();
}

void ColourPanel::removeColour() {
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    auto map_item = row_.find(button);
    if (map_item != std::end(row_)) {
        map_.erase(std::begin(map_) + map_item->second);
        refreshColourList();
    }
}

void ColourPanel::refreshColourList() {
    ui->tableWidget->clear();
    ui->tableWidget->setRowCount(0);
    row_.clear();

    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setColumnCount(3);
    ui->tableWidget->verticalHeader()->hide();

    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "Input" << "Output" << "");
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);

    ui->tableWidget->setSelectionMode(QAbstractItemView::NoSelection);

    for (size_t i = 0; i < map_.size(); ++i) {
        const size_t index = ui->tableWidget->rowCount();
        ui->tableWidget->setRowCount(index + 1);

        const size_t width = 256;
        const size_t height = 32;

        QPixmap srcPixmap(width, height);
        srcPixmap.fill(map_[i].first);
        QLabel* srcLabel = new QLabel(this);
        srcLabel->setPixmap(srcPixmap);
        srcLabel->setAlignment(Qt::AlignCenter);

        QPixmap destPixmap(width, height);
        destPixmap.fill(map_[i].second);
        QLabel* destLabel = new QLabel(this);
        destLabel->setPixmap(destPixmap);
        destLabel->setAlignment(Qt::AlignCenter);

        QPushButton* button = new QPushButton(this);
        button->setIcon(style()->standardIcon(QStyle::StandardPixmap::SP_TitleBarCloseButton));

        connect(button, &QPushButton::clicked, this, &ColourPanel::removeColour);

        ui->tableWidget->setCellWidget(index, 0, srcLabel);
        ui->tableWidget->setCellWidget(index, 1, destLabel);
        ui->tableWidget->setCellWidget(index, 2, button);

        row_.emplace(button, index);
    }
}
