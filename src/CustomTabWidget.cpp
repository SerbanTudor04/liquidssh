// CustomTabWidget.cpp
#include "CustomTabWidget.h"
#include "CustomTabBar.h"
#include <QWidget>

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
    if (index == 0)
        return;

    QWidget *w = widget(index);   // page widget
    removeTab(index);             // detach from tab widget
    if (w) w->deleteLater();      // free it (or keep it if you manage elsewhere)
}
