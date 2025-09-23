#pragma once
#include <QPlainTextEdit>
#include <QRegularExpression>

class TerminalView : public QPlainTextEdit {
    Q_OBJECT
public:
    explicit TerminalView(QWidget *parent=nullptr);

    signals:
        void sendBytes(const QByteArray& data);

public slots:
    void appendRemote(const QByteArray& data);

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;

private:
    QByteArray ansiStrip(const QByteArray& in) const;
};
