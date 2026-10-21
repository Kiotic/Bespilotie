#include "battery.h"

Battery::Battery()
    : cell_count(4),
    voltage(0.0),
    capacity_mAh(5000.0),
    current(0.0),
    power(0.0)
{
}

Battery::Battery(int cells)
    : cell_count(cells),
    voltage(0.0),
    capacity_mAh(5000.0),
    current(0.0),
    power(0.0)
{
    if (cell_count < 1) {
        cell_count = 1;
    }
}

Battery::Battery(int cells,
                 double initial_voltage,
                 double capacity)
    : cell_count(cells),
    voltage(initial_voltage),
    capacity_mAh(capacity),
    current(0.0),
    power(0.0)
{
    if (cell_count < 1) {
        cell_count = 1;
    }

    if (voltage < 0.0) {
        voltage = 0.0;
    }

    if (capacity_mAh < 0.0) {
        capacity_mAh = 0.0;
    }
}

double Battery::get_percentage() const
{
    const double min_voltage = 3.0 * cell_count;
    const double max_voltage = 4.2 * cell_count;

    double result =
        (voltage - min_voltage) /
        (max_voltage - min_voltage) *
        100.0;

    if (result < 0.0) {
        result = 0.0;
    }

    if (result > 100.0) {
        result = 100.0;
    }

    return result;
}

bool Battery::is_low() const
{
    return get_percentage() < 20.0;
}

void Battery::set_voltage(double new_voltage)
{
    if (new_voltage < 0.0) {
        return;
    }

    voltage = new_voltage;
    power = voltage * current;
}

void Battery::set_current(double new_current)
{
    if (new_current < 0.0) {
        return;
    }

    if (new_current > 50.0) {
        return;
    }

    current = new_current;
    power = voltage * current;
}

double Battery::get_power() const
{
    return power;
}

double Battery::get_current() const
{
    return current;
}

double Battery::get_voltage() const
{
    return voltage;
}

double Battery::get_capacity() const
{
    return capacity_mAh;
}

int Battery::get_cell_count() const
{
    return cell_count;
}