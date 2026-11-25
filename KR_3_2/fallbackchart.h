#ifndef FALLBACKCHART_H
#define FALLBACKCHART_H

#include <QWidget>
#include <QVector>
#include <QPointF>

class FallbackChart : public QWidget
{
public:
    explicit FallbackChart(QWidget *parent = nullptr);
    void addPoint(double time, double altitude, double verticalSpeed);
    void clear();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QVector<QPointF> m_altitude;
    QVector<QPointF> m_speed;
};

#endif
