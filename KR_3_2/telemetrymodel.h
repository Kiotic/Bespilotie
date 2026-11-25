#ifndef TELEMETRYMODEL_H
#define TELEMETRYMODEL_H

#include <QObject>
#include <QDateTime>
#include <QString>

class TelemetryModel : public QObject
{
    Q_OBJECT

public:
    explicit TelemetryModel(QObject *parent = nullptr);

    void updateTelemetry(double latitude, double longitude, double altitude,
                         double verticalSpeed, const QString &flightMode);
    void updateLink(bool connected, const QString &address, quint16 port);

    double latitude() const;
    double longitude() const;
    double altitude() const;
    double verticalSpeed() const;
    QString flightMode() const;
    bool isLinkConnected() const;
    QDateTime lastUpdate() const;

signals:
    void telemetryChanged(double latitude, double longitude, double altitude,
                          double verticalSpeed, const QString &flightMode);
    void flightModeChanged(const QString &flightMode);
    void linkStateChanged(bool connected, const QString &address, quint16 port);

private:
    double m_latitude = 0.0;
    double m_longitude = 0.0;
    double m_altitude = 0.0;
    double m_verticalSpeed = 0.0;
    QString m_flightMode = "UNKNOWN";
    bool m_linkConnected = false;
    QString m_lastAddress;
    quint16 m_lastPort = 0;
    QDateTime m_lastUpdate;
};

#endif
