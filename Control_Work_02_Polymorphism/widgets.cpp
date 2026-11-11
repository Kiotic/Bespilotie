#include "widgets.h"

#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QConicalGradient>
#include <QMouseEvent>
#include <QFontMetrics>
#include <QPolygonF>
#include <QtMath>
#include <cmath>

namespace {
QFont font_px(int px, bool bold = false)
{
    QFont f;
    f.setPixelSize(px);
    f.setBold(bold);
    return f;
}

void panel_bg(QPainter& p, const QRectF& r)
{
    p.setPen(QPen(QColor(255, 255, 255, 18), 1));
    p.setBrush(Theme::panel());
    p.drawRoundedRect(r.adjusted(0.5, 0.5, -0.5, -0.5), 10, 10);
}

QColor with_alpha(QColor c, int a) { c.setAlpha(a); return c; }
} // namespace

// =====================================================================
//                            TelemetryChart
// =====================================================================
TelemetryChart::TelemetryChart(const QString& title, const QString& unit, QWidget* parent)
    : QWidget(parent), title_(title), unit_(unit)
{
    setMinimumHeight(150);
}

int TelemetryChart::add_series(const QString& name, const QColor& color, bool fill, bool dashed)
{
    series_.push_back({name, color, fill, dashed, {}});
    return series_.size() - 1;
}

void TelemetryChart::add_point(int s, double t, double v)
{
    if (s < 0 || s >= series_.size()) return;
    auto& pts = series_[s].pts;
    pts.push_back(QPointF(t, v));
    if (!fixed_x_) {
        const double cut = t - window_ * 1.2;
        int drop = 0;
        while (drop < pts.size() && pts[drop].x() < cut) ++drop;
        if (drop > 0) pts.remove(0, drop);
    }
    update();
}

void TelemetryChart::clear()
{
    for (auto& s : series_) s.pts.clear();
    update();
}

void TelemetryChart::set_window(double s)               { window_ = s; fixed_x_ = false; }
void TelemetryChart::set_fixed_x(double a, double b)    { fixed_x_ = true; fx0_ = a; fx1_ = b; }
void TelemetryChart::set_y_range(double lo, double hi)  { fixed_y_ = true; fy0_ = lo; fy1_ = hi; }
void TelemetryChart::set_include_zero(bool on)          { include_zero_ = on; }

double TelemetryChart::nice_step(double raw)
{
    if (raw <= 0) return 1;
    const double e = std::pow(10.0, std::floor(std::log10(raw)));
    const double f = raw / e;
    double n = 10;
    if (f <= 1) n = 1; else if (f <= 2) n = 2; else if (f <= 2.5) n = 2.5; else if (f <= 5) n = 5;
    return n * e;
}

