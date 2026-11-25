#ifndef UAVSIMULATOR_H
#define UAVSIMULATOR_H

#include <QObject>
#include <QUdpSocket>
#include <QTimer>
#include <QElapsedTimer>

class UavSimulator : public QObject
{
    Q_OBJECT

public:
    explicit UavSimulator(QObject *parent = nullptr);

    void start(const QString &targetAddress, quint16 targetPort);
    void stop();
    bool isRunning() const;
    quint16 localPort() const;

    void corruptNextPacket();
    void skipPackets(int count);
    void setAckEnabled(bool enabled);
    void setSensorFailure(bool failed);

public slots:
    void sendTelemetry();

signals:
    void runningChanged(bool running);
    void commandReceived(int command, double parameter, bool ackSent);

private slots:
    void onReadyRead();

private:
    void updateFlight(double dt);
    void sendPacket(quint32 messageId, const QByteArray &payload);
    void handleCommand(const QByteArray &payload);

    QUdpSocket *m_udpSocket = nullptr;
    QTimer *m_telemetryTimer = nullptr;
    QElapsedTimer m_bootTime;
    QString m_targetAddress;
    quint16 m_targetPort = 0;

    double m_homeLatitude = 55.752220;
    double m_homeLongitude = 37.615560;
    double m_latitude = 55.752220;
    double m_longitude = 37.615560;
    double m_altitude = 0.0;
    double m_verticalSpeed = 0.0;
    double m_targetAltitude = 0.0;
    double m_orbitAngle = 0.0;
    quint32 m_flightMode = 0;

    quint8 m_sequence = 0;
    int m_tick = 0;
    int m_packetsToSkip = 0;
    bool m_corruptNextPacket = false;
    bool m_ackEnabled = true;
    bool m_sensorFailure = false;
    bool m_running = false;
};

#endif
