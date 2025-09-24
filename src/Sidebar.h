#pragma once
#include <QWidget>

#include "types/SSHHost.h"

class QListWidget;

class Sidebar : public QWidget {
    Q_OBJECT
public:
    explicit Sidebar(QWidget *parent = nullptr);

    signals:
        void hostSelected(const QString &host);
        void hostDoubleClicked(const QString &host);
        void hostActivated(const HostSpec &spec);   // double-click emits full spec

public slots:
    void addHost(const HostSpec &spec);         // <-- new



private:
    QListWidget *list{nullptr};
    int findItem(const QString &label) const;
    static QString labelFor(const HostSpec &s);
};
