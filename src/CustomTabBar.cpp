#include "CustomTabBar.h"
#include "GlassCloseButton.h"

#include <QPainter>
#include <QPainterPath>
#include <QStyleOptionTab>
#include <QRandomGenerator>
#include <QMouseEvent>
#include <QEnterEvent>
#include <QtMath>

CustomTabBar::CustomTabBar(QWidget *parent) : QTabBar(parent) {
    setMouseTracking(true);
    setExpanding(false);
    setDrawBase(false);
    setUsesScrollButtons(true);
    setElideMode(Qt::ElideRight);
    setContentsMargins(8,8,8,4);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setFocusPolicy(Qt::NoFocus);  // avoid macOS focus ring halo

    // Keep close button visible for current tab, else only on hover
    connect(this, &QTabBar::currentChanged, this, [this] { updateCloseButtonsVisibility(); });
}

QSize CustomTabBar::tabSizeHint(int index) const {
    QSize s = QTabBar::tabSizeHint(index);
    s.setHeight(qMax(28, s.height()));
    s.setWidth(s.width() + 14);
    return s;
}

void CustomTabBar::enterEvent(QEnterEvent *ev) {
    QTabBar::enterEvent(ev);
}

void CustomTabBar::leaveEvent(QEvent *ev) {
    m_hoverIndex = -1;
    updateCloseButtonsVisibility();
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
        updateCloseButtonsVisibility();
        update();
    }
    QTabBar::mouseMoveEvent(ev);
}

QPixmap CustomTabBar::noisePixmap(int dpr) {
    static QPixmap cached;
    if (!cached.isNull()) return cached;

    const int N = 64 * qMax(1, dpr);
    QImage img(N, N, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);

    auto *rng = QRandomGenerator::global();
    for (int y = 0; y < N; ++y) {
        auto *line = reinterpret_cast<QRgb*>(img.scanLine(y));
        for (int x = 0; x < N; ++x) {
            int v = rng->bounded(200, 255);     // tighter, brighter flecks
            line[x] = qRgba(v, v, v, 10);       // very subtle
        }
    }
    cached = QPixmap::fromImage(img);
    cached.setDevicePixelRatio(qMax(1, dpr));
    return cached;
}

void CustomTabBar::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    for (int i = 0; i < count(); ++i) {
        const QRect R = tabRect(i);
        QRectF r = QRectF(R).adjusted(2, 2, -2, -2);

        const bool sel   = (i == currentIndex());
        const bool hover = (i == m_hoverIndex);

        // Pill shape
        const qreal radius = qMin(r.height() * 0.5, 18.0);
        QPainterPath pill; pill.addRoundedRect(r, radius, radius);

        // Base tint (no extra hover fill; Safari-ish)
        QColor base = QColor(255,255,255, sel ? 44 : 24);
        p.fillPath(pill, base);

        // Soft vertical gloss
        QLinearGradient g(r.topLeft(), r.bottomLeft());
        g.setColorAt(0.00, QColor(255,255,255, sel ? 28 : 18));
        g.setColorAt(0.50, QColor(255,255,255,  0));
        g.setColorAt(1.00, QColor(255,255,255, 12));
        p.fillPath(pill, g);

        // Feather strokes
        p.setPen(QPen(QColor(255,255,255, sel ? 58 : 36), 1.0));
        p.setBrush(Qt::NoBrush);
        p.drawPath(pill);

        QPainterPath inner;
        QRectF r2 = r.adjusted(1,1,-1,-1);
        inner.addRoundedRect(r2, radius-1, radius-1);
        p.setPen(QPen(QColor(255,255,255, sel ? 18 : 12), 1.0));
        p.drawPath(inner);

        // Seat shadow (barely there)
        QPainterPath bottomArc;
        QRectF botR = r.adjusted(3, r.height()*0.55, -3, -3);
        bottomArc.addRoundedRect(botR, radius-3, radius-3);
        p.fillPath(bottomArc, QColor(0,0,0, sel ? 18 : 12));

        // Tiny rim tint on hover/active
        if (hover || sel) {
            p.setPen(QPen(QColor(140,190,255, sel ? 50 : 36), 1.0));
            p.drawPath(pill);
        }

        // Subtle noise (soft-light)
        const QPixmap noise = noisePixmap(devicePixelRatioF() >= 1.5 ? 2 : 1);
        p.setClipPath(pill, Qt::IntersectClip);
        p.setCompositionMode(QPainter::CompositionMode_SoftLight);
        p.setOpacity(0.10);
        for (qreal y = r.top(); y < r.bottom(); y += noise.height()/noise.devicePixelRatioF()) {
            for (qreal x = r.left(); x < r.right(); x += noise.width()/noise.devicePixelRatioF()) {
                p.drawPixmap(QPointF(x, y), noise);
            }
        }
        p.setOpacity(1.0);
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
        p.setClipping(false);

        // ----- Centered text (no icon, no native label) -----
        QWidget *btnL = tabButton(i, QTabBar::LeftSide);
        const int leftInset = (btnL && btnL->isVisible()) ? (btnL->width() + 6) : 0;
        QRectF textRect = r.adjusted(leftInset, 0, 0, 0);

        QFont f = font();
        if (sel) f.setWeight(QFont::DemiBold);
        p.setFont(f);

        const QColor textCol = QColor(236,236,236, 235);
        p.setPen(textCol);

        const QString txt = tabText(i);
        const QString el  = QFontMetrics(f).elidedText(txt, elideMode(), int(textRect.width()));
        p.drawText(textRect, Qt::AlignCenter | Qt::AlignVCenter, el);
    }
}

void CustomTabBar::tabInserted(int index) {
    QTabBar::tabInserted(index);

    auto *btn = new GlassCloseButton(this);
    connect(btn, &QAbstractButton::clicked, this, [this, index]() {
        emit tabCloseRequested(index);
    });

    setTabButton(index, QTabBar::LeftSide, btn); // Safari-like
    updateCloseButtonsVisibility();
}

void CustomTabBar::updateCloseButtonsVisibility() {
    for (int i = 0; i < count(); ++i) {
        if (auto *b = tabButton(i, QTabBar::LeftSide)) {
            b->setVisible(i == currentIndex() || i == m_hoverIndex);
        }
    }
    update();
}
