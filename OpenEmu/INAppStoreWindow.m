//
//  INAppStoreWindow.m
//
//  Copyright 2011 Indragie Karunaratne. All rights reserved.
//
//  Licensed under the BSD License <http://www.opensource.org/licenses/bsd-license>
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#import "INAppStoreWindow.h"

/** -----------------------------------------
 - There are 2 sets of colors, one for an active (key) state and one for an inactivate state
 - Each set contains 3 colors. 2 colors for the start and end of the title gradient, and another color to draw the separator line on the bottom
 - These colors are meant to mimic the color of the default titlebar (taken from OS X 10.6), but are subject
 to change at any time
 ----------------------------------------- **/

#define COLOR_KEY_START [NSColor colorWithDeviceRed:0.659 green:0.659 blue:0.659 alpha:1.00]
#define COLOR_KEY_END [NSColor colorWithDeviceRed:0.812 green:0.812 blue:0.812 alpha:1.00]
#define COLOR_KEY_BOTTOM [NSColor colorWithDeviceRed:0.318 green:0.318 blue:0.318 alpha:1.00]

#define COLOR_NOTKEY_START [NSColor colorWithDeviceRed:0.851 green:0.851 blue:0.851 alpha:1.00]
#define COLOR_NOTKEY_END [NSColor colorWithDeviceRed:0.929 green:0.929 blue:0.929 alpha:1.00]
#define COLOR_NOTKEY_BOTTOM [NSColor colorWithDeviceRed:0.600 green:0.600 blue:0.600 alpha:1.00]

/** Corner clipping radius **/
#define CORNER_CLIP_RADIUS 4.0

@interface INAppStoreWindow ()
- (void)_doInitialWindowSetup;
- (void)_createTitlebarView;
- (void)_setupTrafficLightsTrackingArea;
- (void)_recalculateFrameForTitleBarView;
- (void)_layoutTrafficLightsAndContent;
- (float)_minimumTitlebarHeight;
@end

@implementation INTitlebarView
- (void)drawRect:(NSRect)dirtyRect
{
    BOOL key = [[self window] isKeyWindow];
    NSRect drawingRect = [self bounds];
    drawingRect.size.height -= 1.0; // Decrease the height by 1.0px to show the highlight line at the top
    
    NSColor *startColor = key ? COLOR_KEY_START : COLOR_NOTKEY_START;
    NSColor *endColor = key ? COLOR_KEY_END : COLOR_NOTKEY_END;
    NSBezierPath *clipPath = [self clippingPathWithRect:drawingRect cornerRadius:CORNER_CLIP_RADIUS];
    [NSGraphicsContext saveGraphicsState];
    [clipPath addClip];
    NSGradient *gradient = [[NSGradient alloc] initWithStartingColor:startColor endingColor:endColor];
    [gradient drawInRect:drawingRect angle:90];
    [gradient release];
    [NSGraphicsContext restoreGraphicsState];
    
    NSColor *bottomColor = key ? COLOR_KEY_BOTTOM : COLOR_NOTKEY_BOTTOM;
    NSRect bottomRect = NSMakeRect(0.0, NSMinY(drawingRect), NSWidth(drawingRect), 1.0);
    [bottomColor set];
    NSRectFill(bottomRect);
    
    
    INAppStoreWindow *win = (INAppStoreWindow*)[self window];
    if(win.titleBarString)
    {
        NSRect rect = NSMakeRect(0, [self frame].size.height-24, [self frame].size.width, 20);
        
        NSColor *textColor = [NSColor colorWithDeviceRed:0.0 green:0.0 blue:0.157 alpha:1.0];
        NSShadow *shadow = [[[NSShadow alloc] init] autorelease];
        [shadow setShadowBlurRadius:1.0];
        [shadow setShadowOffset:NSMakeSize(0, -1)];
        [shadow setShadowColor:[NSColor colorWithDeviceWhite:1.0 alpha:0.6]];
        
        NSFont *font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:0 weight:4.0 size:13.0];
        NSMutableParagraphStyle *ps = [[[NSMutableParagraphStyle alloc] init] autorelease];
        [ps setAlignment:NSCenterTextAlignment];
        
        NSMutableDictionary *attr = [NSMutableDictionary dictionary];
        [attr setObject:ps forKey:NSParagraphStyleAttributeName];
        [attr setObject:textColor forKey:NSForegroundColorAttributeName];
        [attr setObject:shadow forKey:NSShadowAttributeName];
        [attr setObject:font forKey:NSFontAttributeName];
        
        [win.titleBarString drawInRect:rect withAttributes:attr];
    }
}

