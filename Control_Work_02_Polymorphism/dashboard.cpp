#include "dashboard.h"
#include "widgets.h"
#include "soundengine.h"
#include "tasks.h"

#include <QBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QSlider>
#include <QCheckBox>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QTabWidget>
#include <QScrollArea>
#include <QFrame>
#include <QDebug>
#include <QVariant>
#include <QSignalBlocker>
#include <QShortcut>
#include <QKeySequence>
#include <QtMath>
#include <cmath>

// ---------------------------------------------------------------------
LogBus& LogBus::instance()
{
    static LogBus bus;
    return bus;
}

namespace {
const double kControlDt = 0.1;     // период цикла автопилота (как в задании)

QString t_stamp(double t) { return QString("[t=%1 s]").arg(t, 5, 'f', 1); }

QFrame* card_frame()
{
    auto* f = new QFrame;
    f->setObjectName("card");
    return f;
}

// меняем стиль только при реальной смене цвета (setStyleSheet — дорогой вызов)
void set_label_color(QLabel* l, const char* color)
{
    const QString c(color);
    if (l->property("clr").toString() == c) return;
    l->setProperty("clr", c);
    l->setStyleSheet(QString("color:%1;").arg(c));
}

QLabel* section_label(const QString& s)
{
    auto* l = new QLabel(s);
    l->setObjectName("section");
    return l;
}
} // namespace

// ---------------------------------------------------------------------
Dashboard::Dashboard(QWidget* parent)
    : QWidget(parent),
      pid_(0.4, 0.005, 0.6),
      simple_(0.5),
      fuzzy_(50.0, 15.0)
{
    sensors_ = {&lidar_, &radar_, &camera_};
    controllers_ = {&pid_, &simple_, &fuzzy_};
    autopilot_.reset(new Autopilot(&uav_, sensors_, &gps_, &pid_));
    autopilot_->set_verbose(false);

    sound_ = new SoundEngine(this);

    build_ui();
    connect(&LogBus::instance(), &LogBus::message, this, &Dashboard::append_log);

    reset_sim();
    compute_comparison();

    connect(&frame_timer_, &QTimer::timeout, this, &Dashboard::tick);
    clock_.start();
    frame_timer_.start(16);
}

Dashboard::~Dashboard()
{
    frame_timer_.stop();
    disconnect(&LogBus::instance(), nullptr, this, nullptr);
}

// ---------------------------------------------------------------------
QWidget* Dashboard::make_tile(const QString& title, QLabel*& value, const QString& color)
{
    auto* f = card_frame();
    auto* v = new QVBoxLayout(f);
    v->setContentsMargins(14, 8, 14, 8);
    v->setSpacing(0);
    auto* t = new QLabel(title);
    t->setObjectName("tileTitle");
    value = new QLabel("—");
    value->setObjectName("tileValue");
    value->setStyleSheet(QString("color:%1;").arg(color));
    v->addWidget(t);
    v->addWidget(value);
    return f;
}

