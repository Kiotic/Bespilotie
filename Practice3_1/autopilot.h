#ifndef AUTOPILOT_H
#define AUTOPILOT_H

#include <QObject>
#include <QString>
#include <vector>
#include "uav.h"
#include "sensor.h"
#include "navigation.h"
#include "controller.h"

// Модель с сигналами: ничего не знает об окне.
// Команда — вниз (публичные методы), факт — вверх (сигналы).
class Autopilot : public QObject {
    Q_OBJECT
public:
    Autopilot(UAV* vehicle,
              const std::vector<Sensor*>& sensors,
              NavigationSystem* navigation,
              FlightController* controller,
              QObject* parent = nullptr);

    void set_target_altitude(double altitude);
    double target_altitude() const;
    double mission_time() const;

public slots:
    void run_cycle();

signals:
    void telemetry_updated(double altitude, double error, double control);
    void fault_detected(const QString& reason);

private:
    UAV* vehicle;
    std::vector<Sensor*> sensors;
    NavigationSystem* navigation;
    FlightController* controller;

    double target_altitude_ = 200.0;
    double mission_time_ = 0.0;
    static constexpr double kDt = 0.1;     // период цикла, с (таймер 10 Гц)
};

#endif // AUTOPILOT_H
