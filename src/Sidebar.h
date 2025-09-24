#pragma once
#include <qtextlayout.h>
// #include <QWidget>
#include <QListWidget>
#include <QLabel>
#include "types/SSHHost.h"



class Sidebar : public QWidget {
    Q_OBJECT
public:
    explicit Sidebar(QWidget *parent = nullptr);

    void addHost(const HostSpec &spec);
    int  findItem(const QString &label) const;
    static QString labelFor(const HostSpec &s);

    // Optional helper if you want to set programmatically
    void setFilterText(const QString& text);

    signals:
        void hostSelected(const QString &label);
    void hostDoubleClicked(const QString &label);
    void hostActivated(const HostSpec &spec);

private slots:
    void applyFilter();                 // runs after debounce
    void onFilterTextChanged(const QString&);

private:
    bool itemMatchesFilter(QListWidgetItem *it, const QString &needle) const;

private:
    QListWidget *list{nullptr};
    QLabel      *title{nullptr};
    QLineEdit   *search{nullptr};
    QTimer      *filterTimer{nullptr};
    static constexpr int kSpecRole = Qt::UserRole + 1;
};