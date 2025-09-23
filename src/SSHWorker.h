#pragma once
#include <QObject>
#include <QAtomicInt>

struct ssh_session_struct;
struct ssh_channel_struct;
typedef ssh_session_struct* ssh_session;
typedef ssh_channel_struct* ssh_channel;

class SSHWorker : public QObject {
    Q_OBJECT
public:
    explicit SSHWorker(QObject *parent=nullptr);
    ~SSHWorker();

public slots:
    void connectToHost(QString host, int port, QString user,
                       QString password, bool usePassword,
                       QString keyPassphrase);
    void writeData(QByteArray data);
    void setPtySize(int cols, int rows);
    void disconnectFromHost();

    signals:
        void connected();
    void data(const QByteArray& bytes);
    void error(const QString& msg);
    void closed();

private:
    void readerLoop();

    ssh_session session_ = nullptr;
    ssh_channel channel_ = nullptr;
    QAtomicInt running_{0};
};
