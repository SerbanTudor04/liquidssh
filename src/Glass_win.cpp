#include "Glass.h"
#ifdef _WIN32
#include <windows.h>
#include <dwmapi.h>
#include <QtWidgets/QWidget>
#include <QtGui/QWindow>

#ifndef DWMWA_SYSTEMBACKDROP_TYPE
#define DWMWA_SYSTEMBACKDROP_TYPE 38
#endif
#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

// For acrylic/blur fallback
typedef struct _ACCENT_POLICY {
    int AccentState;
    int AccentFlags;
    int GradientColor;
    int AnimationId;
} ACCENT_POLICY;
typedef struct _WINDOWCOMPOSITIONATTRIBDATA {
    int Attrib;
    PVOID pvData;
    SIZE_T cbData;
} WINDOWCOMPOSITIONATTRIBDATA;

enum WINDOWCOMPOSITIONATTRIB {
    WCA_ACCENT_POLICY = 19
};
enum ACCENT_STATE {
    ACCENT_DISABLED = 0,
    ACCENT_ENABLE_GRADIENT = 1,
    ACCENT_ENABLE_TRANSPARENTGRADIENT = 2,
    ACCENT_ENABLE_BLURBEHIND = 3,
    ACCENT_ENABLE_ACRYLICBLURBEHIND = 4
};

static BOOL setWindowCompositionAttribute(HWND hwnd, WINDOWCOMPOSITIONATTRIBDATA* data) {
    static auto fn = reinterpret_cast<BOOL (WINAPI*)(HWND, WINDOWCOMPOSITIONATTRIBDATA*)>(
        GetProcAddress(GetModuleHandleW(L"user32.dll"), "SetWindowCompositionAttribute"));
    return fn ? fn(hwnd, data) : FALSE;
}

void enableLiquidGlass(QWidget* topLevel) {
    if (!topLevel) return;
    topLevel->setAttribute(Qt::WA_TranslucentBackground, true);

    HWND hwnd = reinterpret_cast<HWND>(topLevel->windowHandle()->winId());

    // Try Windows 11 Mica first
    const int MICA = 2; // DWMSBT_MAINWINDOW = 2
    HRESULT hr = DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &MICA, sizeof(MICA));
    if (SUCCEEDED(hr)) {
        BOOL dark = TRUE;
        DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &dark, sizeof(dark));
        return;
    }

    // Fallback: Acrylic/Blur (works on Win10 1809+)
    ACCENT_POLICY policy = {};
    policy.AccentState = ACCENT_ENABLE_ACRYLICBLURBEHIND; // or ACCENT_ENABLE_BLURBEHIND
    policy.GradientColor = 0x99000000; // AA RR GG BB (alpha ~0x99)

    WINDOWCOMPOSITIONATTRIBDATA data = {};
    data.Attrib = WCA_ACCENT_POLICY;
    data.pvData = &policy;
    data.cbData = sizeof(policy);
    setWindowCompositionAttribute(hwnd, &data);
}
#else
void enableLiquidGlass(QWidget*) {}
#endif
