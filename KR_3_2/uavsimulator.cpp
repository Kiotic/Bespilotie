#include "uavsimulator.h"
#include "mavlinkmessageparser.h"

#include <QNetworkDatagram>
#include <QHostAddress>
#include <QtMath>
#include <cmath>

UavSimulator::UavSimulator(QObject *parent)
    : QObject(parent)
{
    m_udpSocket = new QUdpSocket(this);
    if (!m_udpSocket->bind(QHostAddress::LocalHost, 14551)) {
        m_udpSocket->bind(QHostAddress::LocalHost, 0);
    }
    connect(m_udpSocket, &QUdpSocket::readyRead, this, &UavSimulator::onReadyRead);

    m_telemetryTimer = new QTimer(this);
    m_telemetryTimer->setInterval(100);
    connect(m_telemetryTimer, &QTimer::timeout, this, &UavSimulator::sendTelemetry);

    m_bootTime.start();
}

void UavSimulator::start(const QString &targetAddress, quint16 targetPort)
{
    m_targetAddress = targetAddress;
    m_targetPort = targetPort;
    m_running = true;
    m_telemetryTimer->start();
    emit runningChanged(true);
}

void UavSimulator::stop()
{
    m_telemetryTimer->stop();
    m_running = false;
    emit runningChanged(false);
}

bool UavSimulator::isRunning() const { return m_running; }
quint16 UavSimulator::localPort() const { return m_udpSocket->localPort(); }
void UavSimulator::corruptNextPacket() { m_corruptNextPacket = true; }
void UavSimulator::skipPackets(int count) { m_packetsToSkip += count; }
void UavSimulator::setAckEnabled(bool enabled) { m_ackEnabled = enabled; }
void UavSimulator::setSensorFailure(bool failed) { m_sensorFailure = failed; }

void UavSimulator::sendTelemetry()
{
    const double dt = m_telemetryTimer->interval() / 1000.0;
    updateFlight(dt);

    if (m_tick % 10 == 0) {
        HeartbeatMessage heartbeat;
        heartbeat.customMode = m_flightMode;
        if (m_sensorFailure) {
            heartbeat.systemStatus = Mavlink::STATE_CRITICAL;
        } else {
            heartbeat.systemStatus = m_altitude > 0.1 ? Mavlink::STATE_ACTIVE : Mavlink::STATE_STANDBY;
        }
        sendPacket(Mavlink::MSG_HEARTBEAT, MavlinkMessageParser::encode(heartbeat));
    }
    m_tick++;

    GlobalPositionMessage position;
    position.timeBootMs = static_cast<quint32>(m_bootTime.elapsed());
    position.latitudeE7 = static_cast<qint32>(std::lround(m_latitude * 1e7));
    position.longitudeE7 = static_cast<qint32>(std::lround(m_longitude * 1e7));
    position.altitudeMm = static_cast<qint32>(std::lround((150.0 + m_altitude) * 1000.0));
    position.relativeAltitudeMm = static_cast<qint32>(std::lround(m_altitude * 1000.0));
    position.vz = static_cast<qint16>(std::lround(-m_verticalSpeed * 100.0));
    sendPacket(Mavlink::MSG_GLOBAL_POSITION_INT, MavlinkMessageParser::encode(position));
}

void UavSimulator::updateFlight(double dt)
{
    const double desiredSpeed = qBound(-2.5, (m_targetAltitude - m_altitude) * 0.6, 4.0);
    const double maxChange = 1.5 * dt;
    m_verticalSpeed += qBound(-maxChange, desiredSpeed - m_verticalSpeed, maxChange);
    m_altitude += m_verticalSpeed * dt;

    if (m_altitude <= 0.0) {
        m_altitude = 0.0;
        if (m_verticalSpeed < 0.0) {
            m_verticalSpeed = 0.0;
        }
    }

    if (m_flightMode == Mavlink::MODE_GUIDED && std::abs(m_targetAltitude - m_altitude) < 0.3) {
        m_flightMode = Mavlink::MODE_POS_HOLD;
    }
    if (m_flightMode == Mavlink::MODE_LAND && m_altitude < 0.05) {
        m_flightMode = Mavlink::MODE_MANUAL;
        m_verticalSpeed = 0.0;
    }

    if (m_altitude > 1.0) {
        m_orbitAngle += 0.01;
        m_latitude = m_homeLatitude + 0.0004 * std::sin(m_orbitAngle);
        m_longitude = m_homeLongitude + 0.0007 * (std::cos(m_orbitAngle) - 1.0);
    }
}

void UavSimulator::sendPacket(quint32 messageId, const QByteArray &payload)
{
    if (m_packetsToSkip > 0) {
        m_packetsToSkip--;
        m_sequence++;
        return;
    }

    QByteArray datagram = MavlinkMessageParser::buildPacket(messageId, payload, m_sequence++, 1, 1);

    if (m_corruptNextPacket) {
        datagram[3] = static_cast<char>(static_cast<quint8>(datagram[3]) ^ 0xFF);
        m_corruptNextPacket = false;
    }

    m_udpSocket->writeDatagram(datagram, QHostAddress(m_targetAddress), m_targetPort);
}

void UavSimulator::onReadyRead()
{
    while (m_udpSocket->hasPendingDatagrams()) {
        const QNetworkDatagram datagram = m_udpSocket->receiveDatagram();
        if (!m_running || !datagram.isValid()) {
            continue;
        }

        MavlinkFrame frame;
        if (MavlinkMessageParser::parsePacket(datagram.data(), frame) != MavlinkMessageParser::Status::Ok) {
            continue;
        }
        if (frame.messageId == Mavlink::MSG_COMMAND_LONG) {
            handleCommand(frame.payload);
        }
    }
}

void UavSimulator::handleCommand(const QByteArray &payload)
{
    const CommandLongMessage command = MavlinkMessageParser::decodeCommandLong(payload);
    double parameter = 0.0;
    quint8 result = Mavlink::RESULT_ACCEPTED;

    if (command.command == Mavlink::CMD_NAV_TAKEOFF) {
        parameter = command.param[6];
        if (m_sensorFailure) {
            result = Mavlink::RESULT_DENIED;
        } else if (!(parameter > 0.0 && parameter <= 500.0)) {
            result = Mavlink::RESULT_DENIED;
        } else {
            m_targetAltitude = parameter;
            m_flightMode = Mavlink::MODE_GUIDED;
        }
    } else if (command.command == Mavlink::CMD_NAV_LAND) {
        m_targetAltitude = 0.0;
        m_flightMode = Mavlink::MODE_LAND;
    } else {
        result = Mavlink::RESULT_UNSUPPORTED;
    }

    if (m_ackEnabled) {
        CommandAckMessage ack;
        ack.command = command.command;
        ack.result = result;
        sendPacket(Mavlink::MSG_COMMAND_ACK, MavlinkMessageParser::encode(ack));
    }
    emit commandReceived(command.command, parameter, m_ackEnabled);
}
