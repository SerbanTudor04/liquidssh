#pragma once
#include <QObject>
#include <QVector>

#include "types/SSHHost.h"

class HostsStore : public QObject {
    Q_OBJECT
public:
    explicit HostsStore(QObject *parent = nullptr);
    ~HostsStore();

    bool open();                                   // open or create DB
    bool saveHost(const HostSpec &spec);           // upsert by (host,user,port)
    QVector<HostSpec> loadAll() const;             // list all
    bool removeHost(const HostSpec &spec);         // optional

    signals:
        void error(const QString &msg);

private:
    bool ensureSchema();
    QString dbFilePath() const;
};
