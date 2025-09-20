#include "MainWindow.h"
#include "Glass.h"
#include <QMenuBar>
#include <QStatusBar>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>
#include <QPalette>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("LiquidSSH - Qt6");
    resize(1000, 700);

    // Central widget with transparent background so vibrancy is visible
    auto *central = new QWidget(this);
    central->setAttribute(Qt::WA_TranslucentBackground, true);
    QPalette pal = central->palette();
    pal.setColor(QPalette::Window, QColor(0,0,0,0));
    central->setPalette(pal);
    central->setAutoFillBackground(true);

    auto *layout = new QVBoxLayout(central);
    auto *label = new QLabel("Liquid Glass active ✨", central);
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);
    central->setLayout(layout);
    setCentralWidget(central);

    menuBar()->addMenu("&File")->addAction("Exit", this, &QWidget::close);
    statusBar()->showMessage("Ready");

    // Apply the glass effect
    enableLiquidGlass(this);
}

MainWindow::~MainWindow() = default;
