#include "InfoTab.h"
#include <QTextEdit>
#include <QVBoxLayout>

InfoTab::InfoTab(QWidget *parent) : QWidget(parent) {
    text = new QTextEdit(this);
    text->setReadOnly(true);
    text->setFrameStyle(QFrame::NoFrame);
    text->setAttribute(Qt::WA_TranslucentBackground, true);
    text->viewport()->setAttribute(Qt::WA_TranslucentBackground, true);
    // palette Base controls the editor background
    QPalette pal = text->palette();
    pal.setColor(QPalette::Base, QColor(0,0,0,0));
    pal.setColor(QPalette::Text, QColor(255,255,255)); // readable on dark glass
    text->setPalette(pal);

    // belt & suspenders via stylesheet
    text->setStyleSheet("QTextEdit { background: transparent; }");
    text->setPlainText("Select a host from the sidebar...");

    auto *lay = new QVBoxLayout(this);
    lay->addWidget(text);

    setLayout(lay);
}

void InfoTab::showHostInfo(const QString &host) {
    text->clear();
    text->setPlainText(QString(
        "Information about host:\n\n"
        "%1\n\n"
        "Status: Online\n"
        "User: root\n"
        "Last seen: now"
    ).arg(host));
}