void TelemetryChart::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    panel_bg(p, rect());

    // заголовок
    p.setPen(Theme::text());
    p.setFont(font_px(13, true));
    p.drawText(QRectF(14, 6, width() - 28, 20), Qt::AlignLeft | Qt::AlignVCenter, title_);

    // легенда
    p.setFont(font_px(11));
    double lx = width() - 14;
    for (int i = series_.size() - 1; i >= 0; --i) {
        const auto& s = series_[i];
        const int tw = p.fontMetrics().horizontalAdvance(s.name);
        lx -= tw;
        p.setPen(Theme::muted());
        p.drawText(QPointF(lx, 20), s.name);
        lx -= 18;
        p.setPen(QPen(s.color, 2.5, s.dashed ? Qt::DashLine : Qt::SolidLine, Qt::RoundCap));
        p.drawLine(QPointF(lx, 16), QPointF(lx + 12, 16));
        lx -= 14;
    }

    const QRectF plot(56, 32, width() - 72, height() - 58);
    if (plot.width() < 20 || plot.height() < 20) return;

    // диапазон X
    double tmax = 0;
    for (const auto& s : series_) if (!s.pts.isEmpty()) tmax = qMax(tmax, s.pts.last().x());
    double x0, x1;
    if (fixed_x_) { x0 = fx0_; x1 = fx1_; }
    else { x1 = qMax(tmax, window_); x0 = x1 - window_; }

    // диапазон Y
    double y0 = 1e300, y1 = -1e300;
    if (fixed_y_) { y0 = fy0_; y1 = fy1_; }
    else {
        for (const auto& s : series_)
            for (const auto& pt : s.pts)
                if (pt.x() >= x0 - 1e-9 && pt.x() <= x1 + 1e-9) { y0 = qMin(y0, pt.y()); y1 = qMax(y1, pt.y()); }
        if (y0 > y1) { y0 = 0; y1 = 1; }
        if (include_zero_) { y0 = qMin(y0, 0.0); y1 = qMax(y1, 0.0); }
        const double pad = qMax(1e-6, (y1 - y0) * 0.12);
        y0 -= pad; y1 += pad;
        if (y1 - y0 < 1.0) { const double m = (y0 + y1) / 2; y0 = m - 0.5; y1 = m + 0.5; }
    }

    auto mx = [&](double x) { return plot.left() + (x - x0) / (x1 - x0) * plot.width(); };
    auto my = [&](double y) { return plot.bottom() - (y - y0) / (y1 - y0) * plot.height(); };

    // сетка Y
    p.setFont(font_px(10));
    const double ys = nice_step((y1 - y0) / 4.0);
    for (double y = std::ceil(y0 / ys) * ys; y <= y1 + 1e-9; y += ys) {
        const double py = my(y);
        p.setPen(QPen(Theme::grid(), 1));
        p.drawLine(QPointF(plot.left(), py), QPointF(plot.right(), py));
        p.setPen(Theme::muted());
        p.drawText(QRectF(4, py - 8, plot.left() - 10, 16), Qt::AlignRight | Qt::AlignVCenter,
                   QString::number(y, 'g', 4));
    }
    // ноль
    if (y0 < 0 && y1 > 0) {
        p.setPen(QPen(QColor(255, 255, 255, 60), 1, Qt::DashLine));
        p.drawLine(QPointF(plot.left(), my(0)), QPointF(plot.right(), my(0)));
    }
    // сетка X
    const double xs = nice_step((x1 - x0) / 6.0);
    for (double x = std::ceil(x0 / xs) * xs; x <= x1 + 1e-9; x += xs) {
        const double px = mx(x);
        p.setPen(QPen(Theme::grid(), 1));
        p.drawLine(QPointF(px, plot.top()), QPointF(px, plot.bottom()));
        p.setPen(Theme::muted());
        p.drawText(QRectF(px - 30, plot.bottom() + 4, 60, 14), Qt::AlignCenter,
                   QString::number(x, 'g', 4) + " s");
    }
    p.setPen(Theme::muted());
    p.drawText(QRectF(4, plot.top() - 24, plot.left(), 14), Qt::AlignRight, unit_);

    // серии
    p.save();
    p.setClipRect(plot.adjusted(-2, -2, 2, 2));
    for (const auto& s : series_) {
        if (s.pts.size() < 2) continue;
        QPainterPath path;
        bool started = false;
        double first_x = 0, last_x = 0;
        for (int i = 0; i < s.pts.size(); ++i) {
            const QPointF& pt = s.pts[i];
            if (pt.x() < x0 && i + 1 < s.pts.size() && s.pts[i + 1].x() < x0) continue;
            const QPointF q(mx(pt.x()), my(pt.y()));
            if (!started) { path.moveTo(q); started = true; first_x = q.x(); }
            else path.lineTo(q);
            last_x = q.x();
        }
        if (!started) continue;
        if (s.fill) {
            QPainterPath area = path;
            area.lineTo(last_x, plot.bottom());
            area.lineTo(first_x, plot.bottom());
            area.closeSubpath();
            QLinearGradient g(0, plot.top(), 0, plot.bottom());
            g.setColorAt(0, with_alpha(s.color, 90));
            g.setColorAt(1, with_alpha(s.color, 0));
            p.fillPath(area, g);
        }
        // свечение + линия
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(with_alpha(s.color, 50), 6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        if (!s.dashed) p.drawPath(path);
        p.setPen(QPen(s.color, 2, s.dashed ? Qt::DashLine : Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        p.drawPath(path);
    }
    p.restore();

    // текущие значения (плашки справа)
    p.setFont(font_px(10, true));
    for (const auto& s : series_) {
        if (s.pts.isEmpty() || s.dashed || fixed_x_) continue;
        const QPointF last = s.pts.last();
        const double py = qBound(plot.top(), my(last.y()), plot.bottom());
        const double px = qBound(plot.left(), mx(last.x()), plot.right());
        p.setPen(Qt::NoPen);
        p.setBrush(s.color);
        p.drawEllipse(QPointF(px, py), 3.5, 3.5);
        const QString txt = QString::number(last.y(), 'f', 1);
        const int tw = p.fontMetrics().horizontalAdvance(txt) + 10;
        QRectF pill(qMin(px + 6, double(width() - tw - 4)), py - 8, tw, 16);
        p.setBrush(with_alpha(s.color, 220));
        p.drawRoundedRect(pill, 8, 8);
        p.setPen(Theme::bg());
        p.drawText(pill, Qt::AlignCenter, txt);
    }
}

// =====================================================================
//                               FlightView
// =====================================================================
FlightView::FlightView(QWidget* parent) : QWidget(parent)
{
    setMinimumSize(280, 420);
}

void FlightView::set_state(double alt, double vs, double target, double control, bool ok,
                           const QString& status, const QString& vehicle)
{
    alt_ = alt; vs_ = vs; target_ = target; control_ = control; ok_ = ok;
    status_ = status; vehicle_ = vehicle;
}

void FlightView::push_trail(double t, double alt)
{
    now_ = t;
    trail_.push_back(QPointF(t, alt));
    while (!trail_.isEmpty() && trail_.first().x() < t - 30) trail_.removeFirst();
}

void FlightView::clear_trail() { trail_.clear(); now_ = 0; }

void FlightView::advance(double dt, bool motors_on)
{
    if (motors_on) phase_ += dt * (40.0 + 6.0 * qAbs(control_));
    blink_ += dt;
    clouds_ += dt * 8.0;
    update();
}

void FlightView::draw_drone(QPainter& p, const QPointF& c)
{
    p.save();
    p.translate(c);
    // наклон пропорционален вертикальной скорости (для живости)
    p.rotate(qBound(-8.0, -vs_ * 0.8, 8.0));

    // тяга (стрелка)
    if (qAbs(control_) > 0.05) {
        const double len = qMin(60.0, qAbs(control_) * 9.0);
        const QColor col = control_ > 0 ? Theme::cyan() : Theme::amber();
        QLinearGradient g(0, 14, 0, 14 + len);
        g.setColorAt(0, with_alpha(col, 200));
        g.setColorAt(1, with_alpha(col, 0));
        p.setPen(Qt::NoPen);
        p.setBrush(g);
        QPainterPath jet;
        jet.moveTo(-10, 12); jet.lineTo(10, 12); jet.lineTo(3, 12 + len); jet.lineTo(-3, 12 + len);
        jet.closeSubpath();
        p.drawPath(jet);
    }

    // лучи
    p.setPen(QPen(QColor(150, 165, 195), 4, Qt::SolidLine, Qt::RoundCap));
    p.drawLine(QPointF(-42, -2), QPointF(42, -2));
    // мачты
    p.setPen(QPen(QColor(120, 135, 165), 3, Qt::SolidLine, Qt::RoundCap));
    p.drawLine(QPointF(-42, -2), QPointF(-42, -10));
    p.drawLine(QPointF(42, -2), QPointF(42, -10));

    // винты: размытый диск + вращающаяся лопасть
    for (int side : {-1, 1}) {
        const double x = 42.0 * side;
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(200, 220, 255, 45));
        p.drawEllipse(QPointF(x, -11), 24, 3.5);
        const double half = 24.0 * std::fabs(std::cos(phase_ + side));
        p.setPen(QPen(QColor(220, 230, 250, 200), 2.2, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(QPointF(x - half, -11), QPointF(x + half, -11));
    }

    // корпус
    QLinearGradient body(0, -10, 0, 10);
    body.setColorAt(0, QColor(92, 110, 150));
    body.setColorAt(1, QColor(38, 48, 72));
    p.setPen(QPen(QColor(170, 190, 230), 1));
    p.setBrush(body);
    p.drawRoundedRect(QRectF(-26, -9, 52, 18), 8, 8);
    // «стекло»
    p.setPen(Qt::NoPen);
    p.setBrush(with_alpha(Theme::cyan(), 160));
    p.drawRoundedRect(QRectF(-14, -6, 18, 6), 3, 3);
    // подвес камеры
    p.setBrush(QColor(30, 36, 52));
    p.setPen(QPen(QColor(140, 155, 185), 1));
    p.drawEllipse(QPointF(10, 11), 5, 5);
    p.setBrush(Theme::cyan());
    p.setPen(Qt::NoPen);
    p.drawEllipse(QPointF(12, 12), 1.8, 1.8);

    // навигационные огни
    const bool on = std::fmod(blink_, 1.0) < 0.5;
    p.setBrush(QColor(255, 70, 80, on ? 255 : 90));
    p.drawEllipse(QPointF(-25, 0), 2.5, 2.5);
    p.setBrush(QColor(70, 255, 140, on ? 90 : 255));
    p.drawEllipse(QPointF(25, 0), 2.5, 2.5);

    // статусный ореол
    const QColor st = ok_ ? Theme::green() : Theme::red();
    const int a = ok_ ? 70 : (std::fmod(blink_, 0.5) < 0.25 ? 140 : 30);
    QRadialGradient halo(QPointF(0, 0), 70);
    halo.setColorAt(0, with_alpha(st, a));
    halo.setColorAt(1, with_alpha(st, 0));
    p.setBrush(halo);
    p.drawEllipse(QPointF(0, 0), 70, 70);
    p.restore();
}

void FlightView::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const QRectF r = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
    QPainterPath clip;
    clip.addRoundedRect(r, 10, 10);
    p.setClipPath(clip);

    // небо
    QLinearGradient sky(0, 0, 0, height());
    sky.setColorAt(0.0, QColor(8, 14, 34));
    sky.setColorAt(0.55, QColor(28, 52, 98));
    sky.setColorAt(1.0, QColor(255, 150, 100));
    p.fillRect(r, sky);

    // звёзды
    p.setPen(Qt::NoPen);
    for (int i = 0; i < 40; ++i) {
        const double x = std::fmod(i * 73.13, width());
        const double y = std::fmod(i * 41.7, height() * 0.45);
        const double tw = 0.5 + 0.5 * std::sin(blink_ * 2 + i);
        p.setBrush(QColor(255, 255, 255, int(40 + 80 * tw)));
        p.drawEllipse(QPointF(x, y), 1.0, 1.0);
    }

    const double ground_h = 46;
    const QRectF plot(46, 20, width() - 56, height() - 20 - ground_h);
    const double max_alt = 260.0;
    auto ay = [&](double a) { return plot.bottom() - qBound(0.0, a, max_alt) / max_alt * plot.height(); };

    // облака (параллакс)
    for (int i = 0; i < 4; ++i) {
        const double span = width() + 160;
        const double x = std::fmod(i * 170.0 - clouds_ * (0.6 + 0.2 * i) + span * 10, span) - 80;
        const double y = ay(60 + i * 55);
        p.setBrush(QColor(255, 255, 255, 18 + 6 * i));
        p.drawEllipse(QPointF(x, y), 46, 12);
        p.drawEllipse(QPointF(x + 26, y - 7), 30, 11);
    }

    // земля
    QLinearGradient gr(0, height() - ground_h, 0, height());
    gr.setColorAt(0, QColor(24, 40, 36));
    gr.setColorAt(1, QColor(10, 16, 14));
    QPainterPath hills;
    hills.moveTo(0, height());
    for (int x = 0; x <= width(); x += 8) {
        const double off = clouds_ * 1.5;
        const double y = height() - ground_h + 8 * std::sin((x + off) * 0.03) + 5 * std::sin((x + off) * 0.071);
        hills.lineTo(x, y);
    }
    hills.lineTo(width(), height());
    hills.closeSubpath();
    p.fillPath(hills, gr);

    // шкала высот
    p.setFont(font_px(10));
    for (int a = 0; a <= 250; a += 50) {
        const double y = ay(a);
        p.setPen(QPen(QColor(255, 255, 255, 25), 1));
        p.drawLine(QPointF(plot.left(), y), QPointF(plot.right(), y));
        p.setPen(QColor(200, 210, 235, 170));
        p.drawText(QRectF(4, y - 8, 38, 16), Qt::AlignRight | Qt::AlignVCenter, QString::number(a));
    }
    p.drawText(QRectF(4, 2, 60, 14), Qt::AlignLeft, "ALT, m");

    // целевая высота
    const double ty = ay(target_);
    p.setPen(QPen(Theme::green(), 1.5, Qt::DashLine));
    p.drawLine(QPointF(plot.left(), ty), QPointF(plot.right(), ty));
    p.setFont(font_px(10, true));
    const QString tl = QString("TARGET %1 m").arg(target_, 0, 'f', 0);
    QRectF tag(plot.right() - 92, ty - 18, 90, 15);
    p.setPen(Qt::NoPen);
    p.setBrush(with_alpha(Theme::green(), 200));
    p.drawRoundedRect(tag, 7, 7);
    p.setPen(Theme::bg());
    p.drawText(tag, Qt::AlignCenter, tl);

    // след
    const double cx = plot.left() + plot.width() * 0.62;
    const double px_per_s = 9.0;
    if (trail_.size() > 1) {
        QPainterPath tp;
        bool first = true;
        for (const auto& pt : trail_) {
            const QPointF q(cx - (now_ - pt.x()) * px_per_s, ay(pt.y()));
            if (first) { tp.moveTo(q); first = false; } else tp.lineTo(q);
        }
        QLinearGradient tg(cx - 30 * px_per_s, 0, cx, 0);
        tg.setColorAt(0, with_alpha(Theme::cyan(), 0));
        tg.setColorAt(1, with_alpha(Theme::cyan(), 220));
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(QBrush(tg), 2.2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        p.drawPath(tp);
    }

    draw_drone(p, QPointF(cx, ay(alt_)));

    // индикаторы
    p.setClipping(false);
    QRectF hud(10, height() - ground_h + 6, width() - 20, ground_h - 12);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 110));
    p.drawRoundedRect(hud, 8, 8);
    p.setFont(font_px(11, true));
    p.setPen(Theme::text());
    p.drawText(hud.adjusted(10, 0, 0, 0), Qt::AlignLeft | Qt::AlignVCenter,
               QString("ALT %1 m   V/S %2%3 m/s")
                   .arg(alt_, 0, 'f', 1)
                   .arg(QString(vs_ >= 0 ? "+" : ""))
                   .arg(vs_, 0, 'f', 1));
    p.setPen(ok_ ? Theme::green() : Theme::red());
    p.drawText(hud.adjusted(0, 0, -10, 0), Qt::AlignRight | Qt::AlignVCenter, status_);

    p.setPen(QColor(220, 230, 250, 200));
    p.setFont(font_px(11, true));
    p.drawText(QRectF(0, 2, width() - 10, 16), Qt::AlignRight, vehicle_);

    // рамка
    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(QColor(255, 255, 255, 25), 1));
    p.drawRoundedRect(r, 10, 10);
}

