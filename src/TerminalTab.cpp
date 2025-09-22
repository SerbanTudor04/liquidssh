#include "TerminalTab.h"
#include <QPlainTextEdit>
#include <QVBoxLayout>

TerminalTab::TerminalTab(const QString &host, QWidget *parent) : QWidget(parent) {
    term = new QPlainTextEdit(this);
    term->setReadOnly(false);
    term->setFrameStyle(QFrame::NoFrame);
    term->setAttribute(Qt::WA_TranslucentBackground, true);
    term->viewport()->setAttribute(Qt::WA_TranslucentBackground, true);

    QPalette pal = term->palette();
    pal.setColor(QPalette::Base, QColor(0,0,0,0));
    pal.setColor(QPalette::Text, QColor(230,230,230));
    term->setPalette(pal);

    term->setStyleSheet("QPlainTextEdit { background: transparent; }");
    term->setPlainText(QString("SSH Session to %1\n\n(placeholder)").arg(host));
    auto *lay = new QVBoxLayout(this);
    lay->addWidget(term);
    setLayout(lay);
}
