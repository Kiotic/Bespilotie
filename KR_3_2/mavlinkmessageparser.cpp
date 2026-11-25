#include "mavlinkmessageparser.h"
#include <cstring>

namespace {

void putU8(QByteArray &buffer, quint8 value)
{
    buffer.append(static_cast<char>(value));
}

void putU16(QByteArray &buffer, quint16 value)
{
    putU8(buffer, value & 0xFF);
    putU8(buffer, (value >> 8) & 0xFF);
}

void putU32(QByteArray &buffer, quint32 value)
{
    putU16(buffer, value & 0xFFFF);
    putU16(buffer, (value >> 16) & 0xFFFF);
}

void putFloat(QByteArray &buffer, float value)
{
    quint32 bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    putU32(buffer, bits);
}

quint8 getU8(const QByteArray &buffer, int pos)
{
    return static_cast<quint8>(buffer.at(pos));
}

quint16 getU16(const QByteArray &buffer, int pos)
{
    return static_cast<quint16>(getU8(buffer, pos) | (getU8(buffer, pos + 1) << 8));
}

quint32 getU32(const QByteArray &buffer, int pos)
{
    return static_cast<quint32>(getU16(buffer, pos)) | (static_cast<quint32>(getU16(buffer, pos + 2)) << 16);
}

float getFloat(const QByteArray &buffer, int pos)
{
    const quint32 bits = getU32(buffer, pos);
    float value = 0;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

QByteArray padded(const QByteArray &payload, int length)
{
    QByteArray result = payload;
    if (result.size() < length) {
        result.append(QByteArray(length - result.size(), '\0'));
    }
    return result;
}

}

QByteArray MavlinkMessageParser::buildPacket(quint32 messageId, const QByteArray &payload,
                                             quint8 sequence, quint8 systemId, quint8 componentId)
{
    QByteArray packet;
    putU8(packet, Mavlink::MAGIC_V2);
    putU8(packet, static_cast<quint8>(payload.size()));
    putU8(packet, 0);
    putU8(packet, 0);
    putU8(packet, sequence);
    putU8(packet, systemId);
    putU8(packet, componentId);
    putU8(packet, messageId & 0xFF);
    putU8(packet, (messageId >> 8) & 0xFF);
    putU8(packet, (messageId >> 16) & 0xFF);
    packet.append(payload);

    const quint16 crc = crc16(packet, 1, packet.size() - 1,
                              static_cast<quint8>(crcExtra(messageId)));
    putU16(packet, crc);
    return packet;
}

MavlinkMessageParser::Status MavlinkMessageParser::parsePacket(const QByteArray &data, MavlinkFrame &frame)
{
    if (data.size() < Mavlink::HEADER_SIZE + Mavlink::CHECKSUM_SIZE) {
        return Status::TooShort;
    }
    if (getU8(data, 0) != Mavlink::MAGIC_V2) {
        return Status::BadMagic;
    }
    const int length = getU8(data, 1);
    if (data.size() != Mavlink::HEADER_SIZE + length + Mavlink::CHECKSUM_SIZE) {
        return Status::BadLength;
    }

    const quint32 messageId = getU8(data, 7) | (getU8(data, 8) << 8) | (getU8(data, 9) << 16);
    const int extra = crcExtra(messageId);
    if (extra < 0) {
        return Status::UnknownMessage;
    }

    const quint16 received = getU16(data, Mavlink::HEADER_SIZE + length);
    const quint16 calculated = crc16(data, 1, Mavlink::HEADER_SIZE - 1 + length,
                                     static_cast<quint8>(extra));
    if (received != calculated) {
        return Status::BadChecksum;
    }

    frame.sequence = getU8(data, 4);
    frame.systemId = getU8(data, 5);
    frame.componentId = getU8(data, 6);
    frame.messageId = messageId;
    frame.payload = padded(data.mid(Mavlink::HEADER_SIZE, length), payloadLength(messageId));
    return Status::Ok;
}

bool MavlinkMessageParser::isValid(const QByteArray &data)
{
    MavlinkFrame frame;
    return parsePacket(data, frame) == Status::Ok;
}

// CRC-16/MCRF4XX (X.25), как в MAVLink
quint16 MavlinkMessageParser::crc16(const QByteArray &data, int from, int count, quint8 crcExtra)
{
    quint16 crc = 0xFFFF;
    auto accumulate = [&crc](quint8 byte) {
        quint8 tmp = byte ^ static_cast<quint8>(crc & 0xFF);
        tmp ^= static_cast<quint8>(tmp << 4);
        crc = static_cast<quint16>((crc >> 8) ^ (tmp << 8) ^ (tmp << 3) ^ (tmp >> 4));
    };
    for (int i = from; i < from + count; ++i) {
        accumulate(static_cast<quint8>(data.at(i)));
    }
    accumulate(crcExtra);
    return crc;
}

int MavlinkMessageParser::crcExtra(quint32 messageId)
{
    switch (messageId) {
    case Mavlink::MSG_HEARTBEAT: return 50;
    case Mavlink::MSG_GLOBAL_POSITION_INT: return 104;
    case Mavlink::MSG_COMMAND_LONG: return 152;
    case Mavlink::MSG_COMMAND_ACK: return 143;
    default: return -1;
    }
}

int MavlinkMessageParser::payloadLength(quint32 messageId)
{
    switch (messageId) {
    case Mavlink::MSG_HEARTBEAT: return 9;
    case Mavlink::MSG_GLOBAL_POSITION_INT: return 28;
    case Mavlink::MSG_COMMAND_LONG: return 33;
    case Mavlink::MSG_COMMAND_ACK: return 3;
    default: return 0;
    }
}

QByteArray MavlinkMessageParser::encode(const HeartbeatMessage &m)
{
    QByteArray payload;
    putU32(payload, m.customMode);
    putU8(payload, m.type);
    putU8(payload, m.autopilot);
    putU8(payload, m.baseMode);
    putU8(payload, m.systemStatus);
    putU8(payload, m.mavlinkVersion);
    return payload;
}

QByteArray MavlinkMessageParser::encode(const GlobalPositionMessage &m)
{
    QByteArray payload;
    putU32(payload, m.timeBootMs);
    putU32(payload, static_cast<quint32>(m.latitudeE7));
    putU32(payload, static_cast<quint32>(m.longitudeE7));
    putU32(payload, static_cast<quint32>(m.altitudeMm));
    putU32(payload, static_cast<quint32>(m.relativeAltitudeMm));
    putU16(payload, static_cast<quint16>(m.vx));
    putU16(payload, static_cast<quint16>(m.vy));
    putU16(payload, static_cast<quint16>(m.vz));
    putU16(payload, m.heading);
    return payload;
}

QByteArray MavlinkMessageParser::encode(const CommandLongMessage &m)
{
    QByteArray payload;
    for (float p : m.param) {
        putFloat(payload, p);
    }
    putU16(payload, m.command);
    putU8(payload, m.targetSystem);
    putU8(payload, m.targetComponent);
    putU8(payload, m.confirmation);
    return payload;
}

QByteArray MavlinkMessageParser::encode(const CommandAckMessage &m)
{
    QByteArray payload;
    putU16(payload, m.command);
    putU8(payload, m.result);
    return payload;
}

HeartbeatMessage MavlinkMessageParser::decodeHeartbeat(const QByteArray &payload)
{
    const QByteArray p = padded(payload, 9);
    HeartbeatMessage m;
    m.customMode = getU32(p, 0);
    m.type = getU8(p, 4);
    m.autopilot = getU8(p, 5);
    m.baseMode = getU8(p, 6);
    m.systemStatus = getU8(p, 7);
    m.mavlinkVersion = getU8(p, 8);
    return m;
}

GlobalPositionMessage MavlinkMessageParser::decodeGlobalPosition(const QByteArray &payload)
{
    const QByteArray p = padded(payload, 28);
    GlobalPositionMessage m;
    m.timeBootMs = getU32(p, 0);
    m.latitudeE7 = static_cast<qint32>(getU32(p, 4));
    m.longitudeE7 = static_cast<qint32>(getU32(p, 8));
    m.altitudeMm = static_cast<qint32>(getU32(p, 12));
    m.relativeAltitudeMm = static_cast<qint32>(getU32(p, 16));
    m.vx = static_cast<qint16>(getU16(p, 20));
    m.vy = static_cast<qint16>(getU16(p, 22));
    m.vz = static_cast<qint16>(getU16(p, 24));
    m.heading = getU16(p, 26);
    return m;
}

CommandLongMessage MavlinkMessageParser::decodeCommandLong(const QByteArray &payload)
{
    const QByteArray p = padded(payload, 33);
    CommandLongMessage m;
    for (int i = 0; i < 7; ++i) {
        m.param[i] = getFloat(p, i * 4);
    }
    m.command = getU16(p, 28);
    m.targetSystem = getU8(p, 30);
    m.targetComponent = getU8(p, 31);
    m.confirmation = getU8(p, 32);
    return m;
}

CommandAckMessage MavlinkMessageParser::decodeCommandAck(const QByteArray &payload)
{
    const QByteArray p = padded(payload, 3);
    CommandAckMessage m;
    m.command = getU16(p, 0);
    m.result = getU8(p, 2);
    return m;
}

QString MavlinkMessageParser::statusText(Status status)
{
    switch (status) {
    case Status::Ok: return "пакет корректен";
    case Status::TooShort: return "слишком короткий пакет";
    case Status::BadMagic: return "неверный стартовый байт";
    case Status::BadLength: return "длина не совпадает с заголовком";
    case Status::BadChecksum: return "неверная контрольная сумма";
    case Status::UnknownMessage: return "неизвестный тип сообщения";
    }
    return "неизвестная ошибка";
}

QString MavlinkMessageParser::flightModeName(quint32 mode)
{
    switch (mode) {
    case Mavlink::MODE_MANUAL: return "MANUAL";
    case Mavlink::MODE_STABILIZED: return "STABILIZED";
    case Mavlink::MODE_ALT_HOLD: return "ALT_HOLD";
    case Mavlink::MODE_POS_HOLD: return "POS_HOLD";
    case Mavlink::MODE_GUIDED: return "GUIDED";
    case Mavlink::MODE_AUTO: return "AUTO";
    case Mavlink::MODE_LAND: return "LAND";
    default: return "UNKNOWN";
    }
}

QString MavlinkMessageParser::resultText(quint8 result)
{
    switch (result) {
    case Mavlink::RESULT_ACCEPTED: return "Команда принята";
    case Mavlink::RESULT_TEMPORARILY_REJECTED: return "Команда временно отклонена";
    case Mavlink::RESULT_DENIED: return "Команда отклонена";
    case Mavlink::RESULT_UNSUPPORTED: return "Команда не поддерживается";
    case Mavlink::RESULT_FAILED: return "Ошибка выполнения команды";
    case Mavlink::RESULT_IN_PROGRESS: return "Команда выполняется";
    default: return "Неизвестный результат";
    }
}

QString MavlinkMessageParser::commandName(int command)
{
    switch (command) {
    case Mavlink::CMD_NAV_TAKEOFF: return "MAV_CMD_NAV_TAKEOFF";
    case Mavlink::CMD_NAV_LAND: return "MAV_CMD_NAV_LAND";
    default: return QString("CMD %1").arg(command);
    }
}

QString MavlinkMessageParser::systemStatusName(quint8 status)
{
    switch (status) {
    case Mavlink::STATE_STANDBY: return "STANDBY";
    case Mavlink::STATE_ACTIVE: return "ACTIVE";
    case Mavlink::STATE_CRITICAL: return "CRITICAL";
    default: return QString("STATE %1").arg(status);
    }
}