void Dashboard::build_ui()
{
    setWindowTitle("Контрольная работа №2 · Вариант 3 · Автопилот БПЛА");
    resize(1560, 940);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(14, 12, 14, 12);
    root->setSpacing(10);

    // ---------- заголовок ----------
    auto* head = new QHBoxLayout;
    auto* title = new QLabel("UAV AUTOPILOT <span style='color:#3ad0ff'>·</span> "
                             "<span style='color:#8492af;font-weight:400'>Survey UAV · "
                             "Lidar / Radar / Camera · PID / Simple / Fuzzy</span>");
    title->setObjectName("title");
    head->addWidget(title);
    head->addStretch();
    auto* hint = new QLabel(sound_->has_multimedia()
                                ? "Пробел — старт/пауза · R — сброс · 1–5 — сценарии"
                                : "Пробел — старт/пауза · R — сброс · 1–5 — сценарии · "
                                  "(Qt Multimedia не найден: звук = системный beep)");
    hint->setObjectName("muted");
    head->addWidget(hint);
    root->addLayout(head);

    // ---------- плитки ----------
    auto* tiles = new QHBoxLayout;
    tiles->setSpacing(10);
    tiles->addWidget(make_tile("ВЫСОТА", tile_alt_, "#3ad0ff"));
    tiles->addWidget(make_tile("ОШИБКА ПО ВЫСОТЕ", tile_err_, "#ffb648"));
    tiles->addWidget(make_tile("ВЫХОД РЕГУЛЯТОРА", tile_out_, "#b18cff"));
    tiles->addWidget(make_tile("РЕГУЛЯТОР", tile_ctrl_, "#e2e8f5"));
    tiles->addWidget(make_tile("ЦИКЛОВ АВТОПИЛОТА", tile_cycles_, "#e2e8f5"));
    tiles->addWidget(make_tile("СТАТУС", tile_status_, "#3ee089"), 2);
    root->addLayout(tiles);

    // ---------- основная часть ----------
    auto* body = new QHBoxLayout;
    body->setSpacing(10);

    flight_ = new FlightView;
    flight_->setMinimumWidth(320);
    flight_->setMaximumWidth(380);
    body->addWidget(flight_);

    // центр: вкладки с графиками
    auto* tabs = new QTabWidget;
    auto* tele = new QWidget;
    auto* tv = new QVBoxLayout(tele);
    tv->setContentsMargins(0, 8, 0, 0);
    tv->setSpacing(8);
    alt_chart_ = new TelemetryChart("Высота и цель", "м");
    alt_chart_->add_series("высота", Theme::cyan(), true);
    alt_chart_->add_series("цель", Theme::green(), false, true);
    err_chart_ = new TelemetryChart("Ошибка по высоте  e = target − altitude", "м");
    err_chart_->add_series("ошибка", Theme::amber(), true);
    err_chart_->set_include_zero(true);
    ctrl_chart_ = new TelemetryChart("Управляющее воздействие (после ограничения ±15 м/с²)", "м/с²");
    ctrl_chart_->add_series("команда u", Theme::violet(), true);
    ctrl_chart_->set_include_zero(true);
    tv->addWidget(alt_chart_, 3);
    tv->addWidget(err_chart_, 2);
    tv->addWidget(ctrl_chart_, 2);
    tabs->addTab(tele, "📈  Телеметрия");

    auto* cmp = new QWidget;
    auto* cv = new QVBoxLayout(cmp);
    cv->setContentsMargins(0, 8, 0, 0);
    cmp_chart_ = new TelemetryChart("Переходный процесс 150 → 200 м: один и тот же вызов для трёх регуляторов", "м");
    cmp_chart_->add_series("PID", Theme::cyan());
    cmp_chart_->add_series("Simple", Theme::amber());
    cmp_chart_->add_series("Fuzzy", Theme::violet());
    cmp_chart_->add_series("цель", Theme::green(), false, true);
    cmp_chart_->set_fixed_x(0, 40);
    cv->addWidget(cmp_chart_, 1);
    cmp_metrics_ = new QLabel;
    cmp_metrics_->setObjectName("metrics");
    cmp_metrics_->setTextFormat(Qt::RichText);
    auto* recompute = new QPushButton("↻  Пересчитать сравнение");
    connect(recompute, &QPushButton::clicked, this, [this] { sound_->play_click(); compute_comparison(); });
    auto* mrow = new QHBoxLayout;
    mrow->addWidget(cmp_metrics_, 1);
    mrow->addWidget(recompute, 0, Qt::AlignBottom);
    cv->addLayout(mrow);
    tabs->addTab(cmp, "⚖  Сравнение регуляторов");
    body->addWidget(tabs, 1);

    // правая колонка
    auto* right = new QWidget;
    auto* rv = new QVBoxLayout(right);
    rv->setContentsMargins(0, 0, 4, 0);
    rv->setSpacing(8);
    radar_view_ = new RadarView;
    radar_view_->setMinimumHeight(300);
    rv->addWidget(radar_view_);
    rv->addWidget(section_label("СЕНСОРЫ  (клик по карточке — отказ / восстановление)"));
    for (int i = 0; i < 3; ++i) {
        cards_[i] = new SensorCard;
        connect(cards_[i], &SensorCard::clicked, this, [this, i] { toggle_sensor(i); });
        rv->addWidget(cards_[i]);
    }

    rv->addWidget(section_label("УПРАВЛЕНИЕ"));
    auto* ctl = card_frame();
    auto* g = new QGridLayout(ctl);
    g->setContentsMargins(12, 10, 12, 10);
    g->setHorizontalSpacing(8);
    g->setVerticalSpacing(8);

    run_btn_ = new QPushButton("▶  Старт");
    run_btn_->setObjectName("primary");
    auto* reset_btn = new QPushButton("⟲  Сброс");
    connect(run_btn_, &QPushButton::clicked, this, [this] { set_running(!running_); });
    connect(reset_btn, &QPushButton::clicked, this, [this] { sound_->play_click(); reset_sim(); });
    g->addWidget(run_btn_, 0, 0);
    g->addWidget(reset_btn, 0, 1);

    g->addWidget(new QLabel("Регулятор:"), 1, 0);
    ctrl_combo_ = new QComboBox;
    ctrl_combo_->addItems({"PID (kp=0.4, ki=0.005, kd=0.6)", "Simple (k=0.5)", "Fuzzy (5 термов)",
                           "nullptr — регулятор не задан"});
    connect(ctrl_combo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this](int i) { select_controller(i); });
    g->addWidget(ctrl_combo_, 1, 1);

    target_label_ = new QLabel("Цель: 200 м");
    g->addWidget(target_label_, 2, 0);
    target_slider_ = new QSlider(Qt::Horizontal);
    target_slider_->setRange(5, 25);          // ×10 м
    target_slider_->setValue(20);
    connect(target_slider_, &QSlider::valueChanged, this, [this](int v) {
        autopilot_->set_target_altitude(v * 10.0);
        target_label_->setText(QString("Цель: %1 м").arg(v * 10));
        target_reached_ = false;
    });
    g->addWidget(target_slider_, 2, 1);

    gps_fail_cb_ = new QCheckBox("Отказ GPS  (gps.set_valid(false))");
    connect(gps_fail_cb_, &QCheckBox::toggled, this, [this](bool on) {
        gps_.set_valid(!on);
        qDebug().noquote() << t_stamp(sim_t_) << (on ? "Fault injected: GPS invalid" : "GPS restored");
    });
    neg_alt_cb_ = new QCheckBox("Сбой координат  (высота < 0)");
    connect(neg_alt_cb_, &QCheckBox::toggled, this, [this](bool on) {
        fault_negative_alt_ = on;
        qDebug().noquote() << t_stamp(sim_t_)
                           << (on ? "Fault injected: negative altitude in navigation data"
                                  : "Navigation coordinates restored");
    });
    sound_cb_ = new QCheckBox("Звук");
    sound_cb_->setChecked(true);
    connect(sound_cb_, &QCheckBox::toggled, this, [this](bool on) { sound_->set_enabled(on); });
    g->addWidget(gps_fail_cb_, 3, 0, 1, 2);
    g->addWidget(neg_alt_cb_, 4, 0, 1, 2);
    g->addWidget(sound_cb_, 5, 0, 1, 2);
    rv->addWidget(ctl);

    rv->addWidget(section_label("ТЕСТОВЫЕ СЦЕНАРИИ ИЗ ЗАДАНИЯ"));
    auto* sc = card_frame();
    auto* sg = new QGridLayout(sc);
    sg->setContentsMargins(12, 10, 12, 10);
    sg->setSpacing(6);
    const QStringList names = {"1 · Штатная работа", "2 · Сенсор не готов", "3 · Навигация невалидна",
                               "4 · controller == nullptr", "5 · Отрицательная высота"};
    for (int i = 0; i < names.size(); ++i) {
        auto* b = new QPushButton(names[i]);
        b->setObjectName("scenario");
        connect(b, &QPushButton::clicked, this, [this, i] { run_scenario(i + 1); });
        sg->addWidget(b, i / 2, i % 2);
    }
    auto* all = new QPushButton("☰  Задания 1–3 + сценарии → журнал");
    connect(all, &QPushButton::clicked, this, [this] {
        sound_->play_click();
        run_all_console_tasks();
    });
    sg->addWidget(all, 2, 1);
    rv->addWidget(sc);
    rv->addStretch();

    auto* scroll = new QScrollArea;
    scroll->setWidget(right);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setFixedWidth(400);
    body->addWidget(scroll);

    root->addLayout(body, 1);

    // ---------- журнал qDebug ----------
    log_ = new QPlainTextEdit;
    log_->setReadOnly(true);
    log_->setMaximumBlockCount(4000);
    log_->setFixedHeight(170);
    log_->setObjectName("log");
    root->addWidget(log_);

    // горячие клавиши
    auto* sp = new QShortcut(QKeySequence(Qt::Key_Space), this);
    connect(sp, &QShortcut::activated, this, [this] { set_running(!running_); });
    auto* sr = new QShortcut(QKeySequence(Qt::Key_R), this);
    connect(sr, &QShortcut::activated, this, [this] { reset_sim(); });
    for (int i = 1; i <= 5; ++i) {
        auto* s = new QShortcut(QKeySequence(QString::number(i)), this);
        connect(s, &QShortcut::activated, this, [this, i] { run_scenario(i); });
    }
}