// =====================================================================
//                                RadarView
// =====================================================================
RadarView::RadarView(QWidget* parent) : QWidget(parent)
{
    setMinimumSize(260, 260);
}

void RadarView::set_blips(const QVector<Blip>& b) { blips_ = b; }
void RadarView::set_camera_info(const QString& s, bool ready) { camera_ = s; camera_ready_ = ready; }
void RadarView::advance(double dt)
{
    sweep_ = std::fmod(sweep_ + dt * 140.0, 360.0);
    update();
}

void RadarView::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    panel_bg(p, rect());

    p.setPen(Theme::text());
    p.setFont(font_px(13, true));
    p.drawText(QRectF(14, 6, width() - 28, 20), Qt::AlignLeft | Qt::AlignVCenter, "Обстановка: Lidar / Radar");

    const double R = qMin(width() - 30.0, height() - 62.0) / 2.0;
    const QPointF c(width() / 2.0, 32 + R);
    auto at = [&](double ang_deg, double dist) {
        const double rr = qMin(dist, range_) / range_ * R;
        const double a = qDegreesToRadians(ang_deg);
        return QPointF(c.x() + rr * std::cos(a), c.y() - rr * std::sin(a));
    };

    // фон экрана радара
    QRadialGradient bgg(c, R);
    bgg.setColorAt(0, QColor(10, 40, 34));
    bgg.setColorAt(1, QColor(6, 18, 18));
    p.setPen(QPen(with_alpha(Theme::green(), 90), 1.5));
    p.setBrush(bgg);
    p.drawEllipse(c, R, R);

    // кольца дальности
    p.setFont(font_px(9));
    for (int d = 5; d <= 30; d += 5) {
        const double rr = d / range_ * R;
        QColor col = with_alpha(Theme::green(), 35);
        Qt::PenStyle st = Qt::SolidLine;
        if (d == 10) { col = with_alpha(Theme::red(), 170); st = Qt::DashLine; }
        if (d == 20) { col = with_alpha(Theme::amber(), 150); st = Qt::DashLine; }
        p.setPen(QPen(col, d == 10 || d == 20 ? 1.4 : 1, st));
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(c, rr, rr);
        if (d == 10 || d == 20 || d == 30) {
            p.setPen(col.alpha() < 100 ? with_alpha(Theme::green(), 140) : col);
            p.drawText(QPointF(c.x() + 3, c.y() - rr + 11), QString("%1 m").arg(d));
        }
    }
    p.setPen(QPen(with_alpha(Theme::green(), 35), 1));
    p.drawLine(QPointF(c.x() - R, c.y()), QPointF(c.x() + R, c.y()));
    p.drawLine(QPointF(c.x(), c.y() - R), QPointF(c.x(), c.y() + R));

    // развёртка (конический градиент — «хвост» за лучом)
    QConicalGradient cg(c, sweep_);
    cg.setColorAt(0.0, QColor(0, 0, 0, 0));
    cg.setColorAt(0.80, QColor(0, 0, 0, 0));
    cg.setColorAt(0.995, with_alpha(Theme::green(), 120));
    cg.setColorAt(1.0, with_alpha(Theme::green(), 120));
    p.setPen(Qt::NoPen);
    p.setBrush(cg);
    p.drawEllipse(c, R, R);
    p.setPen(QPen(with_alpha(Theme::green(), 230), 2));
    p.drawLine(c, at(sweep_, range_));

    // отметки целей
    for (const auto& b : blips_) {
        const QPointF q = at(b.angle_deg, b.distance);
        QColor col = !b.ready ? QColor(130, 130, 140)
                   : b.level == 2 ? Theme::red()
                   : b.level == 1 ? Theme::amber() : Theme::green();
        const double behind = std::fmod(sweep_ - b.angle_deg + 720.0, 360.0);
        const double glow = b.ready ? std::exp(-behind / 70.0) : 0.0;
        QRadialGradient hg(q, 18);
        hg.setColorAt(0, with_alpha(col, int(60 + 160 * glow)));
        hg.setColorAt(1, with_alpha(col, 0));
        p.setPen(Qt::NoPen);
        p.setBrush(hg);
        p.drawEllipse(q, 18, 18);
        p.setBrush(col);
        p.drawEllipse(q, 4.5, 4.5);
        p.setPen(col);
        p.setFont(font_px(10, true));
        const QString t = b.ready ? QString("%1 %2 m").arg(b.label).arg(b.distance, 0, 'f', 1)
                                  : b.label + " OFFLINE";
        p.drawText(q + QPointF(8, -6), t);
    }

    // свой БПЛА в центре
    p.setPen(QPen(Theme::cyan(), 1.5));
    p.setBrush(with_alpha(Theme::cyan(), 90));
    QPolygonF tri;
    tri << c + QPointF(0, -8) << c + QPointF(6, 6) << c + QPointF(-6, 6);
    p.drawPolygon(tri);

    // камера
    p.setFont(font_px(11, true));
    p.setPen(camera_ready_ ? Theme::cyan() : QColor(130, 130, 140));
    p.drawText(QRectF(14, height() - 24, width() - 28, 18), Qt::AlignLeft | Qt::AlignVCenter, camera_);
    p.setPen(Theme::muted());
    p.setFont(font_px(10));
    p.drawText(QRectF(14, height() - 24, width() - 28, 18), Qt::AlignRight | Qt::AlignVCenter,
               "пороги 10 / 20 м");
}

