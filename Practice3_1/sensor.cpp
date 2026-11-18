#include "sensor.h"

Gyroscope::Gyroscope(double v) : value_(v) {}
double Gyroscope::read()               { return value_; }
bool Gyroscope::is_ready() const       { return ready_; }
QString Gyroscope::name() const        { return "Gyroscope"; }
QString Gyroscope::unit() const        { return "rad/s"; }
void Gyroscope::set_ready(bool r)      { ready_ = r; }

Accelerometer::Accelerometer(double v) : value_(v) {}
double Accelerometer::read()           { return value_; }
bool Accelerometer::is_ready() const   { return ready_; }
QString Accelerometer::name() const    { return "Accelerometer"; }
QString Accelerometer::unit() const    { return "m/s²"; }
void Accelerometer::set_ready(bool r)  { ready_ = r; }

Barometer::Barometer(double v) : value_(v) {}
double Barometer::read()               { return value_; }
bool Barometer::is_ready() const       { return ready_; }
QString Barometer::name() const        { return "Barometer"; }
QString Barometer::unit() const        { return "hPa"; }
void Barometer::set_ready(bool r)      { ready_ = r; }
