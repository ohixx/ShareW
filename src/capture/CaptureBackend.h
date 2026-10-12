#pragma once

#include <QImage>
#include <QObject>
#include <QString>

namespace shareW {

// A way of getting pixels out of the compositor. The UI only talks to this
// interface, so adding wlr-screencopy or ext-image-copy-capture later does
// not touch the editor.
class CaptureBackend : public QObject {
    Q_OBJECT
public:
    using QObject::QObject;
    ~CaptureBackend() override = default;

    virtual QString name() const = 0;

    // Grabs the whole desktop. Result arrives through finished() or failed().
    virtual void captureScreen() = 0;

signals:
    void finished(const QImage &image);
    void failed(const QString &reason);
};

} // namespace shareW
