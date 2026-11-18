#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "soundengine.h"
#include "fallbackchart.h"

#include <QTimer>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsPathItem>
#include <QGraphicsLineItem>
#include <QGraphicsSimpleTextItem>
#include <QPainterPath>
#include <QListWidgetItem>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QFont>
#include <algorithm>
#include <cmath>

namespace {
const double kSceneXScale = 10.0;   // пикселей на секунду
const double kSceneYScale = 1.0;    // пикселей на метр
const QColor kCyan(58, 208, 255);
const QColor kGreen(62, 224, 137);
const QColor kAmber(255, 182, 72);
const QColor kRed(255, 92, 108);
const QColor kViolet(177, 140, 255);
const QColor kText(226, 232, 245);
const QColor kMuted(132, 146, 175);
}

// =====================================================================
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // ---------- модель: собирается так же, как в консольной версии ----------
    vehicle = new Quadcopter("QC-01", 150.0, 100.0);
    fleet = { vehicle,
              new FixedWingUAV("FW-02", 0.0, 86.0),
              new Quadcopter("QC-03", 0.0, 92.0) };

    gyro = new Gyroscope(1.2);
    accelerometer = new Accelerometer(9.81);
    barometer = new Barometer(1012.5);
    std::vector<Sensor*> sensors = { gyro, accelerometer, barometer };

    navigation = new GPSNavigation(vehicle);            // высота 150 м, следует за аппаратом
    controller = new PIDController(0.5, 0.1, 0.05);

    autopilot = new Autopilot(vehicle, sensors, navigation, controller, this);   // владелец — окно
    autopilot->set_target_altitude(ui->spinTarget->value());

    sound = new SoundEngine(this);

    // ---------- представление ----------
    setup_chart();
    setup_scene();
    fill_fleet_table();
    refresh_sensor_panel();
    ui->barBattery->setValue(int(vehicle->battery()));
    ui->labelAltitude->setText(QString::number(vehicle->altitude(), 'f', 1) + " m");
    set_status("Stopped", "#3a4766");
    log_event("Станция запущена. Таблица парка заполнена, статус: Stopped", kMuted);

    // ---------- таймер цикла 10 Гц ----------
    cycle_timer = new QTimer(this);
    cycle_timer->setInterval(100);

    // ---------- соединения (только указательный синтаксис) ----------
    connect(cycle_timer, &QTimer::timeout, autopilot, &Autopilot::run_cycle);

    // один сигнал — четыре приёмника: метки, график, сцена, таблица
    connect(autopilot, &Autopilot::telemetry_updated, this, &MainWindow::show_telemetry);
    connect(autopilot, &Autopilot::telemetry_updated, this, &MainWindow::update_chart);
    connect(autopilot, &Autopilot::telemetry_updated, this, &MainWindow::update_scene);
    connect(autopilot, &Autopilot::telemetry_updated, this, &MainWindow::update_fleet);

    connect(autopilot, &Autopilot::fault_detected, this, &MainWindow::show_fault);

    connect(ui->btnStart, &QPushButton::clicked, this, &MainWindow::start_mission);
    connect(ui->btnPause, &QPushButton::clicked, this, &MainWindow::pause_mission);
    connect(ui->btnReset, &QPushButton::clicked, this, &MainWindow::reset_mission);
    connect(ui->btnFaultSensor, &QPushButton::clicked, this, &MainWindow::inject_sensor_fault);
    connect(ui->btnFaultGps, &QPushButton::clicked, this, &MainWindow::inject_gps_fault);
    connect(ui->spinTarget, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::change_target);
    connect(ui->checkSound, &QCheckBox::toggled, sound, &SoundEngine::set_enabled);
}

MainWindow::~MainWindow()
{
    cycle_timer->stop();
    delete ui;
    // модельные классы (не QObject) удаляем сами; autopilot удалит родитель-окно
    for (UAV* u : fleet) delete u;
    delete gyro;
    delete accelerometer;
    delete barometer;
    delete navigation;
    delete controller;
}

