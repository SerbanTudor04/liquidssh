#pragma once
#include <QWidget>

class TerminalView;
class SSHWorker;
class QThread;

#include "types/SSHHost.h"   // for HostSpec

class TerminalTab : public QWidget {
    Q_OBJECT
public:
    explicit TerminalTab(const HostSpec& spec, QWidget *parent = nullptr);
    explicit TerminalTab(const QString& host, QWidget *parent = nullptr); // legacy

    ~TerminalTab() override;

protected:
    void resizeEvent(QResizeEvent* e) override;

private slots:
    void onConnected();
    void onData(const QByteArray& b);
    void onClosed();

private:
    static QString currentUser();
    static void computeColsRows(QWidget* w, int& cols, int& rows);
    QString effUser() const;     // spec.user or env
    int     effPort() const;     // spec.port or 22

private:
    HostSpec     spec_;
    TerminalView *term_{nullptr};
    SSHWorker    *worker_{nullptr};
    QThread      *workerThread_{nullptr};
};
