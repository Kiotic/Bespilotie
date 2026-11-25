#ifndef SENSOR_H
#define SENSOR_H

#include <QString>

// Абстрактный сенсор (вариант 1: сенсоры полётного контроллера)
class Sensor {
public:
    virtual double read() = 0;
    virtual bool is_ready() const = 0;
    virtual QString name() const = 0;
    virtual QString unit() const = 0;
    virtual ~Sensor() = default;
};

class Gyroscope : public Sensor {
public:
    explicit Gyroscope(double value = 1.25);
    double read() override;
    bool is_ready() const override;
    QString name() const override;
    QString unit() const override;
    void set_ready(bool ready);
private:
    double value_;
    bool ready_ = true;
};

class Accelerometer : public Sensor {
public:
    explicit Accelerometer(double value = 9.81);
    double read() override;
    bool is_ready() const override;
    QString name() const override;
    QString unit() const override;
    void set_ready(bool ready);
private:
    double value_;
    bool ready_ = true;
};

class Barometer : public Sensor {
public:
    explicit Barometer(double value = 1012.8);
    double read() override;
    bool is_ready() const override;
    QString name() const override;
    QString unit() const override;
    void set_ready(bool ready);
private:
    double value_;
    bool ready_ = true;
};

#endif // SENSOR_H