// =====================================================================
//                         Подготовка отображений
// =====================================================================
void MainWindow::setup_chart()
{
    auto* box = new QVBoxLayout(ui->chartContainer);
    box->setContentsMargins(0, 0, 0, 0);

#ifdef HAVE_QT_CHARTS
    series_altitude = new QLineSeries();
    series_altitude->setName("Altitude, m");
    series_altitude->setPen(QPen(kCyan, 2.5));

    series_target = new QLineSeries();
    series_target->setName("Target, m");
    series_target->setPen(QPen(kGreen, 1.8, Qt::DashLine));

    series_control = new QLineSeries();
    series_control->setName("Control, m/s²");
    series_control->setPen(QPen(kViolet, 1.5));

    chart = new QChart();
    chart->setTheme(QChart::ChartThemeDark);
    chart->setBackgroundBrush(QColor(20, 27, 42));
    chart->setBackgroundRoundness(10);
    chart->setTitle("Altitude, m");
    chart->setTitleBrush(kText);
    chart->addSeries(series_altitude);
    chart->addSeries(series_target);
    chart->addSeries(series_control);

    axis_x = new QValueAxis();
    axis_x->setTitleText("t, s");
    axis_x->setRange(0, 30);
    axis_x->setLabelFormat("%.0f");
    axis_y = new QValueAxis();
    axis_y->setTitleText("m");
    axis_y->setRange(100, 250);
    axis_y->setLabelFormat("%.0f");
    axis_control = new QValueAxis();
    axis_control->setTitleText("control");
    axis_control->setRange(-60, 60);
    axis_control->setLabelFormat("%.0f");
    axis_control->setGridLineVisible(false);

    chart->addAxis(axis_x, Qt::AlignBottom);
    chart->addAxis(axis_y, Qt::AlignLeft);
    chart->addAxis(axis_control, Qt::AlignRight);
    series_altitude->attachAxis(axis_x);
    series_altitude->attachAxis(axis_y);
    series_target->attachAxis(axis_x);
    series_target->attachAxis(axis_y);
    series_control->attachAxis(axis_x);
    series_control->attachAxis(axis_control);

    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignTop);
    chart->legend()->setLabelColor(kText);

    auto* view = new QChartView(chart, ui->chartContainer);
    view->setRenderHint(QPainter::Antialiasing);
    view->setStyleSheet("background: transparent;");
    box->addWidget(view);
#else
    fallback_chart = new FallbackChart(ui->chartContainer);
    box->addWidget(fallback_chart);
#endif
}

void MainWindow::setup_scene()
{
    scene = new QGraphicsScene(this);
    scene->setSceneRect(-60, -520, 4000, 560);
    scene->setBackgroundBrush(QColor(12, 18, 32));
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);
    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // сетка высот
    QPen grid_pen(QColor(255, 255, 255, 25), 1);
    for (int a = 0; a <= 500; a += 50) {
        scene->addLine(-60, -a * kSceneYScale, 100000, -a * kSceneYScale, grid_pen);
        auto* t = scene->addSimpleText(QString("%1 m").arg(a));
        t->setBrush(kMuted);
        t->setPos(-55, -a * kSceneYScale - 14);
    }
    // земля
    scene->addRect(-60, 0, 100000, 40, Qt::NoPen, QColor(24, 40, 36));

    // целевая высота
    target_line = scene->addLine(0, 0, 100000, 0, QPen(kGreen, 1.5, Qt::DashLine));
    target_text = scene->addSimpleText("target");
    target_text->setBrush(kGreen);

    // след: один и тот же path-item, путь удлиняется
    trajectory = scene->addPath(QPainterPath(), QPen(kCyan, 2));

    // маркер аппарата: создаётся один раз, дальше только setPos()
    marker = scene->addRect(-10, -10, 20, 20, QPen(QColor(200, 230, 255)), QBrush(QColor(30, 110, 220)));
    marker->setZValue(10);
    auto* rotor_left = new QGraphicsEllipseItem(-24, -16, 16, 5, marker);
    auto* rotor_right = new QGraphicsEllipseItem(8, -16, 16, 5, marker);
    for (auto* r : {rotor_left, rotor_right}) {
        r->setBrush(QColor(200, 220, 255, 120));
        r->setPen(Qt::NoPen);
    }
    marker_text = scene->addSimpleText("QC-01");
    marker_text->setBrush(kText);
    marker_text->setZValue(10);

    // начальное положение
    QPainterPath start;
    start.moveTo(0, -vehicle->altitude() * kSceneYScale);
    trajectory->setPath(start);
    marker->setPos(0, -vehicle->altitude() * kSceneYScale);
    marker_text->setPos(14, -vehicle->altitude() * kSceneYScale - 26);
    change_target(ui->spinTarget->value());
    ui->graphicsView->centerOn(marker);
}

