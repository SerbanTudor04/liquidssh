#include "MainWindow.h"
#include "Glass.h"
#include "Sidebar.h"
#include "TabArea.h"

#include <QSplitter>
#include <QMenuBar>
#include <QStatusBar>
#include <QMouseEvent>
#include <QWindow>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("LiquidSSH");
    resize(1200, 800);

    // Sidebar + tabs
    auto *sidebar = new Sidebar(this);
    auto *tabs    = new TabArea(this);

    auto *splitter = new QSplitter(this);
    splitter->addWidget(sidebar);
    splitter->addWidget(tabs);
    splitter->setStretchFactor(1, 1);
    splitter->setAttribute(Qt::WA_TranslucentBackground, true);
    splitter->setStyleSheet("background: transparent;");
    setCentralWidget(splitter);

    // Menu & status bar
    menuBar()->addMenu("&File")->addAction("Exit", this, &QWidget::close);
    statusBar()->showMessage("Ready");

    // Connect sidebar to tabs
    connect(sidebar, &Sidebar::hostSelected,
            tabs,    &TabArea::setHostInfo);
    connect(sidebar, &Sidebar::hostDoubleClicked, tabs, &TabArea::openHostTab);



    // Apply glass (macOS only)
    enableLiquidGlass(this);
}

MainWindow::~MainWindow() = default;

void MainWindow::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        if (QWindow *win = windowHandle()) {
            win->startSystemMove();
            event->accept();
            return;
        }
    }
    QMainWindow::mousePressEvent(event);
}
