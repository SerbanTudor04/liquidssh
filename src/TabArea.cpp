#include "CustomTabBar.h"
#include "InfoTab.h"
#include "TerminalTab.h"

#include <QPainter>
#include <QTabBar>
#include <QTabWidget>
#include <QToolButton>
#include <QVBoxLayout>
#include <qstyle.h>

#include "CustomTabWidget.h"

// icon helper from section 2
static QIcon makeStatusIcon(const QColor &c, int d = 12);
QString TabArea::labelFor(const HostSpec &s) {
  if (!s.alias.isEmpty())
    return QString("%1 (%2)").arg(s.alias, s.host);
  if (!s.user.isEmpty())
    return QString("%1@%2").arg(s.user, s.host);
  return s.host;
}

TabArea::TabArea(QWidget *parent) : QWidget(parent) {
  setAttribute(Qt::WA_TranslucentBackground, true);

  tabs = new CustomTabWidget(this);

  // DO NOT make the QTabWidget itself translucent on macOS — it confuses the
  // layout tabs->setAttribute(Qt::WA_TranslucentBackground, true);  // <--
  // remove this

  tabs->setMovable(true);
  tabs->setTabsClosable(true);
  tabs->setDocumentMode(false);
  tabs->setElideMode(Qt::ElideRight);
  tabs->setTabBarAutoHide(false);
  tabs->setIconSize(QSize(12, 12));
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

  auto *addBtn = new QToolButton(this);
  addBtn->setText("+");
  addBtn->setCursor(Qt::PointingHandCursor);
  addBtn->setAutoRaise(true);
  addBtn->setToolTip(tr("New Host…"));
  addBtn->setFixedHeight(28);
  addBtn->setStyleSheet(R"(
      QToolButton {
        color: #ECECEC;
        padding: 0 10px;
        border-radius: 14px;
        background: rgba(255,255,255,0.12);
      }
      QToolButton:hover { background: rgba(255,255,255,0.18); }
      QToolButton:pressed { background: rgba(255,255,255,0.24); }
    )");
  tabs->setCornerWidget(addBtn, Qt::TopRightCorner);
  connect(addBtn, &QToolButton::clicked, this, &TabArea::openNewHostDialog);

  // Shortcut: Cmd/Ctrl+T (Qt maps StandardKey on each platform)
  auto *newHostAct = new QAction(tr("New Host…"), this);
  newHostAct->setShortcut(QKeySequence::AddTab); // usually Cmd/Ctrl+T
  addAction(newHostAct);
  connect(newHostAct, &QAction::triggered, this, &TabArea::openNewHostDialog);

  auto *lay = new QVBoxLayout(this);
  lay->setSpacing(0);

#ifdef Q_OS_MAC
  // Titlebar height is ~22–28 px depending on theme; add a few px padding.
  const int titlebar =
      style()->pixelMetric(static_cast<QStyle::PixelMetric>(9), nullptr, this);
  lay->setContentsMargins(0, titlebar + 6, 0, 0);
#else
  lay->setContentsMargins(0, 0, 0, 0);
#endif

  lay->addWidget(tabs);
  setLayout(lay);
}

int TabArea::findTabByText(const QString &text) const {
  for (int i = 0; i < tabs->count(); ++i)
    if (tabs->tabText(i) == text)
      return i;
  return -1;
}

void TabArea::setHostInfo(const QString &host) {
  info->showHostInfo(host);
  tabs->setCurrentIndex(0);
}

void TabArea::openHostTab(const QString &host) {
  int idx = findTabByText(host);
  if (idx >= 0) {
    tabs->setCurrentIndex(idx);
    return;
  }

  auto *term = new TerminalTab(host, this);
  int newIndex = tabs->addTab(term, host);
  tabs->setCurrentIndex(newIndex);
}

// --- icon helper impl ---
static QIcon makeStatusIcon(const QColor &c, int d) {
  QPixmap px(d, d);
  px.fill(Qt::transparent);
  QPainter p(&px);
  p.setRenderHint(QPainter::Antialiasing, true);

  // soft shadow
  p.setBrush(QColor(0, 0, 0, 50));
  p.setPen(Qt::NoPen);
  p.drawEllipse(QRectF(1, 1, d - 2, d - 2));

  // main fill
  p.setBrush(c);
  p.drawEllipse(QRectF(0, 0, d - 2, d - 2));

  // rim
  p.setPen(QPen(QColor(255, 255, 255, 140), 1));
  p.setBrush(Qt::NoBrush);
  p.drawEllipse(QRectF(0.5, 0.5, d - 3, d - 3));

  // specular highlight
  QRadialGradient g(QPointF(d * 0.35, d * 0.35), d * 0.6);
  g.setColorAt(0.0, QColor(255, 255, 255, 180));
  g.setColorAt(1.0, QColor(255, 255, 255, 0));
  p.setBrush(g);
  p.setPen(Qt::NoPen);
  p.drawEllipse(QRectF(1, 1, d - 4, d - 4));
  return QIcon(px);
}

void TabArea::openNewHostDialog() {
  NewHostDialog dlg(this);
  if (dlg.exec() != QDialog::Accepted)
    return;
  const HostSpec s = dlg.spec();
  emit hostAdded(s); // <-- tell Sidebar
  openHost(s);       // open/focus the tab
}
void TabArea::openHost(const HostSpec &spec) {
  const QString lbl = labelFor(spec);

  if (int idx = findTabByText(lbl); idx >= 0) {
    tabs->setCurrentIndex(idx);
    return;
  }
  auto *term = new TerminalTab(spec, this); // <— use spec here
  const int newIndex = tabs->addTab(term, lbl);
  tabs->setCurrentIndex(newIndex);
}
