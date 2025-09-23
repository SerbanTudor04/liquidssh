// GlassCloseButton.h
#pragma once
#include <QAbstractButton>

class GlassCloseButton : public QAbstractButton {
    Q_OBJECT
public:
    explicit GlassCloseButton(QWidget *parent = nullptr);
    QSize sizeHint() const override { return {18,18}; }

protected:
    void paintEvent(QPaintEvent *event) override;
};
