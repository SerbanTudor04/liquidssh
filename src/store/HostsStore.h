#pragma once
#include <QObject>
#include <qsqldatabase.h>
#include <qsqlerror.h>
#include <QVector>

#include "types/SSHHost.h"

class HostsStore : public QObject {
    Q_OBJECT
public:
    static HostsStore& instance();
    explicit HostsStore(QObject *parent = nullptr);
    ~HostsStore();

    bool open();                                   // open or create DB
    bool saveHost(const HostSpec &spec);           // upsert by (host,user,port)
    QVector<HostSpec> loadAll() const;             // list all
    bool removeHost(const HostSpec &spec);         // optional
    QSqlError lastError();
    signals:
        void error(const QString &msg);




private:
    bool ensureSchema();
    QString dbFilePath() const;



};
