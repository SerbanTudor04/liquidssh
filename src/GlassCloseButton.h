#pragma once
#include <QAbstractButton>

class GlassCloseButton : public QAbstractButton {
    Q_OBJECT
public:
    explicit GlassCloseButton(QWidget *parent = nullptr);
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *) override;
};
