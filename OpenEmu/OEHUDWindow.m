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
#import "NSColor+OEAdditions.h"

#import "OEButton.h"
#import "OEButtonCell.h"
#import "OETheme.h"
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
@property(nonatomic, weak) id<NSWindowDelegate> superDelegate;
@property(nonatomic, weak) id<NSWindowDelegate> localDelegate;
@end

@implementation OEHUDWindow
{
    OEHUDBorderWindow        *_borderWindow;
    OEHUDWindowDelegateProxy *_delegateProxy;
    NSBox                    *_backgroundView;
    BOOL                      _isDeallocating;
}

#pragma mark - Lifecycle
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
    _isDeallocating = YES;
    _borderWindow = nil;
    [super setDelegate:nil];
}

- (BOOL)validateUserInterfaceItem:(id<NSValidatedUserInterfaceItem>)anItem
{
    if([super validateUserInterfaceItem:anItem])
        return YES;

    return [anItem action] == @selector(performClose:);
}

- (void)performClose:(id)sender
{
    NSWindowController *windowController = [self windowController];
    NSDocument *document = [windowController document];

    if(document != nil && windowController != nil)
        [document shouldCloseWindowController:windowController delegate:self shouldCloseSelector:@selector(_document:shouldClose:contextInfo:) contextInfo:NULL];
    else
        [self _document:nil shouldClose:YES contextInfo:NULL];
}

- (void)_document:(NSDocument *)document shouldClose:(BOOL)shouldClose contextInfo:(void  *)contextInfo
{
    if(shouldClose)
    {
        if([[self delegate] respondsToSelector:@selector(windowShouldClose:)])
            shouldClose = [[self delegate] windowShouldClose:self];
        else if([self respondsToSelector:@selector(windowShouldClose:)])
            shouldClose = [self windowShouldClose:self];
    }

    if(shouldClose) [self close];
}

#pragma mark - NSWindow overrides

- (void)setDelegate:(id<NSWindowDelegate>)delegate
{
    if(_isDeallocating) return;

    if(!_delegateProxy)
    {
        _delegateProxy = [OEHUDWindowDelegateProxy new];
        [_delegateProxy setSuperDelegate:self];
    }

    [_delegateProxy setLocalDelegate:delegate];
    [super setDelegate:nil];
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
    
    [self OE_setupBackgroundView];
    [self setMainContentView:[[NSView alloc] initWithFrame:NSZeroRect]];
    
    // Register for notifications
    NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
    [nc addObserver:self selector:@selector(OE_layout) name:NSWindowDidResizeNotification object:self];
    [nc addObserver:self selector:@selector(OE_layout) name:NSWindowDidResignKeyNotification object:self];
    [nc addObserver:self selector:@selector(OE_layout) name:NSWindowDidBecomeKeyNotification object:self];
    
    _borderWindow = [[OEHUDBorderWindow alloc] initWithContentRect:[self frame] styleMask:0 backing:0 defer:0];
    [self addChildWindow:_borderWindow ordered:NSWindowAbove];
}

- (void)OE_setupBackgroundView
{
    NSRect contentRect = [self frame];
    contentRect.origin = NSZeroPoint;

    contentRect.origin.x    += _OEHUDWindowLeftBorder;
    contentRect.origin.y    += _OEHUDWindowBottomBorder;
    contentRect.size.width  -= (_OEHUDWindowLeftBorder + _OEHUDWindowRightBorder);
    contentRect.size.height -= (_OEHUDWindowTopBorder  + _OEHUDWindowBottomBorder);
    _backgroundView = [[NSBox alloc] initWithFrame:contentRect];
    [_backgroundView setBoxType:NSBoxCustom];
    [_backgroundView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];

    [[super contentView] addSubview:_backgroundView positioned:NSWindowBelow relativeTo:nil];
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
- (NSColor *)contentBackgroundColor
{
    return [_backgroundView fillColor];
}

- (void)setContentBackgroundColor:(NSColor *)value
{
    [_backgroundView setFillColor:value];
}

- (void)setMainContentView:(NSView *)value
{
    if(_mainContentView == value)
        return;

    [_mainContentView removeFromSuperview];
    _mainContentView = value;

    [[super contentView] addSubview:_mainContentView];

    const NSRect contentRect = [self convertRectFromScreen:[OEHUDWindow mainContentRectForFrameRect:[self frame]]];
    [_mainContentView setFrame:contentRect];
    [_mainContentView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
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
    [super display];
    [[self contentView] display];
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

    frameImage = [[OETheme sharedTheme] imageForKey:@"hud_window" forState:OEThemeInputStateWindowActive];
    frameImageInactive = [[OETheme sharedTheme] imageForKey:@"hud_window" forState:OEThemeInputStateWindowInactive];
}

- (BOOL)isOpaque
{
    return NO;
}

- (id)initWithFrame:(NSRect)frame
{
    if((self = [super initWithFrame:frame]))
    {
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

    // technically the parent window does not have a title (title-less style mask), so appkit seems not to bother updating it
    NSString *windowTitle = [self.window.parentWindow.windowController.document displayName];
    if(windowTitle)
    {
        NSMutableDictionary *titleAttributes = [NSMutableDictionary dictionary];

        NSMutableParagraphStyle *ps = [[NSMutableParagraphStyle alloc] init];
        [ps setLineBreakMode:NSLineBreakByTruncatingMiddle];
        [ps setAlignment:NSCenterTextAlignment];
        [titleAttributes setObject:ps forKey:NSParagraphStyleAttributeName];

        NSColor *textColor = isFocused ? [NSColor colorWithDeviceWhite:0.86 alpha:1.0] : [NSColor colorWithDeviceWhite:0.61 alpha:1.0];
        NSFont *font = [NSFont systemFontOfSize:13];
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
    if(![self isDragging])
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
    // The default implementation of validRequestorForSendType:returnType: passes the
    // message to next responder which again is OEHUDWindowDelegateProxy thus creating
    // infinite recursion.
    // The bug also exists for every responder method that is overridden by the delegate
    // and passed on to next responder by default!
    //
    // For now validRequestorForSendType:returnType: is the only crash seen in the wild.
    if(selector == @selector(validRequestorForSendType:returnType:))
    {
        return nil;
    }

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
