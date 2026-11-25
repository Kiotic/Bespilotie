#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QColor>

#ifdef HAVE_QT_CHARTS
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
QT_CHARTS_USE_NAMESPACE
#endif
#endif

class TelemetryModel;
class MavlinkManager;
class UavSimulator;
class SoundEngine;
class SignalBars;
class FallbackChart;
class QTimer;
class QLabel;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onStartLinkClicked();
    void onStopLinkClicked();
    void onTakeoffClicked();
    void onLandClicked();
    void onSimStartClicked();
    void onDisconnectUavClicked();

    void onTelemetryChanged(double latitude, double longitude, double altitude,
                            double verticalSpeed, const QString &flightMode);
    void onFlightModeChanged(const QString &flightMode);
    void onLinkStateChanged(bool connected, const QString &address, quint16 port);
    void onLinkLost();
    void onLinkQualityChanged(double packetsPerSecond, int quality);
    void onPacketReceived(quint64 received, quint64 lost);
    void onPacketError(quint64 errors, const QString &reason);
    void onUavStatusChanged(bool healthy, const QString &status);
    void onCommandSent(int command, double parameter);
    void onCommandResult(int command, bool accepted, const QString &message);
    void onMavlinkError(const QString &message);
    void onSimulatorRunningChanged(bool running);
    void onSimulatorCommand(int command, double parameter, bool ackSent);
    void updateAckProgress();

private:
    void setupChart();
    void setupConnections();
    void addEvent(const QString &type, const QString &text, const QColor &color);
    void addCommandRecord(const QString &text, const QColor &color);
    void setCommandsEnabled(bool enabled);
    void setPill(QLabel *label, const QString &text, const QString &color);

    Ui::MainWindow *ui = nullptr;
    TelemetryModel *m_telemetryModel = nullptr;
    MavlinkManager *m_mavlinkManager = nullptr;
    UavSimulator *m_uavSimulator = nullptr;
    SoundEngine *m_sound = nullptr;
    SignalBars *m_signalBars = nullptr;
    QTimer *m_uiTimer = nullptr;

#ifdef HAVE_QT_CHARTS
    QChart *m_chart = nullptr;
    QLineSeries *m_altitudeSeries = nullptr;
    QLineSeries *m_speedSeries = nullptr;
    QValueAxis *m_axisTime = nullptr;
    QValueAxis *m_axisAltitude = nullptr;
    QValueAxis *m_axisSpeed = nullptr;
#else
    FallbackChart *m_fallbackChart = nullptr;
#endif

    double m_chartTime = 0.0;
    double m_maxAltitude = 20.0;
    quint64 m_lastLostCount = 0;
    bool m_wasConnected = false;
    bool m_linkLossDialogShown = false;
};

#endif