// ---------------------------------------------------------------------
void Dashboard::append_log(const QString& text)
{
    if (!log_) return;
    const QStringList lines = text.split('\n');
    for (const QString& line : lines) {
        QString color = "#c9d3e8";
        bool bold = false;
        if (line.contains("Error") || line.contains("aborted") || line.contains("Reason")
            || line.contains("NOT READY") || line.contains("invalid") || line.contains("Fault"))
            color = "#ff5c6c";
        else if (line.contains("Warning"))
            color = "#ffb648";
        else if (line.contains("completed") || line.contains("All sensors are ready")
                 || line.contains("restored") || line.contains("reached"))
            color = "#3ee089";
        if (line.startsWith("==") || line.startsWith('#') || line.startsWith("TASK")
            || line.startsWith("INTEGRATION") || line.startsWith("---")) {
            color = "#3ad0ff";
            bold = true;
        }
        const QString esc = line.isEmpty() ? QString("&nbsp;") : line.toHtmlEscaped().replace(' ', "&nbsp;");
        log_->appendHtml(QString("<span style='color:%1;%2'>%3</span>")
                             .arg(color, bold ? QString("font-weight:700;") : QString(), esc));
    }
}

// ---------------------------------------------------------------------
void Dashboard::set_running(bool on)
{
    running_ = on;
    run_btn_->setText(on ? "❚❚  Пауза" : "▶  Старт");
    if (on) {
        sound_->play_start();
        sound_->start_motor();
        qDebug().noquote() << t_stamp(sim_t_) << "Simulation started, controller:"
                           << (ctrl_combo_->currentIndex() < 3
                                   ? controllers_[ctrl_combo_->currentIndex()]->controller_name()
                                   : QString("nullptr"));
    } else {
        sound_->stop_motor();
        qDebug().noquote() << t_stamp(sim_t_) << "Simulation paused";
    }
}

