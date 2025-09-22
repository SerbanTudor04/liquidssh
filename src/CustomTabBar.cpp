#include "CustomTabBar.h"
#include <QPainter>
#include <QStyleOptionTab>

CustomTabBar::CustomTabBar(QWidget *parent) : QTabBar(parent) {
    setExpanding(false);          // tabs keep natural width
    setDrawBase(false);           // no native base line
    setUsesScrollButtons(true);
    setElideMode(Qt::ElideRight);
    setIconSize(QSize(12,12));
    setContentsMargins(8,8,8,4);  // inner padding so the capsule looks airy
}

QSize CustomTabBar::tabSizeHint(int index) const {
    QSize s = QTabBar::tabSizeHint(index);
    s.setHeight(qMax(28, s.height())); // taller, pill-like
    s.setWidth(s.width() + 14);        // add horizontal breathing room
    return s;
}

void CustomTabBar::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    // --- Draw the capsule tray behind the tabs (Apple Music-like) ---
    const int radius = 18;
    QRectF tray = rect().adjusted(6, 2, -6, -2);
    QColor trayFill(255,255,255, 26);     // subtle translucent white
    QColor trayStroke(255,255,255, 42);

    p.setPen(QPen(trayStroke, 1.0));
    p.setBrush(trayFill);
    p.drawRoundedRect(tray, radius, radius);

    // Light inner stroke to suggest “glass”
    p.setPen(QPen(QColor(255,255,255,22), 1));
    QRectF inner = tray.adjusted(1,1,-1,-1);
    p.drawRoundedRect(inner, radius-1, radius-1);

    // --- Let the style draw each tab on top (we rely on stylesheet below) ---
    for (int i = 0; i < count(); ++i) {
        QStyleOptionTab opt;
        initStyleOption(&opt, i);
        style()->drawControl(QStyle::CE_TabBarTab, &opt, &p, this);
    }
}
