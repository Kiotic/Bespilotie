#include "sensor.h"
#include <QDebug>

// ---------- Lidar ----------
Lidar::Lidar(double d) : distance_m_(d < 0 ? 0 : d) {}
double Lidar::read()                 { return distance_m_; }
bool Lidar::is_ready() const         { return ready_; }
QString Lidar::name() const          { return "Lidar"; }
QString Lidar::describe()            { return QString("%1: nearest obstacle=%2 m").arg(name()).arg(read()); }
bool Lidar::provides_distance() const { return true; }
bool Lidar::is_critical() const      { return true; }
void Lidar::set_ready(bool r)        { ready_ = r; }
void Lidar::set_distance(double m) {
    if (m < 0) { qDebug().noquote() << "Error: Lidar distance cannot be negative"; return; }
    distance_m_ = m;
}

// ---------- Radar ----------
Radar::Radar(double d) : distance_m_(d < 0 ? 0 : d) {}
double Radar::read()                 { return distance_m_; }
bool Radar::is_ready() const         { return ready_; }
QString Radar::name() const          { return "Radar"; }
QString Radar::describe()            { return QString("%1: nearest object=%2 m").arg(name()).arg(read()); }
bool Radar::provides_distance() const { return true; }
bool Radar::is_critical() const      { return true; }
void Radar::set_ready(bool r)        { ready_ = r; }
void Radar::set_distance(double m) {
    if (m < 0) { qDebug().noquote() << "Error: Radar distance cannot be negative"; return; }
    distance_m_ = m;
}

// ---------- Camera ----------
Camera::Camera(int n) : objects_(n < 0 ? 0 : n) {}
double Camera::read()                { return objects_; }
bool Camera::is_ready() const        { return ready_; }
QString Camera::name() const         { return "Camera"; }
QString Camera::describe()           { return QString("%1: detected objects=%2").arg(name()).arg(objects_); }
bool Camera::provides_distance() const { return false; }  // считает объекты, а не метры
bool Camera::is_critical() const     { return false; }     // отказ камеры — предупреждение
void Camera::set_ready(bool r)       { ready_ = r; }
void Camera::set_objects(int n) {
    if (n < 0) { qDebug().noquote() << "Error: object count cannot be negative"; return; }
    objects_ = n;
}

// ---------- функции ----------
bool obstacle_is_near(Sensor& sensor, double threshold)
{
    if (!sensor.is_ready()) {
        qDebug().noquote() << "Warning:" << sensor.name() << "is not ready";
        return false;
    }
    if (!sensor.provides_distance())
        return false;                 // число объектов нельзя сравнивать с метрами
    return sensor.read() < threshold;
}

void print_sensors(const std::vector<Sensor*>& sensors)
{
    for (Sensor* s : sensors) {
        if (!s) { qDebug().noquote() << "Error: null sensor pointer"; continue; }
        if (s->is_ready())
            qDebug().noquote() << s->describe();
        else
            qDebug().noquote() << s->name() + ": NOT READY";
    }
}