void Dashboard::reset_sim()
{
    dyn_ = VehicleDynamics();
    dyn_.altitude = 150.0;
    u_ = 0.0;
    sim_t_ = 0.0;
    acc_ = 0.0;
    cycles_ = 0;
    prev_ok_ = true;
    prev_reason_.clear();
    target_reached_ = false;
    last_warn_t_ = -100.0;
    for (FlightController* c : controllers_) c->reset();
    gps_.set_altitude(150.0);
    alt_chart_->clear();
    err_chart_->clear();
    ctrl_chart_->clear();
    flight_->clear_trail();
    simulate_environment();
    refresh_views();
}

void Dashboard::select_controller(int index)
{
    FlightController* c = (index >= 0 && index < int(controllers_.size())) ? controllers_[index] : nullptr;
    if (c) c->reset();
    autopilot_->set_controller(c);
    target_reached_ = false;
    qDebug().noquote() << t_stamp(sim_t_) << "Controller switched to"
                       << (c ? c->controller_name() : QString("nullptr"));
    refresh_views();
}

void Dashboard::toggle_sensor(int i)
{
    sound_->play_click();
    switch (i) {
    case 0: lidar_.set_ready(!lidar_.is_ready()); break;
    case 1: radar_.set_ready(!radar_.is_ready()); break;
    default: camera_.set_ready(!camera_.is_ready()); break;
    }
    Sensor* s = sensors_[i];
    qDebug().noquote() << t_stamp(sim_t_) << s->name()
                       << (s->is_ready() ? "restored" : (s->is_critical() ? "Fault: NOT READY (critical)"
                                                                          : "Warning: NOT READY (non-critical)"));
    refresh_views();
}

