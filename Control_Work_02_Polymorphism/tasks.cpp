#include "tasks.h"
#include <QDebug>
#include <vector>
#include "uav.h"
#include "sensor.h"
#include "navigation.h"
#include "controller.h"
#include "autopilot.h"

static void header(const char* title)
{
    qDebug().noquote() << "";
    qDebug().noquote() << "==================================================";
    qDebug().noquote() << title;
    qDebug().noquote() << "==================================================";
}

void run_task1()
{
    header("TASK 1 (variant 3): virtual UAV classes");
    SurveyUAV survey;
    CargoUAV cargo(12.0);
    ReconnaissanceUAV recon;

    std::vector<UAV*> fleet = {&survey, &cargo, &recon};
    for (UAV* u : fleet)                                   // ожидаемый вывод
        qDebug().noquote() << u->type() + ": " + u->mission_type();

    for (UAV* u : fleet) execute_test_mission(*u);         // полиморфный вызов

    qDebug().noquote() << "-- error check: negative payload --";
    cargo.set_payload(-3.0);
}

void run_task2()
{
    header("TASK 2 (variant 3): environment sensors");
    Lidar lidar(12.5);
    Radar radar(18.2);
    Camera camera(3);
    std::vector<Sensor*> sensors = {&lidar, &radar, &camera};

    print_sensors(sensors);

    for (double threshold : {10.0, 20.0}) {
        qDebug().noquote() << QString("-- obstacle_is_near, threshold=%1 m --").arg(threshold);
        for (Sensor* s : sensors)
            qDebug().noquote() << QString("%1: %2").arg(s->name(),
                                  QString(obstacle_is_near(*s, threshold) ? "obstacle NEAR" : "clear"));
    }

    qDebug().noquote() << "-- error check: Lidar not ready --";
    lidar.set_ready(false);
    const bool near = obstacle_is_near(lidar, 20.0);
    qDebug().noquote() << "Lidar near (20 m):" << (near ? "yes" : "no");
    print_sensors(sensors);
}

void run_task3()
{
    header("TASK 3 (variant 3): abstract flight controllers");
    PIDController pid(0.6, 0.08, 0.014);
    SimpleController simple(0.5);
    FuzzyController fuzzy(5.0, 2.5);

    std::vector<FlightController*> controllers = {&pid, &simple, &fuzzy};
    for (FlightController* c : controllers) {
        test_controller(*c, 2.5, 0.1);                    // один и тот же вызов
        qDebug().noquote() << "";
    }

    qDebug().noquote() << "-- error check: dt = 0 --";
    pid.reset();
    test_controller(pid, 2.5, 0.0);
}

void run_integration_scenarios()
{
    header("INTEGRATION: Autopilot test scenarios");
    SurveyUAV uav;
    Lidar lidar; Radar radar; Camera camera;
    std::vector<Sensor*> sensors = {&lidar, &radar, &camera};
    GPSNavigation gps({55.75, 37.61, 150.0}, 10, true);
    SimpleController ctrl(0.5);   // 0.5 * 50 = 25 — эталонный вывод

    Autopilot ap(&uav, sensors, &gps, &ctrl);

    qDebug().noquote() << "\n# Scenario 1: normal operation";
    ap.run_cycle();

    qDebug().noquote() << "\n# Scenario 2: sensor not ready";
    lidar.set_ready(false);
    ap.run_cycle();
    lidar.set_ready(true);

    qDebug().noquote() << "\n# Scenario 3: navigation invalid";
    gps.set_valid(false);
    ap.run_cycle();
    gps.set_valid(true);

    qDebug().noquote() << "\n# Scenario 4: controller == nullptr";
    ap.set_controller(nullptr);
    ap.run_cycle();
    ap.set_controller(&ctrl);

    qDebug().noquote() << "\n# Scenario 5: negative altitude";
    gps.set_altitude(-15.0);
    ap.run_cycle();
    gps.set_altitude(150.0);

    qDebug().noquote() << "\n# Extra: vehicle == nullptr";
    Autopilot empty(nullptr, sensors, &gps, &ctrl);
    empty.run_cycle();

    qDebug().noquote() << "\n# Extra: same cycle with PID and Fuzzy (polymorphism)";
    PIDController pid(0.5, 0.05, 0.0);
    FuzzyController fuzzy;
    ap.set_verbose(false);
    for (FlightController* c : std::vector<FlightController*>{&ctrl, &pid, &fuzzy}) {
        ap.set_controller(c);
        ap.run_cycle();
        qDebug().noquote() << QString("%1 -> output %2").arg(c->controller_name()).arg(ap.last_output());
    }
}

void run_all_console_tasks()
{
    run_task1();
    run_task2();
    run_task3();
    run_integration_scenarios();
}
