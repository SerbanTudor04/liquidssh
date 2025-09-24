#include "Sidebar.h"
#include <QListWidget>
#include <QVBoxLayout>
#include <QItemSelectionModel>
#include <QLineEdit>
#include <QTimer>
static constexpr int kSpecRole_local = Qt::UserRole + 1; // mirror header

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
    if (int idx = findItem(lbl); idx >= 0) {
        list->setCurrentRow(idx);
        return;
    }
    auto *it = new QListWidgetItem(lbl);
    it->setData(kSpecRole_local, QVariant::fromValue(spec));
    list->addItem(it);
    list->setCurrentItem(it);
}

Sidebar::Sidebar(QWidget *parent) : QWidget(parent) {
    // --- Widgets
    title  = new QLabel(tr("Hosts"), this);
    search = new QLineEdit(this);
    list   = new QListWidget(this);

    filterTimer = new QTimer(this);
    filterTimer->setSingleShot(true);
    filterTimer->setInterval(120); // debounce

    // --- Title style
    title->setStyleSheet(R"(
        QLabel { color: white; font-size: 32px; font-weight: 600; letter-spacing: 0.3px;padding-left:10px; }
    )");

    // --- Search style (translucent, rounded)
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
        QLineEdit:focus {
            border: 1px solid rgba(255,255,255,0.35);
            background: rgba(255,255,255,0.18);
        }
    )");

    // --- List style (your existing look)
    list->setSelectionMode(QAbstractItemView::SingleSelection);
    list->setSelectionBehavior(QAbstractItemView::SelectItems);
    list->setSelectionRectVisible(false);
    list->setFocusPolicy(Qt::NoFocus);

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

    layout->addWidget(title);
    layout->addWidget(search);
    layout->addWidget(list, /*stretch*/1);
    setLayout(layout);

    // --- Filtering connections
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
        const QVariant v = it->data(kSpecRole_local);
        if (v.isValid() && v.canConvert<HostSpec>())
            emit hostActivated(v.value<HostSpec>());
        emit hostDoubleClicked(it->text());
    });

    // Enter in search activates current highlighted item (if any)
    connect(search, &QLineEdit::returnPressed, this, [this]{
        if (auto *it = list->currentItem()) {
            const QVariant v = it->data(kSpecRole_local);
            if (v.isValid() && v.canConvert<HostSpec>())
                emit hostActivated(v.value<HostSpec>());
        }
    });
}

void Sidebar::onFilterTextChanged(const QString&) {
    filterTimer->start(); // debounce
}

bool Sidebar::itemMatchesFilter(QListWidgetItem *it, const QString &needle) const {
    if (needle.isEmpty()) return true;
    const QString low = needle.trimmed().toLower();
    if (low.isEmpty()) return true;

    // Match label text
    if (it->text().toLower().contains(low)) return true;

    // Match against HostSpec fields
    const QVariant v = it->data(kSpecRole_local);
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

    // Hide/show items
    for (int i = 0; i < list->count(); ++i) {
        auto *it = list->item(i);
        const bool match = itemMatchesFilter(it, needle);
        it->setHidden(!match);
    }

    // Keep selection sane: ensure a visible item is current
    if (auto *cur = list->currentItem(); cur && cur->isHidden()) {
        list->setCurrentItem(nullptr);
    }
    if (!list->currentItem()) {
        // Select first visible item, if any
        for (int i = 0; i < list->count(); ++i) {
            auto *it = list->item(i);
            if (!it->isHidden()) {
                list->setCurrentItem(it);
                break;
            }
        }
    }
}

void Sidebar::setFilterText(const QString& text) {
    search->setText(text);
    applyFilter();
}