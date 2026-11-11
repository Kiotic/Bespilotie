#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <QWidget>
#include <QTimer>
#include <QElapsedTimer>
#include <memory>
#include <vector>

#include "uav.h"
#include "sensor.h"
#include "navigation.h"
#include "controller.h"
#include "autopilot.h"
#include "simulation.h"

class QLabel;
class QComboBox;
class QSlider;
class QCheckBox;
class QPushButton;
class QPlainTextEdit;
class FlightView;
class RadarView;
class SensorCard;
class TelemetryChart;
class SoundEngine;

// Шина сообщений: все qDebug() попадают сюда (и в окно журнала)
class LogBus : public QObject {
    Q_OBJECT
public:
    static LogBus& instance();
signals:
    void message(const QString& text);
};

// ===== Интеграционное задание: визуальный стенд автопилота =====
class Dashboard : public QWidget {
    Q_OBJECT
public:
    explicit Dashboard(QWidget* parent = nullptr);
    ~Dashboard() override;

public slots:
    void append_log(const QString& text);

private:
    // --- модель (те же классы, что и в консольных заданиях) ---
    SurveyUAV uav_;
    Lidar lidar_;
    Radar radar_;
    Camera camera_;
    std::vector<Sensor*> sensors_;
    GPSNavigation gps_;
    PIDController pid_;
    SimpleController simple_;
    FuzzyController fuzzy_;
    std::vector<FlightController*> controllers_;
    std::unique_ptr<Autopilot> autopilot_;

    // --- симуляция ---
    VehicleDynamics dyn_;
    double u_ = 0.0;            // применённая команда
    double sim_t_ = 0.0, acc_ = 0.0;
    int cycles_ = 0;
    bool running_ = false;
    bool prev_ok_ = true;
    QString prev_reason_;
    bool target_reached_ = false;
    double last_warn_t_ = -100.0;
    bool fault_negative_alt_ = false;
    int sensor_level_[3] = {0, 0, 0};

    // --- интерфейс ---
    FlightView* flight_ = nullptr;
    RadarView* radar_view_ = nullptr;
    SensorCard* cards_[3] = {nullptr, nullptr, nullptr};
    TelemetryChart* alt_chart_ = nullptr;
    TelemetryChart* err_chart_ = nullptr;
    TelemetryChart* ctrl_chart_ = nullptr;
    TelemetryChart* cmp_chart_ = nullptr;
    QLabel* cmp_metrics_ = nullptr;
    QLabel* tile_alt_ = nullptr;
    QLabel* tile_err_ = nullptr;
    QLabel* tile_out_ = nullptr;
    QLabel* tile_ctrl_ = nullptr;
    QLabel* tile_cycles_ = nullptr;
    QLabel* tile_status_ = nullptr;
    QComboBox* ctrl_combo_ = nullptr;
    QSlider* target_slider_ = nullptr;
    QLabel* target_label_ = nullptr;
    QCheckBox* gps_fail_cb_ = nullptr;
    QCheckBox* neg_alt_cb_ = nullptr;
    QCheckBox* sound_cb_ = nullptr;
    QPushButton* run_btn_ = nullptr;
    QPlainTextEdit* log_ = nullptr;
    SoundEngine* sound_ = nullptr;

    QTimer frame_timer_;
    QElapsedTimer clock_;

    void build_ui();
    QWidget* make_tile(const QString& title, QLabel*& value, const QString& color);
    void tick();
    void autopilot_cycle();
    void simulate_environment();
    void refresh_views();
    void reset_sim();
    void set_running(bool on);
    void select_controller(int index);
    void run_scenario(int n);
    void compute_comparison();
    void toggle_sensor(int i);
};

#endif // DASHBOARD_H
