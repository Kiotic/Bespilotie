#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "ground_station.h"

class QLineEdit;
class QDoubleSpinBox;
class QPushButton;
class QLabel;
class QPlainTextEdit;
class QProgressBar;

class AltitudeChart;
class AltitudeGauge;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(
        QWidget *parent = nullptr
        );

private slots:

    void connectDrone();

    void disconnectDrone();

    void receiveAltitude();

    void saveMission();

    void showStationStatus();

private:

    GroundStation station;

    QLineEdit *editStation;
    QLineEdit *editDrone;

    QDoubleSpinBox *spinAltitude;

    QDoubleSpinBox *spinTargetAltitude;
    QDoubleSpinBox *spinMaxSpeed;
    QDoubleSpinBox *spinGeofence;
    QDoubleSpinBox *spinReturnBattery;
    QDoubleSpinBox *spinTimeout;

    QPushButton *btnConnect;
    QPushButton *btnDisconnect;
    QPushButton *btnReceive;
    QPushButton *btnSaveMission;
    QPushButton *btnStatus;

    QLabel *labelConnection;
    QLabel *labelDrone;
    QLabel *labelRecords;
    QLabel *labelAverage;
    QLabel *labelCurrentAltitude;

    QLabel *labelTargetAltitude;
    QLabel *labelMaxSpeed;
    QLabel *labelGeofence;
    QLabel *labelReturnBattery;
    QLabel *labelTimeout;

    QProgressBar *batteryBar;

    QPlainTextEdit *textLog;

    AltitudeChart *chart;
    AltitudeGauge *gauge;

    void createInterface();

    void updateInfo();

    void log(
        const QString& message
        );
};

#endif