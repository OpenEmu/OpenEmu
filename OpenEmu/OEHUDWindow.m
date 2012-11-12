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

@interface OEHUDWindow ()
{
	NSWindow *_borderWindow;
}

- (void)OE_commonHUDWindowInit;

@end

@interface OEHUDBorderWindow : NSWindow
@end

@implementation OEHUDWindow

+ (void)initialize
{
    // Make sure not to reinitialize for subclassed objects
    if(self != [OEHUDWindow class]) return;
    
    if([NSImage imageNamed:@"hud_window_active"]) return;
    NSImage *img = [NSImage imageNamed:@"hud_window"];
    
    [img setName:@"hud_window_active"   forSubimageInRect:NSMakeRect(0, 0, img.size.width / 2, img.size.height)];
    [img setName:@"hud_window_inactive" forSubimageInRect:NSMakeRect(img.size.width / 2, 0, img.size.width / 2, img.size.height)];
}

#pragma mark -

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
    self = [self initWithContentRect:frame styleMask:NSBorderlessWindowMask backing:NSBackingStoreBuffered defer:NO];
    if (self) 
    {}
    return self;
}

- (void)awakeFromNib
{
    [super awakeFromNib];
    [self OE_commonHUDWindowInit];
}

- (void)dealloc 
{
    DLog(@"OEHUDWindow");
    _borderWindow = nil;
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

#pragma mark -
#pragma mark Private

- (void)OE_commonHUDWindowInit
{
    DLog(@"OEHUDWindow");
    
    [self setHasShadow:NO];
    [self setOpaque:NO];
    [self setBackgroundColor:[NSColor clearColor]];
    
    [super setContentView:[[NSView alloc] initWithFrame:NSZeroRect]];
    [self setContentView:[[NSView alloc] initWithFrame:NSZeroRect]];
    
    // Register for notifications
    NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
    [nc addObserver:self selector:@selector(_layout) name:NSWindowDidResizeNotification object:self];
    
    [nc addObserver:self selector:@selector(_layout) name:NSWindowDidResignKeyNotification object:self];
    [nc addObserver:self selector:@selector(_layout) name:NSWindowDidBecomeKeyNotification object:self];
    
    _borderWindow = [[OEHUDBorderWindow alloc] init];
    [self addChildWindow:_borderWindow ordered:NSWindowAbove];
    [_borderWindow orderFront:self];
}

- (void)_layout
{
    [_borderWindow setFrame:[self frame] display:YES];
    [_borderWindow display];
}

- (id)contentView
{
    return [[[super contentView] subviews] lastObject];
}

- (void)setContentView:(NSView *)aView
{
    NSView *contentView = [[[super contentView] subviews] lastObject];
    
    if(contentView)[contentView removeFromSuperview];
    
    NSView *actualContentView = [super contentView];
    [actualContentView addSubview:aView];
    
    
    NSRect contentRect;
    contentRect.origin = NSMakePoint(0, 0);
    contentRect.size = [self frame].size;
    
    contentRect.size.height -= 21;
    
    [aView setFrame:contentRect];
    [aView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
}

- (void)setTitle:(NSString *)newTitle
{
    [super setTitle:newTitle];
    [_borderWindow display];
}

#pragma mark -
#pragma mark NSWindow Overrides

- (BOOL)canBecomeKeyWindow
{
    return YES;
}

- (BOOL)canBecomeMainWindow
{
    return YES;
}

@end

@implementation OEHUDBorderWindow

- (id)init
{
    if((self = [self initWithContentRect:NSZeroRect styleMask:NSBorderlessWindowMask backing:NSBackingStoreBuffered defer:NO]))
    {
        DLog(@"OEHUDBorderWindow");
        [self setHasShadow:NO];
        [self setMovableByWindowBackground:NO];
        
        [self setOpaque:NO];
        [self setBackgroundColor:[NSColor clearColor]];
        
        NSView *borderView = [[OEHUDWindowThemeView alloc] initWithFrame:NSZeroRect];
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
    DLog(@"OEHUDBorderWindow");
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

@end

@implementation OEHUDWindowThemeView
{
    NSPoint lastMouseLocation;
    BOOL isResizing;
}

#pragma mark -

- (BOOL)isOpaque
{
    return NO;
}

- (id)init
{
    self = [super init];
    if (self)
    {
        DLog(@"OEHUDWindowThemeView");
    }
    return self;
}

- (void)dealloc
{
    DLog(@"OEHUDWindowThemeView");
}

- (NSRect)titleBarRect
{
    NSRect titleBarRect = [self bounds];
    
    titleBarRect.size.height = 22;
    titleBarRect.origin.y = [self bounds].size.height-titleBarRect.size.height;
    
    return titleBarRect;
}

- (void)drawRect:(NSRect)dirtyRect
{
    [[NSColor clearColor] setFill];
    NSRectFill([self bounds]);
    
    BOOL isFocused = [[self window].parentWindow isMainWindow] && [NSApp isActive];
    
    NSImage *borderImage = isFocused ? [NSImage imageNamed:@"hud_window_active"] : [NSImage imageNamed:@"hud_window_inactive"];
    [borderImage drawInRect:[self bounds] fromRect:NSZeroRect operation:NSCompositeSourceOver/*NSCompositeSourceOver*/ fraction:1.0 respectFlipped:YES hints:nil leftBorder:14 rightBorder:14 topBorder:23 bottomBorder:23];
    
    NSMutableDictionary *titleAttribtues = [NSMutableDictionary dictionary];
    
    NSMutableParagraphStyle *ps = [[NSMutableParagraphStyle alloc] init];
    [ps setLineBreakMode:NSLineBreakByTruncatingMiddle];
    [ps setAlignment:NSCenterTextAlignment];
    [titleAttribtues setObject:ps forKey:NSParagraphStyleAttributeName];
    
    NSColor *textColor = isFocused ? [NSColor colorWithDeviceWhite:0.86 alpha:1.0] : [NSColor colorWithDeviceWhite:0.61 alpha:1.0];
    NSFont *font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:0 weight:2.0 size:13.0];
    NSShadow *shadow = [[NSShadow alloc] init];
    [shadow setShadowColor:[NSColor colorWithDeviceRed:0.129 green:0.129 blue:0.129 alpha:1.0]];
    [shadow setShadowBlurRadius:1.0];
    [shadow setShadowOffset:NSMakeSize(0, 1)];
    
    [titleAttribtues setObject:textColor forKey:NSForegroundColorAttributeName];
    [titleAttribtues setObject:font forKey:NSFontAttributeName];
    [titleAttribtues setObject:shadow forKey:NSShadowAttributeName];
    
    NSRect titleBarRect = NSInsetRect([self titleBarRect], 10, 0);
    titleBarRect.origin.y -= 2;
    
    NSString *windowTitle = [[self window].parentWindow title];
    NSAttributedString *attributedWindowTitle = [[NSAttributedString alloc] initWithString:windowTitle attributes:titleAttribtues];
    [attributedWindowTitle drawInRect:titleBarRect];
}

- (void)mouseDown:(NSEvent *)theEvent
{
    NSPoint pointInView = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    
    lastMouseLocation = NSZeroPoint;
    
    if(!NSPointInRect(pointInView, [self titleBarRect]))
    {
        [[self nextResponder] mouseDown:theEvent];
        return;
    }
    
    NSWindow *window = [self window];
    lastMouseLocation = [window convertBaseToScreen:[theEvent locationInWindow]];
}

- (void)mouseDragged:(NSEvent *)theEvent
{
    if(NSEqualPoints(lastMouseLocation, NSZeroPoint))
    {
        [[self nextResponder] mouseDragged:theEvent];
        return;
    }
    
    NSWindow *window = [[self window] parentWindow];
    NSPoint newMousePosition = [window convertBaseToScreen:[theEvent locationInWindow]];
    
    NSPoint delta = NSMakePoint(newMousePosition.x-lastMouseLocation.x, newMousePosition.y-lastMouseLocation.y);
    
        NSPoint frameOrigin = [window frame].origin;
        
        frameOrigin.x += delta.x;
        frameOrigin.y += delta.y;
        
        [window setFrameOrigin:frameOrigin];
    
    
    lastMouseLocation = newMousePosition;
}

- (void)mouseUp:(NSEvent *)theEvent
{
    if(NSEqualPoints(lastMouseLocation, NSZeroPoint))
    {
        [[self nextResponder] mouseUp:theEvent];
        return;
    }
}

@end
