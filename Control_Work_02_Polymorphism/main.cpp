

#include <QApplication>
#include <QCoreApplication>
#include <QDebug>
#include <QFont>
#include <QTimer>
#include <cstdio>

#include "tasks.h"
#include "dashboard.h"

static QtMessageHandler g_previous_handler = nullptr;

// Перехватываем qDebug(): дублируем в окно журнала и в стандартный вывод
static void message_handler(QtMsgType type, const QMessageLogContext& ctx, const QString& msg)
{
    if (type == QtDebugMsg)
        emit LogBus::instance().message(msg);
    if (g_previous_handler)
        g_previous_handler(type, ctx, msg);
    else
        fprintf(stderr, "%s\n", msg.toLocal8Bit().constData());
}

static const char* kStyle = R"(
QWidget { background: #0b0f19; color: #e2e8f5; font-size: 12px; }
QLabel#title { font-size: 20px; font-weight: 800; letter-spacing: 1px; }
QLabel#muted { color: #8492af; }
QLabel#section { color: #8492af; font-size: 10px; font-weight: 700; letter-spacing: 1px; padding-top: 4px; }
QFrame#card { background: #141b2a; border: 1px solid rgba(255,255,255,18); border-radius: 10px; }
QFrame#card QLabel, QFrame#card QCheckBox { background: transparent; }
QLabel#tileTitle { color: #8492af; font-size: 10px; font-weight: 700; letter-spacing: 1px; }
QLabel#tileValue { font-size: 22px; font-weight: 800; }
QLabel#metrics { background: #141b2a; border: 1px solid rgba(255,255,255,18); border-radius: 10px; padding: 8px; }
QPushButton { background: #1c2539; border: 1px solid rgba(255,255,255,30); border-radius: 8px;
              padding: 7px 10px; color: #e2e8f5; }
QPushButton:hover { background: #26324d; border-color: #3ad0ff; }
QPushButton:pressed { background: #10182a; }
QPushButton#primary { background: #1677ff; border: none; font-weight: 700; }
QPushButton#primary:hover { background: #3a8dff; }
QPushButton#scenario { text-align: left; }
QComboBox { background: #1c2539; border: 1px solid rgba(255,255,255,30); border-radius: 8px; padding: 5px 8px; }
QComboBox QAbstractItemView { background: #1c2539; selection-background-color: #1677ff; }
QSlider::groove:horizontal { height: 6px; background: #26324d; border-radius: 3px; }
QSlider::sub-page:horizontal { background: #3ee089; border-radius: 3px; }
QSlider::handle:horizontal { background: #e2e8f5; width: 16px; margin: -6px 0; border-radius: 8px; }
QCheckBox::indicator { width: 16px; height: 16px; border-radius: 4px; border: 1px solid #3a4766; background: #10182a; }
QCheckBox::indicator:checked { background: #ff5c6c; border-color: #ff5c6c; }
QTabWidget::pane { border: none; }
QTabBar::tab { background: #141b2a; color: #8492af; padding: 8px 16px; margin-right: 4px;
               border-top-left-radius: 8px; border-top-right-radius: 8px; }
QTabBar::tab:selected { background: #1c2539; color: #e2e8f5; }
QPlainTextEdit#log { background: #070a12; border: 1px solid rgba(255,255,255,18); border-radius: 10px;
                     font-family: Consolas, "DejaVu Sans Mono", "Courier New", monospace; font-size: 12px; padding: 6px; }
QScrollBar:vertical { background: transparent; width: 8px; }
QScrollBar::handle:vertical { background: #26324d; border-radius: 4px; min-height: 30px; }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
)";

int main(int argc, char* argv[])
{
    bool console_only = false;
    for (int i = 1; i < argc; ++i)
        if (QString(argv[i]) == "--console") console_only = true;

    if (console_only) {
        QCoreApplication app(argc, argv);
        run_all_console_tasks();
        return 0;
    }

    QApplication app(argc, argv);
    app.setStyleSheet(kStyle);
    LogBus::instance();                       // создаём в главном потоке
    g_previous_handler = qInstallMessageHandler(message_handler);

    Dashboard window;
    window.show();

    // консольная часть (задания 1–3 и 5 сценариев) — в Application Output и в журнал окна
    QTimer::singleShot(0, [] { run_all_console_tasks(); });

    return app.exec();
}
