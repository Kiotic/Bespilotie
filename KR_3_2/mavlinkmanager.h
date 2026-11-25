#ifndef MAVLINKMANAGER_H
#define MAVLINKMANAGER_H

#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>
#include <QDateTime>
#include <QTimer>

#include "mavlinkmessageparser.h"

class TelemetryModel;

class MavlinkManager : public QObject
{
    Q_OBJECT

public:
    enum LinkQuality { QualityBad = 0, QualityWeak = 1, QualityGood = 2 };

    explicit MavlinkManager(TelemetryModel *telemetry, QObject *parent = nullptr);

    bool start(quint16 listenPort);
    void stop();
    bool isListening() const;
    void setTarget(const QString &address, quint16 port);

    quint64 receivedPackets() const;
    quint64 lostPackets() const;
    quint64 errorPackets() const;
    double packetsPerSecond() const;
    QDateTime lastPacketTime() const;
    bool isWaitingAck() const;
    int ackRemainingMs() const;

public slots:
    void sendTakeoff(double targetAltitude);
    void sendLand();

signals:
    void telemetryReceived(double latitude, double longitude, double altitude,
                           double verticalSpeed, const QString &flightMode);
    void linkStateChanged(bool connected, const QString &address, quint16 port);
    void linkLost();
    void linkQualityChanged(double packetsPerSecond, int quality);
    void packetReceived(quint64 received, quint64 lost);
    void packetError(quint64 errors, const QString &reason);
    void uavStatusChanged(bool healthy, const QString &status);
    void commandSent(int command, double parameter);
    void commandResult(int command, bool accepted, const QString &message);
    void errorOccurred(const QString &message);

private slots:
    void onReadyRead();
    void checkLinkTimeout();
    void onAckTimeout();

private:
    bool processDatagram(const QByteArray &data);
    void handleHeartbeat(const MavlinkFrame &frame);
    void handleGlobalPosition(const MavlinkFrame &frame);
    void handleCommandAck(const MavlinkFrame &frame);
    void sendCommand(const CommandLongMessage &command, double parameter);

    static const quint8 kSystemId = 255;
    static const quint8 kComponentId = 190;
    static const int kLinkTimeoutMs = 3000;
    static const int kAckTimeoutMs = 3000;

    QUdpSocket *m_udpSocket = nullptr;
    TelemetryModel *m_telemetry = nullptr;
    QTimer *m_linkTimer = nullptr;
    QTimer *m_ackTimer = nullptr;

    QHostAddress m_targetAddress;
    quint16 m_targetPort = 0;
    quint8 m_targetSystemId = 1;
    quint8 m_targetComponentId = 1;
    quint8 m_sequence = 0;

    quint64 m_receivedPackets = 0;
    quint64 m_lostPackets = 0;
    quint64 m_errorPackets = 0;
    qint64 m_lastSequence = -1;

    bool m_linkConnected = false;
    QString m_lastAddress;
    quint16 m_lastPort = 0;
    QDateTime m_firstPacketTime;
    QDateTime m_lastPacketTime;
    quint64 m_packetsSinceCheck = 0;
    double m_packetsPerSecond = 0.0;

    QString m_flightMode = "UNKNOWN";
    int m_systemStatus = -1;
    int m_pendingCommand = -1;
};

#endif
