#ifndef BATTERY_H
#define BATTERY_H

class Battery
{
private:
    int cell_count;
    double voltage;
    double capacity_mAh;
    double current;
    double power;

public:
    Battery();

    Battery(int cells);

    Battery(int cells,
            double initial_voltage,
            double capacity);

    double get_percentage() const;
    bool is_low() const;

    void set_voltage(double new_voltage);
    void set_current(double new_current);

    double get_power() const;
    double get_current() const;
    double get_voltage() const;
    double get_capacity() const;
    int get_cell_count() const;
};

#endif