// ---------------------------------------------------------------------
void Dashboard::simulate_environment()
{
    const double t = sim_t_;
    lidar_.set_distance(16.0 + 9.0 * std::sin(0.35 * t) + 3.0 * std::sin(1.7 * t + 0.5));
    radar_.set_distance(19.0 + 8.0 * std::sin(0.22 * t + 1.3) + 2.0 * std::sin(1.1 * t));
    camera_.set_objects(qMax(0, int(std::lround(2.0 + 2.0 * std::sin(0.4 * t) + std::sin(1.3 * t)))));
}

void Dashboard::autopilot_cycle()
{
    simulate_environment();
    gps_.set_altitude(fault_negative_alt_ ? -12.0 : dyn_.altitude);

    autopilot_->run_cycle();
    ++cycles_;

    const bool ok = autopilot_->last_ok();
    u_ = ok ? VehicleDynamics::saturate(autopilot_->last_output()) : 0.0;   // авария → удержание (u = 0)

    // смена состояния → звук + запись в журнал
    if (ok != prev_ok_ || (!ok && autopilot_->last_reason() != prev_reason_)) {
        if (!ok) {
            sound_->play_alarm();
            qDebug().noquote() << t_stamp(sim_t_) << "Autopilot cycle aborted. Reason:" << autopilot_->last_reason();
        } else {
            sound_->play_success();
            qDebug().noquote() << t_stamp(sim_t_) << "Autopilot restored, cycle completed";
        }
    }
    prev_ok_ = ok;
    prev_reason_ = autopilot_->last_reason();

    // достижение цели
    const double err = autopilot_->target_altitude() - dyn_.altitude;
    if (ok && !target_reached_ && std::fabs(err) < 1.0 && std::fabs(dyn_.vspeed) < 0.5) {
        target_reached_ = true;
        sound_->play_success();
        qDebug().noquote() << t_stamp(sim_t_) << "Target altitude reached:"
                           << QString::number(dyn_.altitude, 'f', 1) << "m";
    }
    if (std::fabs(err) > 5.0) target_reached_ = false;

    // препятствия: полиморфная проверка через obstacle_is_near(Sensor&, threshold)
    QString near_names;
    for (int i = 0; i < 3; ++i) {
        Sensor* s = sensors_[i];
        int level = 0;
        if (s->is_ready()) {
            if (obstacle_is_near(*s, 10.0)) level = 2;
            else if (obstacle_is_near(*s, 20.0)) level = 1;
        }
        sensor_level_[i] = level;
        if (level == 2) near_names += (near_names.isEmpty() ? QString() : QString(", ")) + s->name();
    }
    if (!near_names.isEmpty() && sim_t_ - last_warn_t_ > 2.5) {
        last_warn_t_ = sim_t_;
        sound_->play_warning();
        qDebug().noquote() << t_stamp(sim_t_) << "Warning: obstacle closer than 10 m —" << near_names;
    }

    // графики
    alt_chart_->add_point(0, sim_t_, dyn_.altitude);
    alt_chart_->add_point(1, sim_t_, autopilot_->target_altitude());
    err_chart_->add_point(0, sim_t_, err);
    ctrl_chart_->add_point(0, sim_t_, u_);
}

