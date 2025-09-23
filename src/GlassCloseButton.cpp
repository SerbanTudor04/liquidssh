// GlassCloseButton.cpp
#include "GlassCloseButton.h"
#include <QPainter>
#include <QPainterPath>

GlassCloseButton::GlassCloseButton(QWidget *parent) : QAbstractButton(parent) {
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::NoFocus);
}

void GlassCloseButton::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    QRectF r = rect().adjusted(1,1,-1,-1);

    bool hover = underMouse();
    bool down  = isDown();

    // --- Glassy background ---
    QColor base(255,255,255, hover ? 60 : 40);
    QColor stroke(255,255,255, 90);
    p.setBrush(base);
    p.setPen(QPen(stroke, 1));
    p.drawEllipse(r);

    // Subtle highlight
    QLinearGradient g(r.topLeft(), r.bottomLeft());
    g.setColorAt(0.0, QColor(255,255,255, hover ? 80 : 60));
    g.setColorAt(1.0, QColor(255,255,255, 0));
    p.setBrush(g);
    p.setPen(Qt::NoPen);
    p.drawEllipse(r);

    // --- The "X" mark ---
    p.setPen(QPen(QColor(220,220,220, hover ? 255 : 200), 2));
    int pad = 5;
    p.drawLine(QPointF(pad, pad), QPointF(width()-pad, height()-pad));
    p.drawLine(QPointF(width()-pad, pad), QPointF(pad, height()-pad));
}
