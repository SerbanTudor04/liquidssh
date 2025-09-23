#include "TerminalTab.h"
#include "TerminalView.h"
#include "SSHWorker.h"

#include <QVBoxLayout>
#include <QInputDialog>
#include <QResizeEvent>
#include <QFontMetrics>

static QString currentUser() {
    return qEnvironmentVariable("USER",
           qEnvironmentVariable("USERNAME", "user"));
}

TerminalTab::TerminalTab(const QString& host, QWidget *parent)
    : QWidget(parent), host_(host)
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
    connect(worker_, &SSHWorker::data, this, &TerminalTab::onData);
    connect(worker_, &SSHWorker::closed, this, &TerminalTab::onClosed);
    connect(worker_, &SSHWorker::error, this, [this](const QString& m){
        term_->appendRemote(("\r\n[SSH ERROR] " + m + "\r\n").toUtf8());
        // Fallback to password prompt on auth failure
        if (m.contains("Auth", Qt::CaseInsensitive)) {
            bool ok=false;
            QString pw = QInputDialog::getText(this, "SSH Password",
                QString("Password for %1@%2").arg(currentUser(), host_),
                QLineEdit::Password, {}, &ok);
            if (ok) {
                QMetaObject::invokeMethod(worker_, [this,pw](){
                    worker_->connectToHost(host_, 22, currentUser(),
                                           pw, true/*use password*/, QString());
                }, Qt::QueuedConnection);
            }
        }
    });

    connect(term_, &TerminalView::sendBytes,
            worker_, &SSHWorker::writeData, Qt::QueuedConnection);

    workerThread_->start();
    // Initiate connection with key auth first
    QMetaObject::invokeMethod(worker_, [this](){
        worker_->connectToHost(host_, 22, currentUser(),
                               QString(), false/*usePassword*/, QString());
    }, Qt::QueuedConnection);

    term_->appendRemote(QString("Connecting to %1 as %2 ...\r\n")
                        .arg(host_, currentUser()).toUtf8());
}

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
