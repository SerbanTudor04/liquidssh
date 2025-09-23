#pragma once
#include <QWidget>
#include <QThread>

class TerminalView;
class SSHWorker;

class TerminalTab : public QWidget {
    Q_OBJECT
public:
    explicit TerminalTab(const QString& host, QWidget *parent=nullptr);
    ~TerminalTab();

protected:
    void resizeEvent(QResizeEvent* e) override;

private slots:
    void onConnected();
    void onData(const QByteArray&);
    void onClosed();

private:
    void startSSH(const QString& host);
    static void computeColsRows(QWidget* w, int& cols, int& rows);

    TerminalView* term_ = nullptr;
    QThread*      workerThread_ = nullptr;
    SSHWorker*    worker_ = nullptr;
    QString       host_;
};