// Uses code from NSBezierPath+PXRoundedRectangleAdditions by Andy Matuschak
// <http://code.andymatuschak.org/pixen/trunk/NSBezierPath+PXRoundedRectangleAdditions.m>

- (NSBezierPath*)clippingPathWithRect:(NSRect)aRect cornerRadius:(float)radius
{
    NSBezierPath *path = [NSBezierPath bezierPath];
    NSRect rect = NSInsetRect(aRect, radius, radius);
    NSPoint cornerPoint = NSMakePoint(NSMinX(aRect), NSMinY(aRect));
    // Create a rounded rectangle path, omitting the bottom left/right corners
    [path appendBezierPathWithPoints:&cornerPoint count:1];
    cornerPoint = NSMakePoint(NSMaxX(aRect), NSMinY(aRect));
    [path appendBezierPathWithPoints:&cornerPoint count:1];
    [path appendBezierPathWithArcWithCenter:NSMakePoint(NSMaxX(rect), NSMaxY(rect)) radius:radius startAngle:  0.0 endAngle: 90.0];
    [path appendBezierPathWithArcWithCenter:NSMakePoint(NSMinX(rect), NSMaxY(rect)) radius:radius startAngle: 90.0 endAngle:180.0];
    [path closePath];
    return path;
}
#pragma mark -
#pragma Moouse Events
- (void)mouseDown:(NSEvent *)theEvent
{
    // start dragging window
    NSWindow *win = [self window];
    NSPoint startPos = [win convertBaseToScreen:[theEvent locationInWindow]];
    NSPoint origin = win.frame.origin;
    while ((theEvent=[win nextEventMatchingMask:(NSLeftMouseDraggedMask | NSLeftMouseUpMask)]) && theEvent && [theEvent type]!=NSLeftMouseUp)
    {
        NSPoint newPos = [win convertBaseToScreen:[theEvent locationInWindow]];
        [win setFrameOrigin:NSMakePoint(origin.x + newPos.x-startPos.x, origin.y + newPos.y-startPos.y)];
    }
}
@end

@implementation INAppStoreWindow

#pragma mark -
#pragma mark Initialization

- (id)init
{
    if ((self = [super init])) {
        [self _doInitialWindowSetup];
    }
    return self;
}

- (void)awakeFromNib
{
    [super awakeFromNib];
    [self _doInitialWindowSetup];
}

#pragma mark -
#pragma mark Memory Management

- (void)dealloc
{
    [_titleBarView release];
    [super dealloc];
}

#pragma mark -
#pragma mark NSWindow Overrides

// Disable window titles
- (NSString*)title{
    return @"";
}

- (void)setTitle:(NSString *)aString{
    return;
}

- (void)setContentSize:(NSSize)aSize{
    float adjustment = self.titleBarHeight - 22;
    if(adjustment > 0)
        aSize.height += adjustment;
    
    [super setContentSize:aSize];
    
    
    [self _recalculateFrameForTitleBarView];
    [self _layoutTrafficLightsAndContent];
    [self display];
}

- (void)setContentView:(NSView *)aView{
    [super setContentView:aView];
    
    [self _recalculateFrameForTitleBarView];
    [self _layoutTrafficLightsAndContent];
    [self display];
}

- (NSRect)contentRectForFrameRect:(NSRect)frameRect{
    NSRect rect = [super contentRectForFrameRect:frameRect];
    
    float adjustment = self.titleBarHeight - 22;
    rect.size.height -= adjustment;
    
    return rect;
}

