#include "navigation.h"

GPSNavigation::GPSNavigation(const Position& p, int satellites, bool valid)
    : position_(p), satellites_(satellites < 0 ? 0 : satellites), valid_(valid) {}

Position GPSNavigation::get_position() const { return position_; }
bool GPSNavigation::is_valid() const         { return satellites_ >= 4 && valid_; }
QString GPSNavigation::system_name() const   { return "GPS"; }
void GPSNavigation::set_position(const Position& p) { position_ = p; }
void GPSNavigation::set_altitude(double alt) { position_.altitude = alt; }
void GPSNavigation::set_valid(bool v)        { valid_ = v; }
void GPSNavigation::set_satellites(int n)    { satellites_ = n < 0 ? 0 : n; }
int GPSNavigation::satellites() const        { return satellites_; }

bool position_is_correct(const Position& p)
{
    return p.latitude  >= -90.0  && p.latitude  <= 90.0
        && p.longitude >= -180.0 && p.longitude <= 180.0
        && p.altitude  >= 0.0;
}
