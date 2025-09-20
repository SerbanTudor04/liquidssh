#import <AppKit/AppKit.h>
#include <QWidget>
#include <QWindow>
#include <QGuiApplication>

static NSString *const kLiquidGlassViewID   = @"com.tgssoftware.liquidssh.glass";
static NSString *const kLiquidDragViewID    = @"com.tgssoftware.liquidssh.drag";

// A blur view that never intercepts mouse events
@interface PassthroughVisualEffectView : NSVisualEffectView
@end
@implementation PassthroughVisualEffectView
- (NSView *)hitTest:(NSPoint)point { return nil; } // pass all events through
@end

// A transparent view that *does* intercept clicks, purely to drag the window
@interface DraggableTitlebarView : NSView
@end
@implementation DraggableTitlebarView
- (BOOL)mouseDownCanMoveWindow { return YES; }
@end

// Compute the titlebar height (container is the superview of contentView)
static CGFloat TitlebarHeight(NSWindow *win) {
    if (!win || !win.contentView) return 0.0;
    // In the container's coordinate space, contentView's Y origin equals titlebar height.
    return win.contentView.frame.origin.y;
}

static void addVibrancyAndDrag(NSWindow* nsWindow) {
    if (!nsWindow) return;

    // Allow translucency in the native window and extend content into titlebar
    [nsWindow setOpaque:NO];
    nsWindow.backgroundColor = [NSColor clearColor];
    nsWindow.titlebarAppearsTransparent = YES;
    nsWindow.titleVisibility = NSWindowTitleHidden;
    nsWindow.styleMask |= NSWindowStyleMaskFullSizeContentView;
    nsWindow.movableByWindowBackground = NO; // we'll define a specific drag zone

    // Container covers titlebar + content area
    NSView *container = nsWindow.contentView.superview ?: nsWindow.contentView;
    if (!container) return;

    // ---- Blur layer (behind everything) ----
    BOOL hasBlur = NO;
    for (NSView *sub in container.subviews) {
        if ([sub isKindOfClass:[NSVisualEffectView class]] &&
            [sub.identifier isEqualToString:kLiquidGlassViewID]) {
            hasBlur = YES;
            break;
        }
    }
    if (!hasBlur) {
        PassthroughVisualEffectView *blur =
            [[PassthroughVisualEffectView alloc] initWithFrame:container.bounds];
        blur.identifier = kLiquidGlassViewID;
        blur.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
        blur.blendingMode = NSVisualEffectBlendingModeBehindWindow;
        blur.state = NSVisualEffectStateActive;

        if (@available(macOS 10.14, *)) {
            // Darker / more “see-through”
            blur.material = NSVisualEffectMaterialHUDWindow;
        } else {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
            blur.material = NSVisualEffectMaterialLight;
#pragma clang diagnostic pop
        }

        [container addSubview:blur positioned:NSWindowBelow relativeTo:nil];
    }

    // ---- Draggable overlay only in the titlebar strip ----
    // (Transparent view that tells AppKit "clicks here drag the window")
    BOOL hasDragView = NO;
    for (NSView *sub in container.subviews) {
        if ([sub.identifier isEqualToString:kLiquidDragViewID]) {
            hasDragView = YES;
            break;
        }
    }
    if (!hasDragView) {
        CGFloat th = TitlebarHeight(nsWindow);
        if (th > 0.0) {
            NSRect frame = NSMakeRect(0,
                                      container.bounds.size.height - th,
                                      container.bounds.size.width,
                                      th);
            DraggableTitlebarView *dragView = [[DraggableTitlebarView alloc] initWithFrame:frame];
            dragView.identifier = kLiquidDragViewID;
            dragView.autoresizingMask = NSViewWidthSizable | NSViewMinYMargin;
            dragView.wantsLayer = YES;
            dragView.layer.backgroundColor = [NSColor clearColor].CGColor; // fully transparent

            // Put it on top so it can receive clicks in empty titlebar areas,
            // but note: native traffic-light buttons still sit above.
            [container addSubview:dragView positioned:NSWindowAbove relativeTo:nil];
        }
    }
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

    addVibrancyAndDrag(nsWindow);
}
