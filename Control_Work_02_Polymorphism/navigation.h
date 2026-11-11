#ifndef NAVIGATION_H
#define NAVIGATION_H

#include <QString>

// Навигационный интерфейс (нужен для интеграционного задания)
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

class GPSNavigation : public NavigationSystem {
public:
    GPSNavigation(const Position& p = {55.75, 37.61, 150.0}, int satellites = 10, bool valid = true);
    Position get_position() const override;
    bool is_valid() const override;         // satellites >= 4 && valid
    QString system_name() const override;

    void set_position(const Position& p);
    void set_altitude(double alt);
    void set_valid(bool v);
    void set_satellites(int n);
    int satellites() const;
private:
    Position position_;
    int satellites_;
    bool valid_;
};

// Проверка корректности координат (используется автопилотом)
bool position_is_correct(const Position& p);

#endif // NAVIGATION_H
