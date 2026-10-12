#pragma once

#include <QColor>
#include <QFont>
#include <QPointF>
#include <QString>
#include <QVector>

namespace shareW {

enum class Tool { Pen, Line, Arrow, Rect, Ellipse, Text };

// One drawn object. Kept as data until export so it stays editable and
// undo / redo is just popping a vector.
struct Annotation {
    Tool tool = Tool::Pen;
    QColor color = Qt::red;
    qreal width = 3.0;
    QVector<QPointF> points; // Pen: path. Others: [start, end]. Text: [anchor].
    QString text;
    QFont font;
};

} // namespace shareW
