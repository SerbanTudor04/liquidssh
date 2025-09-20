#pragma once
#include <QWidget>

class QListWidget;

class Sidebar : public QWidget {
    Q_OBJECT
public:
    explicit Sidebar(QWidget *parent = nullptr);

    signals:
        void hostSelected(const QString &host);
        void hostDoubleClicked(const QString &host);



private:
    QListWidget *list;
};
