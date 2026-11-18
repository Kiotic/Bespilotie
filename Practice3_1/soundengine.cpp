#include "soundengine.h"
#include <QApplication>
#include <QFile>
#include <QtMath>
#include <QRandomGenerator>
#include <cmath>

#ifdef HAVE_QT_MULTIMEDIA
#include <QSoundEffect>
#include <QUrl>
#endif

namespace {
const int kRate = 44100;

// добавить тон с экспоненциальным затуханием и мягкой атакой
void add_tone(QVector<float>& buf, double start_s, double dur_s, double freq,
              double amp, double decay = 6.0, bool bell = false)
{
    const int s0 = int(start_s * kRate);
    const int n  = int(dur_s * kRate);
    if (buf.size() < s0 + n) buf.resize(s0 + n);
    for (int i = 0; i < n; ++i) {
        const double t = double(i) / kRate;
        const double attack = qMin(1.0, t / 0.008);
        const double env = attack * qExp(-decay * t);
        double v = qSin(2 * M_PI * freq * t);
        if (bell) v = 0.6 * v + 0.25 * qSin(2 * M_PI * freq * 2.0 * t) + 0.15 * qSin(2 * M_PI * freq * 3.01 * t);
        buf[s0 + i] += float(amp * env * v);
    }
}

void normalize(QVector<float>& buf, float peak = 0.85f)
{
    float m = 0.0f;
    for (float v : buf) m = qMax(m, qAbs(v));
    if (m > 0) for (float& v : buf) v = v / m * peak;
}

QByteArray to_wav(const QVector<float>& samples)
{
    QByteArray pcm;
    pcm.reserve(samples.size() * 2);
    for (float f : samples) {
        const qint16 s = qint16(qBound(-1.0f, f, 1.0f) * 32767);
        pcm.append(char(s & 0xff));
        pcm.append(char((s >> 8) & 0xff));
    }
    auto le32 = [](QByteArray& b, quint32 v) { for (int i = 0; i < 4; ++i) b.append(char((v >> (8 * i)) & 0xff)); };
    auto le16 = [](QByteArray& b, quint16 v) { b.append(char(v & 0xff)); b.append(char((v >> 8) & 0xff)); };
    QByteArray w;
    w.append("RIFF"); le32(w, 36 + pcm.size()); w.append("WAVE");
    w.append("fmt "); le32(w, 16); le16(w, 1); le16(w, 1);
    le32(w, kRate); le32(w, kRate * 2); le16(w, 2); le16(w, 16);
    w.append("data"); le32(w, pcm.size());
    w.append(pcm);
    return w;
}
} // namespace

