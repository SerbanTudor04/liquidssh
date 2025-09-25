#include "NewHostDialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QIntValidator>

NewHostDialog::NewHostDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle(tr("New Host…"));
    setModal(true);

    aliasEdit = new QLineEdit(this);
    aliasEdit->setPlaceholderText(tr("Optional (e.g., dev-mac)"));

    hostEdit = new QLineEdit(this);
    hostEdit->setPlaceholderText(tr("Hostname or IP (required)"));

    userEdit = new QLineEdit(this);
    userEdit->setPlaceholderText(tr("Optional (e.g., root)"));

    portSpin = new QSpinBox(this);
    portSpin->setRange(1, 65535);
    portSpin->setValue(22);

    auto *form = new QFormLayout;
    form->addRow(tr("Display name:"), aliasEdit);
    form->addRow(tr("Host:"),         hostEdit);
    form->addRow(tr("User:"),         userEdit);
    form->addRow(tr("Port:"),         portSpin);

    auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    okBtn = bb->button(QDialogButtonBox::Ok);
    okBtn->setEnabled(false);

    connect(bb, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(bb, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *root = new QVBoxLayout(this);
    root->addLayout(form);
    root->addWidget(bb);

    connect(hostEdit, &QLineEdit::textChanged, this, &NewHostDialog::validate);
    validate();

#ifdef Q_OS_MAC
    // Light padding to match your pill tabs
    setContentsMargins(12, 12, 12, 12);
#endif
}

void NewHostDialog::validate() {
    okBtn->setEnabled(!hostEdit->text().trimmed().isEmpty());
}

HostSpec NewHostDialog::spec() const {
    HostSpec s;
    s.alias = aliasEdit->text().trimmed();
    s.host  = hostEdit->text().trimmed();
    s.user  = userEdit->text().trimmed();
    s.port  = portSpin->value();
    return s;
}


void NewHostDialog::setSpec(const HostSpec& s) {
    // prefill fields
    aliasEdit->setText(s.alias);
    userEdit->setText(s.user);
    hostEdit->setText(s.host);
    portSpin->setValue(s.port > 0 ? s.port : 22);
    // any key path/auth fields as well
}
