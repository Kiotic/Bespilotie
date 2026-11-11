#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QString>

// ===== Задание 3, вариант 3. Абстрактные контроллеры полёта =====
class FlightController {
public:
    virtual double calculate_control(double error, double dt) = 0;
    virtual QString controller_name() const = 0;
    virtual void reset() = 0;                 // сброс внутреннего состояния
    virtual ~FlightController() = default;
};

class PIDController : public FlightController {
public:
    PIDController(double kp, double ki, double kd);
    double calculate_control(double error, double dt) override;
    QString controller_name() const override;
    void reset() override;

    double kp() const; double ki() const; double kd() const;
    void set_gains(double kp, double ki, double kd);
private:
    double kp_, ki_, kd_;
    double integral_ = 0.0;
    double previous_error_ = 0.0;
};

class SimpleController : public FlightController {
public:
    explicit SimpleController(double coefficient);
    double calculate_control(double error, double dt) override;
    QString controller_name() const override;
    void reset() override;

    double coefficient() const;
    void set_coefficient(double k);
private:
    double coefficient_;
};

// Нечёткий регулятор: 5 термов ошибки (NB, NS, Z, PS, PB),
// треугольные функции принадлежности, дефаззификация по центру тяжести.
class FuzzyController : public FlightController {
public:
    explicit FuzzyController(double error_scale = 50.0, double output_scale = 25.0);
    double calculate_control(double error, double dt) override;
    QString controller_name() const override;
    void reset() override;
private:
    double error_scale_;
    double output_scale_;
    static double triangle(double x, double a, double b, double c);
};

void test_controller(FlightController& controller, double error, double dt);

#endif // CONTROLLER_H
