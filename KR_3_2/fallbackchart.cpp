#include "fallbackchart.h"

#include <QPainter>
#include <QPainterPath>
#include <algorithm>

FallbackChart::FallbackChart(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(260);
}

void FallbackChart::addPoint(double time, double altitude, double verticalSpeed)
{
    m_altitude.append(QPointF(time, altitude));
    m_speed.append(QPointF(time, verticalSpeed));
    if (m_altitude.size() > 300) {
        m_altitude.removeFirst();
        m_speed.removeFirst();
    }
    update();
}

void FallbackChart::clear()
{
    m_altitude.clear();
    m_speed.clear();
    update();
}

void FallbackChart::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor(20, 27, 42));

    const QRectF plot(50, 30, width() - 100, height() - 60);
    const double t1 = m_altitude.isEmpty() ? 30.0 : std::max(30.0, m_altitude.last().x());
    const double t0 = t1 - 30.0;
    double maxAltitude = 20.0;
    for (const QPointF &p : m_altitude) {
        maxAltitude = std::max(maxAltitude, p.y() * 1.1);
    }

    painter.setPen(QColor(132, 146, 175));
    painter.drawText(QRectF(10, 4, width() - 20, 20), Qt::AlignLeft, "Высота БПЛА");
    for (int i = 0; i <= 4; ++i) {
        const double y = plot.bottom() - plot.height() * i / 4.0;
        painter.setPen(QColor(255, 255, 255, 25));
        painter.drawLine(QPointF(plot.left(), y), QPointF(plot.right(), y));
        painter.setPen(QColor(132, 146, 175));
        painter.drawText(QRectF(0, y - 8, 45, 16), Qt::AlignRight | Qt::AlignVCenter,
                         QString::number(maxAltitude * i / 4.0, 'f', 0));
        painter.drawText(QRectF(plot.right() + 5, y - 8, 45, 16), Qt::AlignLeft | Qt::AlignVCenter,
                         QString::number(-5.0 + 2.5 * i, 'f', 1));
    }

    auto drawSeries = [&](const QVector<QPointF> &points, double low, double high, const QColor &color) {
        if (points.size() < 2) {
            return;
        }
        QPainterPath path;
        for (int i = 0; i < points.size(); ++i) {
            const QPointF p(plot.left() + (points[i].x() - t0) / (t1 - t0) * plot.width(),
                            plot.bottom() - (points[i].y() - low) / (high - low) * plot.height());
            if (i == 0) {
                path.moveTo(p);
            } else {
                path.lineTo(p);
            }
        }
        painter.setPen(QPen(color, 2));
        painter.setBrush(Qt::NoBrush);
        painter.drawPath(path);
    };

    painter.save();
    painter.setClipRect(plot);
    drawSeries(m_speed, -5.0, 5.0, QColor(255, 182, 72));
    drawSeries(m_altitude, 0.0, maxAltitude, QColor(58, 208, 255));
    painter.restore();
}