// =====================================================================
//                               SensorCard
// =====================================================================
SensorCard::SensorCard(QWidget* parent) : QWidget(parent)
{
    setFixedHeight(58);
    setCursor(Qt::PointingHandCursor);
    setToolTip("Клик — имитировать отказ / восстановление сенсора (set_ready)");
}

void SensorCard::set_data(const QString& name, const QString& value, bool ready, bool critical,
                          int level, const QString& sub)
{
    name_ = name; value_ = value; ready_ = ready; critical_ = critical; level_ = level; sub_ = sub;
    update();
}

void SensorCard::mousePressEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton) emit clicked();
}

void SensorCard::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const QColor accent = !ready_ ? Theme::red()
                        : level_ == 2 ? Theme::red()
                        : level_ == 1 ? Theme::amber() : Theme::green();
    QRectF r = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
    p.setPen(QPen(with_alpha(accent, 90), 1));
    p.setBrush(Theme::panel());
    p.drawRoundedRect(r, 9, 9);
    // полоса слева
    p.setPen(Qt::NoPen);
    p.setBrush(accent);
    p.drawRoundedRect(QRectF(r.left() + 4, r.top() + 8, 4, r.height() - 16), 2, 2);

    // LED
    QRadialGradient led(QPointF(24, 20), 9);
    led.setColorAt(0, ready_ ? Theme::green() : Theme::red());
    led.setColorAt(1, with_alpha(ready_ ? Theme::green() : Theme::red(), 0));
    p.setBrush(led);
    p.drawEllipse(QPointF(24, 20), 9, 9);

    p.setPen(Theme::text());
    p.setFont(font_px(13, true));
    p.drawText(QRectF(38, 8, 140, 18), Qt::AlignLeft | Qt::AlignVCenter, name_);
    if (critical_) {
        p.setFont(font_px(9, true));
        const QRectF badge(38 + QFontMetrics(font_px(13, true)).horizontalAdvance(name_) + 6, 11, 52, 13);
        p.setPen(Qt::NoPen);
        p.setBrush(with_alpha(Theme::violet(), 170));
        p.drawRoundedRect(badge, 6, 6);
        p.setPen(Theme::bg());
        p.drawText(badge, Qt::AlignCenter, "CRITICAL");
    }
    p.setPen(Theme::muted());
    p.setFont(font_px(10));
    p.drawText(QRectF(38, 30, width() - 44, 18), Qt::AlignLeft | Qt::AlignVCenter, sub_);

    p.setPen(ready_ ? Theme::text() : Theme::red());
    p.setFont(font_px(16, true));
    p.drawText(QRectF(0, 6, width() - 12, 24), Qt::AlignRight | Qt::AlignVCenter,
               ready_ ? value_ : QString("NOT READY"));
}
