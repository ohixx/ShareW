#pragma once

#include <QImage>
#include <QMainWindow>

namespace shareW {

class Canvas;

class EditorWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit EditorWindow(const QImage &image, QWidget *parent = nullptr);

private slots:
    void save();
    void copyToClipboard();
    void pickColor();

private:
    void buildToolbar();

    Canvas *m_canvas;
};

} // namespace shareW
