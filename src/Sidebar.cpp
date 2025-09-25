#include "Sidebar.h"
#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTimer>
#include <QMenu>
#include <QInputDialog>
#include <QMessageBox>
#include <QPushButton>

QString Sidebar::labelFor(const HostSpec &s) {
    if (!s.alias.isEmpty()) return QString("%1 (%2)").arg(s.alias, s.host);
    if (!s.user.isEmpty())  return QString("%1@%2").arg(s.user, s.host);
    return s.host;
}

int Sidebar::findItem(const QString &label) const {
    for (int i = 0; i < list->count(); ++i)
        if (list->item(i)->text() == label) return i;
    return -1;
}

void Sidebar::addHost(const HostSpec &spec) {
    const QString lbl = labelFor(spec);
    if (int idx = findItem(lbl); idx >= 0) { list->setCurrentRow(idx); return; }
    auto *it = new QListWidgetItem(lbl);
    it->setData(kSpecRole, QVariant::fromValue(spec));
    list->addItem(it);
    list->setCurrentItem(it);
}

Sidebar::Sidebar(QWidget *parent) : QWidget(parent) {
    // --- Header: "Hosts" + underlined Add
    title  = new QLabel(tr("Hosts"), this);
    title->setStyleSheet(R"(QLabel { color: white; font-size: 14px; font-weight: 600; letter-spacing: .3px; })");

    addLink = new QPushButton(tr("Add"), this);
    addLink->setCursor(Qt::PointingHandCursor);
    addLink->setFlat(true);
    addLink->setFocusPolicy(Qt::NoFocus);
    addLink->setStyleSheet(R"(
        QPushButton {
            background: transparent; border: none; color: white; text-decoration: underline;
            padding: 0; margin: 0; font-size: 13px; font-weight: 500;
        }
        QPushButton:hover { color: #E6E6E6; }
        QPushButton:pressed { color: #CFCFCF; }
    )");
    connect(addLink, &QPushButton::clicked, this, &Sidebar::onAddClicked);

    auto *header = new QWidget(this);
    auto *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(0,0,0,0);
    headerLayout->addWidget(title);
    headerLayout->addStretch(1);
    headerLayout->addWidget(addLink);

    // --- Search
    search = new QLineEdit(this);
    search->setPlaceholderText(tr("Search hosts…"));
    search->setClearButtonEnabled(true);
    search->setStyleSheet(R"(
        QLineEdit {
            background: rgba(255,255,255,0.12);
            border: 1px solid rgba(255,255,255,0.18);
            border-radius: 14px;
            padding: 8px 10px;
            color: white;
            selection-background-color: rgba(255,255,255,0.35);
        }
        QLineEdit:focus { border: 1px solid rgba(255,255,255,0.35); background: rgba(255,255,255,0.18); }
    )");

    // --- List
    list = new QListWidget(this);
    list->setSelectionMode(QAbstractItemView::SingleSelection);
    list->setSelectionBehavior(QAbstractItemView::SelectItems);
    list->setSelectionRectVisible(false);
    list->setFocusPolicy(Qt::NoFocus);
    list->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(list, &QListWidget::customContextMenuRequested, this, &Sidebar::onListContextMenuRequested);

    setAttribute(Qt::WA_TranslucentBackground, true);
    setStyleSheet("background: transparent;");
    list->setAttribute(Qt::WA_TranslucentBackground, true);
    list->viewport()->setAttribute(Qt::WA_TranslucentBackground, true);
    list->setStyleSheet(R"(
      QListWidget { background: transparent; border: none; }
      QListWidget::item { background: transparent; border-radius: 18px; margin: 6px; padding: 10px 14px; color: white; }
      QListWidget::item:hover   { background: rgba(255,255,255,0.12); }
      QListWidget::item:selected{ background: rgba(255,255,255,0.35); color: black; }
    )");

    // --- Layout
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(8,8,8,8);
    layout->setSpacing(8);
    layout->addWidget(header);
    layout->addWidget(search);
    layout->addWidget(list, 1);
    setLayout(layout);

    // --- Filtering
    filterTimer = new QTimer(this);
    filterTimer->setSingleShot(true);
    filterTimer->setInterval(120);
    connect(search, &QLineEdit::textChanged, this, &Sidebar::onFilterTextChanged);
    connect(filterTimer, &QTimer::timeout, this, &Sidebar::applyFilter);

    // --- Item interactions
    connect(list, &QListWidget::itemClicked, this, [this](QListWidgetItem *it){
        list->clearSelection();
        it->setSelected(true);
        list->setCurrentItem(it);
        emit hostSelected(it->text());
    });
    connect(list, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *it){
        const QVariant v = it->data(kSpecRole);
        if (v.isValid() && v.canConvert<HostSpec>())
            emit hostActivated(v.value<HostSpec>());
        emit hostDoubleClicked(it->text());
    });
    connect(search, &QLineEdit::returnPressed, this, [this]{
        if (auto *it = list->currentItem()) {
            const QVariant v = it->data(kSpecRole);
            if (v.isValid() && v.canConvert<HostSpec>())
                emit hostActivated(v.value<HostSpec>());
        }
    });
}

void Sidebar::onFilterTextChanged(const QString&) { filterTimer->start(); }

bool Sidebar::itemMatchesFilter(QListWidgetItem *it, const QString &needle) const {
    if (needle.isEmpty()) return true;
    const QString low = needle.trimmed().toLower();
    if (low.isEmpty()) return true;
    if (it->text().toLower().contains(low)) return true;

    const QVariant v = it->data(kSpecRole);
    if (v.isValid() && v.canConvert<HostSpec>()) {
        const HostSpec s = v.value<HostSpec>();
        if (s.host.toLower().contains(low))  return true;
        if (s.user.toLower().contains(low))  return true;
        if (s.alias.toLower().contains(low)) return true;
    }
    return false;
}

void Sidebar::applyFilter() {
    const QString needle = search->text();
    for (int i = 0; i < list->count(); ++i) {
        auto *it = list->item(i);
        it->setHidden(!itemMatchesFilter(it, needle));
    }
    if (auto *cur = list->currentItem(); cur && cur->isHidden())
        list->setCurrentItem(nullptr);
    if (!list->currentItem()) {
        for (int i = 0; i < list->count(); ++i) {
            auto *it = list->item(i);
            if (!it->isHidden()) { list->setCurrentItem(it); break; }
        }
    }
}

void Sidebar::setFilterText(const QString& text) {
    search->setText(text);
    applyFilter();
}

void Sidebar::onAddClicked() {
    NewHostDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) return;
    const HostSpec s = dlg.spec();
    addHost(s);
    // Optional: immediately trigger a connection
    emit hostActivated(s);
}



