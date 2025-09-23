#include "CustomTabBar.h"
#include <QPainter>
#include <QPainterPath>
#include <QStyleOptionTab>
#include <QRandomGenerator>
#include <QMouseEvent>
#include <QEnterEvent>
#include <QtMath>

#include "GlassCloseButton.h"

static inline qreal dp(qreal x, QWidget *w) {
    const qreal dpr = w->devicePixelRatioF();
    return x * (dpr >= 1.5 ? 1.0 : 1.0);   // tweak if you want denser spacing on HiDPI
}

CustomTabBar::CustomTabBar(QWidget *parent) : QTabBar(parent) {
    setMouseTracking(true);
    setExpanding(false);
    setDrawBase(false);
    setUsesScrollButtons(true);
    setElideMode(Qt::ElideRight);
    setIconSize(QSize(12,12));
    setContentsMargins(8,8,8,4);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setFocusPolicy(Qt::NoFocus); // avoid macOS focus ring halo
}

QSize CustomTabBar::tabSizeHint(int index) const {
    QSize s = QTabBar::tabSizeHint(index);
    s.setHeight(qMax(28, s.height()));
    s.setWidth(s.width() + 14);
    return s;
}

void CustomTabBar::enterEvent(QEnterEvent *ev) {
    QTabBar::enterEvent(ev);
    // start tracking hover
}

void CustomTabBar::leaveEvent(QEvent *ev) {
    m_hoverIndex = -1;
    update();
    QTabBar::leaveEvent(ev);
}

void CustomTabBar::mouseMoveEvent(QMouseEvent *ev) {
    int idx = -1;
    for (int i = 0; i < count(); ++i) {
        if (tabRect(i).contains(ev->pos())) { idx = i; break; }
    }
    if (idx != m_hoverIndex) {
        m_hoverIndex = idx;
        update();
    }
    QTabBar::mouseMoveEvent(ev);
}

QPixmap CustomTabBar::noisePixmap(int dpr) {
    // cached static 64x64 monochrome noise
    static QPixmap cached;
    if (!cached.isNull()) return cached;

    const int N = 64 * dpr;
    QImage img(N, N, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);
    auto *rng = QRandomGenerator::global();
    for (int y = 0; y < N; ++y) {
        QRgb *line = reinterpret_cast<QRgb*>(img.scanLine(y));
        for (int x = 0; x < N; ++x) {
            int v = rng->bounded(180, 255); // bright flecks
            line[x] = qRgba(v, v, v, 18);   // very subtle alpha
        }
    }
    cached = QPixmap::fromImage(img);
    cached.setDevicePixelRatio(dpr);
    return cached;
}

void CustomTabBar::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    for (int i = 0; i < count(); ++i) {
        QStyleOptionTab opt; initStyleOption(&opt, i);
        const QRect R = tabRect(i);
        QRectF r = QRectF(R).adjusted(2, 2, -2, -2);

        const bool sel   = (i == currentIndex());
        const bool hover = (i == m_hoverIndex);

        // True pill radius based on height
        const qreal radius = qMin(r.height() * 0.5, 18.0);
        QPainterPath pill; pill.addRoundedRect(r, radius, radius);

        // --- Glass base (subtle) ---
        QColor base = QColor(255,255,255, sel ? 44 : (hover ? 34 : 24));
        p.fillPath(pill, base);

        // Vertical gloss gradient (very soft)
        QLinearGradient g(r.topLeft(), r.bottomLeft());
        g.setColorAt(0.00, QColor(255,255,255, sel ? 28 : 18));
        g.setColorAt(0.50, QColor(255,255,255,  0));
        g.setColorAt(1.00, QColor(255,255,255, 12));
        p.fillPath(pill, g);

        // Outer & inner strokes (feather-light)
        p.setPen(QPen(QColor(255,255,255, sel ? 58 : 36), 1.0));
        p.setBrush(Qt::NoBrush);
        p.drawPath(pill);

        QPainterPath inner;
        QRectF r2 = r.adjusted(1,1,-1,-1);
        inner.addRoundedRect(r2, radius-1, radius-1);
        p.setPen(QPen(QColor(255,255,255, sel ? 18 : 12), 1.0));
        p.drawPath(inner);

        // Soft seat shadow (barely there)
        QPainterPath bottomArc;
        QRectF botR = r.adjusted(3, r.height()*0.55, -3, -3);
        bottomArc.addRoundedRect(botR, radius-3, radius-3);
        p.fillPath(bottomArc, QColor(0,0,0, sel ? 18 : 12));

        // Hover/active rim tint
        if (hover || sel) {
            p.setPen(QPen(QColor(140,190,255, sel ? 50 : 36), 1.0));
            p.drawPath(pill);
        }

        // --- Noise overlay (very subtle, soft-light) ---
        const QPixmap noise = noisePixmap(devicePixelRatioF() >= 1.5 ? 2 : 1);
        p.setClipPath(pill, Qt::IntersectClip);
        p.setCompositionMode(QPainter::CompositionMode_SoftLight);
        p.setOpacity(0.10); // << key change: much lower overall visibility
        for (qreal y = r.top(); y < r.bottom(); y += noise.height()/noise.devicePixelRatioF()) {
            for (qreal x = r.left(); x < r.right(); x += noise.width()/noise.devicePixelRatioF()) {
                p.drawPixmap(QPointF(x, y), noise);
            }
        }
        // restore painter
        p.setOpacity(1.0);
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
        p.setClipping(false);

        // Label/icon only (no native bezels)
        // Slight label brighten on selected
        if (sel) {
            QPalette pal = palette();
            pal.setColor(QPalette::ButtonText, QColor(255,255,255,235));
            p.save();
            // Let style use our adjusted palette just for this draw
            QWidget *w = this;
            w->setPalette(pal);
            style()->drawControl(QStyle::CE_TabBarTabLabel, &opt, &p, this);
            p.restore();
        } else {
            style()->drawControl(QStyle::CE_TabBarTabLabel, &opt, &p, this);
        }
    }
}

void CustomTabBar::tabInserted(int index) {
    QTabBar::tabInserted(index);

    auto *btn = new GlassCloseButton(this);
    connect(btn, &QAbstractButton::clicked, this, [this, index]() {
        emit tabCloseRequested(index);
    });

    // place slightly left inside pill
    setTabButton(index, QTabBar::RightSide, btn);
}

