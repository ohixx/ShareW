#pragma once

#include <QImage>
#include <QPoint>
#include <QRect>
#include <QWidget>

namespace shareW {

// Fullscreen frozen-frame overlay: drag to pick a rectangle, Esc to cancel.
class RegionSelector : public QWidget {
    Q_OBJECT
public:
    explicit RegionSelector(const QImage &frame, QWidget *parent = nullptr);

signals:
    // Rectangle in image pixel coordinates.
    void selected(const QRect &region);
    void cancelled();

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;

private:
    QRect widgetRect() const;
    QRect toImageRect(const QRect &r) const;

    QImage m_frame;
    QPoint m_start;
    QPoint m_end;
    bool m_dragging = false;
};

} // namespace shareW
