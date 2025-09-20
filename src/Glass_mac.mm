#import <AppKit/AppKit.h>
#include <QWidget>
#include <QWindow>
#include <QGuiApplication>

#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
#include <QtGui/qnativeinterface.h>   // public API for NSWindow access
#endif

static void addVibrancyToWindow(NSWindow* nsWindow) {
    if (!nsWindow) return;

    // Make sure the window is allowed to be translucent
    [nsWindow setOpaque:NO];
    nsWindow.backgroundColor = [NSColor clearColor];

    NSView* content = nsWindow.contentView;
    if (!content) return;

    // Avoid duplicates
    for (NSView* sub in content.subviews) {
        if ([sub isKindOfClass:[NSVisualEffectView class]]) {
            return;
        }
    }

    NSVisualEffectView* blur = [[NSVisualEffectView alloc] initWithFrame:content.bounds];
    blur.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
    blur.blendingMode = NSVisualEffectBlendingModeBehindWindow;
    blur.state = NSVisualEffectStateActive;

    if (@available(macOS 10.14, *)) {
        blur.material = NSVisualEffectMaterialUnderWindowBackground;
    } else {
        blur.material = NSVisualEffectMaterialMediumLight;
    }

    // Insert below Qt’s content so desktop gets blurred behind
    [content addSubview:blur positioned:NSWindowBelow relativeTo:nil];

    // Optional: seamless titlebar
    nsWindow.titlebarAppearsTransparent = YES;
    nsWindow.titleVisibility = NSWindowTitleHidden;
}

void enableLiquidGlass(QWidget* topLevel) {
    if (!topLevel) return;

    // Qt side translucent
    topLevel->setAttribute(Qt::WA_TranslucentBackground, true);

    QWindow* qwin = topLevel->windowHandle();
    if (!qwin) return;

#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
    auto iface = qwin->nativeInterface<QNativeInterface::QNSWindow>();
    if (!iface) return;
    NSWindow* nsWindow = iface->window();
#else
    // Very old Qt 6.x fallback (rare)
    NSWindow* nsWindow = reinterpret_cast<NSWindow*>(qwin->winId());
#endif
    addVibrancyToWindow(nsWindow);
}
