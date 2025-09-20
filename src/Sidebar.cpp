#include "Sidebar.h"
#include <QListWidget>
#include <QVBoxLayout>

Sidebar::Sidebar(QWidget *parent)
    : QWidget(parent)
{
    list = new QListWidget(this);
    list->addItem("prod-db (10.0.0.5)");
    list->addItem("edge-01 (edge01.example.com)");
    list->addItem("dev-mac (192.168.1.23)");

    // Only one item at a time
    list->setSelectionMode(QAbstractItemView::SingleSelection);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(list);
    setLayout(layout);

    setAttribute(Qt::WA_TranslucentBackground, true);

    // Rounded “island” look
    list->setStyleSheet(R"(
        QListWidget {
            background: transparent;
            border: none;
        }
        QListWidget::item {
            background: rgba(255,255,255,40);
            border-radius: 20px;
            margin: 6px;
            padding: 8px 12px;
            color: white;
        }
        QListWidget::item:selected {
            background: rgba(255,255,255,100);
            color: black;
        }
    )");

    connect(list, &QListWidget::itemSelectionChanged, [this]() {
        if (!list->selectedItems().isEmpty()) {
            emit hostSelected(list->selectedItems().first()->text());
        }
    });

    connect(list, &QListWidget::itemDoubleClicked, [this](QListWidgetItem *item) {
        emit hostDoubleClicked(item->text());
    });
}
