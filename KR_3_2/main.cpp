#include <QApplication>
#include "mainwindow.h"

static const char *kStyleSheet = R"(
QWidget { background: #0b0f19; color: #e2e8f5; font-size: 13px; }
QGroupBox { background: #141b2a; border: 1px solid #252f45; border-radius: 12px;
            margin-top: 16px; padding: 14px 10px 10px 10px; font-weight: bold; }
QGroupBox::title { subcontrol-origin: margin; left: 14px; padding: 0 6px; color: #8492af; }
QGroupBox QLabel, QGroupBox QCheckBox { background: transparent; }
QLabel#labelTitle { font-size: 22px; font-weight: 900; color: #3ad0ff; letter-spacing: 2px; }
QLabel#labelSubtitle { color: #8492af; padding-left: 10px; }
QLabel#labelQuality { font-size: 15px; padding: 0 12px 0 6px; }
QLabel#labelLatitude, QLabel#labelLongitude, QLabel#labelAltitude,
QLabel#labelVerticalSpeed, QLabel#labelFlightMode {
    font-family: Consolas, "DejaVu Sans Mono", monospace; font-size: 17px; font-weight: bold; }
QLabel#labelAltitude { color: #3ad0ff; }
QLabel#labelVerticalSpeed { color: #ffc448; }
QPushButton { background: #1c2539; border: 1px solid #2f3b57; border-radius: 9px;
              padding: 8px 12px; font-weight: bold; }
QPushButton:hover { background: #26324d; border-color: #3ad0ff; }
QPushButton:disabled { color: #4d5a78; background: #121826; border-color: #1c2539; }
QPushButton#btnStartLink, QPushButton#btnSimStart { background: #1677ff; border: none; }
QPushButton#btnTakeoff { background: #17643f; border: none; }
QPushButton#btnLand { background: #8a5a12; border: none; }
QPushButton#btnDisconnectUav, QPushButton#btnCorruptPacket,
QPushButton#btnSkipPackets, QPushButton#btnSensorFault { background: #3a1d27; border: 1px solid #ff5c6c; }
QPushButton#btnSensorFault:checked { background: #d83a4f; }
QPushButton:disabled { background: #121826; border: 1px solid #1c2539; }
QDoubleSpinBox { background: #1c2539; border: 1px solid #2f3b57; border-radius: 8px;
                 padding: 5px 8px; font-size: 15px; font-weight: bold; }
QProgressBar { background: #10182a; border: none; border-radius: 4px; max-height: 8px; }
QProgressBar::chunk { background: #ffc448; border-radius: 4px; }
QListWidget { background: #070a12; border: none; border-radius: 8px;
              font-family: Consolas, "DejaVu Sans Mono", monospace; font-size: 12px; }
QCheckBox::indicator { width: 16px; height: 16px; border-radius: 4px;
                       border: 1px solid #3a4766; background: #10182a; }
QCheckBox::indicator:checked { background: #3ad0ff; border-color: #3ad0ff; }
QStatusBar { color: #ff5c6c; }
)";

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setStyleSheet(kStyleSheet);

    MainWindow window;
    window.showMaximized();

    return app.exec();
}
