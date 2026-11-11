#ifndef SIMULATION_H
#define SIMULATION_H

#include <vector>
#include <utility>
#include "controller.h"

// Упрощённая вертикальная динамика БПЛА: a = sat(u) - drag * v
struct VehicleDynamics {
    static constexpr double drag = 0.6;      // аэродинамическое демпфирование, 1/с
    static constexpr double max_cmd = 15.0;  // ограничение команды (~1.5g), м/с²
    double disturbance = -0.8;               // постоянное возмущение (ветер/вес), м/с²
    double altitude = 150.0;
    double vspeed = 0.0;
    void step(double u, double dt);
    static double saturate(double u);
};

// Переходный процесс: регулятор передаётся по ссылке на абстрактный класс
std::vector<std::pair<double, double>>
simulate_step(FlightController& controller, double start_alt, double target_alt,
              double duration_s, double control_dt = 0.1);

struct StepMetrics {
    double overshoot_pct;   // перерегулирование, %
    double settling_s;      // время входа в трубку ±2 м
    double final_error;     // установившаяся ошибка, м
};
StepMetrics analyze_step(const std::vector<std::pair<double, double>>& trace,
                         double start_alt, double target_alt);

#endif // SIMULATION_H
