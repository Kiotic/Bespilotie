#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QString>

class FlightController {
public:
    virtual double calculate_control(double error, double dt) = 0;
    virtual QString controller_name() const = 0;
    virtual ~FlightController() = default;
};

class PIDController : public FlightController {
public:
    PIDController(double kp, double ki, double kd);
    double calculate_control(double error, double dt) override;
    QString controller_name() const override;
    void reset();
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
private:
    double coefficient_;
};

#endif // CONTROLLER_H
