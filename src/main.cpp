#include "capture/PortalBackend.h"
#include "capture/RegionSelector.h"
#include "editor/EditorWindow.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QTextStream>
#include <QTimer>

using namespace shareW;

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("shareW"));
    QApplication::setApplicationVersion(QStringLiteral("0.1.0"));

    QCommandLineParser parser;
    parser.setApplicationDescription(
        QStringLiteral("Screenshot and annotation tool for Wayland"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(
        QStringLiteral("mode"), QStringLiteral("screen | region"),
        QStringLiteral("[screen|region]"));
    parser.process(app);

    const QStringList args = parser.positionalArguments();
    const QString mode = args.value(0, QStringLiteral("screen"));
    if (mode != QStringLiteral("screen") && mode != QStringLiteral("region")) {
        QTextStream(stderr) << "shareW: unknown mode '" << mode
                            << "' (window capture is not implemented yet)\n";
        return 2;
    }

    PortalBackend backend;
    QObject::connect(&backend, &CaptureBackend::failed, &app, [&](const QString &why) {
        QTextStream(stderr) << "shareW: " << why << '\n';
        app.exit(1);
    });
    QObject::connect(&backend, &CaptureBackend::finished, &app, [&](const QImage &frame) {
        auto openEditor = [](const QImage &img) {
            auto *editor = new EditorWindow(img);
            editor->setAttribute(Qt::WA_DeleteOnClose);
            editor->show();
        };

        if (mode == QStringLiteral("screen")) {
            openEditor(frame);
            return;
        }

        auto *selector = new RegionSelector(frame);
        selector->setAttribute(Qt::WA_DeleteOnClose);
        QObject::connect(selector, &RegionSelector::selected, &app, [=](const QRect &r) {
            openEditor(frame.copy(r));
            selector->close();
        });
        QObject::connect(selector, &RegionSelector::cancelled, &app, [=] {
            selector->close();
            QApplication::quit();
        });
        selector->showFullScreen();
    });

    // Start from the event loop so an early failure can still exit it.
    QTimer::singleShot(0, &backend, &CaptureBackend::captureScreen);
    return app.exec();
}