SoundEngine::SoundEngine(QObject* parent) : QObject(parent)
{
#ifdef HAVE_QT_MULTIMEDIA
    // --- старт: C5-E5-G5-C6 ---
    QVector<float> start;
    const double notes[4] = {523.25, 659.25, 783.99, 1046.5};
    for (int i = 0; i < 4; ++i) add_tone(start, i * 0.11, 0.5, notes[i], 0.5, 5.0, true);
    normalize(start, 0.7f);

    // --- успех: колокольчик из двух нот ---
    QVector<float> ok;
    add_tone(ok, 0.0, 0.6, 880.0, 0.6, 5.0, true);
    add_tone(ok, 0.12, 0.9, 1318.5, 0.6, 4.0, true);
    normalize(ok, 0.6f);

    // --- авария: двухтональная сирена ---
    QVector<float> alarm;
    for (int i = 0; i < 6; ++i)
        add_tone(alarm, i * 0.16, 0.15, (i % 2) ? 660.0 : 880.0, 0.7, 2.0);
    for (float& v : alarm) v = float(std::tanh(2.5 * v));       // лёгкий «перегруз»
    normalize(alarm, 0.75f);

    // --- предупреждение: двойной короткий бип ---
    QVector<float> warn;
    add_tone(warn, 0.0, 0.08, 1400.0, 0.6, 10.0);
    add_tone(warn, 0.12, 0.08, 1400.0, 0.6, 10.0);
    normalize(warn, 0.55f);

    // --- клик ---
    QVector<float> click;
    click.resize(int(0.02 * kRate));
    for (int i = 0; i < click.size(); ++i)
        click[i] = float((QRandomGenerator::global()->generateDouble() * 2 - 1) * qExp(-300.0 * i / kRate));
    normalize(click, 0.4f);

    // --- гул моторов: 1 с, целые частоты => бесшовная петля ---
    QVector<float> motor(kRate);
    for (int i = 0; i < kRate; ++i) {
        const double t = double(i) / kRate;
        const double blade = 1.0 + 0.35 * qSin(2 * M_PI * 24.0 * t);    // частота лопастей
        double v = 0.55 * qSin(2 * M_PI * 118.0 * t)
                 + 0.30 * qSin(2 * M_PI * 236.0 * t)
                 + 0.18 * qSin(2 * M_PI * 354.0 * t)
                 + 0.10 * qSin(2 * M_PI * 590.0 * t);
        v *= blade;
        v += 0.06 * (QRandomGenerator::global()->generateDouble() * 2 - 1);  // шум воздуха
        motor[i] = float(v);
    }
    normalize(motor, 0.5f);

    start_   = make_effect("start.wav",   start, 0.6);
    success_ = make_effect("success.wav", ok,    0.6);
    alarm_   = make_effect("alarm.wav",   alarm, 0.7);
    warning_ = make_effect("warning.wav", warn,  0.5);
    click_   = make_effect("click.wav",   click, 0.4);
    motor_   = make_effect("motor.wav",   motor, 0.0);
    if (motor_) motor_->setLoopCount(QSoundEffect::Infinite);
#endif
}

#ifdef HAVE_QT_MULTIMEDIA
QSoundEffect* SoundEngine::make_effect(const QString& file, const QVector<float>& samples, double volume)
{
    if (!dir_.isValid()) return nullptr;
    const QString path = dir_.filePath(file);
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly)) return nullptr;
    f.write(to_wav(samples));
    f.close();
    auto* e = new QSoundEffect(this);
    e->setSource(QUrl::fromLocalFile(path));
    e->setVolume(volume);
    return e;
}
#endif

void SoundEngine::set_enabled(bool on)
{
    enabled_ = on;
#ifdef HAVE_QT_MULTIMEDIA
    if (motor_) {
        if (on && motor_on_) motor_->play();
        else motor_->stop();
    }
#endif
}
bool SoundEngine::enabled() const { return enabled_; }

bool SoundEngine::has_multimedia() const
{
#ifdef HAVE_QT_MULTIMEDIA
    return true;
#else
    return false;
#endif
}

#ifdef HAVE_QT_MULTIMEDIA
#define PLAY(e) do { if (enabled_ && (e)) (e)->play(); } while (0)
void SoundEngine::play_start()   { PLAY(start_); }
void SoundEngine::play_success() { PLAY(success_); }
void SoundEngine::play_alarm()   { PLAY(alarm_); }
void SoundEngine::play_warning() { PLAY(warning_); }
void SoundEngine::play_click()   { PLAY(click_); }
void SoundEngine::start_motor()
{
    motor_on_ = true;
    if (enabled_ && motor_ && !motor_->isPlaying()) motor_->play();
}
void SoundEngine::stop_motor()   { motor_on_ = false; if (motor_) motor_->stop(); }
void SoundEngine::set_motor_level(double l)
{
    if (motor_) motor_->setVolume(qBound(0.0, l, 1.0) * 0.6);
}
#else
void SoundEngine::play_start()   {}
void SoundEngine::play_success() {}
void SoundEngine::play_alarm()   { if (enabled_) QApplication::beep(); }
void SoundEngine::play_warning() { if (enabled_) QApplication::beep(); }
void SoundEngine::play_click()   {}
void SoundEngine::start_motor()  { motor_on_ = true; }
void SoundEngine::stop_motor()   { motor_on_ = false; }
void SoundEngine::set_motor_level(double) {}
#endif
