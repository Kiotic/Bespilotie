#ifndef NAVIGATION_H
#define NAVIGATION_H

#include <QString>

class UAV;

struct Position {
    double latitude;
    double longitude;
    double altitude;
};

class NavigationSystem {
public:
    virtual Position get_position() const = 0;
    virtual bool is_valid() const = 0;
    virtual QString system_name() const = 0;
    virtual ~NavigationSystem() = default;
};

// GPS: если указан отслеживаемый аппарат, высота берётся из его движения
class GPSNavigation : public NavigationSystem {
public:
    explicit GPSNavigation(const UAV* tracked = nullptr, int satellites = 10, bool valid = true);
    Position get_position() const override;
    bool is_valid() const override;                 // satellites >= 4 && valid
    QString system_name() const override;
    void set_valid(bool valid);
    void set_satellites(int count);
    int satellites() const;
private:
    const UAV* tracked_;
    Position position_ = {55.75, 37.61, 150.0};
    int satellites_;
    bool valid_;
};

// Визуальная навигация: валидна, пока цель отслеживается
class VisualNavigation : public NavigationSystem {
public:
    explicit VisualNavigation(const UAV* tracked = nullptr, bool target_tracked = true);
    Position get_position() const override;
    bool is_valid() const override;
    QString system_name() const override;
    void set_target_tracked(bool tracked);
private:
    const UAV* tracked_;
    Position position_ = {55.75, 37.61, 150.0};
    bool target_tracked_;
};

#endif // NAVIGATION_H
