// CustomTabWidget.cpp
#include "CustomTabWidget.h"
#include "CustomTabBar.h"
#include <QWidget>
#include <QPointer>

#include "TerminalTab.h"

CustomTabWidget::CustomTabWidget(QWidget *parent)
    : QTabWidget(parent)
{
    setTabBar(new CustomTabBar(this));

    // 1) show close buttons
    setTabsClosable(true);

    // 2) remove the tab when its close button is clicked
    connect(this, &QTabWidget::tabCloseRequested,
            this, &CustomTabWidget::onTabCloseRequested);
}

void CustomTabWidget::onTabCloseRequested(int index)
{
    if (index == 0) return;

    QWidget *w = widget(index);
    if (auto term = qobject_cast<TerminalTab*>(w)) {
        QPointer<TerminalTab> guard = term;
        term->shutdownSessionAsync([this, index, guard]() {
            if (!guard) return; // already deleted
            removeTab(index);
            guard->deleteLater();
        });
        return;
    }

    removeTab(index);
    if (w) w->deleteLater();
}
