#include "batterychart.h"

#include <QPainter>
#include <QPaintEvent>
#include <QString>
#include <algorithm>

BatteryChart::BatteryChart(QWidget *parent)
    : QWidget(parent),
    chargeValue(0.0),
    currentValue(0.0),
    powerValue(0.0)
{
    setMinimumSize(360, 300);
}

void BatteryChart::setValues(double charge,
                             double current,
                             double power)
{
    chargeValue = charge;
    currentValue = current;
    powerValue = power;

    update();
}

void BatteryChart::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);

    QFont titleFont = painter.font();

    titleFont.setPointSize(14);
    titleFont.setBold(true);

    painter.setFont(titleFont);

    painter.drawText(
        QRect(20, 10, width() - 40, 35),
        Qt::AlignCenter,
        "Battery diagram"
        );

    QFont normalFont = painter.font();

    normalFont.setPointSize(10);
    normalFont.setBold(false);

    painter.setFont(normalFont);

    const int left = 30;
    const int barWidth = width() - 60;
    const int barHeight = 30;

    painter.drawText(
        left,
        70,
        QString("Charge: %1 %")
            .arg(chargeValue, 0, 'f', 2)
        );

    QRect chargeBackground(
        left,
        80,
        barWidth,
        barHeight
        );

    painter.drawRect(chargeBackground);

    double chargeRatio =
        chargeValue / 100.0;

    chargeRatio =
        std::clamp(chargeRatio, 0.0, 1.0);

    QRect chargeBar(
        left,
        80,
        static_cast<int>(barWidth * chargeRatio),
        barHeight
        );

    painter.fillRect(chargeBar, Qt::green);

    painter.drawText(
        left,
        145,
        QString("Current: %1 A")
            .arg(currentValue, 0, 'f', 2)
        );

    QRect currentBackground(
        left,
        155,
        barWidth,
        barHeight
        );

    painter.drawRect(currentBackground);

    double currentRatio =
        currentValue / 50.0;

    currentRatio =
        std::clamp(currentRatio, 0.0, 1.0);

    QRect currentBar(
        left,
        155,
        static_cast<int>(barWidth * currentRatio),
        barHeight
        );

    painter.fillRect(currentBar, Qt::yellow);

    painter.drawText(
        left,
        220,
        QString("Power: %1 W")
            .arg(powerValue, 0, 'f', 2)
        );

    QRect powerBackground(
        left,
        230,
        barWidth,
        barHeight
        );

    painter.drawRect(powerBackground);

    double powerRatio =
        powerValue / 2000.0;

    powerRatio =
        std::clamp(powerRatio, 0.0, 1.0);

    QRect powerBar(
        left,
        230,
        static_cast<int>(barWidth * powerRatio),
        barHeight
        );

    painter.fillRect(powerBar, Qt::cyan);
}