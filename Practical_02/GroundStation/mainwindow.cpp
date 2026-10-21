#include "mainwindow.h"

#include "altitudechart.h"
#include "altitudegauge.h"

#include <QWidget>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QPlainTextEdit>
#include <QProgressBar>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>

MainWindow::MainWindow(
    QWidget *parent
    )
    : QMainWindow(parent),
    station("Base-1")
{
    createInterface();

    log(
        "Program started"
        );

    updateInfo();
}

void MainWindow::createInterface()
{
    setWindowTitle(
        "Ground Station Monitor"
        );

    resize(
        1250,
        850
        );

    QWidget *central =
        new QWidget(this);

    setCentralWidget(
        central
        );

    editStation =
        new QLineEdit(
            "Base-1",
            this
            );

    editStation->setReadOnly(
        true
        );

    editDrone =
        new QLineEdit(
            "Survey-01",
            this
            );

    spinAltitude =
        new QDoubleSpinBox(this);

    spinAltitude->setRange(
        -500.0,
        10000.0
        );

    spinAltitude->setDecimals(
        1
        );

    spinAltitude->setValue(
        100.0
        );

    spinAltitude->setSuffix(
        " m"
        );

    btnConnect =
        new QPushButton(
            "Connect Drone",
            this
            );

    btnDisconnect =
        new QPushButton(
            "Disconnect",
            this
            );

    btnReceive =
        new QPushButton(
            "Receive Altitude",
            this
            );

    btnStatus =
        new QPushButton(
            "Print Status",
            this
            );

    QGroupBox *connectionGroup =
        new QGroupBox(
            "Ground station",
            this
            );

    QFormLayout *connectionForm =
        new QFormLayout;

    connectionForm->addRow(
        "Station:",
        editStation
        );

    connectionForm->addRow(
        "Drone:",
        editDrone
        );

    connectionForm->addRow(
        "Altitude:",
        spinAltitude
        );

    connectionGroup->setLayout(
        connectionForm
        );

    QHBoxLayout *buttons =
        new QHBoxLayout;

    buttons->addWidget(
        btnConnect
        );

    buttons->addWidget(
        btnDisconnect
        );

    buttons->addWidget(
        btnReceive
        );

    labelConnection =
        new QLabel(
            "DISCONNECTED",
            this
            );

    labelDrone =
        new QLabel(
            "-",
            this
            );

    labelRecords =
        new QLabel(
            "0",
            this
            );

    labelAverage =
        new QLabel(
            "0.00 m",
            this
            );

    labelCurrentAltitude =
        new QLabel(
            "0.00 m",
            this
            );

    QGroupBox *statusGroup =
        new QGroupBox(
            "Station status",
            this
            );

    QFormLayout *statusForm =
        new QFormLayout;

    statusForm->addRow(
        "Connection:",
        labelConnection
        );

    statusForm->addRow(
        "Drone:",
        labelDrone
        );

    statusForm->addRow(
        "Current altitude:",
        labelCurrentAltitude
        );

    statusForm->addRow(
        "Records:",
        labelRecords
        );

    statusForm->addRow(
        "Average altitude:",
        labelAverage
        );

    statusGroup->setLayout(
        statusForm
        );

    spinTargetAltitude =
        new QDoubleSpinBox(this);

    spinTargetAltitude->setRange(
        -100.0,
        10000.0
        );

    spinTargetAltitude->setValue(
        120.0
        );

    spinTargetAltitude->setSuffix(
        " m"
        );

    spinMaxSpeed =
        new QDoubleSpinBox(this);

    spinMaxSpeed->setRange(
        -100.0,
        500.0
        );

    spinMaxSpeed->setValue(
        15.0
        );

    spinMaxSpeed->setSuffix(
        " m/s"
        );

    spinGeofence =
        new QDoubleSpinBox(this);

    spinGeofence->setRange(
        0.0,
        100000.0
        );

    spinGeofence->setValue(
        1000.0
        );

    spinGeofence->setSuffix(
        " m"
        );

    spinReturnBattery =
        new QDoubleSpinBox(this);

    spinReturnBattery->setRange(
        -10.0,
        110.0
        );

    spinReturnBattery->setValue(
        25.0
        );

    spinReturnBattery->setSuffix(
        " %"
        );

    spinTimeout =
        new QDoubleSpinBox(this);

    spinTimeout->setRange(
        0.0,
        100000.0
        );

    spinTimeout->setValue(
        600.0
        );

    spinTimeout->setSuffix(
        " s"
        );

    btnSaveMission =
        new QPushButton(
            "Save Mission Parameters",
            this
            );

    QGroupBox *missionInput =
        new QGroupBox(
            "Mission parameters",
            this
            );

    QFormLayout *missionForm =
        new QFormLayout;

    missionForm->addRow(
        "Target altitude:",
        spinTargetAltitude
        );

    missionForm->addRow(
        "Maximum speed:",
        spinMaxSpeed
        );

    missionForm->addRow(
        "Geofence radius:",
        spinGeofence
        );

    missionForm->addRow(
        "Return battery:",
        spinReturnBattery
        );

    missionForm->addRow(
        "Mission timeout:",
        spinTimeout
        );

    missionForm->addRow(
        btnSaveMission
        );

    missionInput->setLayout(
        missionForm
        );

    labelTargetAltitude =
        new QLabel(
            "0 m",
            this
            );

    labelMaxSpeed =
        new QLabel(
            "0 m/s",
            this
            );

    labelGeofence =
        new QLabel(
            "0 m",
            this
            );

    labelReturnBattery =
        new QLabel(
            "0 %",
            this
            );

    labelTimeout =
        new QLabel(
            "0 s",
            this
            );

    batteryBar =
        new QProgressBar(this);

    batteryBar->setRange(
        0,
        100
        );

    batteryBar->setValue(
        0
        );

    batteryBar->setFormat(
        "%p%"
        );

    QGroupBox *missionStatus =
        new QGroupBox(
            "Saved mission",
            this
            );

    QFormLayout *savedMissionForm =
        new QFormLayout;

    savedMissionForm->addRow(
        "Target altitude:",
        labelTargetAltitude
        );

    savedMissionForm->addRow(
        "Maximum speed:",
        labelMaxSpeed
        );

    savedMissionForm->addRow(
        "Geofence:",
        labelGeofence
        );

    savedMissionForm->addRow(
        "Return battery:",
        labelReturnBattery
        );

    savedMissionForm->addRow(
        "Return level:",
        batteryBar
        );

    savedMissionForm->addRow(
        "Timeout:",
        labelTimeout
        );

    missionStatus->setLayout(
        savedMissionForm
        );

    chart =
        new AltitudeChart(this);

    gauge =
        new AltitudeGauge(this);

    QVBoxLayout *leftLayout =
        new QVBoxLayout;

    leftLayout->addWidget(
        connectionGroup
        );

    leftLayout->addLayout(
        buttons
        );

    leftLayout->addWidget(
        statusGroup
        );

    leftLayout->addWidget(
        btnStatus
        );

    leftLayout->addWidget(
        missionInput
        );

    leftLayout->addWidget(
        missionStatus
        );

    leftLayout->addStretch();

    QVBoxLayout *visualLayout =
        new QVBoxLayout;

    visualLayout->addWidget(
        gauge
        );

    visualLayout->addWidget(
        chart
        );

    QHBoxLayout *topLayout =
        new QHBoxLayout;

    topLayout->addLayout(
        leftLayout,
        1
        );

    topLayout->addLayout(
        visualLayout,
        2
        );

    textLog =
        new QPlainTextEdit(this);

    textLog->setReadOnly(
        true
        );

    textLog->setMaximumHeight(
        150
        );

    QVBoxLayout *mainLayout =
        new QVBoxLayout(central);

    mainLayout->addLayout(
        topLayout
        );

    mainLayout->addWidget(
        new QLabel(
            "Event log:",
            this
            )
        );

    mainLayout->addWidget(
        textLog
        );

    connect(
        btnConnect,
        &QPushButton::clicked,
        this,
        &MainWindow::connectDrone
        );

    connect(
        btnDisconnect,
        &QPushButton::clicked,
        this,
        &MainWindow::disconnectDrone
        );

    connect(
        btnReceive,
        &QPushButton::clicked,
        this,
        &MainWindow::receiveAltitude
        );

    connect(
        btnSaveMission,
        &QPushButton::clicked,
        this,
        &MainWindow::saveMission
        );

    connect(
        btnStatus,
        &QPushButton::clicked,
        this,
        &MainWindow::showStationStatus
        );
}

