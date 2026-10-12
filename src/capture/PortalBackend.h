#pragma once

#include "capture/CaptureBackend.h"

namespace shareW {

// xdg-desktop-portal Screenshot. Works on any compositor that ships a portal
// backend (GNOME, KDE, wlroots via xdg-desktop-portal-wlr, Hyprland, ...).
class PortalBackend : public CaptureBackend {
    Q_OBJECT
public:
    explicit PortalBackend(QObject *parent = nullptr);

    QString name() const override { return QStringLiteral("portal"); }
    void captureScreen() override;

private slots:
    void onResponse(uint code, const QVariantMap &results);

private:
    bool m_waiting = false;
};

} // namespace shareW
