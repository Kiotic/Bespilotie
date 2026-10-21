#include "altitudechart.h"

#include <QPainter>
#include <QPaintEvent>
#include <QPolygonF>

#include <algorithm>

AltitudeChart::AltitudeChart(
    QWidget *parent
    )
    : QWidget(parent),
    targetAltitude(0.0)
{
    setMinimumSize(
        430,
        280
        );
}

void AltitudeChart::setData(
    const QVector<double>& values,
    double target
    )
{
    altitudeData =
        values;

    targetAltitude =
        target;

    update();
}

void AltitudeChart::paintEvent(
    QPaintEvent *event
    )
{
    Q_UNUSED(event);

    QPainter painter(this);

    painter.setRenderHint(
        QPainter::Antialiasing
        );

    const int left = 60;
    const int right = 25;
    const int top = 45;
    const int bottom = 45;

    const int graphWidth =
        width() - left - right;

    const int graphHeight =
        height() - top - bottom;

    QFont titleFont =
        painter.font();

    titleFont.setBold(true);
    titleFont.setPointSize(12);

    painter.setFont(
        titleFont
        );

    painter.drawText(
        QRect(
            0,
            5,
            width(),
            30
            ),
        Qt::AlignCenter,
        "Altitude history"
        );

    QFont normalFont =
        painter.font();

    normalFont.setBold(false);
    normalFont.setPointSize(9);

    painter.setFont(
        normalFont
        );

    painter.drawLine(
        left,
        top,
        left,
        top + graphHeight
        );

    painter.drawLine(
        left,
        top + graphHeight,
        left + graphWidth,
        top + graphHeight
        );

    if (altitudeData.isEmpty()) {

        painter.drawText(
            QRect(
                left,
                top,
                graphWidth,
                graphHeight
                ),
            Qt::AlignCenter,
            "No telemetry"
            );

        return;
    }

    double maxAltitude =
        *std::max_element(
            altitudeData.begin(),
            altitudeData.end()
            );

    if (targetAltitude >
        maxAltitude) {

        maxAltitude =
            targetAltitude;
    }

    if (maxAltitude < 1.0) {
        maxAltitude = 1.0;
    }

    maxAltitude *= 1.1;

    painter.drawText(
        5,
        top + 5,
        QString::number(
            maxAltitude,
            'f',
            0
            )
            + " m"
        );

    painter.drawText(
        25,
        top + graphHeight,
        "0"
        );

    if (targetAltitude > 0.0) {

        double targetRatio =
            targetAltitude /
            maxAltitude;

        double targetY =
            top +
            graphHeight -
            targetRatio *
                graphHeight;

        QPen targetPen(
            Qt::DashLine
            );

        targetPen.setWidth(2);

        painter.setPen(
            targetPen
            );

        painter.drawLine(
            left,
            static_cast<int>(
                targetY
                ),
            left + graphWidth,
            static_cast<int>(
                targetY
                )
            );

        painter.drawText(
            left + 5,
            static_cast<int>(
                targetY
                ) - 5,
            QString(
                "Target %1 m"
                ).arg(
                    targetAltitude,
                    0,
                    'f',
                    0
                    )
            );

        painter.setPen(
            Qt::SolidLine
            );
    }

    QPolygonF points;

    for (int i = 0;
         i < altitudeData.size();
         ++i) {

        double x;

        if (altitudeData.size() == 1) {

            x =
                left +
                graphWidth / 2.0;

        } else {

            x =
                left +
                i *
                    graphWidth /
                    static_cast<double>(
                        altitudeData.size() - 1
                        );
        }

        double ratio =
            altitudeData[i] /
            maxAltitude;

        double y =
            top +
            graphHeight -
            ratio *
                graphHeight;

        points.append(
            QPointF(
                x,
                y
                )
            );
    }

    QPen graphPen;

    graphPen.setWidth(
        2
        );

    painter.setPen(
        graphPen
        );

    if (points.size() > 1) {

        painter.drawPolyline(
            points
            );
    }

    for (const QPointF& point
         : points) {

        painter.drawEllipse(
            point,
            4,
            4
            );
    }

    painter.drawText(
        QRect(
            left,
            top + graphHeight + 10,
            graphWidth,
            25
            ),
        Qt::AlignCenter,
        QString(
            "Telemetry records: %1"
            ).arg(
                altitudeData.size()
                )
        );
}