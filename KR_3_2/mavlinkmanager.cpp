#include "mavlinkmanager.h"
#include "telemetrymodel.h"

#include <QNetworkDatagram>
#include <cmath>

MavlinkManager::MavlinkManager(TelemetryModel *telemetry, QObject *parent)
    : QObject(parent),
      m_telemetry(telemetry)
{
    m_linkTimer = new QTimer(this);
    m_linkTimer->setInterval(1000);
    connect(m_linkTimer, &QTimer::timeout, this, &MavlinkManager::checkLinkTimeout);

    m_ackTimer = new QTimer(this);
    m_ackTimer->setSingleShot(true);
    m_ackTimer->setInterval(kAckTimeoutMs);
    connect(m_ackTimer, &QTimer::timeout, this, &MavlinkManager::onAckTimeout);
}

bool MavlinkManager::start(quint16 listenPort)
{
    if (m_udpSocket) {
        m_udpSocket->close();
        m_udpSocket->deleteLater();
        m_udpSocket = nullptr;
    }

    m_udpSocket = new QUdpSocket(this);
    if (!m_udpSocket->bind(QHostAddress::AnyIPv4, listenPort)) {
        emit errorOccurred(QString("Не удалось открыть UDP-порт %1: %2")
                               .arg(listenPort)
                               .arg(m_udpSocket->errorString()));
        m_udpSocket->deleteLater();
        m_udpSocket = nullptr;
        return false;
    }

    connect(m_udpSocket, &QUdpSocket::readyRead, this, &MavlinkManager::onReadyRead);

    m_lastSequence = -1;
    m_packetsSinceCheck = 0;
    m_linkConnected = false;
    m_linkTimer->start();
    emit linkStateChanged(false, QString(), 0);
    emit linkQualityChanged(0.0, QualityBad);
    return true;
}

void MavlinkManager::stop()
{
    m_linkTimer->stop();
    m_ackTimer->stop();
    m_pendingCommand = -1;

    if (m_udpSocket) {
        m_udpSocket->close();
        m_udpSocket->deleteLater();
        m_udpSocket = nullptr;
    }

    m_linkConnected = false;
    m_packetsPerSecond = 0.0;
    emit linkStateChanged(false, QString(), 0);
    emit linkQualityChanged(0.0, QualityBad);
}

bool MavlinkManager::isListening() const
{
    return m_udpSocket != nullptr;
}

void MavlinkManager::setTarget(const QString &address, quint16 port)
{
    m_targetAddress = QHostAddress(address);
    m_targetPort = port;
}

quint64 MavlinkManager::receivedPackets() const { return m_receivedPackets; }
quint64 MavlinkManager::lostPackets() const { return m_lostPackets; }
quint64 MavlinkManager::errorPackets() const { return m_errorPackets; }
double MavlinkManager::packetsPerSecond() const { return m_packetsPerSecond; }
QDateTime MavlinkManager::lastPacketTime() const { return m_lastPacketTime; }
bool MavlinkManager::isWaitingAck() const { return m_pendingCommand >= 0; }

int MavlinkManager::ackRemainingMs() const
{
    return m_ackTimer->isActive() ? m_ackTimer->remainingTime() : 0;
}

void MavlinkManager::onReadyRead()
{
    while (m_udpSocket && m_udpSocket->hasPendingDatagrams()) {
        const QNetworkDatagram datagram = m_udpSocket->receiveDatagram();
        if (!datagram.isValid()) {
            continue;
        }
        if (!processDatagram(datagram.data())) {
            continue;
        }

        m_receivedPackets++;
        m_packetsSinceCheck++;
        m_lastPacketTime = QDateTime::currentDateTime();
        if (!m_firstPacketTime.isValid()) {
            m_firstPacketTime = m_lastPacketTime;
        }

        const QString address = datagram.senderAddress().toString();
        const quint16 port = static_cast<quint16>(datagram.senderPort());
        m_targetAddress = datagram.senderAddress();
        m_targetPort = port;

        if (!m_linkConnected || address != m_lastAddress || port != m_lastPort) {
            m_linkConnected = true;
            m_lastAddress = address;
            m_lastPort = port;
            emit linkStateChanged(true, m_lastAddress, m_lastPort);
        }
        emit packetReceived(m_receivedPackets, m_lostPackets);
    }
}

bool MavlinkManager::processDatagram(const QByteArray &data)
{
    MavlinkFrame frame;
    const MavlinkMessageParser::Status status = MavlinkMessageParser::parsePacket(data, frame);
    if (status != MavlinkMessageParser::Status::Ok) {
        m_errorPackets++;
        emit packetError(m_errorPackets, MavlinkMessageParser::statusText(status));
        return false;
    }

    if (m_lastSequence >= 0) {
        const qint64 expected = (m_lastSequence + 1) % 256;
        if (frame.sequence != expected) {
            const qint64 lost = (frame.sequence - expected + 256) % 256;
            if (lost > 0 && lost < 100) {
                m_lostPackets += static_cast<quint64>(lost);
            }
        }
    }
    m_lastSequence = frame.sequence;

    switch (frame.messageId) {
    case Mavlink::MSG_HEARTBEAT:
        handleHeartbeat(frame);
        break;
    case Mavlink::MSG_GLOBAL_POSITION_INT:
        handleGlobalPosition(frame);
        break;
    case Mavlink::MSG_COMMAND_ACK:
        handleCommandAck(frame);
        break;
    default:
        break;
    }
    return true;
}

