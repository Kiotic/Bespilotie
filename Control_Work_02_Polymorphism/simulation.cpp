#include "simulation.h"
#include <algorithm>
#include <cmath>

double VehicleDynamics::saturate(double u)
{
    return std::max(-max_cmd, std::min(max_cmd, u));
}

void VehicleDynamics::step(double u, double dt)
{
    vspeed += (saturate(u) + disturbance - drag * vspeed) * dt;
    altitude += vspeed * dt;
    if (altitude < 0.0) { altitude = 0.0; if (vspeed < 0) vspeed = 0.0; }
}

std::vector<std::pair<double, double>>
simulate_step(FlightController& controller, double start_alt, double target_alt,
              double duration_s, double control_dt)
{
    controller.reset();
    VehicleDynamics v;
    v.altitude = start_alt;
    const double phys_dt = 0.01;
    const int sub = std::max(1, int(std::lround(control_dt / phys_dt)));
    std::vector<std::pair<double, double>> trace;
    double t = 0.0, u = 0.0;
    for (int k = 0; t <= duration_s + 1e-9; ++k) {
        if (k % sub == 0)
            u = controller.calculate_control(target_alt - v.altitude, control_dt);
        if (k % 5 == 0) trace.push_back({t, v.altitude});
        v.step(u, phys_dt);
        t += phys_dt;
    }
    return trace;
}

StepMetrics analyze_step(const std::vector<std::pair<double, double>>& tr,
                         double start_alt, double target_alt)
{
    StepMetrics m{0.0, 0.0, 0.0};
    if (tr.empty()) return m;
    const double span = std::fabs(target_alt - start_alt);
    const double dir = target_alt >= start_alt ? 1.0 : -1.0;
    double peak = 0.0;
    for (const auto& p : tr) peak = std::max(peak, dir * (p.second - target_alt));
    m.overshoot_pct = span > 0 ? peak / span * 100.0 : 0.0;
    m.settling_s = 0.0;
    for (const auto& p : tr)
        if (std::fabs(p.second - target_alt) > 2.0) m.settling_s = p.first;
    m.final_error = target_alt - tr.back().second;
    return m;
}
