#pragma once
#include <QWidget>
class QPlainTextEdit;

class TerminalTab : public QWidget {
    Q_OBJECT
public:
    explicit TerminalTab(const QString &host, QWidget *parent = nullptr);
private:
    QPlainTextEdit *term{};
};
