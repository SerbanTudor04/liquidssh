#include "SSHWorker.h"
#include <libssh/libssh.h>
#include <QThread>

SSHWorker::SSHWorker(QObject *p): QObject(p) {}
SSHWorker::~SSHWorker() { disconnectFromHost(); }

void SSHWorker::connectToHost(QString host, int port, QString user,
                              QString password, bool usePassword,
                              QString keyPassphrase)
{
    if (session_) disconnectFromHost();

    session_ = ssh_new();
    if (!session_) { emit error("ssh_new failed"); return; }

    ssh_options_set(session_, SSH_OPTIONS_HOST, host.toUtf8().constData());
    ssh_options_set(session_, SSH_OPTIONS_USER, user.toUtf8().constData());
    ssh_options_set(session_, SSH_OPTIONS_PORT, &port);

    if (ssh_connect(session_) != SSH_OK) {
        emit error(QString("ssh_connect: %1").arg(ssh_get_error(session_)));
        ssh_free(session_); session_ = nullptr; return;
    }

    int state = ssh_is_server_known(session_);
    if (state == SSH_SERVER_NOT_KNOWN || state == SSH_SERVER_FILE_NOT_FOUND) {
        if (ssh_write_knownhost(session_) != SSH_OK) {
            emit error("Failed to write known_hosts");
        }
    } else if (state != SSH_SERVER_KNOWN_OK) {
        emit error("Host key verification failed");
        disconnectFromHost(); return;
    }

    int rc;
    if (usePassword) {
        rc = ssh_userauth_password(session_, nullptr, password.toUtf8().constData());
    } else {
        rc = ssh_userauth_publickey_auto(session_, nullptr,
                                         keyPassphrase.isEmpty() ? nullptr : keyPassphrase.toUtf8().constData());
    }
    if (rc != SSH_OK) {
        emit error(QString("Auth failed: %1").arg(ssh_get_error(session_)));
        disconnectFromHost(); return;
    }

    channel_ = ssh_channel_new(session_);
    if (!channel_) { emit error("ssh_channel_new failed"); disconnectFromHost(); return; }

    if (ssh_channel_open_session(channel_) != SSH_OK) {
        emit error("channel_open_session failed"); disconnectFromHost(); return;
    }

    ssh_channel_request_pty(channel_);
    ssh_channel_change_pty_size(channel_, 120, 32);
    if (ssh_channel_request_shell(channel_) != SSH_OK) {
        emit error("channel_request_shell failed"); disconnectFromHost(); return;
    }

    running_.storeRelease(1);
    emit connected();
    readerLoop();
}

void SSHWorker::readerLoop() {
    QByteArray buf; buf.resize(16 * 1024);
    while (running_.loadAcquire()) {
        int n = ssh_channel_read_nonblocking(channel_, buf.data(), buf.size(), 0);
        if (n > 0) {
            emit data(QByteArray(buf.constData(), n));
        } else if (n == SSH_EOF || ssh_channel_is_closed(channel_)) {
            break;
        }
        QThread::msleep(10);
    }
    emit closed();
}

void SSHWorker::writeData(QByteArray data) {
    if (!channel_ || data.isEmpty()) return;
    (void)ssh_channel_write(channel_, data.constData(), data.size());
}

void SSHWorker::setPtySize(int cols, int rows) {
    if (channel_) ssh_channel_change_pty_size(channel_, cols, rows);
}

void SSHWorker::disconnectFromHost() {
    running_.storeRelease(0);
    if (channel_) {
        ssh_channel_send_eof(channel_);
        ssh_channel_close(channel_);
        ssh_channel_free(channel_);
        channel_ = nullptr;
    }
    if (session_) {
        ssh_disconnect(session_);
        ssh_free(session_);
        session_ = nullptr;
    }
}