void MainWindow::fill_fleet_table()
{
    ui->tableFleet->setRowCount(0);
    for (UAV* u : fleet) {
        const int row = ui->tableFleet->rowCount();
        ui->tableFleet->insertRow(row);
        ui->tableFleet->setVerticalHeaderItem(row, new QTableWidgetItem(u->callsign()));
        ui->tableFleet->setItem(row, 0, new QTableWidgetItem(u->type()));          // полиморфный вызов
        const int motors = u->motor_count();                                        // −1 → «—»
        ui->tableFleet->setItem(row, 1, new QTableWidgetItem(motors >= 0 ? QString::number(motors) : QString("—")));
        ui->tableFleet->setItem(row, 2, new QTableWidgetItem(QString::number(u->battery(), 'f', 0) + " %"));
        auto* status = new QTableWidgetItem(u == vehicle ? "Stopped" : "Connected");
        status->setForeground(u == vehicle ? kMuted : kGreen);
        ui->tableFleet->setItem(row, 3, status);
        for (int c = 0; c < 4; ++c)
            ui->tableFleet->item(row, c)->setTextAlignment(Qt::AlignCenter);
    }
    ui->tableFleet->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableFleet->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableFleet->setSelectionMode(QAbstractItemView::NoSelection);
}

// =====================================================================
//                          Вспомогательные
// =====================================================================
void MainWindow::set_status(const QString& text, const QString& background)
{
    ui->labelStatus->setText(text);
    ui->labelStatus->setStyleSheet(QString("background:%1; color:#ffffff; font-weight:800; "
                                           "border-radius:12px; padding:6px 18px;").arg(background));
}

void MainWindow::set_fleet_status(const QString& text, const QColor& color)
{
    QTableWidgetItem* item = ui->tableFleet->item(0, 3);
    if (!item) return;
    item->setText(text);
    item->setForeground(color);
}

void MainWindow::log_event(const QString& text, const QColor& color)
{
    const QString stamp = QString("[T+%1 s]  ").arg(autopilot ? autopilot->mission_time() : 0.0, 6, 'f', 1);
    auto* item = new QListWidgetItem(stamp + text);
    item->setForeground(color);
    ui->listLog->addItem(item);
    ui->listLog->scrollToBottom();
}

void MainWindow::refresh_sensor_panel()
{
    const std::vector<std::pair<Sensor*, QLabel*>> rows = {
        {gyro, ui->labelGyro}, {accelerometer, ui->labelAccel}, {barometer, ui->labelBaro}};
    for (const auto& r : rows) {
        Sensor* s = r.first;
        if (s->is_ready()) {
            r.second->setText(QString("● %1 %2").arg(s->read()).arg(s->unit()));
            r.second->setStyleSheet("color:#3ee089;");
        } else {
            r.second->setText("● NOT READY");
            r.second->setStyleSheet("color:#ff5c6c; font-weight:700;");
        }
    }
    if (navigation->is_valid()) {
        ui->labelGps->setText(QString("● valid, %1 sat").arg(navigation->satellites()));
        ui->labelGps->setStyleSheet("color:#3ee089;");
    } else {
        ui->labelGps->setText("● INVALID");
        ui->labelGps->setStyleSheet("color:#ff5c6c; font-weight:700;");
    }
}

// =====================================================================
//                        Команды оператора
// =====================================================================
void MainWindow::start_mission()
{
    if (cycle_timer->isActive()) return;
    cycle_timer->start();
    if (!fault_active) {
        set_status("Running", "#1f9d61");
        set_fleet_status("Active", kGreen);
    }
    log_event("Старт: цикл автопилота 10 Гц", kGreen);
    sound->play_start();
    sound->start_motor();
}

void MainWindow::pause_mission()
{
    if (!cycle_timer->isActive()) return;
    cycle_timer->stop();                       // модель не пересоздаётся — состояние сохраняется
    set_status("Paused", "#c98a1e");
    set_fleet_status("Paused", kAmber);
    log_event("Пауза: цикл остановлен, значения замерли", kAmber);
    sound->stop_motor();
    sound->play_click();
}

void MainWindow::reset_mission()
{
    // восстановление — через те же публичные сеттеры модели
    gyro->set_ready(true);
    accelerometer->set_ready(true);
    barometer->set_ready(true);
    navigation->set_valid(true);
    first_fault_reported = false;
    fault_active = false;
    refresh_sensor_panel();

    set_status("Mission status: Active", "#1f9d61");
    set_fleet_status("Active", kGreen);
    log_event("Сброс: отказы устранены, миссия продолжается", kGreen);
    sound->play_success();
    sound->start_motor();
    cycle_timer->start();
}

void MainWindow::inject_sensor_fault()
{
    gyro->set_ready(false);                    // следующее срабатывание таймера обнаружит отказ
    refresh_sensor_panel();
    log_event("Оператор: имитация отказа Gyroscope (set_ready(false))", kAmber);
    sound->play_click();
}

void MainWindow::inject_gps_fault()
{
    navigation->set_valid(false);
    refresh_sensor_panel();
    log_event("Оператор: имитация отказа GPS (set_valid(false))", kAmber);
    sound->play_click();
}

