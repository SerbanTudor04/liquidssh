#import <AppKit/AppKit.h>
#include <QWidget>
#include <QWindow>
#include <QGuiApplication>

// ---- helper ----
static void addVibrancyToWindow(NSWindow* nsWindow) {
    if (!nsWindow) return;

    // allow translucency
    [nsWindow setOpaque:NO];
    nsWindow.backgroundColor = [NSColor clearColor];

    NSView* content = nsWindow.contentView;
    if (!content) return;

    // avoid duplicates
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
        blur.material = NSVisualEffectMaterialUnderWindowBackground; // semantic, not deprecated
    } else {
        blur.material = NSVisualEffectMaterialLight; // older fallback
    }

    [content addSubview:blur positioned:NSWindowBelow relativeTo:nil];

    // optional: minimal titlebar look
    nsWindow.titlebarAppearsTransparent = YES;
    nsWindow.titleVisibility = NSWindowTitleHidden;
}

// ---- API ----
void enableLiquidGlass(QWidget* topLevel) {
    if (!topLevel) return;

    // Make Qt surface translucent so vibrancy shows through
    topLevel->setAttribute(Qt::WA_TranslucentBackground, true);

    QWindow* qwin = topLevel->windowHandle();
    if (!qwin) return;

    // Bridge WId -> void* -> NSView* (ARC-safe)
    void* raw = reinterpret_cast<void*>(qwin->winId());
    NSView* nsView = (__bridge NSView*)raw;
    if (!nsView) return;

    NSWindow* nsWindow = nsView.window;
    if (!nsWindow) return;

    addVibrancyToWindow(nsWindow);
}
