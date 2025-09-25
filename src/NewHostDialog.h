#pragma once
#include <QDialog>
#include <QString>

#include "types/SSHHost.h"


class QLineEdit;
class QSpinBox;
class QPushButton;

class NewHostDialog : public QDialog {
    Q_OBJECT
public:
    explicit NewHostDialog(QWidget *parent = nullptr);
    HostSpec spec() const;
    void setSpec(const HostSpec& s);
private slots:
    void validate();

private:
    QLineEdit  *aliasEdit{nullptr};
    QLineEdit  *hostEdit{nullptr};
    QLineEdit  *userEdit{nullptr};
    QSpinBox   *portSpin{nullptr};
    QPushButton *okBtn{nullptr};
};