void MainWindow::change_target(double value)
{
    autopilot->set_target_altitude(value);     // команда — вниз, через публичный метод
    target_line->setLine(-60, -value * kSceneYScale, 100000, -value * kSceneYScale);
    target_text->setPos(-55, -value * kSceneYScale + 2);
    target_text->setText(QString("target %1 m").arg(value, 0, 'f', 0));
    if (cycle_timer)
        log_event(QString("Новая целевая высота: %1 m").arg(value, 0, 'f', 0), kCyan);
}

// =====================================================================
//                    Приёмники сигнала telemetry_updated
// =====================================================================
void MainWindow::show_telemetry(double altitude, double error, double control)
{
    ui->labelAltitude->setText(QString::number(altitude, 'f', 1) + " m");
    ui->labelError->setText(QString::number(error, 'f', 1) + " m");
    ui->labelOutput->setText(QString::number(control, 'f', 2));
    ui->labelSpeed->setText(QString::number(vehicle->vertical_speed(), 'f', 1) + " m/s");
    ui->labelMissionTime->setText(QString("T+ %1 s").arg(autopilot->mission_time(), 0, 'f', 1));
    ui->barBattery->setValue(int(std::round(vehicle->battery())));
    refresh_sensor_panel();
    sound->set_motor_level(0.25 + 0.75 * std::min(1.0, std::abs(control) / 20.0));
}

void MainWindow::update_chart(double altitude, double, double control)
{
    const double t = autopilot->mission_time();
    const double target = autopilot->target_altitude();
#ifdef HAVE_QT_CHARTS
    series_altitude->append(t, altitude);
    series_target->append(t, target);
    series_control->append(t, control);

    // храним последние 60 с (600 точек), окно оси — 30 с
    const int extra = series_altitude->count() - 600;
    if (extra > 0) {
        series_altitude->removePoints(0, extra);
        series_target->removePoints(0, extra);
        series_control->removePoints(0, extra);
    }
    const double x1 = std::max(30.0, t);
    axis_x->setRange(x1 - 30.0, x1);

    double lo = target, hi = target;
    for (int i = 0; i < series_altitude->count(); ++i) {
        const QPointF p = series_altitude->at(i);
        if (p.x() < x1 - 30.0) continue;
        lo = std::min(lo, p.y());
        hi = std::max(hi, p.y());
    }
    const double pad = std::max(10.0, (hi - lo) * 0.1);
    axis_y->setRange(std::floor((lo - pad) / 10) * 10, std::ceil((hi + pad) / 10) * 10);
#else
    fallback_chart->add_point(t, altitude, target, control);
#endif
}

void MainWindow::update_scene(double altitude, double, double)
{
    const double x = autopilot->mission_time() * kSceneXScale;
    const double y = -altitude * kSceneYScale;

    // путь удлиняется, а не пересоздаётся
    QPainterPath path = trajectory->path();
    path.lineTo(x, y);
    trajectory->setPath(path);

    // маркер перемещается (не пересоздаётся); лёгкий наклон по вертикальной скорости
    marker->setPos(x, y);
    marker->setRotation(std::clamp(-vehicle->vertical_speed() * 1.5, -15.0, 15.0));
    marker_text->setPos(x + 14, y - 26);

    if (x + 200 > scene->sceneRect().right())
        scene->setSceneRect(scene->sceneRect().adjusted(0, 0, 2000, 0));
    ui->graphicsView->centerOn(marker);
}

void MainWindow::update_fleet(double, double, double)
{
    if (QTableWidgetItem* item = ui->tableFleet->item(0, 2))
        item->setText(QString::number(vehicle->battery(), 'f', 1) + " %");
}

// =====================================================================
//              Приёмник fault_detected: три уровня информирования
// =====================================================================
void MainWindow::show_fault(const QString& reason)
{
    cycle_timer->stop();
    fault_active = true;
    sound->stop_motor();
    sound->play_alarm();

    // уровень 1: постоянный статус (цветная метка)
    set_status("FAULT: " + reason, "#d83a4f");
    set_fleet_status("Fault", kRed);
    refresh_sensor_panel();

    // уровень 2: журнал событий — каждая авария
    log_event("FAULT: " + reason, kRed);

    // уровень 3: диалог — только при первой аварии
    if (!first_fault_reported) {
        first_fault_reported = true;
        QMessageBox::warning(this, "Авария",
                             "Обнаружена авария:\n\n" + reason +
                             "\n\nЦикл автопилота остановлен. Устраните причину и нажмите «Сброс».");
    }
}
