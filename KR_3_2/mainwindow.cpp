#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "telemetrymodel.h"
#include "mavlinkmanager.h"
#include "mavlinkmessageparser.h"
#include "uavsimulator.h"
#include "soundengine.h"
#include "signalbars.h"
#include "fallbackchart.h"

#include <QTimer>
#include <QTime>
#include <QMessageBox>
#include <QStatusBar>
#include <QPainter>
#include <QListWidgetItem>
#include <QVBoxLayout>
#include <algorithm>

namespace {
const quint16 kListenPort = 14540;
const QColor kGreen(62, 224, 137);
const QColor kYellow(255, 196, 72);
const QColor kRed(255, 92, 108);
const QColor kBlue(58, 208, 255);
const QColor kGray(150, 160, 185);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_telemetryModel = new TelemetryModel(this);
    m_mavlinkManager = new MavlinkManager(m_telemetryModel, this);
    m_uavSimulator = new UavSimulator(this);
    m_sound = new SoundEngine(this);

    m_signalBars = new SignalBars(this);
    ui->layoutHeader->insertWidget(ui->layoutHeader->indexOf(ui->labelQuality), m_signalBars);

    m_uiTimer = new QTimer(this);
    m_uiTimer->setInterval(50);

    setupChart();
    setupConnections();

