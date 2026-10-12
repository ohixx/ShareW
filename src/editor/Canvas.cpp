#include "editor/Canvas.h"

#include <QInputDialog>
#include <QLineF>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPolygonF>
#include <cmath>

namespace shareW {

Canvas::Canvas(const QImage &image, QWidget *parent) : QWidget(parent), m_image(image) {
    setCursor(Qt::CrossCursor);
    setMinimumSize(320, 200);
}

QRectF Canvas::targetRect() const {
    // Fit the image into the widget, keeping aspect ratio and never upscaling.
    QSizeF s = m_image.size();
    s.scale(QSizeF(size()).boundedTo(s), Qt::KeepAspectRatio);
    return QRectF(QPointF((width() - s.width()) / 2, (height() - s.height()) / 2), s);
}

QPointF Canvas::toImage(const QPointF &pos) const {
    const QRectF t = targetRect();
    return QPointF((pos.x() - t.x()) * m_image.width() / t.width(),
                   (pos.y() - t.y()) * m_image.height() / t.height());
}

void Canvas::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    const QRectF t = targetRect();
    p.drawImage(t, m_image);
    p.translate(t.topLeft());
    p.scale(t.width() / m_image.width(), t.height() / m_image.height());
    for (const auto &a : m_items)
        paintAnnotation(p, a);
    if (m_drawing)
        paintAnnotation(p, m_current);
}

void Canvas::paintAnnotation(QPainter &p, const Annotation &a) {
    if (a.points.isEmpty())
        return;
    QPen pen(a.color, a.width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);

    const QPointF s = a.points.first();
    const QPointF e = a.points.last();

    switch (a.tool) {
    case Tool::Pen: {
        QPainterPath path(s);
        for (int i = 1; i < a.points.size(); ++i)
            path.lineTo(a.points[i]);
        p.drawPath(path);
        break;
    }
    case Tool::Line:
        p.drawLine(s, e);
        break;
    case Tool::Arrow: {
        p.drawLine(s, e);
        const QLineF l(e, s);
        if (l.length() < 1)
            break;
        const double head = 10 + a.width * 2;
        const double ang = std::atan2(-l.dy(), l.dx());
        const QPointF p1 = e + QPointF(std::sin(ang + M_PI / 3) * head,
                                       std::cos(ang + M_PI / 3) * head);
        const QPointF p2 = e + QPointF(std::sin(ang + M_PI - M_PI / 3) * head,
                                       std::cos(ang + M_PI - M_PI / 3) * head);
        p.setBrush(a.color);
        p.drawPolygon(QPolygonF{e, p1, p2});
        break;
    }
    case Tool::Rect:
        p.drawRect(QRectF(s, e).normalized());
        break;
    case Tool::Ellipse:
        p.drawEllipse(QRectF(s, e).normalized());
        break;
    case Tool::Text:
        p.setFont(a.font);
        p.drawText(s, a.text);
        break;
    }
}

void Canvas::mousePressEvent(QMouseEvent *e) {
    if (e->button() != Qt::LeftButton)
        return;
    const QPointF pos = toImage(e->position());

    if (m_tool == Tool::Text) {
        bool ok = false;
        const QString text = QInputDialog::getText(this, tr("Text"), tr("Text:"),
                                                   QLineEdit::Normal, {}, &ok);
        if (ok && !text.isEmpty()) {
            Annotation a;
            a.tool = Tool::Text;
            a.color = m_color;
            a.text = text;
            a.font = QFont(font().family(), int(12 + m_width * 3), QFont::Bold);
            a.points = {pos};
            m_items.append(a);
            m_redo.clear();
            update();
        }
        return;
    }

    m_current = Annotation{};
    m_current.tool = m_tool;
    m_current.color = m_color;
    m_current.width = m_width;
    m_current.points = {pos};
    m_drawing = true;
}

void Canvas::mouseMoveEvent(QMouseEvent *e) {
    if (!m_drawing)
        return;
    const QPointF pos = toImage(e->position());
    if (m_tool == Tool::Pen) {
        m_current.points.append(pos);
    } else {
        m_current.points.resize(1);
        m_current.points.append(pos);
    }
    update();
}

void Canvas::mouseReleaseEvent(QMouseEvent *e) {
    if (e->button() != Qt::LeftButton || !m_drawing)
        return;
    m_drawing = false;
    if (m_current.points.size() > 1) {
        m_items.append(m_current);
        m_redo.clear();
    }
    update();
}

void Canvas::undo() {
    if (m_items.isEmpty())
        return;
    m_redo.append(m_items.takeLast());
    update();
}

void Canvas::redo() {
    if (m_redo.isEmpty())
        return;
    m_items.append(m_redo.takeLast());
    update();
}

QImage Canvas::render() const {
    QImage out = m_image.convertToFormat(QImage::Format_ARGB32);
    QPainter p(&out);
    p.setRenderHint(QPainter::Antialiasing);
    for (const auto &a : m_items)
        paintAnnotation(p, a);
    return out;
}

} // namespace shareW
