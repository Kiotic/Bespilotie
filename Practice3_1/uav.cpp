#include "uav.h"
#include <algorithm>
#include <cmath>

UAV::UAV(const QString& callsign, double start_altitude, double battery)
    : callsign_(callsign),
      altitude_(start_altitude < 0 ? 0 : start_altitude),
      battery_(std::clamp(battery, 0.0, 100.0)) {}

int UAV::motor_count() const          { return -1; }
QString UAV::callsign() const         { return callsign_; }
double UAV::altitude() const          { return altitude_; }
double UAV::vertical_speed() const    { return vertical_speed_; }
double UAV::battery() const           { return battery_; }
bool UAV::in_air() const              { return in_air_; }
void UAV::set_in_air(bool value)      { in_air_ = value; }

void UAV::apply_control(double control, double dt)
{
    if (dt <= 0) return;
    const double u = std::clamp(control, -kMaxControl, kMaxControl);
    // a = u − c·v  (гравитация считается скомпенсированной тягой висения)
    vertical_speed_ += (u - kDrag * vertical_speed_) * dt;
    altitude_ += vertical_speed_ * dt;
    if (altitude_ < 0) { altitude_ = 0; vertical_speed_ = 0; }
    // расход батареи: висение + доля от тяги
    battery_ = std::max(0.0, battery_ - (0.02 + 0.002 * std::abs(u)) * dt);
}

// ---------- Quadcopter ----------
Quadcopter::Quadcopter(const QString& c, double a, double b) : UAV(c, a, b) {}
void Quadcopter::takeoff()            { set_in_air(true); }   // вертикальный взлёт
void Quadcopter::land()               { set_in_air(false); }  // вертикальная посадка
QString Quadcopter::type() const      { return "Quadcopter"; }
int Quadcopter::motor_count() const   { return 4; }

// ---------- FixedWingUAV ----------
FixedWingUAV::FixedWingUAV(const QString& c, double a, double b) : UAV(c, a, b) {}
void FixedWingUAV::takeoff()          { set_in_air(true); }   // взлёт с полосы
void FixedWingUAV::land()             { set_in_air(false); }  // посадка планированием
QString FixedWingUAV::type() const    { return "Fixed-wing UAV"; }
