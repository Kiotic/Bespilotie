#ifndef WIDGETS_H
#define WIDGETS_H

#include <QWidget>
#include <QVector>
#include <QPointF>
#include <QColor>
#include <QString>

class QPainter;
class QMouseEvent;

// Цветовая схема дашборда
namespace Theme {
inline QColor bg()      { return QColor(11, 15, 25); }
inline QColor panel()   { return QColor(20, 27, 42); }
inline QColor panel2()  { return QColor(28, 37, 57); }
inline QColor grid()    { return QColor(255, 255, 255, 22); }
inline QColor text()    { return QColor(226, 232, 245); }
inline QColor muted()   { return QColor(132, 146, 175); }
inline QColor cyan()    { return QColor(58, 208, 255); }
inline QColor green()   { return QColor(62, 224, 137); }
inline QColor amber()   { return QColor(255, 182, 72); }
inline QColor red()     { return QColor(255, 92, 108); }
inline QColor violet()  { return QColor(177, 140, 255); }
}

// ---------- Универсальный график телеметрии (QPainter, без QtCharts) ----------
class TelemetryChart : public QWidget {
    Q_OBJECT
public:
    explicit TelemetryChart(const QString& title, const QString& unit, QWidget* parent = nullptr);

    int  add_series(const QString& name, const QColor& color, bool fill = false, bool dashed = false);
    void add_point(int series, double t, double v);
    void clear();
    void set_window(double seconds);               // «бегущее» окно по X
    void set_fixed_x(double x0, double x1);        // фиксированная ось X
    void set_y_range(double lo, double hi);        // фиксированная ось Y
    void set_include_zero(bool on);

protected:
    void paintEvent(QPaintEvent*) override;

private:
    struct Series { QString name; QColor color; bool fill; bool dashed; QVector<QPointF> pts; };
    QString title_, unit_;
    QVector<Series> series_;
    double window_ = 30.0;
    bool fixed_x_ = false; double fx0_ = 0, fx1_ = 1;
    bool fixed_y_ = false; double fy0_ = 0, fy1_ = 1;
    bool include_zero_ = false;
    static double nice_step(double raw);
};

// ---------- Вид полёта сбоку: небо, цель, анимированный дрон, след ----------
class FlightView : public QWidget {
    Q_OBJECT
public:
    explicit FlightView(QWidget* parent = nullptr);
    void set_state(double alt, double vspeed, double target, double control, bool ok,
                   const QString& status, const QString& vehicle);
    void push_trail(double t, double alt);
    void clear_trail();
    void advance(double dt, bool motors_on);

protected:
    void paintEvent(QPaintEvent*) override;

private:
    double alt_ = 150, vs_ = 0, target_ = 200, control_ = 0;
    bool ok_ = true;
    QString status_, vehicle_;
    double phase_ = 0, blink_ = 0, clouds_ = 0, now_ = 0;
    QVector<QPointF> trail_;
    void draw_drone(QPainter& p, const QPointF& c);
};

// ---------- Радар окружающей обстановки (Lidar / Radar / Camera) ----------
class RadarView : public QWidget {
    Q_OBJECT
public:
    struct Blip {
        QString label;
        double angle_deg;
        double distance;
        bool ready;
        int level;          // 0 — свободно, 1 — ближе 20 м, 2 — ближе 10 м
    };
    explicit RadarView(QWidget* parent = nullptr);
    void set_blips(const QVector<Blip>& blips);
    void set_camera_info(const QString& s, bool ready);
    void advance(double dt);

protected:
    void paintEvent(QPaintEvent*) override;

private:
    QVector<Blip> blips_;
    QString camera_;
    bool camera_ready_ = true;
    double sweep_ = 90.0;
    const double range_ = 30.0;
};

// ---------- Карточка сенсора (клик — отказ/восстановление) ----------
class SensorCard : public QWidget {
    Q_OBJECT
public:
    explicit SensorCard(QWidget* parent = nullptr);
    void set_data(const QString& name, const QString& value, bool ready, bool critical,
                  int level, const QString& sub);
signals:
    void clicked();
protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
private:
    QString name_, value_, sub_;
    bool ready_ = true, critical_ = false;
    int level_ = 0;
};

#endif // WIDGETS_H
