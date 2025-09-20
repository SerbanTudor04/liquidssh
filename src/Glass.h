#pragma once
#include <QWidget>

/// Apply “glass”/blur behind the window content where supported.
/// Safe to call multiple times (no-op on unsupported platforms).
void enableLiquidGlass(QWidget* topLevel);