void Sidebar::onListContextMenuRequested(const QPoint& pos) {
    if (!list) return;
    QListWidgetItem *it = list->itemAt(pos);
    QMenu menu(this);

    if (!it) {
        QAction *addAct = menu.addAction(tr("Add Host…"));
        connect(addAct, &QAction::triggered, this, &Sidebar::onAddClicked);
        menu.exec(list->viewport()->mapToGlobal(pos));
        return;
    }

    const QVariant v = it->data(kSpecRole);
    HostSpec spec = v.isValid() && v.canConvert<HostSpec>() ? v.value<HostSpec>() : HostSpec{};

    QAction *connectAct = menu.addAction(tr("Connect"));
    QAction *editAct    = menu.addAction(tr("Edit…"));
    QAction *removeAct  = menu.addAction(tr("Remove"));

    QAction *chosen = menu.exec(list->viewport()->mapToGlobal(pos));
    if (!chosen) return;

    if (chosen == connectAct) {
        if (v.isValid() && v.canConvert<HostSpec>()) emit hostActivated(spec);
        return;
    }

    if (chosen == editAct) {
        HostSpec oldSpec = spec;

        // If your dialog supports pre-filling:
        NewHostDialog dlg(this);
        dlg.setWindowTitle(tr("Edit Host"));
        dlg.setSpec(oldSpec);                // <-- add setSpec(const HostSpec&) in your dialog
        if (dlg.exec() != QDialog::Accepted) return;

        HostSpec newSpec = dlg.spec();
        it->setText(labelFor(newSpec));
        it->setData(kSpecRole, QVariant::fromValue(newSpec));
        emit hostEdited(oldSpec, newSpec);
        return;
    }

    if (chosen == removeAct) {
        auto reply = QMessageBox::question(this, tr("Remove Host"),
                                           tr("Remove \"%1\"?").arg(it->text()),
                                           QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            emit hostRemoved(spec);
            delete list->takeItem(list->row(it));
        }
    }
}

