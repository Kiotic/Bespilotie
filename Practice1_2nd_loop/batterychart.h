#ifndef BATTERYCHART_H
#define BATTERYCHART_H

#include <QWidget>

class BatteryChart : public QWidget
{
    Q_OBJECT

public:
    explicit BatteryChart(QWidget *parent = nullptr);

    void setValues(double charge,
                   double current,
                   double power);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    double chargeValue;
    double currentValue;
    double powerValue;
};

#endif