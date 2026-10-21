#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "battery.h"

class QSpinBox;
class QDoubleSpinBox;
class QPushButton;
class QLabel;
class QPlainTextEdit;
class BatteryChart;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void createBattery();
    void setVoltage();
    void setCurrent();

private:
    Battery battery;

    QSpinBox *spinCells;

    QDoubleSpinBox *spinVoltage;
    QDoubleSpinBox *spinCapacity;
    QDoubleSpinBox *spinCurrent;

    QPushButton *btnCreate;
    QPushButton *btnSetVoltage;
    QPushButton *btnSetCurrent;

    QLabel *labelCharge;
    QLabel *labelStatus;
    QLabel *labelPower;

    QPlainTextEdit *textLog;

    BatteryChart *chart;

    void createInterface();
    void updateInfo();
    void writeLog(const QString &message);
};

#endif