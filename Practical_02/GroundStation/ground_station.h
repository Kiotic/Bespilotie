#ifndef GROUND_STATION_H
#define GROUND_STATION_H

#include <QString>
#include <QVector>
#include <QMap>
#include <QDebug>

class GroundStation
{
private:
    QString station_name;
    QString drone_name;

    bool connected;

    QVector<double> altitude_history;

    QMap<QString, double> mission_parameters;

public:
    GroundStation(
        const QString& name = "Base-1"
        );

    void connect_drone(
        const QString& drone
        );

    void disconnect_drone();

    bool is_connected() const;

    void receive_altitude(
        double altitude
        );

    double average_altitude() const;

    double current_altitude() const;

    void print_status() const;

    void set_mission_parameter(
        const QString& name,
        double value
        );

    double get_mission_parameter(
        const QString& name
        ) const;

    QString get_station_name() const;

    QString get_drone_name() const;

    int record_count() const;

    QVector<double>
    get_altitude_history() const;

    friend QDebug operator<<(
        QDebug debug,
        const GroundStation& station
        );
};

#endif