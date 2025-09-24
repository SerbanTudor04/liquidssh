#include "TerminalTab.h"
#include "TerminalView.h"
#include "SSHWorker.h"

#include <QVBoxLayout>
#include <QInputDialog>
#include <QResizeEvent>
#include <QFontMetrics>
#include <QThread>

static QString envUser() {
    return qEnvironmentVariable("USER",
           qEnvironmentVariable("USERNAME", "user"));
}

QString TerminalTab::currentUser() { return envUser(); }

QString TerminalTab::effUser() const {
    return spec_.user.isEmpty() ? envUser() : spec_.user;
}

int TerminalTab::effPort() const {
    return (spec_.port >= 1 && spec_.port <= 65535) ? spec_.port : 22;
}

TerminalTab::TerminalTab(const HostSpec& spec, QWidget *parent)
    : QWidget(parent), spec_(spec)
{
    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(12,12,12,12);

    term_ = new TerminalView(this);
    lay->addWidget(term_);

    // worker thread + object
    workerThread_ = new QThread(this);
    worker_ = new SSHWorker();
    worker_->moveToThread(workerThread_);
    connect(workerThread_, &QThread::finished, worker_, &QObject::deleteLater);

    connect(worker_, &SSHWorker::connected, this, &TerminalTab::onConnected);
    connect(worker_, &SSHWorker::data,      this, &TerminalTab::onData);
    connect(worker_, &SSHWorker::closed,    this, &TerminalTab::onClosed);

    connect(worker_, &SSHWorker::error, this, [this](const QString& m){
        term_->appendRemote(("\r\n[SSH ERROR] " + m + "\r\n").toUtf8());

        // Fallback to password prompt on auth failure
        if (m.contains("Auth", Qt::CaseInsensitive)) {
            bool ok=false;
            const QString u = effUser();
            const QString prompt = QString("Password for %1@%2")
                                       .arg(u, spec_.host);
            const QString pw = QInputDialog::getText(this, tr("SSH Password"),
                                                     prompt,
                                                     QLineEdit::Password, {}, &ok);
            if (ok) {
                const int port = effPort();
                QMetaObject::invokeMethod(worker_, [this, u, pw, port](){
                    worker_->connectToHost(spec_.host, port, u,
                                           pw, /*usePassword*/true, QString());
                }, Qt::QueuedConnection);
            }
        }
    });

    connect(term_, &TerminalView::sendBytes,
            worker_, &SSHWorker::writeData, Qt::QueuedConnection);

    workerThread_->start();

    // Initiate connection with key auth first
    const QString u = effUser();
    const int port = effPort();
    qDebug() << "Connecting as" << effUser() << "to" << spec_.host << ":" << effPort();

    QMetaObject::invokeMethod(worker_, [this, u, port](){
        worker_->connectToHost(spec_.host, port, u,
                               QString(), /*usePassword*/false, QString());
    }, Qt::QueuedConnection);

    term_->appendRemote(QString("Connecting to %1 as %2 ...\r\n")
                        .arg(spec_.host, u).toUtf8());
}

TerminalTab::TerminalTab(const QString& host, QWidget *parent)
    : TerminalTab(HostSpec{QString(), host, QString(), 22}, parent) {}

TerminalTab::~TerminalTab() {
    if (worker_) {
        QMetaObject::invokeMethod(worker_, &SSHWorker::disconnectFromHost,
                                  Qt::BlockingQueuedConnection);
    }
    workerThread_->quit();
    workerThread_->wait();
}

void TerminalTab::computeColsRows(QWidget* w, int& cols, int& rows) {
    QFontMetrics fm(w->font());
    const int cw = qMax(6, fm.horizontalAdvance(QLatin1Char('M')));
    const int ch = qMax(10, fm.height());
    cols = qMax(20, w->width()  / cw);
    rows = qMax(10, w->height() / ch);
}

void TerminalTab::resizeEvent(QResizeEvent* e) {
    QWidget::resizeEvent(e);
    if (!worker_) return;
    int c=120, r=32; computeColsRows(term_, c, r);
    QMetaObject::invokeMethod(worker_, [=](){
        worker_->setPtySize(c, r);
    }, Qt::QueuedConnection);
}

void TerminalTab::onConnected() {
    term_->appendRemote("[SSH] Connected. Starting shell...\r\n");
    int cols=120, rows=32; computeColsRows(term_, cols, rows);
    QMetaObject::invokeMethod(worker_, [=](){
        worker_->setPtySize(cols, rows);
    }, Qt::QueuedConnection);
}

void TerminalTab::onData(const QByteArray& b) {
    term_->appendRemote(b);
}

void TerminalTab::onClosed() {
    term_->appendRemote("\r\n[SSH] Connection closed.\r\n");
}