- (NSRect)frameRectForContentRect:(NSRect)contentRect
{
    NSRect rect = [super frameRectForContentRect:contentRect];
    
    float adjustment = self.titleBarHeight - 22;
    rect.size.height += adjustment;
    
    return rect;
}

- (void)setFrame:(NSRect)frameRect display:(BOOL)flag
{
    if(!NSEqualSizes(frameRect.size, [self frame].size))
    {
        float topPos = [self frame].origin.y + [self frame].size.height;
        frameRect.origin.y = topPos-frameRect.size.height;
    }
    
    [super setFrame:frameRect display:flag];
}

#pragma mark -
#pragma mark Accessors
- (void)setTitleBarView:(NSView *)newTitleBarView{
    if ((_titleBarView != newTitleBarView) && newTitleBarView) {
        [_titleBarView removeFromSuperview];
        [_titleBarView release];
        _titleBarView = [newTitleBarView retain];
        
        // Configure the view properties and add it as a subview of the theme frame
        NSView *contentView = [self contentView];
        NSView *themeFrame = [contentView superview];
        NSView *firstSubview = [[themeFrame subviews] objectAtIndex:0];
        [_titleBarView setAutoresizingMask:(NSViewMinYMargin | NSViewWidthSizable)];
        [self _recalculateFrameForTitleBarView];
        [themeFrame addSubview:_titleBarView positioned:NSWindowBelow relativeTo:firstSubview];
        [self _layoutTrafficLightsAndContent];
        [self display];
    }
}

- (NSView*)titleBarView
{
    return _titleBarView;
}

- (void)setTitleBarHeight:(float)newTitleBarHeight
{
    float minTitleHeight = [self _minimumTitlebarHeight];
    if (newTitleBarHeight < minTitleHeight) {
        newTitleBarHeight = minTitleHeight;
    }
    _titleBarHeight = newTitleBarHeight;
    [self _recalculateFrameForTitleBarView];
    [self _layoutTrafficLightsAndContent];
    [self display];
}

- (float)titleBarHeight
{
    return _titleBarHeight;
}

- (void)setTrafficLightAlignment:(int)trafficLightAlignment{
    _trafficLightAlignment = trafficLightAlignment;
    
    [self _recalculateFrameForTitleBarView];
    [self _layoutTrafficLightsAndContent];
    [self display];
}

- (int)trafficLightAlignment{
    return _trafficLightAlignment;
}

- (void)setTitleBarString:(NSString *)titleBarString{
    [titleBarString retain];
    [_titleBarString release];
    
    _titleBarString = [titleBarString copy];
    [titleBarString release];
    
    [self display];
}
- (NSString*)titleBarString{
    return _titleBarString;
}
#pragma mark -
#pragma mark Private

- (void)_doInitialWindowSetup
{
    // Calculate titlebar height
    _titleBarHeight = [self _minimumTitlebarHeight];
    [self setMovableByWindowBackground:NO];
    /** -----------------------------------------
     - The window automatically does layout every time its moved or resized, which means that the traffic lights and content view get reset at the original positions, so we need to put them back in place
     - NSWindow is hardcoded to redraw the traffic lights in a specific rect, so when they are moved down, only part of the buttons get redrawn, causing graphical artifacts. Therefore, the window must be force redrawn every time it becomes key/resigns key
     ----------------------------------------- **/
    NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
    [nc addObserver:self selector:@selector(_layoutTrafficLightsAndContent) name:NSWindowDidResizeNotification object:self];
    [nc addObserver:self selector:@selector(_layoutTrafficLightsAndContent) name:NSWindowDidMoveNotification object:self];
    
    [nc addObserver:self selector:@selector(display) name:NSWindowDidResignKeyNotification object:self];
    [nc addObserver:self selector:@selector(display) name:NSWindowDidBecomeKeyNotification object:self];
    [nc addObserver:self selector:@selector(_setupTrafficLightsTrackingArea) name:NSWindowDidBecomeKeyNotification object:self];
    
    [self _createTitlebarView];
    [self _layoutTrafficLightsAndContent];
    [self _setupTrafficLightsTrackingArea];
}

