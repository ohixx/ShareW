#include "editor/EditorWindow.h"

#include "editor/Canvas.h"

#include <QActionGroup>
#include <QApplication>
#include <QClipboard>
#include <QColorDialog>
#include <QDateTime>
#include <QFileDialog>
#include <QMessageBox>
#include <QSpinBox>
#include <QStandardPaths>
#include <QToolBar>

namespace shareW {

EditorWindow::EditorWindow(const QImage &image, QWidget *parent)
    : QMainWindow(parent), m_canvas(new Canvas(image)) {
    setWindowTitle(QStringLiteral("ShareW"));
    setCentralWidget(m_canvas);
    buildToolbar();
    resize(m_canvas->sizeHint().boundedTo(QSize(1280, 800)) + QSize(0, 48));
}

void EditorWindow::buildToolbar() {
    auto *bar = addToolBar(tr("Tools"));
    bar->setMovable(false);

    auto *group = new QActionGroup(this);
    const struct { const char *label; Tool tool; } tools[] = {
        {"Pen", Tool::Pen},     {"Line", Tool::Line},       {"Arrow", Tool::Arrow},
        {"Rect", Tool::Rect},   {"Ellipse", Tool::Ellipse}, {"Text", Tool::Text},
    };
    for (const auto &t : tools) {
        QAction *a = bar->addAction(tr(t.label));
        a->setCheckable(true);
        a->setActionGroup(group);
        const Tool tool = t.tool;
        connect(a, &QAction::triggered, this, [this, tool] { m_canvas->setTool(tool); });
        if (tool == Tool::Pen)
            a->setChecked(true);
    }

    bar->addSeparator();
    bar->addAction(tr("Color"), this, &EditorWindow::pickColor);
    auto *width = new QSpinBox;
    width->setRange(1, 40);
    width->setValue(3);
    width->setSuffix(QStringLiteral(" px"));
    connect(width, &QSpinBox::valueChanged, this, [this](int v) { m_canvas->setStrokeWidth(v); });
    bar->addWidget(width);

    bar->addSeparator();
    bar->addAction(tr("Undo"), QKeySequence::Undo, m_canvas, &Canvas::undo);
    bar->addAction(tr("Redo"), QKeySequence::Redo, m_canvas, &Canvas::redo);

    bar->addSeparator();
    bar->addAction(tr("Copy"), QKeySequence::Copy, this, &EditorWindow::copyToClipboard);
    bar->addAction(tr("Save"), QKeySequence::Save, this, &EditorWindow::save);
}

void EditorWindow::pickColor() {
    const QColor c = QColorDialog::getColor(Qt::red, this, tr("Color"));
    if (c.isValid())
        m_canvas->setColor(c);
}

void EditorWindow::copyToClipboard() {
    QApplication::clipboard()->setImage(m_canvas->render());
}

void EditorWindow::save() {
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    const QString name = QDateTime::currentDateTime().toString(QStringLiteral("'ShareW_'yyyy-MM-dd_HH-mm-ss'.png'"));
    const QString path = QFileDialog::getSaveFileName(this, tr("Save"), dir + '/' + name,
                                                      tr("Images (*.png *.jpg *.webp)"));
    if (path.isEmpty())
        return;
    if (!m_canvas->render().save(path))
        QMessageBox::warning(this, tr("Save"), tr("Could not write %1").arg(path));
}

} // namespace shareW