    setPill(ui->labelLinkStatus, "Связь отсутствует", "#d83a4f");
    ui->btnStopLink->setEnabled(false);
    ui->btnDisconnectUav->setEnabled(false);
    setCommandsEnabled(false);
    addEvent("НСУ", "Станция запущена, UDP-порт не открыт", kGray);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupChart()
{
    auto *layout = new QVBoxLayout(ui->chartContainer);
    layout->setContentsMargins(0, 0, 0, 0);

#ifdef HAVE_QT_CHARTS
    m_altitudeSeries = new QLineSeries();
    m_altitudeSeries->setName("Altitude, м");
    m_altitudeSeries->setPen(QPen(kBlue, 2.5));

    m_speedSeries = new QLineSeries();
    m_speedSeries->setName("Vertical speed, м/с");
    m_speedSeries->setPen(QPen(kYellow, 2.0));

    m_chart = new QChart();
    m_chart->setTheme(QChart::ChartThemeDark);
    m_chart->setBackgroundBrush(QColor(20, 27, 42));
    m_chart->setTitle("Высота БПЛА");
    m_chart->addSeries(m_altitudeSeries);
    m_chart->addSeries(m_speedSeries);
    m_chart->legend()->setVisible(true);
    m_chart->legend()->setAlignment(Qt::AlignTop);

    m_axisTime = new QValueAxis();
    m_axisTime->setTitleText("Время, с");
    m_axisTime->setRange(0, 30);
    m_axisTime->setLabelFormat("%.0f");

    m_axisAltitude = new QValueAxis();
    m_axisAltitude->setTitleText("Высота, м");
    m_axisAltitude->setRange(0, m_maxAltitude);
    m_axisAltitude->setLabelFormat("%.0f");

    m_axisSpeed = new QValueAxis();
    m_axisSpeed->setTitleText("Скорость, м/с");
    m_axisSpeed->setRange(-5, 5);
    m_axisSpeed->setLabelFormat("%.1f");
    m_axisSpeed->setGridLineVisible(false);

    m_chart->addAxis(m_axisTime, Qt::AlignBottom);
    m_chart->addAxis(m_axisAltitude, Qt::AlignLeft);
    m_chart->addAxis(m_axisSpeed, Qt::AlignRight);
    m_altitudeSeries->attachAxis(m_axisTime);
    m_altitudeSeries->attachAxis(m_axisAltitude);
    m_speedSeries->attachAxis(m_axisTime);
    m_speedSeries->attachAxis(m_axisSpeed);

    auto *chartView = new QChartView(m_chart, ui->chartContainer);
    chartView->setObjectName("chartView");
    chartView->setRenderHint(QPainter::Antialiasing);
    layout->addWidget(chartView);
#else
    m_fallbackChart = new FallbackChart(ui->chartContainer);
    layout->addWidget(m_fallbackChart);
#endif
}

void MainWindow::setupConnections()
{
    connect(m_mavlinkManager, &MavlinkManager::telemetryReceived,
            m_telemetryModel, &TelemetryModel::updateTelemetry);
    connect(m_telemetryModel, &TelemetryModel::telemetryChanged,
            this, &MainWindow::onTelemetryChanged);
    connect(m_telemetryModel, &TelemetryModel::flightModeChanged,
            this, &MainWindow::onFlightModeChanged);
    connect(m_mavlinkManager, &MavlinkManager::linkStateChanged,
            m_telemetryModel, &TelemetryModel::updateLink);
    connect(m_telemetryModel, &TelemetryModel::linkStateChanged,
            this, &MainWindow::onLinkStateChanged);

    connect(m_mavlinkManager, &MavlinkManager::linkLost, this, &MainWindow::onLinkLost);
    connect(m_mavlinkManager, &MavlinkManager::linkQualityChanged, this, &MainWindow::onLinkQualityChanged);
    connect(m_mavlinkManager, &MavlinkManager::packetReceived, this, &MainWindow::onPacketReceived);
    connect(m_mavlinkManager, &MavlinkManager::packetError, this, &MainWindow::onPacketError);
    connect(m_mavlinkManager, &MavlinkManager::uavStatusChanged, this, &MainWindow::onUavStatusChanged);
    connect(m_mavlinkManager, &MavlinkManager::commandSent, this, &MainWindow::onCommandSent);
    connect(m_mavlinkManager, &MavlinkManager::commandResult, this, &MainWindow::onCommandResult);
    connect(m_mavlinkManager, &MavlinkManager::errorOccurred, this, &MainWindow::onMavlinkError);

    connect(m_uavSimulator, &UavSimulator::runningChanged, this, &MainWindow::onSimulatorRunningChanged);
    connect(m_uavSimulator, &UavSimulator::commandReceived, this, &MainWindow::onSimulatorCommand);

    connect(ui->btnStartLink, &QPushButton::clicked, this, &MainWindow::onStartLinkClicked);
    connect(ui->btnStopLink, &QPushButton::clicked, this, &MainWindow::onStopLinkClicked);
    connect(ui->btnTakeoff, &QPushButton::clicked, this, &MainWindow::onTakeoffClicked);
    connect(ui->btnLand, &QPushButton::clicked, this, &MainWindow::onLandClicked);
    connect(ui->btnSimStart, &QPushButton::clicked, this, &MainWindow::onSimStartClicked);
    connect(ui->btnDisconnectUav, &QPushButton::clicked, this, &MainWindow::onDisconnectUavClicked);

    connect(ui->btnCorruptPacket, &QPushButton::clicked, this, [this]() {
        m_uavSimulator->corruptNextPacket();
        addEvent("ЭМУЛЯТОР", "Следующий пакет будет повреждён", kYellow);
    });
    connect(ui->btnSkipPackets, &QPushButton::clicked, this, [this]() {
        m_uavSimulator->skipPackets(2);
        addEvent("ЭМУЛЯТОР", "Следующие 2 пакета не будут отправлены", kYellow);
    });
    connect(ui->btnSensorFault, &QPushButton::toggled, this, [this](bool failed) {
        m_uavSimulator->setSensorFailure(failed);
        ui->btnSensorFault->setText(failed ? "Восстановить сенсор" : "Отказ сенсора");
        addEvent("ЭМУЛЯТОР", failed ? "Имитация отказа сенсора" : "Сенсор восстановлен", kYellow);
    });
    connect(ui->checkAckEnabled, &QCheckBox::toggled, this, [this](bool enabled) {
        m_uavSimulator->setAckEnabled(enabled);
        addEvent("ЭМУЛЯТОР", enabled ? "COMMAND_ACK включён" : "COMMAND_ACK отключён", kYellow);
    });
    connect(ui->checkSound, &QCheckBox::toggled, m_sound, &SoundEngine::setEnabled);

    connect(m_uiTimer, &QTimer::timeout, this, &MainWindow::updateAckProgress);
}

void MainWindow::onStartLinkClicked()
{
    if (m_mavlinkManager->start(kListenPort)) {
        ui->btnStartLink->setEnabled(false);
        ui->btnStopLink->setEnabled(true);
        setPill(ui->labelLinkStatus, "Ожидание БПЛА…", "#8a6a1e");
        addEvent("СВЯЗЬ", QString("UDP-порт %1 открыт").arg(kListenPort), kGreen);
    }
}

void MainWindow::onStopLinkClicked()
{
    m_mavlinkManager->stop();
    m_wasConnected = false;
    ui->btnStartLink->setEnabled(true);
    ui->btnStopLink->setEnabled(false);
    addEvent("СВЯЗЬ", "UDP-порт закрыт оператором", kGray);
}

void MainWindow::onTakeoffClicked()
{
    if (!m_telemetryModel->isLinkConnected()) {
        QMessageBox::warning(this, "Команда недоступна",
                             "Невозможно отправить команду взлёта: нет связи с БПЛА.");
        return;
    }
    m_mavlinkManager->sendTakeoff(ui->spinTakeoffAltitude->value());
}

void MainWindow::onLandClicked()
{
    if (!m_telemetryModel->isLinkConnected()) {
        QMessageBox::warning(this, "Команда недоступна",
                             "Невозможно отправить команду посадки: нет связи с БПЛА.");
        return;
    }
    m_mavlinkManager->sendLand();
}

void MainWindow::onSimStartClicked()
{
    m_uavSimulator->start("127.0.0.1", kListenPort);
}

void MainWindow::onDisconnectUavClicked()
{
    m_uavSimulator->stop();
}

void MainWindow::onTelemetryChanged(double latitude, double longitude, double altitude,
                                    double verticalSpeed, const QString &flightMode)
{
    ui->labelLatitude->setText(QString::number(latitude, 'f', 7));
    ui->labelLongitude->setText(QString::number(longitude, 'f', 7));
    ui->labelAltitude->setText(QString::number(altitude, 'f', 1) + " м");
    ui->labelVerticalSpeed->setText(QString::number(verticalSpeed, 'f', 2) + " м/с");
    ui->labelFlightMode->setText(flightMode);

    m_maxAltitude = std::max(m_maxAltitude, altitude * 1.15);

#ifdef HAVE_QT_CHARTS
    m_altitudeSeries->append(m_chartTime, altitude);
    m_speedSeries->append(m_chartTime, verticalSpeed);
    if (m_altitudeSeries->count() > 300) {
        m_altitudeSeries->remove(0);
        m_speedSeries->remove(0);
    }
    m_axisTime->setRange(std::max(0.0, m_chartTime - 30.0), std::max(30.0, m_chartTime));
    m_axisAltitude->setRange(0, m_maxAltitude);
#else
    m_fallbackChart->addPoint(m_chartTime, altitude, verticalSpeed);
#endif
    m_chartTime += 0.1;
}

void MainWindow::onFlightModeChanged(const QString &flightMode)
{
    addEvent("БПЛА", "Режим полёта: " + flightMode, kBlue);
}

void MainWindow::onLinkStateChanged(bool connected, const QString &address, quint16 port)
{
    setCommandsEnabled(connected);

    if (connected) {
        setPill(ui->labelLinkStatus, "Связь установлена", "#1f9d61");
        ui->labelSender->setText(address + ":" + QString::number(port));
        addEvent("СВЯЗЬ", m_wasConnected ? "Связь восстановлена" : "Связь установлена с " + address + ":" + QString::number(port), kGreen);
        m_sound->playConnected();
        m_wasConnected = true;
    } else if (!m_mavlinkManager->isListening()) {
        setPill(ui->labelLinkStatus, "Связь отсутствует", "#d83a4f");
        ui->labelSender->setText("—");
    }
}

void MainWindow::onLinkLost()
{
    const QString lastPacket = m_mavlinkManager->lastPacketTime().toString("hh:mm:ss");
    setPill(ui->labelLinkStatus, "Связь потеряна", "#d83a4f");

    addEvent("СВЯЗЬ", QString("Связь потеряна. Последний пакет: %1, потеряно пакетов: %2")
                          .arg(lastPacket)
                          .arg(m_mavlinkManager->lostPackets()), kRed);
    m_sound->playAlarm();

    if (!m_linkLossDialogShown) {
        m_linkLossDialogShown = true;
        QMessageBox::warning(this, "Связь потеряна",
                             QString("Связь с БПЛА потеряна.\n\nПоследний пакет: %1\nПотеряно пакетов: %2\n\n"
                                     "Команды заблокированы до восстановления связи.")
                                 .arg(lastPacket)
                                 .arg(m_mavlinkManager->lostPackets()));
    }
}

void MainWindow::onLinkQualityChanged(double packetsPerSecond, int quality)
{
    QColor color = kRed;
    int bars = packetsPerSecond > 0 ? 1 : 0;
    if (quality == MavlinkManager::QualityGood) {
        color = kGreen;
        bars = 4;
    } else if (quality == MavlinkManager::QualityWeak) {
        color = kYellow;
        bars = 2;
    }
    m_signalBars->setLevel(bars, color);
    ui->labelQuality->setText(QString("%1 пакет/с").arg(packetsPerSecond, 0, 'f', 1));
    ui->labelQuality->setStyleSheet(QString("color: %1; font-weight: bold;").arg(color.name()));
}

void MainWindow::onPacketReceived(quint64 received, quint64 lost)
{
    ui->labelPackets->setText(QString("Принято: %1 | Потеряно: %2").arg(received).arg(lost));
    ui->labelLastPacket->setText(m_mavlinkManager->lastPacketTime().toString("hh:mm:ss.zzz"));

    if (lost > m_lastLostCount) {
        addEvent("ПАКЕТ", QString("Потеря пакетов: %1").arg(lost - m_lastLostCount), kYellow);
        m_lastLostCount = lost;
    }
}

void MainWindow::onPacketError(quint64 errors, const QString &reason)
{
    ui->labelErrors->setText(QString::number(errors));
    ui->labelErrors->setStyleSheet("color: #ff5c6c; font-weight: bold;");
    addEvent("ПАКЕТ", "Пакет отброшен: " + reason, kRed);
    m_sound->playWarning();
}

void MainWindow::onUavStatusChanged(bool healthy, const QString &status)
{
    ui->labelUavStatus->setText(status);
    ui->labelUavStatus->setStyleSheet(healthy ? "color: #3ee089; font-weight: bold;"
                                              : "color: #ff5c6c; font-weight: bold;");
    if (!healthy) {
        addEvent("БПЛА", "Отказ БПЛА: состояние " + status, kRed);
        m_sound->playAlarm();
    } else {
        addEvent("БПЛА", "Состояние: " + status, kGray);
    }
}

void MainWindow::onCommandSent(int command, double parameter)
{
    QString text = MavlinkMessageParser::commandName(command);
    if (command == Mavlink::CMD_NAV_TAKEOFF) {
        text += QString(", высота %1 м").arg(parameter, 0, 'f', 0);
    }
    setPill(ui->labelCommandStatus, "WAITING_ACK: ожидание подтверждения…", "#8a6a1e");
    addCommandRecord("Отправлена " + text, kYellow);
    addEvent("КОМАНДА", "Отправлена " + text, kYellow);
    ui->progressAck->setValue(ui->progressAck->maximum());
    m_uiTimer->start();
    m_sound->playClick();
}

void MainWindow::onCommandResult(int command, bool accepted, const QString &message)
{
    m_uiTimer->stop();
    ui->progressAck->setValue(0);

    setPill(ui->labelCommandStatus, "Статус: " + message, accepted ? "#1f9d61" : "#d83a4f");

    const QString record = QString("COMMAND %1 (%2): %3")
                               .arg(command)
                               .arg(MavlinkMessageParser::commandName(command))
                               .arg(message);
    addCommandRecord(record, accepted ? kGreen : kRed);
    addEvent("КОМАНДА", record, accepted ? kGreen : kRed);

    if (accepted) {
        m_sound->playConnected();
    } else {
        m_sound->playWarning();
    }
}

void MainWindow::onMavlinkError(const QString &message)
{
    addEvent("ОШИБКА", message, kRed);
    statusBar()->showMessage("Ошибка: " + message, 5000);
    m_sound->playWarning();
}

void MainWindow::onSimulatorRunningChanged(bool running)
{
    ui->btnSimStart->setEnabled(!running);
    ui->btnDisconnectUav->setEnabled(running);
    setPill(ui->labelSimState,
            running ? QString("Передаёт телеметрию 10 Гц → 127.0.0.1:%1").arg(kListenPort)
                    : QString("Остановлен"),
            running ? "#1f9d61" : "#3a4766");
    addEvent("ЭМУЛЯТОР", running ? "БПЛА запущен" : "БПЛА отключён", running ? kGreen : kYellow);
}

void MainWindow::onSimulatorCommand(int command, double parameter, bool ackSent)
{
    Q_UNUSED(parameter);
    addCommandRecord(QString("БПЛА получил %1, COMMAND_ACK %2")
                         .arg(MavlinkMessageParser::commandName(command))
                         .arg(ackSent ? "отправлен" : "НЕ отправлен"),
                     kGray);
}

void MainWindow::updateAckProgress()
{
    ui->progressAck->setValue(m_mavlinkManager->ackRemainingMs());
    if (!m_mavlinkManager->isWaitingAck()) {
        m_uiTimer->stop();
    }
}

void MainWindow::addEvent(const QString &type, const QString &text, const QColor &color)
{
    const QString line = QString("[%1] [%2] %3")
                             .arg(QTime::currentTime().toString("hh:mm:ss"))
                             .arg(type, -8)
                             .arg(text);
    auto *item = new QListWidgetItem(line);
    item->setForeground(color);
    ui->listEventLog->addItem(item);
    if (ui->listEventLog->count() > 500) {
        delete ui->listEventLog->takeItem(0);
    }
    ui->listEventLog->scrollToBottom();
}

void MainWindow::addCommandRecord(const QString &text, const QColor &color)
{
    auto *item = new QListWidgetItem(QString("[%1] %2")
                                         .arg(QTime::currentTime().toString("hh:mm:ss"))
                                         .arg(text));
    item->setForeground(color);
    ui->listCommandLog->insertItem(0, item);
}

void MainWindow::setCommandsEnabled(bool enabled)
{
    ui->btnTakeoff->setEnabled(enabled);
    ui->btnLand->setEnabled(enabled);
}

void MainWindow::setPill(QLabel *label, const QString &text, const QString &color)
{
    label->setText(text);
    label->setStyleSheet(QString("background: %1; color: white; font-weight: bold; "
                                 "border-radius: 10px; padding: 5px 14px;").arg(color));
}
