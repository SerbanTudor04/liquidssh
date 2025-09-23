#pragma once
#include <QTabBar>
#include <QPixmap>

class CustomTabBar : public QTabBar {
    Q_OBJECT
public:
    explicit CustomTabBar(QWidget *parent = nullptr);

    QSize tabSizeHint(int index) const override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEnterEvent *ev) override;
    void leaveEvent(QEvent *ev) override;
    void mouseMoveEvent(QMouseEvent *ev) override;
    void tabInserted(int index) override;
    void tabRemoved(int index) override { QTabBar::tabRemoved(index); updateCloseButtonsVisibility(); }

private:
    static QPixmap noisePixmap(int dpr = 1);
    void updateCloseButtonsVisibility();

    int m_hoverIndex = -1;
};
