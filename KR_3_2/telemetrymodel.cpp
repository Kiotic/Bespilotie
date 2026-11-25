#include "telemetrymodel.h"

TelemetryModel::TelemetryModel(QObject *parent)
    : QObject(parent)
{
}

void TelemetryModel::updateTelemetry(double latitude, double longitude, double altitude,
                                     double verticalSpeed, const QString &flightMode)
{
    m_latitude = latitude;
    m_longitude = longitude;
    m_altitude = altitude;
    m_verticalSpeed = verticalSpeed;
    m_lastUpdate = QDateTime::currentDateTime();

    if (flightMode != m_flightMode) {
        m_flightMode = flightMode;
        emit flightModeChanged(m_flightMode);
    }

    emit telemetryChanged(m_latitude, m_longitude, m_altitude, m_verticalSpeed, m_flightMode);
}

void TelemetryModel::updateLink(bool connected, const QString &address, quint16 port)
{
    if (connected == m_linkConnected && address == m_lastAddress && port == m_lastPort) {
        return;
    }
    m_linkConnected = connected;
    m_lastAddress = address;
    m_lastPort = port;
    emit linkStateChanged(m_linkConnected, m_lastAddress, m_lastPort);
}

double TelemetryModel::latitude() const { return m_latitude; }
double TelemetryModel::longitude() const { return m_longitude; }
double TelemetryModel::altitude() const { return m_altitude; }
double TelemetryModel::verticalSpeed() const { return m_verticalSpeed; }
QString TelemetryModel::flightMode() const { return m_flightMode; }
bool TelemetryModel::isLinkConnected() const { return m_linkConnected; }
QDateTime TelemetryModel::lastUpdate() const { return m_lastUpdate; }
