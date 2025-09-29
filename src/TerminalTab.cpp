#include "TerminalTab.h"
#include "TerminalView.h"
#include "SSHWorker.h"

#include <QVBoxLayout>
#include <QInputDialog>
#include <QResizeEvent>
#include <QFontMetrics>
#include <QThread>
#include <QTimer>
#include <QPointer>
#include <QObject>

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
                    term_->clearScreen();

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
    QMetaObject::invokeMethod(worker_, [=, this](){ worker_->setPtySize(c, r); }, Qt::QueuedConnection);
}

void TerminalTab::onConnected() {
    term_->clearScreen();
    term_->appendRemote("[SSH] Connected. Starting shell...\r\n");
    term_->clearScreen();

    int cols=120, rows=32; computeColsRows(term_, cols, rows);
    QMetaObject::invokeMethod(worker_, [=, this](){ worker_->setPtySize(cols, rows); }, Qt::QueuedConnection);
}

void TerminalTab::onData(const QByteArray& b) {
    term_->appendRemote(b);
}

void TerminalTab::onClosed() {
    term_->appendRemote("\r\n[SSH] Connection closed.\r\n");
}

void TerminalTab::shutdownSessionAsync(std::function<void()> done) {
    if (shuttingDown_) { // already in progress
        QMetaObject::invokeMethod(this, [d = std::move(done)]{ if (d) d(); }, Qt::QueuedConnection);
        return;
    }
    shuttingDown_ = true;

    // 1) Stop sending user input to a dying backend
    if (term_ && worker_) {
        QObject::disconnect(term_, &TerminalView::sendBytes, worker_, nullptr);
    }

    // 2) Disconnect all signals from worker -> this widget to prevent use-after-free
    if (worker_) {
        QObject::disconnect(worker_, nullptr, this, nullptr);
    }

    // We’ll still need one last notification that the worker closed,
    // so wire a *temporary* one-shot connection:
    QPointer<TerminalTab> self(this);
    auto finish = [this, self, done = std::move(done)]() mutable {
        if (!self) return;

        // Ensure we don't process more worker signals
        if (worker_) QObject::disconnect(worker_, nullptr, this, nullptr);

        // 3) Stop the thread cleanly
        if (workerThread_) {
            workerThread_->quit();
            workerThread_->wait(1500);
        }

        // 4) Delete worker & thread objects on GUI thread
        if (worker_)       { worker_->deleteLater();       worker_ = nullptr; }
        if (workerThread_) { workerThread_->deleteLater(); workerThread_ = nullptr; }

        shuttingDown_ = false;
        if (done) done();
    };

    if (worker_) {
        // One-shot finish on normal close
        QObject::connect(
            worker_, &SSHWorker::closed,
            this,
            [finish]() mutable { finish(); },
            Qt::SingleShotConnection
        );

        // 2a) Ask backend to stop
        QMetaObject::invokeMethod(worker_, "disconnectFromHost", Qt::QueuedConnection);
        // (Optional) If your SSHWorker supports a soft stop: emit requestStop();
        // QMetaObject::invokeMethod(worker_, "stop", Qt::QueuedConnection);
    }

    // 2b) Safety net: force finish if the worker doesn’t close in time
    QTimer::singleShot(2000, this, [this, finish = std::move(finish)]() mutable {
        if (!worker_) { finish(); return; }
        QMetaObject::invokeMethod(worker_, "forceClose", Qt::QueuedConnection);
        QTimer::singleShot(300, this, [this, finish = std::move(finish)]() mutable {
            finish();
        });
    });

}