void MainWindow::connectDrone()
{
    QString drone =
        editDrone->text().trimmed();

    if (drone.isEmpty()) {

        log(
            "Error: enter drone name"
            );

        return;
    }

    if (station.is_connected()) {

        log(
            "Error: another drone is already connected"
            );

        return;
    }

    station.connect_drone(
        drone
        );

    log(
        "Connected to drone: "
        + drone
        );

    updateInfo();
}

void MainWindow::disconnectDrone()
{
    if (!station.is_connected()) {

        log(
            "Error: no drone connected"
            );

        return;
    }

    QString drone =
        station.get_drone_name();

    station.disconnect_drone();

    log(
        "Disconnected from: "
        + drone
        );

    updateInfo();
}

void MainWindow::receiveAltitude()
{
    double altitude =
        spinAltitude->value();

    if (!station.is_connected()) {

        log(
            "Telemetry rejected: no connection"
            );

        return;
    }

    if (altitude < 0.0) {

        log(
            "Telemetry rejected: invalid altitude"
            );

        return;
    }

    station.receive_altitude(
        altitude
        );

    log(
        QString(
            "Altitude received: %1 m"
            ).arg(
                altitude,
                0,
                'f',
                1
                )
        );

    updateInfo();
}

void MainWindow::saveMission()
{
    double target =
        spinTargetAltitude->value();

    double speed =
        spinMaxSpeed->value();

    double geofence =
        spinGeofence->value();

    double battery =
        spinReturnBattery->value();

    double timeout =
        spinTimeout->value();

    if (target < 0.0) {

        log(
            "Error: target altitude cannot be negative"
            );

        return;
    }

    if (speed < 0.0) {

        log(
            "Error: maximum speed cannot be negative"
            );

        return;
    }

    if (geofence <= 0.0) {

        log(
            "Error: geofence radius must be greater than zero"
            );

        return;
    }

    if (battery < 0.0 ||
        battery > 100.0) {

        log(
            "Error: return battery must be 0-100%"
            );

        return;
    }

    station.set_mission_parameter(
        "target_altitude",
        target
        );

    station.set_mission_parameter(
        "max_speed",
        speed
        );

    station.set_mission_parameter(
        "geofence_radius",
        geofence
        );

    station.set_mission_parameter(
        "return_battery",
        battery
        );

    station.set_mission_parameter(
        "mission_timeout",
        timeout
        );

    log(
        "Mission parameters saved"
        );

    updateInfo();
}

