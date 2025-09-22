#pragma once
#include <QTabWidget>

class CustomTabBar; // from your CustomTabBar.h

class CustomTabWidget : public QTabWidget {
    Q_OBJECT
public:
    explicit CustomTabWidget(QWidget *parent = nullptr);
};
