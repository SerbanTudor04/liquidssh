#pragma once
#include <QTabBar>
#include <QPointer>

class CustomTabBar : public QTabBar {
    Q_OBJECT
public:
    explicit CustomTabBar(QWidget *parent = nullptr);

    QSize tabSizeHint(int index) const override;
protected:
    void paintEvent(QPaintEvent *event) override;

    void tabInserted(int index) override;

    void enterEvent(QEnterEvent *ev) override;
    void leaveEvent(QEvent *ev) override;
    void mouseMoveEvent(QMouseEvent *ev) override;

private:
    int m_hoverIndex = -1;

    // one-time noise pixmap for the glass texture
    static QPixmap noisePixmap(int dpr = 1);
};
