#pragma once
#include <QWidget>
class QTabWidget; class InfoTab;

class TabArea : public QWidget {
    Q_OBJECT
public:
    explicit TabArea(QWidget *parent = nullptr);
    void setHostInfo(const QString &host);    // update Info tab
    void openHostTab(const QString &host);    // open / focus session

private:
    int findTabByText(const QString &text) const;

    QTabWidget *tabs{};
    InfoTab *info{};
};