void MavlinkManager::handleHeartbeat(const MavlinkFrame &frame)
{
    const HeartbeatMessage heartbeat = MavlinkMessageParser::decodeHeartbeat(frame.payload);
    m_targetSystemId = frame.systemId;
    m_targetComponentId = frame.componentId;
    m_flightMode = MavlinkMessageParser::flightModeName(heartbeat.customMode);

    if (heartbeat.systemStatus != m_systemStatus) {
        m_systemStatus = heartbeat.systemStatus;
        const bool healthy = heartbeat.systemStatus < Mavlink::STATE_CRITICAL;
        emit uavStatusChanged(healthy, MavlinkMessageParser::systemStatusName(heartbeat.systemStatus));
    }
}

void MavlinkManager::handleGlobalPosition(const MavlinkFrame &frame)
{
    const GlobalPositionMessage position = MavlinkMessageParser::decodeGlobalPosition(frame.payload);

    const double latitude = position.latitudeE7 / 1e7;
    const double longitude = position.longitudeE7 / 1e7;
    const double altitude = position.relativeAltitudeMm / 1000.0;
    const double verticalSpeed = -position.vz / 100.0;

    emit telemetryReceived(latitude, longitude, altitude, verticalSpeed, m_flightMode);
}

void MavlinkManager::handleCommandAck(const MavlinkFrame &frame)
{
    const CommandAckMessage ack = MavlinkMessageParser::decodeCommandAck(frame.payload);
    const bool accepted = ack.result == Mavlink::RESULT_ACCEPTED
                       || ack.result == Mavlink::RESULT_IN_PROGRESS;
    QString text = MavlinkMessageParser::resultText(ack.result);

    if (m_pendingCommand == ack.command) {
        m_ackTimer->stop();
        m_pendingCommand = -1;
    } else {
        text += " (подтверждение пришло после тайм-аута)";
    }
    emit commandResult(ack.command, accepted, text);
}

void MavlinkManager::checkLinkTimeout()
{
    const double seconds = m_linkTimer->interval() / 1000.0;
    m_packetsPerSecond = m_packetsSinceCheck / seconds;
    m_packetsSinceCheck = 0;

    const qint64 silenceMs = m_lastPacketTime.isValid()
            ? m_lastPacketTime.msecsTo(QDateTime::currentDateTime())
            : -1;

    LinkQuality quality = QualityBad;
    if (m_linkConnected && silenceMs <= kLinkTimeoutMs) {
        if (m_packetsPerSecond > 5.0) {
            quality = QualityGood;
        } else if (m_packetsPerSecond >= 1.0) {
            quality = QualityWeak;
        }
    }
    emit linkQualityChanged(m_packetsPerSecond, quality);

    if (m_linkConnected && silenceMs > kLinkTimeoutMs) {
        m_linkConnected = false;
        emit linkStateChanged(false, m_lastAddress, m_lastPort);
        emit linkLost();
    }
}

void MavlinkManager::sendTakeoff(double targetAltitude)
{
    CommandLongMessage command;
    command.command = Mavlink::CMD_NAV_TAKEOFF;
    command.param[3] = NAN;
    command.param[4] = NAN;
    command.param[5] = NAN;
    command.param[6] = static_cast<float>(targetAltitude);
    sendCommand(command, targetAltitude);
}

void MavlinkManager::sendLand()
{
    CommandLongMessage command;
    command.command = Mavlink::CMD_NAV_LAND;
    command.param[3] = NAN;
    command.param[4] = NAN;
    command.param[5] = NAN;
    sendCommand(command, 0.0);
}

void MavlinkManager::sendCommand(const CommandLongMessage &command, double parameter)
{
    if (!m_udpSocket) {
        emit errorOccurred("Канал не открыт");
        return;
    }
    if (m_targetAddress.isNull() || m_targetPort == 0) {
        emit errorOccurred("Адрес БПЛА не задан");
        return;
    }
    if (!m_telemetry || !m_telemetry->isLinkConnected()) {
        emit errorOccurred("Невозможно отправить команду: нет связи");
        return;
    }
    if (m_pendingCommand >= 0) {
        emit errorOccurred("Предыдущая команда ещё ждёт подтверждения");
        return;
    }

    CommandLongMessage message = command;
    message.targetSystem = m_targetSystemId;
    message.targetComponent = m_targetComponentId;

    const QByteArray datagram = MavlinkMessageParser::buildPacket(
        Mavlink::MSG_COMMAND_LONG, MavlinkMessageParser::encode(message),
        m_sequence++, kSystemId, kComponentId);

    const qint64 sent = m_udpSocket->writeDatagram(datagram, m_targetAddress, m_targetPort);
    if (sent == -1) {
        emit errorOccurred("Ошибка отправки команды: " + m_udpSocket->errorString());
        return;
    }

    m_pendingCommand = message.command;
    m_ackTimer->start();
    emit commandSent(message.command, parameter);
}

void MavlinkManager::onAckTimeout()
{
    const int command = m_pendingCommand;
    m_pendingCommand = -1;
    emit commandResult(command, false, "Подтверждение не получено");
}
