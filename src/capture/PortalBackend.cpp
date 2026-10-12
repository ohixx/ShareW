#include "capture/PortalBackend.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusObjectPath>
#include <QFile>
#include <QUrl>
#include <QUuid>

namespace shareW {

namespace {
constexpr auto kService = "org.freedesktop.portal.Desktop";
constexpr auto kPath = "/org/freedesktop/portal/desktop";
constexpr auto kScreenshot = "org.freedesktop.portal.Screenshot";
constexpr auto kRequest = "org.freedesktop.portal.Request";
} // namespace

PortalBackend::PortalBackend(QObject *parent) : CaptureBackend(parent) {}

void PortalBackend::captureScreen() {
    if (m_waiting)
        return;

    auto bus = QDBusConnection::sessionBus();
    if (!bus.isConnected()) {
        emit failed(tr("No D-Bus session bus"));
        return;
    }

    // The portal replies on a Request object whose path is derived from our
    // unique bus name and a token we choose. Subscribe before calling so the
    // response cannot be missed.
    const QString token = QStringLiteral("shareW_")
        + QUuid::createUuid().toString(QUuid::Id128);
    QString sender = bus.baseService().mid(1);
    sender.replace('.', '_');
    const QString requestPath = QStringLiteral("/org/freedesktop/portal/desktop/request/%1/%2")
        .arg(sender, token);

    bus.connect(kService, requestPath, kRequest, QStringLiteral("Response"),
                this, SLOT(onResponse(uint, QVariantMap)));

    QDBusMessage msg = QDBusMessage::createMethodCall(
        kService, kPath, kScreenshot, QStringLiteral("Screenshot"));
    QVariantMap options{
        {QStringLiteral("handle_token"), token},
        {QStringLiteral("interactive"), false},
    };
    msg << QString() << options;

    m_waiting = true;
    auto *watcher = new QDBusPendingCallWatcher(bus.asyncCall(msg), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            [this](QDBusPendingCallWatcher *w) {
                QDBusPendingReply<QDBusObjectPath> reply = *w;
                w->deleteLater();
                if (reply.isError()) {
                    m_waiting = false;
                    emit failed(reply.error().message());
                }
            });
}

void PortalBackend::onResponse(uint code, const QVariantMap &results) {
    m_waiting = false;
    if (code != 0) {
        emit failed(code == 1 ? tr("Capture cancelled") : tr("Portal capture failed"));
        return;
    }

    const QUrl uri(results.value(QStringLiteral("uri")).toString());
    QImage image(uri.toLocalFile());
    if (image.isNull()) {
        emit failed(tr("Could not read screenshot: %1").arg(uri.toString()));
        return;
    }
    // The portal leaves the file in the user's Pictures directory; the editor
    // owns the pixels from here on.
    QFile::remove(uri.toLocalFile());
    emit finished(image);
}

} // namespace shareW
