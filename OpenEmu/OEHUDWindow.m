/*
 Copyright (c) 2011, OpenEmu Team
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 * Neither the name of the OpenEmu Team nor the
 names of its contributors may be used to endorse or promote products
 derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#import "OEHUDWindow.h"
#import "NSImage+OEDrawingAdditions.h"
#import "NSColor+OEAdditions.h"

#import "OEButton.h"
#import "OEButtonCell.h"

#pragma mark - Private variables

static const CGFloat _OEHUDWindowLeftBorder            =  1.0;
static const CGFloat _OEHUDWindowRightBorder           =  1.0;
static const CGFloat _OEHUDWindowBottomBorder          =  1.0;
static const CGFloat _OEHUDWindowTopBorder             = 22.0;
static const CGFloat _OEHUDWindowCloseButtonLeftBorder =  1.0;
static const NSSize  _OEHUDWindowCloseButtonSize       = {21.0, 21.0};
static const CGFloat _OEHUDWindowTitleTextLeftMargin   = 1.0 /*_OEHUDWindowCloseButtonLeftBorder*/ + 21.0 /*_OEHUDWindowCloseButtonSize*/ + 2.0;
static const CGFloat _OEHUDWindowTitleTextRightMargin  = 10.0;
static const CGFloat _OEHUDWindowTitleTextTopMargin    =  2.0;

// Layout of OEHUDWindow:
//
// +------------------------+  black line,     height =  1.0
// | x       title          |  title content,  height = 20.0
// +------------------------+  black line,     height =  1.0
// |------------------------|  highlight line, height =  1.0 (applied over the main content view)
// |                        |
// |                        |  main content
// |                        |
// +------------------------+  black line,     height =  1.0
//

@interface OEHUDWindow () <NSWindowDelegate>

- (void)OE_commonHUDWindowInit;
- (void)windowDraggingDidBegin;
- (void)windowDraggingDidEnd;

@end

@interface OEHUDBorderWindow ()
- (BOOL)isDragging;
- (void)windowDraggingDidBegin;
- (void)windowDraggingDidEnd;
@end

// We need to use a different NSWindowDelegate implementation because of our custom implementation of window moving
// in OEHUDWindowThemeView: -mouseDown:, -mouseDragged: and -mouseUp:. Whenever the user drags the mouse to move
// the window, we set its frame origin to the new location, which means that the delegate receives -windowDidMove:
// every time the mouse is dragged instead of only when dragging ends.
// This is our solution:
// - Keep two separate delegates in OEHUDWindowDelegateProxy:
//   - superDelegate is the delegate used by NSWindow. We set it to be OEHUDWindow
//   - localDelegate is the delegate set by clients of OEHUDWindow
// - Trap -windowDidMove: messages in  superDelegate, i.e., OEHUDWindow. If the window is being dragged, do nothing
// - All other delegate methods are forwarded from superDelegate to localDelegate
// - Upon -mouseUp:, if the window is being dragged then dragging has ended, so send -windowDidMove: to localDelegate
@interface OEHUDWindowDelegateProxy : NSObject <NSWindowDelegate>
@property(nonatomic, unsafe_unretained) id<NSWindowDelegate> superDelegate; // TODO: replace unsafe_unretained with weak when we start requiring 10.8
@property(nonatomic, unsafe_unretained) id<NSWindowDelegate> localDelegate;
@end

@implementation OEHUDWindow
{
    OEHUDBorderWindow        *_borderWindow;
    OEHUDWindowDelegateProxy *_delegateProxy;
}

#pragma mark - Lifecycle

+ (void)initialize
{
    // Make sure not to reinitialize for subclassed objects
    if(self != [OEHUDWindow class]) return;

    if([NSImage imageNamed:@"hud_window_active"]) return;
    NSImage *img = [NSImage imageNamed:@"hud_window"];
    
    [img setName:@"hud_window_active"   forSubimageInRect:NSMakeRect(0, 0, img.size.width / 2, img.size.height)];
    [img setName:@"hud_window_inactive" forSubimageInRect:NSMakeRect(img.size.width / 2, 0, img.size.width / 2, img.size.height)];
}

