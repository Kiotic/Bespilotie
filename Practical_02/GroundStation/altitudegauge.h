#ifndef ALTITUDEGAUGE_H
#define ALTITUDEGAUGE_H

#include <QWidget>

class AltitudeGauge : public QWidget
{
    Q_OBJECT

public:
    explicit AltitudeGauge(
        QWidget *parent = nullptr
        );

    void setAltitude(
        double altitude
        );

    void setTargetAltitude(
        double altitude
        );

protected:
    void paintEvent(
        QPaintEvent *event
        ) override;

private:
    double currentAltitude;
    double targetAltitude;

    double maximumAltitude;
};

#endif