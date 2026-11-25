#include "signalbars.h"
#include <QPainter>

SignalBars::SignalBars(QWidget *parent)
    : QWidget(parent)
{
    setFixedSize(46, 30);
}

void SignalBars::setLevel(int level, const QColor &color)
{
    m_level = qBound(0, level, 4);
    m_color = color;
    update();
}

void SignalBars::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    const int barWidth = 8;
    const int gap = 4;
    for (int i = 0; i < 4; ++i) {
        const int barHeight = 8 + i * 7;
        const QRect bar(i * (barWidth + gap), height() - barHeight, barWidth, barHeight);
        painter.setBrush(i < m_level ? m_color : QColor(255, 255, 255, 35));
        painter.drawRoundedRect(bar, 2, 2);
    }
}
