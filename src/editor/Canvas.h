#pragma once

#include "editor/Annotation.h"

#include <QImage>
#include <QWidget>

namespace shareW {

class Canvas : public QWidget {
    Q_OBJECT
public:
    explicit Canvas(const QImage &image, QWidget *parent = nullptr);

    void setTool(Tool t) { m_tool = t; }
    void setColor(const QColor &c) { m_color = c; }
    void setStrokeWidth(qreal w) { m_width = w; }

    void undo();
    void redo();

    // Base image with all annotations burned in.
    QImage render() const;

    QSize sizeHint() const override { return m_image.size(); }

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;

private:
    static void paintAnnotation(QPainter &p, const Annotation &a);
    QPointF toImage(const QPointF &widgetPos) const;
    QRectF targetRect() const;

    QImage m_image;
    QVector<Annotation> m_items;
    QVector<Annotation> m_redo;
    Annotation m_current;
    bool m_drawing = false;

    Tool m_tool = Tool::Pen;
    QColor m_color = QColor(0xe5, 0x39, 0x35);
    qreal m_width = 3.0;
};

} // namespace shareW