- (id)initWithContentRect:(NSRect)contentRect styleMask:(NSUInteger)windowStyle backing:(NSBackingStoreType)bufferingType defer:(BOOL)deferCreation
{
    if((self = [super initWithContentRect:contentRect styleMask:NSBorderlessWindowMask | NSResizableWindowMask backing:bufferingType defer:deferCreation]))
    {
        [self OE_commonHUDWindowInit];
    }
    return self;
}

- (id)initWithContentRect:(NSRect)frame
{
    return [self initWithContentRect:frame styleMask:NSBorderlessWindowMask | NSResizableWindowMask backing:NSBackingStoreBuffered defer:NO];
}

- (void)awakeFromNib
{
    [super awakeFromNib];
    [self OE_commonHUDWindowInit];
}

- (void)dealloc 
{
    _borderWindow = nil;
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

#pragma mark - NSWindow overrides

- (void)setDelegate:(id<NSWindowDelegate>)delegate
{
    if(!_delegateProxy)
    {
        _delegateProxy = [OEHUDWindowDelegateProxy new];
        [_delegateProxy setSuperDelegate:self];
    }

    [_delegateProxy setLocalDelegate:delegate];
    [super setDelegate:_delegateProxy];
}

- (BOOL)canBecomeKeyWindow
{
    return YES;
}

- (BOOL)canBecomeMainWindow
{
    return YES;
}

// Notes:
// 1. [self setHasShadow:YES] is not enough for the shadow to appear;
// 2. [self setHasShadow:NO]; [self setHasShadow:YES] in -OE_layout makes the shadow appear only when the user resizes the window.
//    This same -setHasShadow: dance does *not* make the shadow appear upon the window being ordered front;
// 3. -display may prevent the shadow from appearing.
- (BOOL)hasShadow
{
    return YES;
}

// If we don’t override -orderWindow:relativeTo:, the border window may be ordered below the HUD window even
// if it was added as a child window ordered above its parent window, thus preventing OEHUDWindowThemeView
// from receiving mouse events.
- (void)orderWindow:(NSWindowOrderingMode)place relativeTo:(NSInteger)otherWin
{
    [super orderWindow:place relativeTo:otherWin];
    if(place != NSWindowOut)
        [_borderWindow orderWindow:NSWindowAbove relativeTo:[self windowNumber]];
}

- (void)setTitle:(NSString *)newTitle
{
    [super setTitle:newTitle];
    [_borderWindow display];
}

#pragma mark - NSWindowDelegate

- (void)windowDidMove:(NSNotification *)notification
{
    if(![_borderWindow isDragging] && [[_delegateProxy localDelegate] respondsToSelector:@selector(windowDidMove:)])
        [[_delegateProxy localDelegate] windowDidMove:notification];
}

#pragma mark - Private

- (void)OE_commonHUDWindowInit
{
    [self setHasShadow:YES];
    [self setOpaque:NO];
    [self setBackgroundColor:[NSColor clearColor]];
    
    [self setContentView:[[NSView alloc] initWithFrame:NSZeroRect]];
    [self setMainContentView:[[NSView alloc] initWithFrame:NSZeroRect]];
    
    // Register for notifications
    NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
    [nc addObserver:self selector:@selector(OE_layout) name:NSWindowDidResizeNotification object:self];
    [nc addObserver:self selector:@selector(OE_layout) name:NSWindowDidResignKeyNotification object:self];
    [nc addObserver:self selector:@selector(OE_layout) name:NSWindowDidBecomeKeyNotification object:self];
    
    _borderWindow = [[OEHUDBorderWindow alloc] initWithContentRect:[self frame] styleMask:0 backing:0 defer:0];
    [self addChildWindow:_borderWindow ordered:NSWindowAbove];
}

- (void)OE_layout
{
    [_borderWindow setFrame:[self frame] display:NO];
    [_borderWindow display];
}

- (void)windowDraggingDidEnd
{
    if([[_delegateProxy localDelegate] respondsToSelector:@selector(windowDidMove:)])
    {
        NSNotification *notification = [NSNotification notificationWithName:NSWindowDidMoveNotification object:self];
        [[_delegateProxy localDelegate] windowDidMove:notification];
    }
}

- (void)windowDraggingDidBegin
{
    if([[_delegateProxy localDelegate] respondsToSelector:@selector(windowWillMove:)])
    {
        NSNotification *notification = [NSNotification notificationWithName:NSWindowWillMoveNotification object:self];
        [[_delegateProxy localDelegate] windowWillMove:notification];
    }
}

#pragma mark - Public

- (OEHUDBorderWindow *)borderWindow
{
    return _borderWindow;
}

- (NSView *)mainContentView
{
    return [[[self contentView] subviews] lastObject];
}

- (void)setMainContentView:(NSView *)mainContentView
{
    if(mainContentView == [self mainContentView])
        return;

    [[self mainContentView] removeFromSuperview];

    [[super contentView] addSubview:mainContentView];

    const NSRect contentRect = [self convertRectFromScreen:[OEHUDWindow mainContentRectForFrameRect:[self frame]]];
    [mainContentView setFrame:contentRect];
    [mainContentView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
}

+ (NSRect)mainContentRectForFrameRect:(NSRect)windowFrame
{
    NSRect contentRect = windowFrame;

    contentRect.origin.x    += _OEHUDWindowLeftBorder;
    contentRect.origin.y    += _OEHUDWindowBottomBorder;
    contentRect.size.width  -= (_OEHUDWindowLeftBorder + _OEHUDWindowRightBorder);
    contentRect.size.height -= (_OEHUDWindowTopBorder  + _OEHUDWindowBottomBorder);

    return contentRect;
}

+ (NSRect)frameRectForMainContentRect:(NSRect)contentFrame
{
    NSRect windowFrame = contentFrame;

    windowFrame.origin.x    -= _OEHUDWindowLeftBorder;
    windowFrame.origin.y    -= _OEHUDWindowBottomBorder;
    windowFrame.size.width  += (_OEHUDWindowLeftBorder + _OEHUDWindowRightBorder);
    windowFrame.size.height += (_OEHUDWindowTopBorder  + _OEHUDWindowBottomBorder);

    return windowFrame;
}

@end


@implementation OEHUDBorderWindow

- (id)initWithContentRect:(NSRect)contentRect styleMask:(NSUInteger)aStyle backing:(NSBackingStoreType)bufferingType defer:(BOOL)flag
{
    if((self = [super initWithContentRect:contentRect styleMask:NSBorderlessWindowMask backing:NSBackingStoreBuffered defer:NO]))
    {
        [self setHasShadow:NO];
        [self setMovableByWindowBackground:NO];
        
        [self setOpaque:NO];
        [self setBackgroundColor:[NSColor clearColor]];
        
        NSView *borderView = [[OEHUDWindowThemeView alloc] initWithFrame:contentRect];
        [super setContentView:borderView];
    }
    
    return self;
}

- (void)setContentView:(NSView *)aView
{
}

- (void)display
{
    [[self contentView] display];
}

- (void)setParentWindow:(NSWindow *)window
{
    [super setParentWindow:window];
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (BOOL)canBecomeKeyWindow
{
    return NO;
}

- (BOOL)canBecomeMainWindow
{
    return NO;
}

- (BOOL)isDragging
{
    return [[self contentView] isDragging];
}

- (void)windowDraggingDidEnd
{
    [(OEHUDWindow *)[self parentWindow] windowDraggingDidEnd];
}

- (void)windowDraggingDidBegin
{
    [(OEHUDWindow *)[self parentWindow] windowDraggingDidBegin];
}

@end

@implementation OEHUDWindowThemeView
{
    NSPoint baseMouseLocation;
    NSPoint baseOrigin;
}

static NSImage *frameImage, *frameImageInactive;
#pragma mark -
+ (void)initialize
{
    if(self != [OEHUDWindowThemeView class]) return;

    NSRect centerRect = (NSRect){{14, 26}, {1, 1}};
    frameImage = [[NSImage imageNamed:@"hud_window_active"] ninePartImageWithStretchedRect:centerRect];
    frameImageInactive = [[NSImage imageNamed:@"hud_window_inactive"] ninePartImageWithStretchedRect:centerRect];
}

- (BOOL)isOpaque
{
    return NO;
}

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        NSRect closeButtonRect    = [self titleBarRect];
        closeButtonRect.origin.x += _OEHUDWindowCloseButtonLeftBorder;
        closeButtonRect.size      = _OEHUDWindowCloseButtonSize;

        OEButton *closeButton = [[OEButton alloc] initWithFrame:closeButtonRect];
        [closeButton setCell:[[OEButtonCell alloc] initTextCell:@""]];
        [closeButton setThemeKey:@"hud_close_button"];
        
        [closeButton setAutoresizingMask:NSViewMaxXMargin | NSViewMinYMargin];
        [closeButton setAction:@selector(performClose:)];
        [self addSubview:closeButton];
    }
    return self;
}

- (NSRect)titleBarRect
{
    NSRect titleBarRect      = [self bounds];

    titleBarRect.size.height = _OEHUDWindowTopBorder;
    titleBarRect.origin.y    = [self bounds].size.height - titleBarRect.size.height;
    
    return titleBarRect;
}
- (void)drawRect:(NSRect)dirtyRect
{
    [[NSColor clearColor] setFill];
    NSRectFill([self bounds]);

    BOOL isFocused = [[[self window] parentWindow] isKeyWindow] && [NSApp isActive];
    NSImage *image = isFocused ? frameImage : frameImageInactive;
    [image drawInRect:[self bounds] fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0];

    // If the border window has been ordered out (e.g., when going full screen), [[self window] parentWindow] returns nil.
    // In this case, don’t bother drawing the window title
    NSString *windowTitle = [[[self window] parentWindow] title];
    if(windowTitle)
    {
        NSMutableDictionary *titleAttributes = [NSMutableDictionary dictionary];

        NSMutableParagraphStyle *ps = [[NSMutableParagraphStyle alloc] init];
        [ps setLineBreakMode:NSLineBreakByTruncatingMiddle];
        [ps setAlignment:NSCenterTextAlignment];
        [titleAttributes setObject:ps forKey:NSParagraphStyleAttributeName];

        NSColor *textColor = isFocused ? [NSColor colorWithDeviceWhite:0.86 alpha:1.0] : [NSColor colorWithDeviceWhite:0.61 alpha:1.0];
        NSFont *font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:0 weight:2.0 size:13.0];
        NSShadow *shadow = [[NSShadow alloc] init];
        [shadow setShadowColor:[NSColor colorWithDeviceRed:0.129 green:0.129 blue:0.129 alpha:1.0]];
        [shadow setShadowBlurRadius:1.0];
        [shadow setShadowOffset:NSMakeSize(0, 1)];

        [titleAttributes setObject:textColor forKey:NSForegroundColorAttributeName];
        [titleAttributes setObject:font forKey:NSFontAttributeName];
        [titleAttributes setObject:shadow forKey:NSShadowAttributeName];

        NSRect titleTextRect = [self titleBarRect];
        titleTextRect.origin.x   += _OEHUDWindowTitleTextLeftMargin;
        titleTextRect.size.width -= (_OEHUDWindowTitleTextLeftMargin + _OEHUDWindowTitleTextRightMargin);
        titleTextRect.origin.y   -= _OEHUDWindowTitleTextTopMargin;

        NSAttributedString *attributedWindowTitle = [[NSAttributedString alloc] initWithString:windowTitle attributes:titleAttributes];
        [attributedWindowTitle drawInRect:titleTextRect];
    }
}

