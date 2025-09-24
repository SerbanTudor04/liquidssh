#include "Sidebar.h"
#include <QListWidget>
#include <QVBoxLayout>
#include <QItemSelectionModel>

static constexpr int kSpecRole = Qt::UserRole + 1;

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
    it->setData(kSpecRole, QVariant::fromValue(spec));
    list->addItem(it);
    list->setCurrentItem(it);
}

Sidebar::Sidebar(QWidget *parent) : QWidget(parent) {
    list = new QListWidget(this);
    list->addItem("prod-db (10.0.0.5)");
    list->addItem("edge-01 (edge01.example.com)");
    list->addItem("dev-mac (192.168.1.23)");

    list->setSelectionMode(QAbstractItemView::SingleSelection);
    list->setSelectionBehavior(QAbstractItemView::SelectItems);
    list->setSelectionRectVisible(false);
    list->setFocusPolicy(Qt::NoFocus);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(8,8,8,8);
    layout->addWidget(list);
    setLayout(layout);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setStyleSheet("background: transparent;");

    // For the list itself (viewport background is the key)
    list->setAttribute(Qt::WA_TranslucentBackground, true);
    list->viewport()->setAttribute(Qt::WA_TranslucentBackground, true);
    list->setStyleSheet(R"(
  QListWidget { background: transparent; border: none; }
  QListWidget::item { background: transparent; border-radius: 18px; margin: 6px; padding: 10px 14px; color: white; }
  QListWidget::item:hover   { background: rgba(255,255,255,0.12); }
  QListWidget::item:selected{ background: rgba(255,255,255,0.35); color: black; }
)");

    // Single click → enforce one selection and emit hostSelected
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
        emit hostDoubleClicked(it->text()); // keep old signal if you still use it
    });
}
