#include "CustomTabWidget.h"
#include "CustomTabBar.h"

CustomTabWidget::CustomTabWidget(QWidget *parent)
    : QTabWidget(parent)
{
    // This is allowed here because setTabBar is protected in QTabWidget,
    // but we're inside a subclass.
    setTabBar(new CustomTabBar(this));
}
