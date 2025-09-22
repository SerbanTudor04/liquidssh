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

    auto *sidebar = new Sidebar(this);
    auto *tabs    = new TabArea(this);

    auto *splitter = new QSplitter(this);
    splitter->addWidget(sidebar);
    splitter->addWidget(tabs);
    splitter->setStretchFactor(1, 1);
    splitter->setAttribute(Qt::WA_TranslucentBackground, true);
    splitter->setStyleSheet("background: transparent;");
    setCentralWidget(splitter);

    // Connect with UniqueConnection to prevent accidental duplicates
    connect(sidebar, &Sidebar::hostSelected,
            tabs,    &TabArea::setHostInfo,
            Qt::UniqueConnection);

    connect(sidebar, &Sidebar::hostDoubleClicked,
            tabs,    &TabArea::openHostTab,
            Qt::UniqueConnection);


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