- (void)_layoutTrafficLightsAndContent
{
    NSView *contentView = [self contentView];
    NSButton *close = [self standardWindowButton:NSWindowCloseButton];
    NSButton *minimize = [self standardWindowButton:NSWindowMiniaturizeButton];
    NSButton *zoom = [self standardWindowButton:NSWindowZoomButton];
    
    // Set the frame of the window buttons
    NSRect closeFrame = [close frame];
    NSRect minimizeFrame = [minimize frame];
    NSRect zoomFrame = [zoom frame];
    
    
    float buttonOrigin;
    
    switch (_trafficLightAlignment) {
        case 0:
            buttonOrigin = [close frame].origin.y;
            break;
            
        case 1:
            buttonOrigin = floor(NSMidY([_titleBarView frame]) - (closeFrame.size.height / 2.0));
            break;
            
        case 2:
            buttonOrigin = floor(NSMinY([_titleBarView frame]) + (closeFrame.size.height / 2.0));
            break;
            
        default:
            buttonOrigin = floor(NSMidY([_titleBarView frame]) - (closeFrame.size.height / 2.0));
            break;
    }
    
    
    closeFrame.origin.y = buttonOrigin;
    minimizeFrame.origin.y = buttonOrigin;
    zoomFrame.origin.y = buttonOrigin;
    [close setFrame:closeFrame];
    [minimize setFrame:minimizeFrame];
    [zoom setFrame:zoomFrame];
    
    // Reposition the content view
    NSRect windowFrame = [self frame];
    NSRect newFrame = [contentView frame];
    float titleHeight = windowFrame.size.height - newFrame.size.height;
    float extraHeight = _titleBarHeight - titleHeight;
    newFrame.size.height -= extraHeight;
    [contentView setFrame:newFrame];
}

- (void)_createTitlebarView
{
    // Create the title bar view
    self.titleBarView = [[[INTitlebarView alloc] initWithFrame:NSZeroRect] autorelease];
}

// Solution for tracking area issue thanks to @Perspx (Alex Rozanski) <https://gist.github.com/972958>
- (void)_setupTrafficLightsTrackingArea
{
    NSView *themeFrame = [[self contentView] superview];
    NSArray *trackingAreas = [themeFrame trackingAreas];
    if (![trackingAreas count]) { return; } // safety in case there are no tracking areas
    NSTrackingArea *trackingArea = [trackingAreas objectAtIndex:0];
    NSRect closeFrame = [[self standardWindowButton:NSWindowCloseButton] frame];
    // Alter the tracking area rectangle    
    NSRect trackingRect = [trackingArea rect];
    trackingRect.origin.y = NSMinY(closeFrame);
    // Create the new tracking area and set it on the window's theme frame view
    NSTrackingArea *newTrackingArea = [[NSTrackingArea alloc] initWithRect:trackingRect options:[trackingArea options] owner:[trackingArea owner] userInfo:[NSDictionary dictionary]];
    [themeFrame removeTrackingArea:trackingArea];
    [themeFrame addTrackingArea:newTrackingArea];
    [newTrackingArea release];
}

- (void)_recalculateFrameForTitleBarView
{
    NSView *contentView = [self contentView];
    NSView *themeFrame = [contentView superview];
    NSRect themeFrameRect = [themeFrame frame];
    NSRect titleFrame = NSMakeRect(0.0, NSMaxY(themeFrameRect) - _titleBarHeight, themeFrameRect.size.width, _titleBarHeight);
    [_titleBarView setFrame:titleFrame];
}

- (float)_minimumTitlebarHeight
{
    static float minTitleHeight = 0.0;
    if (!minTitleHeight) {
        NSRect frameRect = [self frame];
        NSRect contentRect = [self contentRectForFrameRect:frameRect];
        minTitleHeight = (frameRect.size.height - contentRect.size.height);
    }
    return minTitleHeight;
}
@end
