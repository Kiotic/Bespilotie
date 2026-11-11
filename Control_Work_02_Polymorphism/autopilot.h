#ifndef AUTOPILOT_H
#define AUTOPILOT_H

#include <QString>
#include <vector>
#include "uav.h"
#include "sensor.h"
#include "navigation.h"
#include "controller.h"

// ===== Интеграционное задание. Класс Autopilot =====
class Autopilot {
public:
    Autopilot(UAV* vehicle,
              const std::vector<Sensor*>& sensors,
              NavigationSystem* navigation,
              FlightController* controller);

    void run_cycle();

    // --- дополнительные методы (нужны дашборду) ---
    void set_controller(FlightController* c);
    void set_navigation(NavigationSystem* n);
    void set_target_altitude(double alt);
    double target_altitude() const;
    void set_verbose(bool v);                 // выключить qDebug при частых циклах

    bool last_ok() const;                     // успешен ли последний цикл
    QString last_reason() const;              // причина прерывания
    double last_altitude() const;
    double last_error() const;
    double last_output() const;

private:
    UAV* vehicle;
    std::vector<Sensor*> sensors;
    NavigationSystem* navigation;
    FlightController* controller;

    double target_altitude_ = 200.0;
    double dt_ = 0.1;
    bool verbose_ = true;

    bool last_ok_ = false;
    QString last_reason_;
    double last_altitude_ = 0.0, last_error_ = 0.0, last_output_ = 0.0;

    void log(const QString& s) const;
    void abort_cycle(const QString& reason);
};

#endif // AUTOPILOT_H
