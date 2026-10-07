#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QPointF>
#include <QString>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class QGraphicsScene;
class QGraphicsEllipseItem;
class QGraphicsLineItem;
class QTimer;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    // Состояния программы
    enum Mode {
        READY,    // Ожидание старта
        MISSION,  // Полёт по маршруту
        RTL,      // Возврат на базу
        LANDED    // Посадка завершена
    };

    Mode mode = READY;

    // Графические объекты
    QGraphicsScene *scene = nullptr;
    QGraphicsEllipseItem *uav = nullptr;
    QGraphicsLineItem *rtlLine = nullptr;

    // Таймер движения
    QTimer *timer = nullptr;

    // Маршрут и положение БПЛА
    QList<QPointF> waypoints;
    QPointF base;
    QPointF uavPos;

    int wpIndex = 1;

    void startMission();
    void loseLink();
    void resetSimulation();
    void tick();

    void log(const QString &message);
};

#endif // MAINWINDOW_H