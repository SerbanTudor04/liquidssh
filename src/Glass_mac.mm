#import <AppKit/AppKit.h>
#include <QWidget>
#include <QWindow>
#include <QGuiApplication>

static NSString *const kLiquidGlassViewID = @"com.tgssoftware.liquidssh.glass";

// Create/attach a single NSVisualEffectView that covers titlebar + content
static void addVibrancyToWindow(NSWindow* nsWindow) {
    if (!nsWindow) return;

    // Allow translucency in the native window
    [nsWindow setOpaque:NO];
    nsWindow.backgroundColor = [NSColor clearColor];

    // Make content extend under the titlebar so glass is continuous
    nsWindow.titlebarAppearsTransparent = YES;
    nsWindow.titleVisibility = NSWindowTitleHidden;
    nsWindow.styleMask |= NSWindowStyleMaskFullSizeContentView;
    nsWindow.movableByWindowBackground = YES;

    // We want the blur BELOW all Qt content, including the titlebar controls.
    // The superview of contentView is the "titlebar container" (includes titlebar area).
    NSView *container = nsWindow.contentView.superview ?: nsWindow.contentView;
    if (!container) return;

    // Avoid duplicates
    for (NSView *sub in container.subviews) {
        if ([sub isKindOfClass:[NSVisualEffectView class]] &&
            [sub.identifier isEqualToString:kLiquidGlassViewID]) {
            return;
        }
    }

    // Size to the container (titlebar + content), and autoresize with it
    NSVisualEffectView *blur = [[NSVisualEffectView alloc] initWithFrame:container.bounds];
    blur.identifier = kLiquidGlassViewID;
    blur.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
    blur.ignoresMouseEvents = YES;

    // Make it a proper background blur
    blur.blendingMode = NSVisualEffectBlendingModeBehindWindow;
    blur.state = NSVisualEffectStateActive;

    // Choose a material: HUDWindow is darker/more transparent.
    // Alternatives: NSVisualEffectMaterialTitlebar (brighter),
    //              NSVisualEffectMaterialSidebar (balanced),
    //              NSVisualEffectMaterialUnderWindowBackground (subtle).
    if (@available(macOS 10.14, *)) {
        blur.material = NSVisualEffectMaterialHUDWindow;
    } else {
        blur.material = NSVisualEffectMaterialLight; // older fallback
    }

    // Insert at the back so all app content and traffic lights stay on top
    [container addSubview:blur positioned:NSWindowBelow relativeTo:nil];
}

void enableLiquidGlass(QWidget* topLevel) {
    if (!topLevel) return;

    // Let Qt paint with translucency so the macOS blur shows through
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
