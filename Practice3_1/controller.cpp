#include "controller.h"

PIDController::PIDController(double kp, double ki, double kd) : kp_(kp), ki_(ki), kd_(kd) {}

double PIDController::calculate_control(double error, double dt)
{
    if (dt <= 0.0) return 0.0;                 // защита от деления на ноль
    integral_ += error * dt;
    const double derivative = (error - previous_error_) / dt;
    const double output = kp_ * error + ki_ * integral_ + kd_ * derivative;
    previous_error_ = error;
    return output;
}
QString PIDController::controller_name() const { return "PID"; }
void PIDController::reset() { integral_ = 0.0; previous_error_ = 0.0; }

SimpleController::SimpleController(double k) : coefficient_(k) {}
double SimpleController::calculate_control(double error, double) { return coefficient_ * error; }
QString SimpleController::controller_name() const { return "Simple"; }
