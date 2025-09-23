#include "TabArea.h"
#include "InfoTab.h"
#include "TerminalTab.h"
#include "CustomTabBar.h"

#include <QTabWidget>
#include <QTabBar>
#include <QVBoxLayout>
#include <QPainter>
#include <qstyle.h>

#include "CustomTabWidget.h"

// icon helper from section 2
static QIcon makeStatusIcon(const QColor &c, int d = 12);

TabArea::TabArea(QWidget *parent) : QWidget(parent) {
    setAttribute(Qt::WA_TranslucentBackground, true);

    tabs = new CustomTabWidget(this);

    // DO NOT make the QTabWidget itself translucent on macOS — it confuses the layout
    // tabs->setAttribute(Qt::WA_TranslucentBackground, true);  // <-- remove this

    tabs->setMovable(true);
    tabs->setTabsClosable(true);
    tabs->setDocumentMode(false);
    tabs->setElideMode(Qt::ElideRight);
    tabs->setTabBarAutoHide(false);
    tabs->setIconSize(QSize(12,12));
    tabs->setTabPosition(QTabWidget::North);
    tabs->tabBar()->setExpanding(false);

    tabs->setStyleSheet(R"(
    QTabBar { background: transparent; qproperty-drawBase: 0; }
    QTabBar::tab {
      color: #ECECEC;
      border: none;            /* painter handles strokes */
      border-radius: 16px;
      padding: 6px 12px;
      margin: 6px 6px 0 6px;
    }
    QTabWidget::pane {
      background: transparent;
      border: 1px solid rgba(255,255,255,0.10);
      border-top: none;
      padding-top: 8px;
    }
    QTabWidget::tab-bar { alignment: left; }
    QTabBar::close-button { width: 0; height: 0; } /* disable native one */


    )");

    info = new InfoTab(this);
    int infoIndex = tabs->addTab(info, "Info");
    tabs->tabBar()->setTabButton(infoIndex, QTabBar::RightSide, nullptr);

    auto *lay = new QVBoxLayout(this);
    lay->setSpacing(0);

#ifdef Q_OS_MAC
    // Titlebar height is ~22–28 px depending on theme; add a few px padding.
    const int titlebar = style()->pixelMetric(static_cast<QStyle::PixelMetric>(9), nullptr, this);
    lay->setContentsMargins(0, titlebar + 6, 0, 0);
#else
    lay->setContentsMargins(0, 0, 0, 0);
#endif

    lay->addWidget(tabs);
    setLayout(lay);

}


int TabArea::findTabByText(const QString &text) const {
    for (int i = 0; i < tabs->count(); ++i)
        if (tabs->tabText(i) == text) return i;
    return -1;
}

void TabArea::setHostInfo(const QString &host) {
    info->showHostInfo(host);
    tabs->setCurrentIndex(0);
}

void TabArea::openHostTab(const QString &host) {
    int idx = findTabByText(host);
    if (idx >= 0) { tabs->setCurrentIndex(idx); return; }

    auto *term = new TerminalTab(host, this);
    int newIndex = tabs->addTab(term, host);
    tabs->setCurrentIndex(newIndex);
}

// --- icon helper impl ---
static QIcon makeStatusIcon(const QColor &c, int d) {
    QPixmap px(d, d); px.fill(Qt::transparent);
    QPainter p(&px); p.setRenderHint(QPainter::Antialiasing, true);

    // soft shadow
    p.setBrush(QColor(0,0,0,50)); p.setPen(Qt::NoPen);
    p.drawEllipse(QRectF(1,1,d-2,d-2));

    // main fill
    p.setBrush(c); p.drawEllipse(QRectF(0,0,d-2,d-2));

    // rim
    p.setPen(QPen(QColor(255,255,255,140), 1));
    p.setBrush(Qt::NoBrush);
    p.drawEllipse(QRectF(0.5,0.5,d-3,d-3));

    // specular highlight
    QRadialGradient g(QPointF(d*0.35, d*0.35), d*0.6);
    g.setColorAt(0.0, QColor(255,255,255,180));
    g.setColorAt(1.0, QColor(255,255,255,0));
    p.setBrush(g); p.setPen(Qt::NoPen);
    p.drawEllipse(QRectF(1,1,d-4,d-4));
    return QIcon(px);
}
