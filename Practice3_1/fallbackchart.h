#ifndef FALLBACKCHART_H
#define FALLBACKCHART_H

#include <QWidget>
#include <QVector>
#include <QPointF>

// Запасной график (QPainter), используется только если модуль Qt Charts не установлен
class FallbackChart : public QWidget {
public:
    explicit FallbackChart(QWidget* parent = nullptr);
    void add_point(double t, double altitude, double target, double control);
protected:
    void paintEvent(QPaintEvent*) override;
private:
    QVector<QPointF> altitude_, target_, control_;
};

#endif
