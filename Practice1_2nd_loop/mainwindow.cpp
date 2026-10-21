#include "mainwindow.h"

#include "batterychart.h"

#include <QWidget>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QPlainTextEdit>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>

#include <QString>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    createInterface();

    writeLog("Program started");

    updateInfo();
}

void MainWindow::createInterface()
{
    setWindowTitle("UAV Battery Monitor");

    resize(900, 600);

    QWidget *central =
        new QWidget(this);

    setCentralWidget(central);

    spinCells =
        new QSpinBox(this);

    spinCells->setRange(1, 20);
    spinCells->setValue(4);

    spinVoltage =
        new QDoubleSpinBox(this);

    spinVoltage->setRange(-100.0, 100.0);
    spinVoltage->setDecimals(2);
    spinVoltage->setSingleStep(0.1);
    spinVoltage->setValue(15.2);
    spinVoltage->setSuffix(" V");

    spinCapacity =
        new QDoubleSpinBox(this);

    spinCapacity->setRange(0.0, 50000.0);
    spinCapacity->setDecimals(0);
    spinCapacity->setSingleStep(100.0);
    spinCapacity->setValue(5200.0);
    spinCapacity->setSuffix(" mAh");

    spinCurrent =
        new QDoubleSpinBox(this);

    spinCurrent->setRange(-100.0, 100.0);
    spinCurrent->setDecimals(2);
    spinCurrent->setSingleStep(1.0);
    spinCurrent->setValue(10.0);
    spinCurrent->setSuffix(" A");

    btnCreate =
        new QPushButton(
            "Create Battery",
            this
            );

    btnSetVoltage =
        new QPushButton(
            "Set Voltage",
            this
            );

    btnSetCurrent =
        new QPushButton(
            "Set Current",
            this
            );

    labelCharge =
        new QLabel(
            "0.00 %",
            this
            );

    labelStatus =
        new QLabel(
            "NORMAL",
            this
            );

    labelPower =
        new QLabel(
            "0.00 W",
            this
            );

    textLog =
        new QPlainTextEdit(this);

    textLog->setReadOnly(true);

    chart =
        new BatteryChart(this);

    QGroupBox *inputGroup =
        new QGroupBox(
            "Battery parameters",
            this
            );

    QFormLayout *inputLayout =
        new QFormLayout;

    inputLayout->addRow(
        "Cells:",
        spinCells
        );

    inputLayout->addRow(
        "Voltage:",
        spinVoltage
        );

    inputLayout->addRow(
        "Capacity:",
        spinCapacity
        );

    inputLayout->addRow(
        "Current:",
        spinCurrent
        );

    inputGroup->setLayout(
        inputLayout
        );

    QVBoxLayout *buttonsLayout =
        new QVBoxLayout;

    buttonsLayout->addWidget(
        btnCreate
        );

    buttonsLayout->addWidget(
        btnSetVoltage
        );

    buttonsLayout->addWidget(
        btnSetCurrent
        );

    QGroupBox *resultGroup =
        new QGroupBox(
            "Battery status",
            this
            );

    QFormLayout *resultLayout =
        new QFormLayout;

    resultLayout->addRow(
        "Charge:",
        labelCharge
        );

    resultLayout->addRow(
        "Status:",
        labelStatus
        );

    resultLayout->addRow(
        "Power:",
        labelPower
        );

    resultGroup->setLayout(
        resultLayout
        );

    QVBoxLayout *leftLayout =
        new QVBoxLayout;

    leftLayout->addWidget(
        inputGroup
        );

    leftLayout->addLayout(
        buttonsLayout
        );

    leftLayout->addWidget(
        resultGroup
        );

    leftLayout->addStretch();

    QHBoxLayout *topLayout =
        new QHBoxLayout;

    topLayout->addLayout(
        leftLayout,
        1
        );

    topLayout->addWidget(
        chart,
        2
        );

    QVBoxLayout *mainLayout =
        new QVBoxLayout(central);

    mainLayout->addLayout(
        topLayout
        );

    QLabel *logTitle =
        new QLabel(
            "Event log:",
            this
            );

    mainLayout->addWidget(
        logTitle
        );

    mainLayout->addWidget(
        textLog
        );

    connect(
        btnCreate,
        &QPushButton::clicked,
        this,
        &MainWindow::createBattery
        );

    connect(
        btnSetVoltage,
        &QPushButton::clicked,
        this,
        &MainWindow::setVoltage
        );

    connect(
        btnSetCurrent,
        &QPushButton::clicked,
        this,
        &MainWindow::setCurrent
        );
}

void MainWindow::createBattery()
{
    int cells =
        spinCells->value();

    double voltage =
        spinVoltage->value();

    double capacity =
        spinCapacity->value();

    double current =
        spinCurrent->value();

    if (voltage < 0.0) {

        writeLog(
            "Error: invalid voltage"
            );

        return;
    }

    if (current < 0.0) {

        writeLog(
            "Error: invalid current"
            );

        return;
    }

    if (current > 50.0) {

        writeLog(
            "Error: current is too high"
            );

        return;
    }

    battery = Battery(
        cells,
        voltage,
        capacity
        );

    battery.set_current(
        current
        );

    writeLog(
        "Battery created"
        );

    writeLog(
        QString("Current: %1 A")
            .arg(
                current,
                0,
                'f',
                2
                )
        );

    updateInfo();
}

void MainWindow::setVoltage()
{
    double voltage =
        spinVoltage->value();

    if (voltage < 0.0) {

        writeLog(
            "Error: invalid voltage"
            );

        return;
    }

    battery.set_voltage(
        voltage
        );

    writeLog(
        QString("Voltage updated: %1 V")
            .arg(
                voltage,
                0,
                'f',
                2
                )
        );

    updateInfo();
}

void MainWindow::setCurrent()
{
    double current =
        spinCurrent->value();

    if (current < 0.0) {

        writeLog(
            "Error: invalid current"
            );

        return;
    }

    if (current > 50.0) {

        writeLog(
            "Error: current is too high"
            );

        return;
    }

    battery.set_current(
        current
        );

    writeLog(
        QString("Current updated: %1 A")
            .arg(
                current,
                0,
                'f',
                2
                )
        );

    updateInfo();
}

void MainWindow::updateInfo()
{
    const double charge =
        battery.get_percentage();

    const double power =
        battery.get_power();

    const double current =
        battery.get_current();

    labelCharge->setText(
        QString::number(
            charge,
            'f',
            2
            )
        + " %"
        );

    labelPower->setText(
        QString::number(
            power,
            'f',
            2
            )
        + " W"
        );

    if (battery.is_low()) {

        labelStatus->setText(
            "LOW"
            );

    } else {

        labelStatus->setText(
            "NORMAL"
            );
    }

    chart->setValues(
        charge,
        current,
        power
        );
}

void MainWindow::writeLog(
    const QString &message
    )
{
    textLog->appendPlainText(
        message
        );
}