#include "TabArea.h"
#include "InfoTab.h"
#include "TerminalTab.h"

#include <QTabWidget>
#include <QTabBar>
#include <QVBoxLayout>

TabArea::TabArea(QWidget *parent) : QWidget(parent) {
    tabs = new QTabWidget(this);
    tabs->setMovable(true);
    tabs->setTabsClosable(true);
    tabs->setDocumentMode(true);
    tabs->setElideMode(Qt::ElideRight);
    tabs->setTabBarAutoHide(false);

    info = new InfoTab(this);
    int infoIndex = tabs->addTab(info, "Info");

    // Make "Info" not closable
    tabs->tabBar()->setTabButton(infoIndex, QTabBar::RightSide, nullptr);

    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0,0,0,0);
    lay->addWidget(tabs);
    setLayout(lay);

    // Optional styling so blur shows through
    setAttribute(Qt::WA_TranslucentBackground, true);
    tabs->setStyleSheet(R"(
        QTabWidget::pane { background: transparent; border: 1px solid rgba(255,255,255,0.15); }
        QTabBar::tab { padding: 6px 10px; margin: 2px; }
    )");
}

int TabArea::findTabByText(const QString &text) const {
    for (int i = 0; i < tabs->count(); ++i) {
        if (tabs->tabText(i) == text) return i;
    }
    return -1;
}

void TabArea::setHostInfo(const QString &host) {
    info->showHostInfo(host);           // replace content
    tabs->setCurrentIndex(0);           // focus Info tab
}

void TabArea::openHostTab(const QString &host) {
    // Avoid duplicates: if a tab with same title exists, just activate it
    int idx = findTabByText(host);
    if (idx >= 0) {
        tabs->setCurrentIndex(idx);
        return;
    }

    auto *term = new TerminalTab(host, this);
    int newIndex = tabs->addTab(term, host);
    tabs->setCurrentIndex(newIndex);
}
