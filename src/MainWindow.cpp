#include "MainWindow.h"
#include "Glass.h"
#include "Sidebar.h"
#include "TabArea.h"

#include <QSplitter>
#include <QMenuBar>

#include <QMouseEvent>
#include <QWindow>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("LiquidSSH");
    resize(1200, 800);

#ifdef Q_OS_MAC
    // Let the OS blur show through the window itself
    setAttribute(Qt::WA_TranslucentBackground, true);
    setStyleSheet("QMainWindow { background: transparent; }");
#endif

    // Sidebar + tabs
    auto *sidebar = new Sidebar(this);   // <-- make sure Sidebar.cpp sets widget + viewport transparent
    auto *tabs    = new TabArea(this);   // <-- TabArea.cpp should make QTabWidget::pane transparent

    // Central splitter (single stylesheet call; previous multiple calls were overwriting)
    auto *splitter = new QSplitter(this);
    splitter->addWidget(sidebar);
    splitter->addWidget(tabs);
    splitter->setStretchFactor(1, 1);
    splitter->setHandleWidth(2);
    splitter->setAttribute(Qt::WA_TranslucentBackground, true);
    splitter->setStyleSheet(R"(
        QSplitter { background: transparent; }
        QSplitter::handle { background: rgba(255,255,255,0.08); border: none; }
    )");

    setCentralWidget(splitter);
    setContentsMargins(0,0,0,0);

    // Menu & status
    menuBar()->addMenu("&File")->addAction("Exit", this, &QWidget::close);
    // statusBar()->showMessage("Ready");

    // Signals (guard against accidental dupes)
    connect(sidebar, &Sidebar::hostSelected,
            tabs,    &TabArea::setHostInfo,
            Qt::UniqueConnection);
    connect(sidebar, &Sidebar::hostDoubleClicked,
            tabs,    &TabArea::openHostTab,
            Qt::UniqueConnection);
    connect(tabs, &TabArea::hostAdded,     sidebar, &Sidebar::addHost);
    connect(sidebar, &Sidebar::hostActivated, tabs, &TabArea::openHost);
    // Install the vibrancy AFTER the window is realized to avoid timing issues
#ifdef Q_OS_MAC
    QTimer::singleShot(0, this, [this]{
        enableLiquidGlass(this);
    });
#endif
}

MainWindow::~MainWindow() = default;

void MainWindow::mousePressEvent(QMouseEvent *event) {
#ifdef Q_OS_MAC
    // Drag the window from any empty area (reliable cross-platform)
    if (event->button() == Qt::LeftButton) {
        if (QWindow *win = windowHandle()) {
            win->startSystemMove();
            event->accept();
            return;
        }
    }
#endif
    QMainWindow::mousePressEvent(event);
}
