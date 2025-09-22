#pragma once
#include <QWidget>

class QTabWidget;
class InfoTab;

class TabArea : public QWidget {
    Q_OBJECT
public:
    explicit TabArea(QWidget *parent = nullptr);
    void setHostInfo(const QString &host);
    void openHostTab(const QString &host);

private:
    QIcon makeCircleIcon(const QColor &c, int d = 10) const;
    int findTabByText(const QString &text) const;

    QTabWidget *tabs{};
    InfoTab *info{};
};
