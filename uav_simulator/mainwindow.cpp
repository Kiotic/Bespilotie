#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    scene = new QGraphicsScene(this);
    scene->setSceneRect(0, 0, 800, 600);
    ui->graphicsView->setScene(scene);

    uav = scene->addEllipse(
        0, 0, 20, 20,
        QPen(Qt::black),
        QBrush(Qt::red)
        );

    x = 50;
    y = 50;
    direction = 0;
    speed = 2;
    targetX = 0;
    targetY = 0;
    isFlying = false;

    uav->setPos(x, y);

    timer = new QTimer(this);

    connect(timer, &QTimer::timeout,
            this, &MainWindow::moveUAV);

    connect(ui->btnStart, &QPushButton::clicked,
            this, [this]()
            {
                isFlying = false;
                timer->start(20);
            });

    connect(ui->btnFlyTo, &QPushButton::clicked,
            this, &MainWindow::flyToTarget);

    connect(ui->btnReset, &QPushButton::clicked,
            this, &MainWindow::resetUAV);

    ui->textEdit->append("Статус: Остановлен");
    ui->textEdit->append("Координаты: X=50, Y=50");
}


void MainWindow::moveUAV()
{
    if (isFlying)
    {
        double dx = targetX - x;
        double dy = targetY - y;
        double dist = qSqrt(dx * dx + dy * dy);

        if (dist > 3)
        {
            x += (dx / dist) * speed;
            y += (dy / dist) * speed;
        }
        else
        {
            x = targetX;
            y = targetY;

            uav->setPos(x, y);

            timer->stop();
            isFlying = false;

            ui->labelStatus->setText("Цель достигнута");

            ui->textEdit->clear();
            ui->textEdit->append("Статус: Остановлен");

            ui->textEdit->append(
                QString("Координаты: X=%1, Y=%2")
                    .arg(x, 0, 'f', 1)
                    .arg(y, 0, 'f', 1)
                );

            return;
        }
    }
    else
    {
        if (direction == 0)
        {
            x += speed;
            if (x >= 250)
            {
                x = 250;
                direction = 1;
            }
        }
        else if (direction == 1)
        {
            y += speed;

            if (y >= 250)
            {
                y = 250;
                direction = 2;
            }
        }
        else if (direction == 2)
        {
            x -= speed;

            if (x <= 50)
            {
                x = 50;
                direction = 3;
            }
        }
        else if (direction == 3)
        {
            y -= speed;

            if (y <= 50)
            {
                y = 50;
                direction = 0;
            }
        }
    }

    uav->setPos(x, y);

    ui->labelStatus->setText(
        QString("X: %1 Y: %2")
            .arg(x, 0, 'f', 1)
            .arg(y, 0, 'f', 1)
        );

    ui->textEdit->clear();
    ui->textEdit->append("Статус: Активен");

    ui->textEdit->append(
        QString("Координаты: X=%1, Y=%2")
            .arg(x, 0, 'f', 1)
            .arg(y, 0, 'f', 1)
        );
}


void MainWindow::flyToTarget()
{
    targetX = ui->lineEditX->text().toDouble();
    targetY = ui->lineEditY->text().toDouble();

    isFlying = true;
    timer->start(20);
    ui->labelStatus->setText(
        QString("Летим к точке X=%1 Y=%2")
            .arg(targetX)
            .arg(targetY)
        );
}


void MainWindow::resetUAV()
{
    timer->stop();
    x = 50;
    y = 50;
    direction = 0;
    isFlying = false;
    uav->setPos(x, y);
    uav->setBrush(QBrush(Qt::red));
    ui->lineEditX->clear();
    ui->lineEditY->clear();
    ui->labelStatus->setText("Сброс");
    ui->textEdit->clear();
    ui->textEdit->append("Статус: Остановлен");
    ui->textEdit->append("Координаты: X=50, Y=50");
}


MainWindow::~MainWindow()
{
    delete ui;
}