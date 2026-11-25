#ifndef SIGNALBARS_H
#define SIGNALBARS_H

#include <QWidget>
#include <QColor>

class SignalBars : public QWidget
{
public:
    explicit SignalBars(QWidget *parent = nullptr);
    void setLevel(int level, const QColor &color);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int m_level = 0;
    QColor m_color = QColor(255, 92, 108);
};

#endif
