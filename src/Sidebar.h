// Sidebar.h
#pragma once
#include <QWidget>

class QListWidget;
class QListWidgetItem;
class QLineEdit;
class QLabel;
class QPushButton;
class QTimer;

#include "types/SSHHost.h"
#include "NewHostDialog.h"        // <-- use your existing dialog

class Sidebar : public QWidget {
    Q_OBJECT
public:
    explicit Sidebar(QWidget *parent = nullptr);

    void addHost(const HostSpec &spec);
    int  findItem(const QString &label) const;
    static QString labelFor(const HostSpec &s);
    void setFilterText(const QString& text);

    signals:
        void hostSelected(const QString &label);
    void hostDoubleClicked(const QString &label);
    void hostActivated(const HostSpec &spec);

    void hostEdited(const HostSpec& oldSpec, const HostSpec& newSpec);
    void hostRemoved(const HostSpec& spec);

private slots:
    void applyFilter();
    void onFilterTextChanged(const QString&);
    void onAddClicked();                         // will open NewHostDialog
    void onListContextMenuRequested(const QPoint& pos);

private:
    bool itemMatchesFilter(QListWidgetItem *it, const QString &needle) const;

    QListWidget *list{nullptr};
    QLabel      *title{nullptr};
    QPushButton *addLink{nullptr};
    QLineEdit   *search{nullptr};
    QTimer      *filterTimer{nullptr};

    static constexpr int kSpecRole = Qt::UserRole + 1;
};
