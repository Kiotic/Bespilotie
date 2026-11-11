#ifndef UAV_H
#define UAV_H

#include <QString>

// ===== Задание 1, вариант 3. Абстрактный базовый класс БПЛА =====
class UAV {
public:
    virtual void takeoff() = 0;
    virtual void land() = 0;
    virtual QString type() const = 0;
    virtual QString mission_type() const = 0;   // добавлено по варианту 3
    virtual ~UAV() = default;
};

class SurveyUAV : public UAV {
public:
    void takeoff() override;
    void land() override;
    QString type() const override;
    QString mission_type() const override;
};

class CargoUAV : public UAV {
public:
    explicit CargoUAV(double payload_kg = 5.0);
    void takeoff() override;
    void land() override;
    QString type() const override;
    QString mission_type() const override;

    double payload() const;
    void set_payload(double kg);        // отрицательная масса отклоняется
private:
    double payload_kg_;
};

class ReconnaissanceUAV : public UAV {
public:
    void takeoff() override;
    void land() override;
    QString type() const override;
    QString mission_type() const override;
};

// Работает с любым наследником UAV без if/dynamic_cast
void execute_test_mission(UAV& vehicle);

#endif // UAV_H
