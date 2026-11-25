#ifndef MAVLINKMESSAGEPARSER_H
#define MAVLINKMESSAGEPARSER_H

#include <QByteArray>
#include <QString>
#include <QtGlobal>

namespace Mavlink {
const quint8 MAGIC_V2 = 0xFD;
const int HEADER_SIZE = 10;
const int CHECKSUM_SIZE = 2;

const quint32 MSG_HEARTBEAT = 0;
const quint32 MSG_GLOBAL_POSITION_INT = 33;
const quint32 MSG_COMMAND_LONG = 76;
const quint32 MSG_COMMAND_ACK = 77;

const quint16 CMD_NAV_LAND = 21;
const quint16 CMD_NAV_TAKEOFF = 22;

const quint8 RESULT_ACCEPTED = 0;
const quint8 RESULT_TEMPORARILY_REJECTED = 1;
const quint8 RESULT_DENIED = 2;
const quint8 RESULT_UNSUPPORTED = 3;
const quint8 RESULT_FAILED = 4;
const quint8 RESULT_IN_PROGRESS = 5;

const quint8 STATE_STANDBY = 3;
const quint8 STATE_ACTIVE = 4;
const quint8 STATE_CRITICAL = 5;

const quint32 MODE_MANUAL = 0;
const quint32 MODE_STABILIZED = 1;
const quint32 MODE_ALT_HOLD = 2;
const quint32 MODE_POS_HOLD = 3;
const quint32 MODE_GUIDED = 4;
const quint32 MODE_AUTO = 5;
const quint32 MODE_LAND = 6;
}

struct MavlinkFrame
{
    quint8 sequence = 0;
    quint8 systemId = 0;
    quint8 componentId = 0;
    quint32 messageId = 0;
    QByteArray payload;
};

struct HeartbeatMessage
{
    quint32 customMode = 0;
    quint8 type = 2;
    quint8 autopilot = 12;
    quint8 baseMode = 0;
    quint8 systemStatus = Mavlink::STATE_STANDBY;
    quint8 mavlinkVersion = 3;
};

struct GlobalPositionMessage
{
    quint32 timeBootMs = 0;
    qint32 latitudeE7 = 0;
    qint32 longitudeE7 = 0;
    qint32 altitudeMm = 0;
    qint32 relativeAltitudeMm = 0;
    qint16 vx = 0;
    qint16 vy = 0;
    qint16 vz = 0;
    quint16 heading = 0;
};

struct CommandLongMessage
{
    float param[7] = {0, 0, 0, 0, 0, 0, 0};
    quint16 command = 0;
    quint8 targetSystem = 0;
    quint8 targetComponent = 0;
    quint8 confirmation = 0;
};

struct CommandAckMessage
{
    quint16 command = 0;
    quint8 result = 0;
};

class MavlinkMessageParser
{
public:
    enum class Status { Ok, TooShort, BadMagic, BadLength, BadChecksum, UnknownMessage };

    static QByteArray buildPacket(quint32 messageId, const QByteArray &payload,
                                  quint8 sequence, quint8 systemId, quint8 componentId);
    static Status parsePacket(const QByteArray &data, MavlinkFrame &frame);
    static bool isValid(const QByteArray &data);

    static QByteArray encode(const HeartbeatMessage &message);
    static QByteArray encode(const GlobalPositionMessage &message);
    static QByteArray encode(const CommandLongMessage &message);
    static QByteArray encode(const CommandAckMessage &message);

    static HeartbeatMessage decodeHeartbeat(const QByteArray &payload);
    static GlobalPositionMessage decodeGlobalPosition(const QByteArray &payload);
    static CommandLongMessage decodeCommandLong(const QByteArray &payload);
    static CommandAckMessage decodeCommandAck(const QByteArray &payload);

    static quint16 crc16(const QByteArray &data, int from, int count, quint8 crcExtra);

    static QString statusText(Status status);
    static QString flightModeName(quint32 mode);
    static QString resultText(quint8 result);
    static QString commandName(int command);
    static QString systemStatusName(quint8 status);

private:
    static int crcExtra(quint32 messageId);
    static int payloadLength(quint32 messageId);
};

#endif
