#ifndef ALTITUDECHART_H
#define ALTITUDECHART_H

#include <QWidget>
#include <QVector>

class AltitudeChart : public QWidget
{
    Q_OBJECT

public:
    explicit AltitudeChart(
        QWidget *parent = nullptr
        );

    void setData(
        const QVector<double>& values,
        double target
        );

protected:
    void paintEvent(
        QPaintEvent *event
        ) override;

private:
    QVector<double> altitudeData;

    double targetAltitude;
};

#endif