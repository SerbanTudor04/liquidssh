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

    [nsWindow setOpaque:NO];
    nsWindow.backgroundColor = [NSColor clearColor];
    nsWindow.titlebarAppearsTransparent = YES;
    nsWindow.titleVisibility = NSWindowTitleHidden;
    nsWindow.styleMask |= NSWindowStyleMaskFullSizeContentView;
    nsWindow.movableByWindowBackground = NO;

    NSView *content  = nsWindow.contentView;
    if (!content) return;

    // IMPORTANT: add blur/drag to the *container* (superview of contentView),
    // and put the blur BELOW the contentView so Qt paints above it.
    NSView *container = content.superview;
    if (!container) return;

    // ---- Blur behind content ----
    BOOL hasBlur = NO;
    for (NSView *sub in container.subviews) {
        if ([sub isKindOfClass:[NSVisualEffectView class]] &&
            [sub.identifier isEqualToString:kLiquidGlassViewID]) { hasBlur = YES; break; }
    }
    if (!hasBlur) {
        PassthroughVisualEffectView *blur =
            [[PassthroughVisualEffectView alloc] initWithFrame:container.bounds];
        blur.identifier = kLiquidGlassViewID;
        blur.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
        blur.blendingMode = NSVisualEffectBlendingModeBehindWindow;
        blur.state = NSVisualEffectStateActive;
        if (@available(macOS 10.14, *)) {
            blur.material = NSVisualEffectMaterialUnderWindowBackground;
        } else {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
            blur.material = NSVisualEffectMaterialLight;
#pragma clang diagnostic pop
        }

        // Put blur BELOW the Qt contentView
        [container addSubview:blur positioned:NSWindowBelow relativeTo:content];
    }

    // ---- Draggable overlay in titlebar strip (on container, above content) ----
    BOOL hasDragView = NO;
    for (NSView *sub in container.subviews) {
        if ([sub.identifier isEqualToString:kLiquidDragViewID]) { hasDragView = YES; break; }
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
            dragView.layer.backgroundColor = [NSColor clearColor].CGColor;

            [container addSubview:dragView positioned:NSWindowAbove relativeTo:content];
        }
    }
}


void enableLiquidGlass(QWidget* topLevel) {
    if (!topLevel) return;

    // Let Qt paint with translucency
    topLevel->setAttribute(Qt::WA_TranslucentBackground, true);

    QWindow* qwin = topLevel->windowHandle();
    if (!qwin && topLevel->nativeParentWidget())
        qwin = topLevel->nativeParentWidget()->windowHandle();
    if (!qwin) return;

    // On macOS, Qt's WId is the NSView*
    WId wid = qwin->winId();
    if (!wid) return;

    NSView* nsView = (__bridge NSView*)(void*)wid;
    if (!nsView) return;

    NSWindow* nsWindow = nsView.window;
    if (!nsWindow) return;

    addVibrancyAndDrag(nsWindow);   // your function
}
