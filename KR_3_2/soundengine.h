#ifndef SOUNDENGINE_H
#define SOUNDENGINE_H

#include <QObject>
#include <QVector>

class QSoundEffect;

#ifdef HAVE_QT_MULTIMEDIA
#include <QTemporaryDir>
#endif

class SoundEngine : public QObject
{
    Q_OBJECT

public:
    explicit SoundEngine(QObject *parent = nullptr);

    void setEnabled(bool enabled);
    void playConnected();
    void playAlarm();
    void playWarning();
    void playClick();

private:
    bool m_enabled = true;
#ifdef HAVE_QT_MULTIMEDIA
    QTemporaryDir m_dir;
    QSoundEffect *m_connected = nullptr;
    QSoundEffect *m_alarm = nullptr;
    QSoundEffect *m_warning = nullptr;
    QSoundEffect *m_click = nullptr;
    QSoundEffect *createSound(const QString &fileName, const QVector<float> &samples);
#endif
    void play(QSoundEffect *sound);
};

#endif
