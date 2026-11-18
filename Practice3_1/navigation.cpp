#include "navigation.h"
#include "uav.h"

GPSNavigation::GPSNavigation(const UAV* tracked, int satellites, bool valid)
    : tracked_(tracked), satellites_(satellites < 0 ? 0 : satellites), valid_(valid) {}

Position GPSNavigation::get_position() const
{
    Position p = position_;
    if (tracked_) p.altitude = tracked_->altitude();
    return p;
}
bool GPSNavigation::is_valid() const       { return satellites_ >= 4 && valid_; }
QString GPSNavigation::system_name() const { return "GPS"; }
void GPSNavigation::set_valid(bool v)      { valid_ = v; }
void GPSNavigation::set_satellites(int n)  { satellites_ = n < 0 ? 0 : n; }
int GPSNavigation::satellites() const      { return satellites_; }

VisualNavigation::VisualNavigation(const UAV* tracked, bool target_tracked)
    : tracked_(tracked), target_tracked_(target_tracked) {}

Position VisualNavigation::get_position() const
{
    Position p = position_;
    if (tracked_) p.altitude = tracked_->altitude();
    return p;
}
bool VisualNavigation::is_valid() const        { return target_tracked_; }
QString VisualNavigation::system_name() const  { return "Visual"; }
void VisualNavigation::set_target_tracked(bool t) { target_tracked_ = t; }
