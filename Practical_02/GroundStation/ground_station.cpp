#include "ground_station.h"

GroundStation::GroundStation(
    const QString& name
    )
    : station_name(name),
    connected(false)
{
}

void GroundStation::connect_drone(
    const QString& drone
    )
{
    if (connected) {
        qDebug()
        << "Another drone is already connected";

        return;
    }

    drone_name = drone;

    connected = true;

    qDebug()
        << "Connected to drone:"
        << drone_name;
}

void GroundStation::disconnect_drone()
{
    if (!connected) {
        qDebug()
        << "No drone connected";

        return;
    }

    qDebug()
        << "Disconnected from:"
        << drone_name;

    drone_name.clear();

    connected = false;

    altitude_history.clear();
}

bool GroundStation::is_connected() const
{
    return connected;
}

void GroundStation::receive_altitude(
    double altitude
    )
{
    if (!connected) {

        qDebug()
        << "Telemetry rejected:"
        << "no connection";

        return;
    }

    if (altitude < 0.0) {

        qDebug()
        << "Telemetry rejected:"
        << "invalid altitude";

        return;
    }

    altitude_history.append(
        altitude
        );
}

double GroundStation::average_altitude() const
{
    if (altitude_history.isEmpty()) {
        return 0.0;
    }

    double sum = 0.0;

    for (double value : altitude_history) {
        sum += value;
    }

    return sum /
           altitude_history.size();
}

double GroundStation::current_altitude() const
{
    if (altitude_history.isEmpty()) {
        return 0.0;
    }

    return altitude_history.last();
}

void GroundStation::print_status() const
{
    qDebug()
    << "Station:"
    << station_name;

    qDebug()
        << "Connection:"
        << (connected
                ? "Connected"
                : "Disconnected");

    if (connected) {

        qDebug()
        << "Drone:"
        << drone_name;

        qDebug()
            << "Records:"
            << altitude_history.size();

        qDebug()
            << "Average altitude:"
            << average_altitude();
    }
}

void GroundStation::set_mission_parameter(
    const QString& name,
    double value
    )
{
    if (name == "target_altitude") {

        if (value < 0.0) {
            return;
        }
    }

    else if (name == "max_speed") {

        if (value < 0.0) {
            return;
        }
    }

    else if (name == "geofence_radius") {

        if (value <= 0.0) {
            return;
        }
    }

    else if (name == "return_battery") {

        if (value < 0.0 ||
            value > 100.0) {
            return;
        }
    }

    else if (name == "mission_timeout") {

        if (value < 0.0) {
            return;
        }
    }

    mission_parameters[name] =
        value;
}

double GroundStation::get_mission_parameter(
    const QString& name
    ) const
{
    if (!mission_parameters.contains(name)) {
        return 0.0;
    }

    return mission_parameters.value(
        name
        );
}

QString GroundStation::get_station_name() const
{
    return station_name;
}

QString GroundStation::get_drone_name() const
{
    return drone_name;
}

int GroundStation::record_count() const
{
    return altitude_history.size();
}

QVector<double>
GroundStation::get_altitude_history() const
{
    return altitude_history;
}

QDebug operator<<(
    QDebug debug,
    const GroundStation& station
    )
{
    QDebugStateSaver saver(debug);

    debug.nospace()
        << "GroundStation("
        << "name=\""
        << station.station_name
        << "\", "
        << "drone=\""
        << station.drone_name
        << "\", "
        << "connected="
        << (station.connected
                ? "true"
                : "false")
        << ", "
        << "records="
        << station.altitude_history.size()
        << ")";

    return debug;
}