#pragma once
#include <QPlainTextEdit>

class TerminalView : public QPlainTextEdit {
    Q_OBJECT
public:
    explicit TerminalView(QWidget *parent=nullptr);

    signals:
        void sendBytes(const QByteArray& data);

public slots:
    void appendRemote(const QByteArray& data);
    void clearScreen();

protected:
    void keyPressEvent(QKeyEvent *e) override;

private:
    QByteArray ansiStrip(const QByteArray& in) const;
};
