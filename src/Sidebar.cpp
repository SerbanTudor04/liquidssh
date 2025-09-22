#include "Sidebar.h"
#include <QListWidget>
#include <QVBoxLayout>
#include <QItemSelectionModel>
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
    list->setStyleSheet(R"(
        QListWidget { background: transparent; border: none; }
        QListWidget::item {
            background: transparent;
            border-radius: 18px;
            margin: 6px; padding: 10px 14px; color: white;
        }
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

    // Double click → open session tab
    connect(list, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *it){
        emit hostDoubleClicked(it->text());
    });
}
