#include "TerminalTab.h"
#include <QPlainTextEdit>
#include <QVBoxLayout>

TerminalTab::TerminalTab(const QString &host, QWidget *parent) : QWidget(parent) {
    term = new QPlainTextEdit(this);
    term->setReadOnly(false);
    term->setPlainText(QString("SSH Session to %1\n\n(placeholder)").arg(host));
    auto *lay = new QVBoxLayout(this);
    lay->addWidget(term);
    setLayout(lay);
}
