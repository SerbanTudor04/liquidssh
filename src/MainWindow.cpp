#include "MainWindow.h"
#include <QMenuBar>
#include <QStatusBar>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Set window title and size
    setWindowTitle("LiquidSSH");
    resize(800, 600);

    // Example menu
    QMenu *fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction("Exit", this, &QWidget::close);

    // Example status bar
    statusBar()->showMessage("Ready");

    // Example central widget
    auto *label = new QLabel("Hello from Qt6 + C++23!", this);
    label->setAlignment(Qt::AlignCenter);
    setCentralWidget(label);
}

MainWindow::~MainWindow() = default;
