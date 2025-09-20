#include "TabArea.h"

#include <qtabbar.h>

#include "InfoTab.h"
#include <QTabWidget>
#include <QTextEdit>
#include <QVBoxLayout>

TabArea::TabArea(QWidget *parent)
    : QWidget(parent)
{
    tabs = new QTabWidget(this);
    tabs->setTabsClosable(true);
    tabs->setMovable(true);

    info = new InfoTab(this);

    tabs->addTab(info, "Info");
    tabs->setTabsClosable(true);
    tabs->tabBar()->setTabButton(0, QTabBar::RightSide, nullptr);
    auto *layout = new QVBoxLayout(this);
    layout->addWidget(tabs);
    setLayout(layout);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setStyleSheet("background: transparent;");
//     tabs->setStyleSheet(R"(
//     QTabWidget::pane { background: transparent; border: none; }
//     QTabBar::tab { background: rgba(255,255,255,50); padding: 6px; }
//     QTabBar::tab:selected { background: rgba(255,255,255,100); }
// )");

}

void TabArea::setHostInfo(const QString &host) {
    info->showHostInfo(host);
}
void TabArea::openHostTab(const QString &host) {
    auto *term = new QTextEdit(this); // later replace with real terminal widget
    term->setPlainText(QString("SSH Session to %1").arg(host));
    int index = tabs->addTab(term, host);
    tabs->setCurrentIndex(index);
}