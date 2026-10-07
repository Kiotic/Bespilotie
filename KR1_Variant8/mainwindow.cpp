#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QTimer>
#include <QLineF>
#include <QPen>
#include <QBrush>
#include <QColor>
#include <QPainter>
#include <QTime>
#include <QPushButton>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    ui->setupUi(this);

    //сцена камера мотор
    scene = new QGraphicsScene(0, 0, 600, 400, this);

    ui->graphicsView->setScene(scene);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);
    ui->graphicsView->setBackgroundBrush(QBrush(Qt::white));


    base = QPointF(300, 200);

    waypoints = {
        QPointF(80, 70),
        QPointF(520, 70),
        QPointF(520, 330),
        QPointF(80, 330)
    };

    for (int i = 0; i < 4; ++i) {
        const QPointF a = waypoints[i];
        const QPointF b = waypoints[(i + 1) % 4];

        scene->addLine(
            QLineF(a, b),
            QPen(Qt::lightGray, 2)
            );
    }

    for (int i = 0; i < 4; ++i) {
        const QPointF p = waypoints[i];

        scene->addEllipse(
            p.x() - 5,
            p.y() - 5,
            10,
            10,
            QPen(Qt::blue),
            QBrush(Qt::blue)
            );

        auto *label = scene->addText(
            QString("Т%1").arg(i + 1)
            );

        label->setPos(
            p.x() + 8,
            p.y() - 24
            );
    }


    scene->addRect(
        base.x() - 12,
        base.y() - 12,
        24,
        24,
        QPen(Qt::darkGreen, 2),
        QBrush(Qt::green)
        );

    auto *baseLabel = scene->addText("БАЗА");

    baseLabel->setPos(
        base.x() + 16,
        base.y() - 14
        );

    uav = scene->addEllipse(
        -8,
        -8,
        16,
        16,
        QPen(Qt::black, 2),
        QBrush(Qt::green)
        );

    // Круг отображается поверх маршрута и базы.
    uav->setZValue(10);

    timer = new QTimer(this);
    timer->setInterval(50);

    connect(
        timer,
        &QTimer::timeout,
        this,
        &MainWindow::tick
        );

    //Кнопка запуска.
    connect(
        ui->btnStart,
        &QPushButton::clicked,
        this,
        &MainWindow::startMission
        );

    connect(
        ui->btnLostLink,
        &QPushButton::clicked,
        this,
        &MainWindow::loseLink
        );

    // Кнопка сброса.
    connect(
        ui->btnReset,
        &QPushButton::clicked,
        this,
        [this]() {
            resetSimulation();
            QApplication::beep();
        }
        );

    // Пользователь не может вручную изменять журнал.
    ui->textLog->setReadOnly(true);

    // Начальное состояние, без звукового сигнала.
    resetSimulation();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Добавляем сообщение со временем в журнал.
void MainWindow::log(const QString &message)
{
    ui->textLog->append(
        QTime::currentTime().toString("HH:mm:ss")
        + " | "
        + message
        );
}

// Запуск полёта по маршруту.
void MainWindow::startMission()
{
    // Запуск возможен только из состояния ожидания.
    if (mode != READY)
        return;

    mode = MISSION;

    // Обычный полёт — зелёный цвет.
    uav->setBrush(QBrush(Qt::green));

    ui->labelStatus->setText("Статус: ПОЛЁТ");
    ui->labelLink->setText("Связь: есть");

    // Повторный старт запрещён.
    ui->btnStart->setEnabled(false);

    ui->btnLostLink->setEnabled(true);

    log("Миссия запущена. Полёт к Т2.");

    QApplication::beep();

    timer->start();
}

// Имитация потери связи и переход в RTL.
void MainWindow::loseLink()
{
    // Обрабатываем событие только во время миссии
    if (mode != MISSION)
        return;

    mode = RTL;


    uav->setBrush(QBrush(Qt::yellow));

    // Пунктир от места потери связи до базы.
    rtlLine = scene->addLine(
        QLineF(uavPos, base),
        QPen(QColor(190, 140, 0), 2, Qt::DashLine)
        );

    // Пунктир выше маршрута, но ниже БПЛА.
    rtlLine->setZValue(1);

    ui->labelStatus->setText("Статус: ВОЗВРАТ");
    ui->labelLink->setText("Связь: потеряна");

    // Повторная потеря связи не обрабатывается.
    ui->btnLostLink->setEnabled(false);

    log("Потеря связи! Переход в режим RTL.");

    QApplication::beep();
}

// Возврат программы в исходное состояние.
void MainWindow::resetSimulation()
{
    timer->stop();

    // Удаляем только пунктир возврата.
    if (rtlLine != nullptr) {
        scene->removeItem(rtlLine);
        delete rtlLine;
        rtlLine = nullptr;
    }

    mode = READY;

    uavPos = waypoints[0];

    wpIndex = 1;

    uav->setPos(uavPos);
    uav->setBrush(QBrush(Qt::green));

    ui->labelStatus->setText("Статус: ГОТОВ");
    ui->labelLink->setText("Связь: есть");

    ui->btnStart->setEnabled(true);
    ui->btnLostLink->setEnabled(false);

    ui->textLog->clear();
    log("Сброс. БПЛА в Т1. Ожидание старта.");

}

void MainWindow::tick()
{
    if (mode != MISSION && mode != RTL)
        return;


    const QPointF target =
        (mode == MISSION) ? waypoints[wpIndex] : base;

    // Направление и расстояние до цели.
    const QPointF direction = target - uavPos;
    const double distance = QLineF(uavPos, target).length();

    // Длина одного шага.
    const double step = 3.0;

    if (distance > step) {

        uavPos += direction / distance * step;
        uav->setPos(uavPos);


        return;
    }


    uavPos = target;
    uav->setPos(uavPos);

    if (mode == MISSION) {
        log(
            QString("Достигнута Т%1.")
                .arg(wpIndex + 1)
            );

        QApplication::beep();


        wpIndex = (wpIndex + 1) % 4;
    } else {

        mode = LANDED;

        timer->stop();

        uav->setBrush(QBrush(Qt::red));

        ui->labelStatus->setText(
            "Статус: ПОСАДКА ЗАВЕРШЕНА"
            );

        log("База достигнута. Посадка завершена.");

        QApplication::beep();
    }
}