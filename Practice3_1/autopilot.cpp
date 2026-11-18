#include "autopilot.h"

Autopilot::Autopilot(UAV* v, const std::vector<Sensor*>& s,
                     NavigationSystem* n, FlightController* c, QObject* parent)
    : QObject(parent), vehicle(v), sensors(s), navigation(n), controller(c) {}

void Autopilot::set_target_altitude(double altitude)
{
    if (altitude < 0) return;              // некорректная команда игнорируется
    target_altitude_ = altitude;
}
double Autopilot::target_altitude() const { return target_altitude_; }
double Autopilot::mission_time() const    { return mission_time_; }

void Autopilot::run_cycle()
{
    // 1. Аппарат
    if (vehicle == nullptr) {
        emit fault_detected("vehicle is not set");
        return;
    }
    // 2. Сенсоры
    for (Sensor* sensor : sensors) {
        if (sensor == nullptr || !sensor->is_ready()) {
            emit fault_detected(QString("sensor failure: ")
                                + (sensor ? sensor->name() : QString("unknown")));
            return;
        }
    }
    // 3. Навигация
    if (navigation == nullptr || !navigation->is_valid()) {
        emit fault_detected("navigation data invalid");
        return;
    }
    // 4. Позиция
    const Position position = navigation->get_position();
    if (position.altitude < 0) {
        emit fault_detected("invalid navigation position");
        return;
    }
    // 5. Регулятор
    if (controller == nullptr) {
        emit fault_detected("flight controller is not set");
        return;
    }
    const double error = target_altitude_ - position.altitude;
    const double control = controller->calculate_control(error, kDt);

    // 6. Аппарат отрабатывает команду, время миссии идёт
    vehicle->apply_control(control, kDt);
    mission_time_ += kDt;

    // 7. Факт — вверх
    emit telemetry_updated(position.altitude, error, control);
}