void MainWindow::showStationStatus()
{
    qDebug()
    << station;

    station.print_status();

    log(
        QString(
            "Station: %1 | Drone: %2 | Records: %3"
            )
            .arg(
                station.get_station_name()
                )
            .arg(
                station.get_drone_name()
                )
            .arg(
                station.record_count()
                )
        );
}

void MainWindow::updateInfo()
{
    if (station.is_connected()) {

        labelConnection->setText(
            "CONNECTED"
            );

        labelConnection->setStyleSheet(
            "color: green; font-weight: bold;"
            );

        labelDrone->setText(
            station.get_drone_name()
            );

    } else {

        labelConnection->setText(
            "DISCONNECTED"
            );

        labelConnection->setStyleSheet(
            "color: red; font-weight: bold;"
            );

        labelDrone->setText(
            "-"
            );
    }

    labelRecords->setText(
        QString::number(
            station.record_count()
            )
        );

    labelAverage->setText(
        QString::number(
            station.average_altitude(),
            'f',
            2
            )
        + " m"
        );

    labelCurrentAltitude->setText(
        QString::number(
            station.current_altitude(),
            'f',
            2
            )
        + " m"
        );

    double target =
        station.get_mission_parameter(
            "target_altitude"
            );

    double speed =
        station.get_mission_parameter(
            "max_speed"
            );

    double geofence =
        station.get_mission_parameter(
            "geofence_radius"
            );

    double battery =
        station.get_mission_parameter(
            "return_battery"
            );

    double timeout =
        station.get_mission_parameter(
            "mission_timeout"
            );

    labelTargetAltitude->setText(
        QString::number(
            target,
            'f',
            1
            )
        + " m"
        );

    labelMaxSpeed->setText(
        QString::number(
            speed,
            'f',
            1
            )
        + " m/s"
        );

    labelGeofence->setText(
        QString::number(
            geofence,
            'f',
            0
            )
        + " m"
        );

    labelReturnBattery->setText(
        QString::number(
            battery,
            'f',
            0
            )
        + " %"
        );

    labelTimeout->setText(
        QString::number(
            timeout,
            'f',
            0
            )
        + " s"
        );

    batteryBar->setValue(
        static_cast<int>(
            battery
            )
        );

    if (battery < 10.0) {

        batteryBar->setStyleSheet(
            "QProgressBar::chunk {"
            "background-color: red;"
            "}"
            );

    } else if (battery < 30.0) {

        batteryBar->setStyleSheet(
            "QProgressBar::chunk {"
            "background-color: orange;"
            "}"
            );

    } else {

        batteryBar->setStyleSheet(
            "QProgressBar::chunk {"
            "background-color: green;"
            "}"
            );
    }

    chart->setData(
        station.get_altitude_history(),
        target
        );

    gauge->setAltitude(
        station.current_altitude()
        );

    gauge->setTargetAltitude(
        target
        );
}

void MainWindow::log(
    const QString& message
    )
{
    textLog->appendPlainText(
        message
        );
}