- (NSView *)hitTest:(NSPoint)aPoint
{
    // This makes sure the parent window becomes key window, even when clicking the close button
    [[[self window] parentWindow] makeKeyWindow];

    return [super hitTest:aPoint];
}

- (void)mouseDown:(NSEvent *)theEvent
{
    NSPoint pointInView = [self convertPoint:[theEvent locationInWindow] fromView:nil];

    [self setDragging:NO];

    if(!NSPointInRect(pointInView, [self titleBarRect]))
    {
        [[self nextResponder] mouseDown:theEvent];
        return;
    }
    
    NSWindow *window = [self window];
    baseMouseLocation = [window convertRectToScreen:(NSRect){[theEvent locationInWindow], NSZeroSize}].origin;
    baseOrigin = [window frame].origin;
    [self setDragging:YES];
    [(OEHUDBorderWindow *)[self window] windowDraggingDidBegin];
}

- (void)mouseDragged:(NSEvent *)theEvent
{
    if (![self isDragging])
    {
        [[self nextResponder] mouseDragged:theEvent];
        return;
    }
    
    NSWindow *window = [[self window] parentWindow];
    NSPoint newMousePosition = [window convertRectToScreen:(NSRect){[theEvent locationInWindow], NSZeroSize}].origin;
    NSPoint delta = NSMakePoint(newMousePosition.x-baseMouseLocation.x, newMousePosition.y-baseMouseLocation.y);
    
    NSScreen *primaryScreen = [[NSScreen screens] objectAtIndex:0];
    NSRect menuRect = (NSRect){
        .origin = NSZeroPoint,
        .size = {
            .width = [primaryScreen frame].size.width,
            .height = [[NSApp mainMenu] menuBarHeight]
        }
    };
    menuRect.origin.y = [primaryScreen frame].size.height - menuRect.size.height;
    
    NSRect frame = [window frame];
    BOOL isAboveMenu = (NSMaxX(frame) > NSMinX(menuRect) && NSMinX(frame) < NSMaxX(menuRect) && NSMaxY(frame) > NSMaxY(menuRect)); // are we already above the menubar somehow?
    frame.origin = (NSPoint){baseOrigin.x + delta.x, baseOrigin.y + delta.y};
    
    if (!isAboveMenu) {
        // we're not already above the menubar. Does this mouse movement attempt to intersect us with the menubar?
        if (NSIntersectsRect(frame, menuRect)) {
            // prohibit the movement into the rect. Depending on which side we were on to start with (left, bottom, right), constrain in that direction
            NSRect origFrame = [window frame];
            if (NSMaxX(origFrame) <= NSMinX(menuRect)) {
                // left
                frame.origin.x = menuRect.origin.x - frame.size.width;
            } else if (NSMinX(origFrame) >= NSMaxX(menuRect)) {
                // right
                frame.origin.x = NSMaxX(menuRect);
            } else {
                // assume bottom
                frame.origin.y = menuRect.origin.y - frame.size.height;
            }
        }
    }
    
    [window setFrameOrigin:frame.origin];
}

- (void)mouseUp:(NSEvent *)theEvent
{
    if (![self isDragging])
    {
        [[self nextResponder] mouseUp:theEvent];
        return;
    }
    [self setDragging:NO];
    [(OEHUDBorderWindow *)[self window] windowDraggingDidEnd];
}

- (id)validRequestorForSendType:(NSString *)sendType returnType:(NSString *)returnType
{
    return nil;
}

@end

@implementation OEHUDWindowDelegateProxy

- (BOOL)respondsToSelector:(SEL)selector
{
    return [_superDelegate respondsToSelector:selector] || [_localDelegate respondsToSelector:selector];
}

- (id)forwardingTargetForSelector:(SEL)selector
{
    // OEHUDWindow takes precedence over its (local) delegate
    if([_superDelegate respondsToSelector:selector])
        return _superDelegate;

    if([_localDelegate respondsToSelector:selector])
        return _localDelegate;

    return nil;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"super delegate is %@, local delegate is %@", _superDelegate, _localDelegate];
}

@end
