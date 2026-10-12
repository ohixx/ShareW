#include "capture/RegionSelector.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>

namespace shareW {

RegionSelector::RegionSelector(const QImage &frame, QWidget *parent)
    : QWidget(parent), m_frame(frame) {
    setWindowFlag(Qt::FramelessWindowHint);
    setCursor(Qt::CrossCursor);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

QRect RegionSelector::widgetRect() const {
    return QRect(m_start, m_end).normalized();
}

QRect RegionSelector::toImageRect(const QRect &r) const {
    const double sx = double(m_frame.width()) / width();
    const double sy = double(m_frame.height()) / height();
    return QRect(qRound(r.x() * sx), qRound(r.y() * sy),
                 qRound(r.width() * sx), qRound(r.height() * sy))
        .intersected(m_frame.rect());
}

void RegionSelector::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.drawImage(rect(), m_frame);
    p.fillRect(rect(), QColor(0, 0, 0, 110));

    if (!m_dragging && m_start == m_end)
        return;

    const QRect r = widgetRect();
    p.setClipRect(r);
    p.drawImage(rect(), m_frame);
    p.setClipping(false);
    p.setPen(QPen(QColor(0x3a, 0xc9, 0x7b), 1));
    p.drawRect(r.adjusted(0, 0, -1, -1));

    const QRect img = toImageRect(r);
    const QString label = QStringLiteral("%1 × %2").arg(img.width()).arg(img.height());
    p.setPen(Qt::white);
    p.fillRect(QRect(r.x(), qMax(0, r.y() - 22), 90, 20), QColor(0, 0, 0, 180));
    p.drawText(QRect(r.x() + 4, qMax(0, r.y() - 22), 86, 20), Qt::AlignVCenter, label);
}

void RegionSelector::mousePressEvent(QMouseEvent *e) {
    if (e->button() != Qt::LeftButton)
        return;
    m_start = m_end = e->position().toPoint();
    m_dragging = true;
    update();
}

void RegionSelector::mouseMoveEvent(QMouseEvent *e) {
    if (!m_dragging)
        return;
    m_end = e->position().toPoint();
    update();
}

void RegionSelector::mouseReleaseEvent(QMouseEvent *e) {
    if (e->button() != Qt::LeftButton || !m_dragging)
        return;
    m_dragging = false;
    m_end = e->position().toPoint();
    const QRect img = toImageRect(widgetRect());
    if (img.width() < 2 || img.height() < 2) {
        update();
        return;
    }
    emit selected(img);
}

void RegionSelector::keyPressEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_Escape)
        emit cancelled();
}

} // namespace shareW
