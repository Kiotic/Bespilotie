#include "soundengine.h"

#include <QApplication>
#include <QFile>
#include <QtMath>

#ifdef HAVE_QT_MULTIMEDIA
#include <QSoundEffect>
#include <QUrl>
#endif

namespace {

const int kSampleRate = 44100;

void addTone(QVector<float> &samples, double start, double duration, double frequency, double volume)
{
    const int first = int(start * kSampleRate);
    const int count = int(duration * kSampleRate);
    if (samples.size() < first + count) {
        samples.resize(first + count);
    }
    for (int i = 0; i < count; ++i) {
        const double t = double(i) / kSampleRate;
        const double envelope = qMin(1.0, t / 0.01) * qExp(-5.0 * t);
        samples[first + i] += float(volume * envelope * qSin(2.0 * M_PI * frequency * t));
    }
}

QByteArray toWav(const QVector<float> &samples)
{
    QByteArray pcm;
    for (float value : samples) {
        const qint16 sample = qint16(qBound(-1.0f, value, 1.0f) * 32767);
        pcm.append(char(sample & 0xFF));
        pcm.append(char((sample >> 8) & 0xFF));
    }

    auto put32 = [](QByteArray &b, quint32 v) { for (int i = 0; i < 4; ++i) b.append(char((v >> (8 * i)) & 0xFF)); };
    auto put16 = [](QByteArray &b, quint16 v) { b.append(char(v & 0xFF)); b.append(char((v >> 8) & 0xFF)); };

    QByteArray wav;
    wav.append("RIFF");
    put32(wav, 36 + pcm.size());
    wav.append("WAVEfmt ");
    put32(wav, 16);
    put16(wav, 1);
    put16(wav, 1);
    put32(wav, kSampleRate);
    put32(wav, kSampleRate * 2);
    put16(wav, 2);
    put16(wav, 16);
    wav.append("data");
    put32(wav, pcm.size());
    wav.append(pcm);
    return wav;
}

}

SoundEngine::SoundEngine(QObject *parent)
    : QObject(parent)
{
#ifdef HAVE_QT_MULTIMEDIA
    QVector<float> connected;
    addTone(connected, 0.00, 0.4, 660.0, 0.5);
    addTone(connected, 0.12, 0.6, 990.0, 0.5);

    QVector<float> alarm;
    for (int i = 0; i < 6; ++i) {
        addTone(alarm, i * 0.16, 0.15, (i % 2) ? 620.0 : 880.0, 0.7);
    }

    QVector<float> warning;
    addTone(warning, 0.00, 0.10, 1200.0, 0.5);
    addTone(warning, 0.14, 0.10, 1200.0, 0.5);

    QVector<float> click;
    addTone(click, 0.0, 0.05, 2000.0, 0.3);

    m_connected = createSound("connected.wav", connected);
    m_alarm = createSound("alarm.wav", alarm);
    m_warning = createSound("warning.wav", warning);
    m_click = createSound("click.wav", click);
#endif
}

#ifdef HAVE_QT_MULTIMEDIA
QSoundEffect *SoundEngine::createSound(const QString &fileName, const QVector<float> &samples)
{
    if (!m_dir.isValid()) {
        return nullptr;
    }
    QFile file(m_dir.filePath(fileName));
    if (!file.open(QIODevice::WriteOnly)) {
        return nullptr;
    }
    file.write(toWav(samples));
    file.close();

    auto *sound = new QSoundEffect(this);
    sound->setSource(QUrl::fromLocalFile(file.fileName()));
    sound->setVolume(0.6);
    return sound;
}

void SoundEngine::play(QSoundEffect *sound)
{
    if (m_enabled && sound) {
        sound->play();
    }
}

void SoundEngine::playConnected() { play(m_connected); }
void SoundEngine::playAlarm() { play(m_alarm); }
void SoundEngine::playWarning() { play(m_warning); }
void SoundEngine::playClick() { play(m_click); }
#else
void SoundEngine::play(QSoundEffect *)
{
    if (m_enabled) {
        QApplication::beep();
    }
}

void SoundEngine::playConnected() {}
void SoundEngine::playAlarm() { play(nullptr); }
void SoundEngine::playWarning() { play(nullptr); }
void SoundEngine::playClick() {}
#endif

void SoundEngine::setEnabled(bool enabled)
{
    m_enabled = enabled;
}