void Dashboard::tick()
{
    double dt = clock_.restart() / 1000.0;
    if (dt > 0.05) dt = 0.05;

    if (running_) {
        // физика с мелким шагом, автопилот — каждые 0.1 с
        double rest = dt;
        while (rest > 1e-9) {
            const double h = qMin(rest, 0.01);
            dyn_.step(u_, h);
            sim_t_ += h;
            acc_ += h;
            rest -= h;
            if (acc_ >= kControlDt) {
                acc_ -= kControlDt;
                autopilot_cycle();
            }
        }
        flight_->push_trail(sim_t_, dyn_.altitude);
        sound_->set_motor_level(0.25 + 0.75 * qMin(1.0, std::fabs(u_) / VehicleDynamics::max_cmd));
    }

    flight_->advance(dt, running_);
    radar_view_->advance(dt);
    refresh_views();
}

void Dashboard::refresh_views()
{
    const bool ok = autopilot_->last_ok() || cycles_ == 0;
    const QString reason = autopilot_->last_reason();
    const QString status = cycles_ == 0 ? QString("READY")
                         : ok ? (running_ ? QString("CYCLE OK") : QString("PAUSED"))
                              : QString("ABORTED: ") + reason;
    const double target = autopilot_->target_altitude();
    const double err = target - dyn_.altitude;

    flight_->set_state(dyn_.altitude, dyn_.vspeed, target, u_, ok, status,
                       uav_.type() + " · " + uav_.mission_type());

    tile_alt_->setText(QString("%1 м").arg(dyn_.altitude, 0, 'f', 1));
    tile_err_->setText(QString("%1 м").arg(err, 0, 'f', 1));
    tile_out_->setText(cycles_ == 0 ? QString("—") : QString::number(autopilot_->last_output(), 'f', 2));
    const int ci = ctrl_combo_->currentIndex();
    tile_ctrl_->setText(ci >= 0 && ci < 3 ? controllers_[ci]->controller_name() : QString("nullptr"));
    set_label_color(tile_ctrl_, ci == 3 ? "#ff5c6c" : "#e2e8f5");
    tile_cycles_->setText(QString::number(cycles_));
    tile_status_->setText(status);
    set_label_color(tile_status_, ok ? "#3ee089" : "#ff5c6c");

    // карточки сенсоров и радар — полиморфный обход
    QVector<RadarView::Blip> blips;
    const double angles[2] = {75.0 + 30.0 * std::sin(0.15 * sim_t_), -35.0 + 25.0 * std::sin(0.11 * sim_t_ + 2.0)};
    for (int i = 0; i < 3; ++i) {
        Sensor* s = sensors_[i];
        const QString d = s->describe();
        QString sub = d.section('=', 0, 0).section(':', 1).trimmed();
        if (s->provides_distance())
            sub += QString("  ·  <10 м: %1  ·  <20 м: %2")
                       .arg(QString(sensor_level_[i] == 2 ? "ДА" : "нет"), QString(sensor_level_[i] >= 1 ? "ДА" : "нет"));
        else
            sub += "  ·  не измеряет дистанцию";
        cards_[i]->set_data(s->name(), d.section('=', 1), s->is_ready(), s->is_critical(),
                            sensor_level_[i], sub);
        if (s->provides_distance() && i < 2)
            blips.push_back({s->name(), angles[i], s->read(), s->is_ready(), sensor_level_[i]});
    }
    radar_view_->set_blips(blips);
    radar_view_->set_camera_info(camera_.is_ready()
                                     ? QString("📷 Camera: %1 объектов в кадре").arg(int(camera_.read()))
                                     : QString("📷 Camera: OFFLINE"),
                                 camera_.is_ready());
}

