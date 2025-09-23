// CustomTabWidget.h
#pragma once
#include <QTabWidget>

class CustomTabWidget : public QTabWidget {
    Q_OBJECT
public:
    explicit CustomTabWidget(QWidget *parent = nullptr);

private slots:
    void onTabCloseRequested(int index);
};
