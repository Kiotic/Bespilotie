#include "altitudegauge.h"

#include <QPainter>
#include <QPaintEvent>
#include <QtMath>

#include <algorithm>

AltitudeGauge::AltitudeGauge(
    QWidget *parent
    )
    : QWidget(parent),
    currentAltitude(0.0),
    targetAltitude(120.0),
    maximumAltitude(500.0)
{
    setMinimumSize(
        300,
        260
        );
}

void AltitudeGauge::setAltitude(
    double altitude
    )
{
    currentAltitude =
        altitude;

    if (currentAltitude >
        maximumAltitude) {

        maximumAltitude =
            currentAltitude * 1.2;
    }

    update();
}

void AltitudeGauge::setTargetAltitude(
    double altitude
    )
{
    targetAltitude =
        altitude;

    if (targetAltitude >
        maximumAltitude) {

        maximumAltitude =
            targetAltitude * 1.2;
    }

    update();
}

void AltitudeGauge::paintEvent(
    QPaintEvent *event
    )
{
    Q_UNUSED(event);

    QPainter painter(this);

    painter.setRenderHint(
        QPainter::Antialiasing
        );

    const QPointF center(
        width() / 2.0,
        height() / 2.0 + 35
        );

    const double radius =
        std::min(
            width(),
            height()
            ) * 0.36;

    QRectF gaugeRect(
        center.x() - radius,
        center.y() - radius,
        radius * 2.0,
        radius * 2.0
        );

    QPen arcPen;

    arcPen.setWidth(
        12
        );

    painter.setPen(
        arcPen
        );

    painter.drawArc(
        gaugeRect,
        225 * 16,
        -270 * 16
        );

    for (int i = 0;
         i <= 10;
         ++i) {

        double angle =
            225.0 -
            i * 27.0;

        double radians =
            qDegreesToRadians(
                angle
                );

        QPointF outer(
            center.x() +
                qCos(radians) *
                    radius,

            center.y() +
                qSin(radians) *
                    radius
            );

        QPointF inner(
            center.x() +
                qCos(radians) *
                    (radius - 15),

            center.y() +
                qSin(radians) *
                    (radius - 15)
            );

        painter.drawLine(
            inner,
            outer
            );

        double value =
            maximumAltitude *
            i / 10.0;

        QPointF textPoint(
            center.x() +
                qCos(radians) *
                    (radius - 35),

            center.y() +
                qSin(radians) *
                    (radius - 35)
            );

        painter.drawText(
            QRectF(
                textPoint.x() - 25,
                textPoint.y() - 10,
                50,
                20
                ),
            Qt::AlignCenter,
            QString::number(
                value,
                'f',
                0
                )
            );
    }

    double altitudeRatio =
        currentAltitude /
        maximumAltitude;

    altitudeRatio =
        std::clamp(
            altitudeRatio,
            0.0,
            1.0
            );

    double needleAngle =
        225.0 -
        altitudeRatio *
            270.0;

    double needleRadians =
        qDegreesToRadians(
            needleAngle
            );

    QPointF needleEnd(
        center.x() +
            qCos(
                needleRadians
                ) *
                (radius - 25),

        center.y() +
            qSin(
                needleRadians
                ) *
                (radius - 25)
        );

    QPen needlePen(
        Qt::red
        );

    needlePen.setWidth(
        4
        );

    painter.setPen(
        needlePen
        );

    painter.drawLine(
        center,
        needleEnd
        );

    painter.setBrush(
        Qt::red
        );

    painter.drawEllipse(
        center,
        6,
        6
        );

    double targetRatio =
        targetAltitude /
        maximumAltitude;

    targetRatio =
        std::clamp(
            targetRatio,
            0.0,
            1.0
            );

    double targetAngle =
        225.0 -
        targetRatio *
            270.0;

    double targetRadians =
        qDegreesToRadians(
            targetAngle
            );

    QPointF targetOuter(
        center.x() +
            qCos(targetRadians) *
                radius,

        center.y() +
            qSin(targetRadians) *
                radius
        );

    QPointF targetInner(
        center.x() +
            qCos(targetRadians) *
                (radius - 22),

        center.y() +
            qSin(targetRadians) *
                (radius - 22)
        );

    QPen targetPen(
        Qt::darkGreen
        );

    targetPen.setWidth(
        5
        );

    painter.setPen(
        targetPen
        );

    painter.drawLine(
        targetInner,
        targetOuter
        );

    painter.setPen(
        Qt::black
        );

    QFont valueFont =
        painter.font();

    valueFont.setBold(
        true
        );

    valueFont.setPointSize(
        16
        );

    painter.setFont(
        valueFont
        );

    painter.drawText(
        QRectF(
            0,
            height() - 70,
            width(),
            30
            ),
        Qt::AlignCenter,
        QString(
            "%1 m"
            ).arg(
                currentAltitude,
                0,
                'f',
                1
                )
        );

    QFont captionFont =
        painter.font();

    captionFont.setBold(
        false
        );

    captionFont.setPointSize(
        10
        );

    painter.setFont(
        captionFont
        );

    painter.drawText(
        QRectF(
            0,
            5,
            width(),
            25
            ),
        Qt::AlignCenter,
        "Current altitude"
        );

    painter.drawText(
        QRectF(
            0,
            height() - 40,
            width(),
            25
            ),
        Qt::AlignCenter,
        QString(
            "Target: %1 m"
            ).arg(
                targetAltitude,
                0,
                'f',
                0
                )
        );
}