// ---------------------------------------------------------------------
void Dashboard::run_scenario(int n)
{
    sound_->play_click();
    {
        // штатное состояние без лишних сообщений
        const QSignalBlocker b1(gps_fail_cb_), b2(neg_alt_cb_);
        gps_fail_cb_->setChecked(false);
        neg_alt_cb_->setChecked(false);
    }
    lidar_.set_ready(true);
    radar_.set_ready(true);
    camera_.set_ready(true);
    gps_.set_valid(true);
    fault_negative_alt_ = false;
    ctrl_combo_->setCurrentIndex(n == 4 ? 3 : 1);   // эталон: Simple k=0.5 → 0.5·50 = 25
    reset_sim();

    static const char* titles[6] = {"", "normal operation", "sensor not ready", "navigation invalid",
                                    "controller == nullptr", "negative altitude"};
    switch (n) {
    case 2: lidar_.set_ready(false); break;
    case 3: gps_fail_cb_->setChecked(true); break;
    case 5: neg_alt_cb_->setChecked(true); break;
    default: break;
    }

    // эталонные значения сенсоров из задания
    lidar_.set_distance(12.5);
    radar_.set_distance(18.2);
    camera_.set_objects(3);
    gps_.set_altitude(fault_negative_alt_ ? -12.0 : 150.0);

    qDebug().noquote() << "";
    qDebug().noquote() << QString("# Scenario %1: %2").arg(n).arg(QString(titles[n]));
    autopilot_->set_verbose(true);
    autopilot_->run_cycle();
    autopilot_->set_verbose(false);
    ++cycles_;

    const bool ok = autopilot_->last_ok();
    prev_ok_ = ok;
    prev_reason_ = autopilot_->last_reason();
    u_ = ok ? VehicleDynamics::saturate(autopilot_->last_output()) : 0.0;
    if (ok) sound_->play_success(); else sound_->play_alarm();

    alt_chart_->add_point(0, 0, dyn_.altitude);
    alt_chart_->add_point(1, 0, autopilot_->target_altitude());
    err_chart_->add_point(0, 0, autopilot_->target_altitude() - dyn_.altitude);
    ctrl_chart_->add_point(0, 0, u_);
    refresh_views();
}

// ---------------------------------------------------------------------
void Dashboard::compute_comparison()
{
    // отдельные экземпляры, чтобы не трогать состояние «живого» регулятора
    PIDController pid(0.4, 0.005, 0.6);
    SimpleController simple(0.5);
    FuzzyController fuzzy(50.0, 15.0);
    const std::vector<FlightController*> list = {&pid, &simple, &fuzzy};
    const char* colors[3] = {"#3ad0ff", "#ffb648", "#b18cff"};

    cmp_chart_->clear();
    QString html = "<table cellspacing='0' cellpadding='4'><tr>"
                   "<td style='color:#8492af'>Регулятор</td>"
                   "<td style='color:#8492af'>Перерегулирование</td>"
                   "<td style='color:#8492af'>Время входа в ±2 м</td>"
                   "<td style='color:#8492af'>Статическая ошибка</td></tr>";
    for (int i = 0; i < 3; ++i) {
        FlightController& c = *list[i];
        const auto trace = simulate_step(c, 150.0, 200.0, 40.0);     // полиморфный вызов
        for (const auto& p : trace) cmp_chart_->add_point(i, p.first, p.second);
        const StepMetrics m = analyze_step(trace, 150.0, 200.0);
        html += QString("<tr><td style='color:%1;font-weight:700'>%2</td><td>%3 %</td><td>%4 с</td>"
                        "<td>%5 м</td></tr>")
                    .arg(QString(colors[i]), c.controller_name())
                    .arg(m.overshoot_pct, 0, 'f', 1)
                    .arg(m.settling_s, 0, 'f', 1)
                    .arg(m.final_error, 0, 'f', 2);
    }
    cmp_chart_->add_point(3, 0, 200.0);
    cmp_chart_->add_point(3, 40, 200.0);
    html += "</table><div style='color:#8492af'>Модель: a = sat(u, ±15) − 0.8 (возмущение) − 0.6·v. "
            "Интегральная составляющая PID компенсирует возмущение — у Simple и Fuzzy остаётся статическая ошибка.</div>";
    cmp_metrics_->setText(html);
}
