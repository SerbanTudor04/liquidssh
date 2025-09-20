#pragma once
#include <QWidget>

class QTextEdit;

class InfoTab : public QWidget {
    Q_OBJECT
public:
    explicit InfoTab(QWidget *parent = nullptr);
    void showHostInfo(const QString &host);

private:
    QTextEdit *text;
};
