#include "HostsStore.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QStandardPaths>
#include <QDir>

static const char* kConnName = "liquidssh_hosts";

HostsStore::HostsStore(QObject *p) : QObject(p) {}
HostsStore::~HostsStore() {
    if (QSqlDatabase::contains(kConnName)) {
        QSqlDatabase::database(kConnName).close();
        QSqlDatabase::removeDatabase(kConnName);
    }
}

QString HostsStore::dbFilePath() const {
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(base);
#ifdef Q_OS_MAC
    // ends up: ~/Library/Application Support/<AppName>/
#endif
    return base + "/hosts.sqlite";
}

bool HostsStore::open() {
    if (QSqlDatabase::contains(kConnName))
        return ensureSchema();

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", kConnName);
    QString dbPath=dbFilePath();
    qDebug() << "Database: "<< dbPath;
    db.setDatabaseName(dbPath);
    if (!db.open()) {
        emit error("DB open failed: " + db.lastError().text());
        return false;
    }

    // sensible pragmas for desktop app
    QSqlQuery q(db);
    q.exec("PRAGMA journal_mode=WAL;");
    q.exec("PRAGMA synchronous=NORMAL;");
    q.exec("PRAGMA foreign_keys=ON;");

    return ensureSchema();
}

bool HostsStore::ensureSchema() {
    QSqlDatabase db = QSqlDatabase::database(kConnName);
    if (!db.isOpen()) return false;

    QSqlQuery q(db);
    auto execOrLog = [&](const char* sql) {
        if (!q.exec(QString::fromUtf8(sql))) {
            qDebug() << "Schema step failed:" << q.lastError().text() << "\nSQL:" << sql;
            return false;
        }
        return true;
    };

    // wrap in a transaction for atomicity (SQLite will auto-commit on error if we don't check)
    db.transaction();

    // 1) Table
    if (!execOrLog(
        "CREATE TABLE IF NOT EXISTS hosts ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  alias TEXT,"
        "  host  TEXT NOT NULL,"
        "  user  TEXT,"
        "  port  INTEGER NOT NULL DEFAULT 22,"
        "  key_path TEXT,"
        "  proxy_jump TEXT,"
        "  favorite INTEGER NOT NULL DEFAULT 0,"
        "  color TEXT,"
        "  created_at INTEGER NOT NULL DEFAULT (strftime('%s','now')),"
        "  updated_at INTEGER NOT NULL DEFAULT (strftime('%s','now'))"
        ");"
    )) { db.rollback(); return false; }

    // 2) Unique index
    if (!execOrLog(
        "CREATE UNIQUE INDEX IF NOT EXISTS ux_hosts_hup "
        "ON hosts(host, COALESCE(user,''), port);"
    )) { db.rollback(); return false; }

    // 3) Trigger to bump updated_at on any UPDATE
    if (!execOrLog(
        "CREATE TRIGGER IF NOT EXISTS trg_hosts_updated_at "
        "AFTER UPDATE ON hosts FOR EACH ROW "
        "BEGIN "
        "  UPDATE hosts SET updated_at=strftime('%s','now') WHERE id=NEW.id;"
        "END;"
    )) { db.rollback(); return false; }

    // 4) Optional helpful indexes
    if (!execOrLog("CREATE INDEX IF NOT EXISTS idx_hosts_host ON hosts(host);")) { db.rollback(); return false; }
    if (!execOrLog("CREATE INDEX IF NOT EXISTS idx_hosts_fav  ON hosts(favorite);")) { db.rollback(); return false; }

    db.commit();
    return true;
}


bool HostsStore::saveHost(const HostSpec &s) {
    QSqlDatabase db = QSqlDatabase::database(kConnName);
    QSqlQuery q(db);
    q.prepare(
        "INSERT INTO hosts(alias,host,user,port) "
        "VALUES(?,?,?,?) "
        "ON CONFLICT(host, COALESCE(user,''), port) DO UPDATE SET "
        " alias=excluded.alias, updated_at=strftime('%s','now')"
    );
    q.addBindValue(s.alias);
    q.addBindValue(s.host);
    q.addBindValue(s.user);
    q.addBindValue(s.port > 0 ? s.port : 22);
    if (!q.exec()) {
        emit error("saveHost failed: " + q.lastError().text());
        return false;
    }
    return true;
}

QVector<HostSpec> HostsStore::loadAll() const {
    QVector<HostSpec> out;
    QSqlDatabase db = QSqlDatabase::database(kConnName);
    QSqlQuery q(db);
    if (!q.exec("SELECT alias,host,user,port FROM hosts ORDER BY favorite DESC, alias, host")) {
        return out;
    }
    while (q.next()) {
        HostSpec s;
        s.alias = q.value(0).toString();
        s.host  = q.value(1).toString();
        s.user  = q.value(2).toString();
        s.port  = q.value(3).toInt();
        out.push_back(std::move(s));
    }
    return out;
}

bool HostsStore::removeHost(const HostSpec &s) {
    QSqlDatabase db = QSqlDatabase::database(kConnName);
    QSqlQuery q(db);
    q.prepare("DELETE FROM hosts WHERE host=? AND COALESCE(user,'')=? AND port=?");
    q.addBindValue(s.host);
    q.addBindValue(s.user);
    q.addBindValue(s.port > 0 ? s.port : 22);
    if (!q.exec()) {
        emit error("removeHost failed: " + q.lastError().text());
        return false;
    }
    return q.numRowsAffected() > 0;
}


HostsStore& HostsStore::instance() {
    static HostsStore s; // constructed on first use, destroyed on app exit
    return s;
}


QSqlError HostsStore::lastError() {
    QSqlDatabase db = QSqlDatabase::database(kConnName);
    return db.lastError();
}
