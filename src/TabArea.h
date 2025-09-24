#pragma once
#include "NewHostDialog.h"
#include <QWidget>

class QTabWidget;
class InfoTab;

class TabArea : public QWidget {
    Q_OBJECT
public:
    QString labelFor(const HostSpec &s);

    explicit TabArea(QWidget *parent = nullptr);
    void setHostInfo(const QString &host);
    void openHostTab(const QString &host);
    void openNewHostDialog();
    void openHost(const HostSpec &spec);

    signals:
        void hostAdded(const HostSpec &spec);
private:
    QIcon makeCircleIcon(const QColor &c, int d = 10) const;
    int findTabByText(const QString &text) const;

    QTabWidget *tabs{};
    InfoTab *info{};
};
