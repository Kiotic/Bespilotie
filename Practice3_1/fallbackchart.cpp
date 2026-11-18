#include "fallbackchart.h"
#include <QPainter>
#include <QPainterPath>
#include <algorithm>
#include <cmath>

FallbackChart::FallbackChart(QWidget* parent) : QWidget(parent) { setMinimumHeight(240); }

void FallbackChart::add_point(double t, double a, double tg, double c)
{
    altitude_.append(QPointF(t, a));
    target_.append(QPointF(t, tg));
    control_.append(QPointF(t, c));
    while (!altitude_.isEmpty() && altitude_.first().x() < t - 30.0) {
        altitude_.removeFirst(); target_.removeFirst(); control_.removeFirst();
    }
    update();
}

void FallbackChart::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(20, 27, 42));
    p.drawRoundedRect(rect(), 10, 10);

    const QRectF plot(50, 30, width() - 70, height() - 55);
    double t1 = altitude_.isEmpty() ? 30.0 : std::max(30.0, altitude_.last().x());
    double t0 = t1 - 30.0;
    double lo = 1e9, hi = -1e9;
    for (const auto& v : {altitude_, target_})
        for (const auto& q : v) { lo = std::min(lo, q.y()); hi = std::max(hi, q.y()); }
    if (lo > hi) { lo = 100; hi = 250; }
    lo -= 10; hi += 10;

    auto map = [&](const QPointF& q, double ylo, double yhi) {
        return QPointF(plot.left() + (q.x() - t0) / (t1 - t0) * plot.width(),
                       plot.bottom() - (q.y() - ylo) / (yhi - ylo) * plot.height());
    };
    p.setPen(QColor(132, 146, 175));
    p.drawText(QRectF(10, 4, width() - 20, 20), Qt::AlignLeft, "Altitude, m (Qt Charts не найден — упрощённый график)");
    for (int i = 0; i <= 4; ++i) {
        const double y = lo + (hi - lo) * i / 4.0;
        const double py = plot.bottom() - plot.height() * i / 4.0;
        p.setPen(QColor(255, 255, 255, 25));
        p.drawLine(QPointF(plot.left(), py), QPointF(plot.right(), py));
        p.setPen(QColor(132, 146, 175));
        p.drawText(QRectF(0, py - 8, 45, 16), Qt::AlignRight | Qt::AlignVCenter, QString::number(y, 'f', 0));
    }
    auto draw = [&](const QVector<QPointF>& v, QColor c, Qt::PenStyle st, double ylo, double yhi) {
        if (v.size() < 2) return;
        QPainterPath path(map(v.first(), ylo, yhi));
        for (const auto& q : v) path.lineTo(map(q, ylo, yhi));
        p.setPen(QPen(c, 2, st));
        p.setBrush(Qt::NoBrush);
        p.drawPath(path);
    };
    draw(control_, QColor(177, 140, 255), Qt::SolidLine, -60, 60);
    draw(target_, QColor(62, 224, 137), Qt::DashLine, lo, hi);
    draw(altitude_, QColor(58, 208, 255), Qt::SolidLine, lo, hi);
}
