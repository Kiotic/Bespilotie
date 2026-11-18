#ifndef UAV_H
#define UAV_H

#include <QString>

// Абстрактный летательный аппарат (вариант 1: Quadcopter, FixedWingUAV).
// Кроме интерфейса, базовый класс хранит упрощённое вертикальное движение:
// высоту, скорость и заряд батареи. Все поля закрыты, доступ — через методы.
class UAV {
public:
    explicit UAV(const QString& callsign, double start_altitude = 150.0, double battery = 100.0);
    virtual ~UAV() = default;

    virtual void takeoff() = 0;
    virtual void land() = 0;
    virtual QString type() const = 0;
    // число моторов; −1 — «нет данных» для аппаратов, где метод не переопределён
    virtual int motor_count() const;

    QString callsign() const;
    double altitude() const;
    double vertical_speed() const;
    double battery() const;
    bool in_air() const;

    // Применить управляющее воздействие (ускорение, м/с²) на интервале dt
    void apply_control(double control, double dt);

protected:
    void set_in_air(bool value);

private:
    QString callsign_;
    double altitude_;
    double vertical_speed_ = 0.0;
    double battery_;
    bool in_air_ = true;

    static constexpr double kDrag = 1.0;        // аэродинамическое демпфирование
    static constexpr double kMaxControl = 20.0; // ограничение тяги, м/с²
};

class Quadcopter : public UAV {
public:
    explicit Quadcopter(const QString& callsign, double start_altitude = 150.0, double battery = 100.0);
    void takeoff() override;
    void land() override;
    QString type() const override;
    int motor_count() const override;
};

// У самолётной схемы motor_count() не переопределён — базовый вернёт −1
class FixedWingUAV : public UAV {
public:
    explicit FixedWingUAV(const QString& callsign, double start_altitude = 150.0, double battery = 100.0);
    void takeoff() override;
    void land() override;
    QString type() const override;
};

#endif // UAV_H
