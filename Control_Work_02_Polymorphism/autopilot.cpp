#include "autopilot.h"
#include <QDebug>

Autopilot::Autopilot(UAV* v, const std::vector<Sensor*>& s,
                     NavigationSystem* n, FlightController* c)
    : vehicle(v), sensors(s), navigation(n), controller(c) {}

void Autopilot::log(const QString& s) const { if (verbose_) qDebug().noquote() << s; }

void Autopilot::abort_cycle(const QString& reason)
{
    last_ok_ = false;
    last_reason_ = reason;
    last_output_ = 0.0;
    log("Autopilot cycle aborted");
    log("Reason: " + reason);
}

void Autopilot::run_cycle()
{
    log("Autopilot cycle started");

    // 1. БПЛА
    if (vehicle == nullptr) {
        log("Error: vehicle is not set");
        abort_cycle("vehicle is not set");
        return;
    }

    // 2. Сенсоры
    bool critical_failure = false;
    for (Sensor* s : sensors) {
        if (s == nullptr) {
            log("Error: null sensor pointer");
            critical_failure = true;
            continue;
        }
        if (!s->is_ready()) {
            log(s->name() + (s->is_critical() ? ": NOT READY (critical)" : ": NOT READY (non-critical, warning)"));
            if (s->is_critical()) critical_failure = true;
            continue;
        }
        const double value = s->read();       // значение снимается для контроля
        Q_UNUSED(value);
    }
    if (sensors.empty() || critical_failure) {
        abort_cycle("sensor failure");
        return;
    }
    log("All sensors are ready");

    // 3. Навигация
    if (navigation == nullptr) {
        log("Error: navigation system is not set");
        abort_cycle("navigation is not set");
        return;
    }
    if (!navigation->is_valid()) {
        log("Navigation data invalid");
        abort_cycle("navigation data invalid");
        return;
    }
    log("Navigation: " + navigation->system_name());

    // 4. Позиция
    const Position p = navigation->get_position();
    if (!position_is_correct(p)) {
        log("Error: invalid navigation position");
        abort_cycle("invalid navigation position");
        return;
    }

    // 5. Ошибка по высоте
    const double error = target_altitude_ - p.altitude;
    log(QString("Current altitude: %1 m").arg(p.altitude));
    log(QString("Target altitude: %1 m").arg(target_altitude_));
    log(QString("Altitude error: %1 m").arg(error));

    // 6. Управление
    if (controller == nullptr) {
        log("Error: flight controller is not set");
        last_altitude_ = p.altitude;
        last_error_ = error;
        abort_cycle("flight controller is not set");
        return;
    }
    const double output = controller->calculate_control(error, dt_);

    // 7. Результат
    log(QString("Controller output: %1").arg(output));
    log("Autopilot cycle completed");

    last_ok_ = true;
    last_reason_.clear();
    last_altitude_ = p.altitude;
    last_error_ = error;
    last_output_ = output;
}

void Autopilot::set_controller(FlightController* c) { controller = c; }
void Autopilot::set_navigation(NavigationSystem* n) { navigation = n; }
void Autopilot::set_target_altitude(double a)
{
    if (a < 0) { log("Error: target altitude cannot be negative"); return; }
    target_altitude_ = a;
}
double Autopilot::target_altitude() const { return target_altitude_; }
void Autopilot::set_verbose(bool v)       { verbose_ = v; }
bool Autopilot::last_ok() const           { return last_ok_; }
QString Autopilot::last_reason() const    { return last_reason_; }
double Autopilot::last_altitude() const   { return last_altitude_; }
double Autopilot::last_error() const      { return last_error_; }
double Autopilot::last_output() const     { return last_output_; }
