#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QColor>
#include <vector>

#include "uav.h"
#include "sensor.h"
#include "navigation.h"
#include "controller.h"
#include "autopilot.h"

class QTimer;
class QGraphicsScene;
class QGraphicsRectItem;
class QGraphicsPathItem;
class QGraphicsLineItem;
class QGraphicsSimpleTextItem;
class SoundEngine;
class FallbackChart;

#ifdef HAVE_QT_CHARTS
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
QT_CHARTS_USE_NAMESPACE
#endif
#endif

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// Представление: только отображение и команды оператора.
// Ни формул ПИД, ни проверок навигации здесь нет — всё в модели.
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    // команды оператора (команда — вниз)
    void start_mission();
    void pause_mission();
    void reset_mission();
    void inject_sensor_fault();
    void inject_gps_fault();
    void change_target(double value);

    // приёмники одного сигнала telemetry_updated (один сигнал — много слотов)
    void show_telemetry(double altitude, double error, double control);
    void update_chart(double altitude, double error, double control);
    void update_scene(double altitude, double error, double control);
    void update_fleet(double altitude, double error, double control);

    // приёмник сигнала fault_detected
    void show_fault(const QString& reason);

private:
    Ui::MainWindow* ui = nullptr;

    // ---- модель ----
    Quadcopter* vehicle = nullptr;
    std::vector<UAV*> fleet;
    Gyroscope* gyro = nullptr;
    Accelerometer* accelerometer = nullptr;
    Barometer* barometer = nullptr;
    GPSNavigation* navigation = nullptr;
    PIDController* controller = nullptr;
    Autopilot* autopilot = nullptr;

    // ---- представление ----
    QTimer* cycle_timer = nullptr;
    SoundEngine* sound = nullptr;
    bool first_fault_reported = false;   // флаг «первой аварии» для QMessageBox
    bool fault_active = false;

#ifdef HAVE_QT_CHARTS
    QChart* chart = nullptr;
    QLineSeries* series_altitude = nullptr;
    QLineSeries* series_target = nullptr;
    QLineSeries* series_control = nullptr;
    QValueAxis* axis_x = nullptr;
    QValueAxis* axis_y = nullptr;
    QValueAxis* axis_control = nullptr;
#else
    FallbackChart* fallback_chart = nullptr;
#endif

    QGraphicsScene* scene = nullptr;
    QGraphicsRectItem* marker = nullptr;
    QGraphicsPathItem* trajectory = nullptr;
    QGraphicsLineItem* target_line = nullptr;
    QGraphicsSimpleTextItem* target_text = nullptr;
    QGraphicsSimpleTextItem* marker_text = nullptr;

    void setup_chart();
    void setup_scene();
    void fill_fleet_table();
    void set_status(const QString& text, const QString& background);
    void set_fleet_status(const QString& text, const QColor& color);
    void log_event(const QString& text, const QColor& color);
    void refresh_sensor_panel();
};

#endif // MAINWINDOW_H
