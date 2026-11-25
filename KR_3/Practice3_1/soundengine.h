#ifndef SOUNDENGINE_H
#define SOUNDENGINE_H

#include <QObject>
#include <QString>
#include <QVector>

#ifdef HAVE_QT_MULTIMEDIA
#include <QTemporaryDir>
class QSoundEffect;
#endif

// Все звуки синтезируются кодом (синусы/огибающие) и пишутся во временные WAV.
// Если модуль Qt Multimedia не установлен — используется системный beep.
class SoundEngine : public QObject {
    Q_OBJECT
public:
    explicit SoundEngine(QObject* parent = nullptr);

    void set_enabled(bool on);
    bool enabled() const;
    bool has_multimedia() const;

    void play_start();      // запуск: восходящее арпеджио
    void play_success();    // цель достигнута / восстановление
    void play_alarm();      // авария (цикл прерван)
    void play_warning();    // препятствие близко
    void play_click();

    void start_motor();
    void stop_motor();
    void set_motor_level(double level01);   // громкость гула моторов 0..1

private:
    bool enabled_ = true;
    bool motor_on_ = false;
#ifdef HAVE_QT_MULTIMEDIA
    QTemporaryDir dir_;
    QSoundEffect* motor_ = nullptr;
    QSoundEffect* start_ = nullptr;
    QSoundEffect* success_ = nullptr;
    QSoundEffect* alarm_ = nullptr;
    QSoundEffect* warning_ = nullptr;
    QSoundEffect* click_ = nullptr;
    QSoundEffect* make_effect(const QString& file, const QVector<float>& samples, double volume);
#endif
};

#endif // SOUNDENGINE_H
