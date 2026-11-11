#include "controller.h"
#include <QDebug>
#include <algorithm>
#include <cmath>

// ---------- PID ----------
PIDController::PIDController(double kp, double ki, double kd) : kp_(kp), ki_(ki), kd_(kd) {}

double PIDController::calculate_control(double error, double dt)
{
    if (dt <= 0.0) {                       // защита от деления на ноль
        qDebug().noquote() << "Error: dt must be positive";
        return 0.0;
    }
    integral_ += error * dt;
    const double derivative = (error - previous_error_) / dt;
    const double output = kp_ * error + ki_ * integral_ + kd_ * derivative;
    previous_error_ = error;
    return output;
}
QString PIDController::controller_name() const { return "PID"; }
void PIDController::reset() { integral_ = 0.0; previous_error_ = 0.0; }
double PIDController::kp() const { return kp_; }
double PIDController::ki() const { return ki_; }
double PIDController::kd() const { return kd_; }
void PIDController::set_gains(double kp, double ki, double kd) { kp_ = kp; ki_ = ki; kd_ = kd; }

// ---------- Simple ----------
SimpleController::SimpleController(double k) : coefficient_(k) {}
double SimpleController::calculate_control(double error, double) { return coefficient_ * error; }
QString SimpleController::controller_name() const { return "Simple"; }
void SimpleController::reset() {}
double SimpleController::coefficient() const { return coefficient_; }
void SimpleController::set_coefficient(double k) { coefficient_ = k; }

// ---------- Fuzzy ----------
FuzzyController::FuzzyController(double es, double os)
    : error_scale_(es > 0 ? es : 1.0), output_scale_(os) {}

double FuzzyController::triangle(double x, double a, double b, double c)
{
    if (x <= a || x >= c) return (x == b) ? 1.0 : 0.0;
    return x < b ? (x - a) / (b - a) : (c - x) / (c - b);
}

double FuzzyController::calculate_control(double error, double)
{
    // нормированная ошибка в [-1, 1]
    const double e = std::max(-1.0, std::min(1.0, error / error_scale_));
    // центры термов и соответствующие выходы правил
    const double centers[5] = {-1.0, -0.5, 0.0, 0.5, 1.0};
    const double outputs[5] = {-1.0, -0.6, 0.0, 0.6, 1.0};
    double num = 0.0, den = 0.0;
    for (int i = 0; i < 5; ++i) {
        double mu = triangle(e, centers[i] - 0.5, centers[i], centers[i] + 0.5);
        if (i == 0 && e <= -1.0) mu = 1.0;  // насыщение крайних термов
        if (i == 4 && e >=  1.0) mu = 1.0;
        num += mu * outputs[i];
        den += mu;
    }
    return den > 0.0 ? output_scale_ * num / den : 0.0;
}
QString FuzzyController::controller_name() const { return "Fuzzy"; }
void FuzzyController::reset() {}

// ---------- универсальный тест ----------
void test_controller(FlightController& controller, double error, double dt)
{
    const double out = controller.calculate_control(error, dt);
    qDebug().noquote() << "Controller:" << controller.controller_name();
    qDebug().noquote() << "Error:" << QString::number(error);
    qDebug().noquote() << "Control output:" << QString::number(out, 'f', 2);
}
