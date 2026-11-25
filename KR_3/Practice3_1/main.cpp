// Практическое занятие №3. Наземная станция управления БПЛА. Вариант 1 во всех заданиях.
#include <QApplication>
#include "mainwindow.h"

static const char* kStyle = R"(
QWidget { background: #0b0f19; color: #e2e8f5; font-size: 13px; }
QGroupBox { background: #141b2a; border: 1px solid rgba(255,255,255,22); border-radius: 12px;
            margin-top: 16px; padding: 14px 10px 10px 10px; font-weight: 700; }
QGroupBox::title { subcontrol-origin: margin; left: 14px; padding: 0 6px; color: #8492af; }
QGroupBox QLabel, QGroupBox QCheckBox { background: transparent; }
QLabel#labelTitle { font-size: 22px; font-weight: 900; letter-spacing: 2px; color: #3ad0ff; }
QLabel#labelSubtitle { color: #8492af; padding-left: 12px; }
QLabel#labelMissionTime { font-family: Consolas, "DejaVu Sans Mono", monospace; font-size: 16px;
                          color: #e2e8f5; padding-right: 12px; }
QLabel#labelAltitude, QLabel#labelError, QLabel#labelOutput, QLabel#labelSpeed {
    font-family: Consolas, "DejaVu Sans Mono", monospace; font-size: 22px; font-weight: 800; }
QLabel#labelAltitude { color: #3ad0ff; }
QLabel#labelError    { color: #ffb648; }
QLabel#labelOutput   { color: #b18cff; }
QLabel#labelSpeed    { color: #e2e8f5; }
QPushButton { background: #1c2539; border: 1px solid rgba(255,255,255,30); border-radius: 9px;
              padding: 9px 12px; font-weight: 700; }
QPushButton:hover { background: #26324d; border-color: #3ad0ff; }
QPushButton:pressed { background: #10182a; }
QPushButton#btnStart { background: #1677ff; border: none; }
QPushButton#btnStart:hover { background: #3a8dff; }
QPushButton#btnPause { background: #8a5a12; border: none; }
QPushButton#btnFaultSensor, QPushButton#btnFaultGps { background: #4a1d27; border: 1px solid #ff5c6c; }
QPushButton#btnReset { background: #17643f; border: none; }
QDoubleSpinBox { background: #1c2539; border: 1px solid rgba(255,255,255,30); border-radius: 8px;
                 padding: 5px 8px; font-size: 15px; font-weight: 700; }
QProgressBar { background: #10182a; border: 1px solid rgba(255,255,255,30); border-radius: 7px;
               text-align: center; height: 18px; }
QProgressBar::chunk { background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #1f9d61, stop:1 #3ee089);
                      border-radius: 6px; }
QTableWidget { background: #10182a; border: none; gridline-color: rgba(255,255,255,20); border-radius: 8px; }
QHeaderView::section { background: #1c2539; color: #8492af; border: none; padding: 6px; font-weight: 700; }
QTableCornerButton::section { background: #1c2539; border: none; }
QListWidget { background: #070a12; border: none; border-radius: 8px;
              font-family: Consolas, "DejaVu Sans Mono", monospace; font-size: 12px; }
QGraphicsView { border: none; border-radius: 8px; }
QCheckBox::indicator { width: 16px; height: 16px; border-radius: 4px; border: 1px solid #3a4766; background: #10182a; }
QCheckBox::indicator:checked { background: #3ad0ff; border-color: #3ad0ff; }
QMessageBox { background: #141b2a; }
)";

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setStyleSheet(kStyle);

    MainWindow window;
    window.showMaximized();

    return app.exec();          // цикл событий — никаких while(true)
}
