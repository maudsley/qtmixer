#ifndef COLOURPANEL_H
#define COLOURPANEL_H

#include <QDockWidget>
#include <QPushButton>
#include <vector>
#include <map>

namespace Ui {
class ColourPanel;
}

class ColourPanel : public QDockWidget
{
    Q_OBJECT

public:
    explicit ColourPanel(QWidget *parent = nullptr);
    ~ColourPanel();

    std::vector<std::pair<QColor, QColor>> getColours() const;

    void setInputEnabled(const bool state);

    void addColourMapping(const QColor& src, const QColor& dest);

private:
    void removeColour();

    void refreshColourList();

    Ui::ColourPanel *ui;
    std::map<QPushButton*, size_t> row_;
    std::vector<std::pair<QColor, QColor>> map_;
};

#endif // COLOURPANEL_H
