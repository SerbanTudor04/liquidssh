#include "InfoTab.h"
#include <QTextEdit>
#include <QVBoxLayout>

InfoTab::InfoTab(QWidget *parent)
    : QWidget(parent)
{
    text = new QTextEdit(this);
    text->setReadOnly(true);
    text->setText("Select a host from the sidebar...");
    auto *layout = new QVBoxLayout(this);
    layout->addWidget(text);
    setLayout(layout);
}

void InfoTab::showHostInfo(const QString &host) {
    text->setPlainText(QString("Information about host:\n\n%1\n\n"
                               "Status: Online\nUser: root\nLast seen: now")
                               .arg(host));
}



