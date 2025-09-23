#include "GlassCloseButton.h"
#include <QPainter>

GlassCloseButton::GlassCloseButton(QWidget *parent)
    : QAbstractButton(parent) {
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::NoFocus);
    setFixedSize(16, 16);
    setToolTip(QStringLiteral("Close Tab"));
}

QSize GlassCloseButton::sizeHint() const { return {16, 16}; }

void GlassCloseButton::paintEvent(QPaintEvent *) {
    // No hover background – just the “×”
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const int pad = 4;
    const QColor c = underMouse() ? QColor(255,255,255,230)
                                  : QColor(255,255,255,190);
    p.setPen(QPen(c, 1.8, Qt::SolidLine, Qt::RoundCap));
    p.drawLine(QPointF(pad, pad), QPointF(width()-pad, height()-pad));
    p.drawLine(QPointF(width()-pad, pad), QPointF(pad, height()-pad));
}
