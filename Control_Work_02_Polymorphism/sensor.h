#ifndef SENSOR_H
#define SENSOR_H

#include <QString>
#include <vector>

// ===== Задание 2, вариант 3. Сенсоры окружающей обстановки =====
class Sensor {
public:
    virtual double read() = 0;
    virtual bool is_ready() const = 0;
    virtual QString name() const = 0;
    // Расширение интерфейса (полиморфно, без проверки типа):
    virtual QString describe() = 0;              // "Lidar: nearest obstacle=12.5 m"
    virtual bool provides_distance() const = 0;  // измеряет ли сенсор расстояние
    virtual bool is_critical() const = 0;        // критичен ли для автопилота
    virtual ~Sensor() = default;
};

class Lidar : public Sensor {
public:
    explicit Lidar(double distance_m = 12.5);
    double read() override;
    bool is_ready() const override;
    QString name() const override;
    QString describe() override;
    bool provides_distance() const override;
    bool is_critical() const override;

    void set_ready(bool ready);
    void set_distance(double m);      // отрицательное значение отклоняется
private:
    double distance_m_;
    bool ready_ = true;
};

class Radar : public Sensor {
public:
    explicit Radar(double distance_m = 18.2);
    double read() override;
    bool is_ready() const override;
    QString name() const override;
    QString describe() override;
    bool provides_distance() const override;
    bool is_critical() const override;

    void set_ready(bool ready);
    void set_distance(double m);
private:
    double distance_m_;
    bool ready_ = true;
};

class Camera : public Sensor {
public:
    explicit Camera(int objects = 3);
    double read() override;
    bool is_ready() const override;
    QString name() const override;
    QString describe() override;
    bool provides_distance() const override;
    bool is_critical() const override;

    void set_ready(bool ready);
    void set_objects(int n);
private:
    int objects_;
    bool ready_ = true;
};

// true, если сенсор готов, измеряет расстояние и оно меньше порога
bool obstacle_is_near(Sensor& sensor, double threshold);

// Полиморфный обход набора сенсоров
void print_sensors(const std::vector<Sensor*>& sensors);

#endif // SENSOR_H
