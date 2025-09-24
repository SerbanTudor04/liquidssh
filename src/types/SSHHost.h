#pragma once
#include <qmetatype.h>
#include <QString>

struct HostSpec {
    QString alias;   // Optional display name (e.g., "dev-mac")
    QString host;    // Hostname or IP (required)
    QString user;    // Optional (e.g., "root")
    int     port{22};
};
Q_DECLARE_METATYPE(HostSpec)