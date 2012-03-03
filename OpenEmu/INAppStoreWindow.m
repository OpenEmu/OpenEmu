//
//  INAppStoreWindow.m
//
//  Copyright 2011 Indragie Karunaratne. All rights reserved.
//
//  Licensed under the BSD License <http://www.opensource.org/licenses/bsd-license>
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
//  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
//  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
//  SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
//  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
//  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
//  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
//  THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#import "INAppStoreWindow.h"

#define IN_RUNNING_LION (floor(NSAppKitVersionNumber) > NSAppKitVersionNumber10_6)
#define IN_COMPILING_LION __MAC_OS_X_VERSION_MAX_ALLOWED >= 1070
#define IN_COMPILING_MOUNTAIN __MAC_OS_X_VERSION_MAX_ALLOWED >= 1080

/** -----------------------------------------
 - There are 2 sets of colors, one for an active (key) state and one for an inactivate state
 - Each set contains 3 colors. 2 colors for the start and end of the title gradient, and another color to draw the separator line on the bottom
 - These colors are meant to mimic the color of the default titlebar (taken from OS X 10.6), but are subject
 to change at any time
 ----------------------------------------- **/

#define IN_COLOR_MAIN_START [NSColor colorWithDeviceWhite:0.659 alpha:1.0]
#define IN_COLOR_MAIN_END [NSColor colorWithDeviceWhite:0.812 alpha:1.0]
#define IN_COLOR_MAIN_BOTTOM [NSColor colorWithDeviceWhite:0.318 alpha:1.0]

#define IN_COLOR_NOTMAIN_START [NSColor colorWithDeviceWhite:0.851 alpha:1.0]
#define IN_COLOR_NOTMAIN_END [NSColor colorWithDeviceWhite:0.929 alpha:1.0]
#define IN_COLOR_NOTMAIN_BOTTOM [NSColor colorWithDeviceWhite:0.600 alpha:1.0]

/** Lion */

#define IN_COLOR_MAIN_START_L [NSColor colorWithDeviceWhite:0.66 alpha:1.0]
#define IN_COLOR_MAIN_END_L [NSColor colorWithDeviceWhite:0.9 alpha:1.0]
#define IN_COLOR_MAIN_BOTTOM_L [NSColor colorWithDeviceWhite:0.408 alpha:1.0]

#define IN_COLOR_NOTMAIN_START_L [NSColor colorWithDeviceWhite:0.878 alpha:1.0]
#define IN_COLOR_NOTMAIN_END_L [NSColor colorWithDeviceWhite:0.976 alpha:1.0]
#define IN_COLOR_NOTMAIN_BOTTOM_L [NSColor colorWithDeviceWhite:0.655 alpha:1.0]

/** Corner clipping radius **/
#if IN_COMPILING_MOUNTAIN
const CGFloat INCornerClipRadius = 6.0;
#else
const CGFloat INCornerClipRadius = 4.0;
#endif

const CGFloat INButtonTopOffset = 3.0;

NS_INLINE CGFloat INMidHeight(NSRect aRect){
    return (aRect.size.height * (CGFloat)0.5);
}

static CGImageRef createNoiseImageRef(NSUInteger width, NSUInteger height, CGFloat factor)
{
    NSUInteger size = width*height;
    char *rgba = (char *)malloc(size); srand(124);
    for(NSUInteger i=0; i < size; ++i){rgba[i] = rand()%256*factor;}
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceGray();
    CGContextRef bitmapContext = 
    CGBitmapContextCreate(rgba, width, height, 8, width, colorSpace, kCGImageAlphaNone);
    CFRelease(colorSpace);
    free(rgba);
    CGImageRef image = CGBitmapContextCreateImage(bitmapContext);
    CFRelease(bitmapContext);
    return image;
}

@interface INAppStoreWindow ()
@property (INAppStoreWindowCopy) NSString *windowMenuTitle;
- (void)_doInitialWindowSetup;
- (void)_createTitlebarView;
- (void)_setupTrafficLightsTrackingArea;
- (void)_recalculateFrameForTitleBarView;
- (void)_layoutTrafficLightsAndContent;
- (CGFloat)_minimumTitlebarHeight;
- (void)_displayWindowAndTitlebar;
- (void)_hideTitleBarView:(BOOL)hidden;
- (CGFloat)_defaultTrafficLightLeftMargin;
- (CGFloat)_trafficLightSeparation;
@end

@implementation INTitlebarView

- (void)drawRect:(NSRect)dirtyRect
{
    BOOL drawsAsMainWindow = ([[self window] isMainWindow] && [[NSApplication sharedApplication] isActive]);
    NSRect drawingRect = [self bounds];
    drawingRect.size.height -= 1.0; // Decrease the height by 1.0px to show the highlight line at the top
    NSColor *startColor = nil;
    NSColor *endColor = nil;
    if (IN_RUNNING_LION) {
        startColor = drawsAsMainWindow ? IN_COLOR_MAIN_START_L : IN_COLOR_NOTMAIN_START_L;
        endColor = drawsAsMainWindow ? IN_COLOR_MAIN_END_L : IN_COLOR_NOTMAIN_END_L;
    } else {
        startColor = drawsAsMainWindow ? IN_COLOR_MAIN_START : IN_COLOR_NOTMAIN_START;
        endColor = drawsAsMainWindow ? IN_COLOR_MAIN_END : IN_COLOR_NOTMAIN_END;
    }
    [[self clippingPathWithRect:drawingRect cornerRadius:INCornerClipRadius] addClip];
    NSGradient *gradient = [[NSGradient alloc] initWithStartingColor:startColor endingColor:endColor];
    [gradient drawInRect:drawingRect angle:90];
    #if !__has_feature(objc_arc)
    [gradient release];
    #endif
    if (IN_RUNNING_LION && drawsAsMainWindow) {
        static CGImageRef noisePattern = nil;
        if (noisePattern == nil) noisePattern = createNoiseImageRef(128, 128, 0.015);
        [NSGraphicsContext saveGraphicsState];
        [[NSGraphicsContext currentContext] setCompositingOperation:NSCompositePlusLighter];
        CGRect noisePatternRect = CGRectZero;
        noisePatternRect.size = CGSizeMake(CGImageGetWidth(noisePattern), CGImageGetHeight(noisePattern));        
        CGContextRef context = [[NSGraphicsContext currentContext] graphicsPort];
        CGContextDrawTiledImage(context, noisePatternRect, noisePattern);
        [NSGraphicsContext restoreGraphicsState];
    }
    
    if ([(INAppStoreWindow *)[self window] showsBaselineSeparator]) {
        NSColor *bottomColor = nil;
        if (IN_RUNNING_LION) {
          bottomColor = drawsAsMainWindow ? IN_COLOR_MAIN_BOTTOM_L : IN_COLOR_NOTMAIN_BOTTOM_L;
        } else {
          bottomColor = drawsAsMainWindow ? IN_COLOR_MAIN_BOTTOM : IN_COLOR_NOTMAIN_BOTTOM;
        }
        NSRect bottomRect = NSMakeRect(0.0, NSMinY(drawingRect), NSWidth(drawingRect), 1.0);
        [bottomColor set];
        NSRectFill(bottomRect);
        
        if (IN_RUNNING_LION) {
          bottomRect.origin.y += 1.0;
          [[NSColor colorWithDeviceWhite:1.0 alpha:0.12] setFill];
          [[NSBezierPath bezierPathWithRect:bottomRect] fill];
        }
    }
}

// Uses code from NSBezierPath+PXRoundedRectangleAdditions by Andy Matuschak
// <http://code.andymatuschak.org/pixen/trunk/NSBezierPath+PXRoundedRectangleAdditions.m>

- (NSBezierPath*)clippingPathWithRect:(NSRect)aRect cornerRadius:(CGFloat)radius
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

- (void)mouseUp:(NSEvent *)theEvent 
{
    if ([theEvent clickCount] == 2) {
        // Get settings from "System Preferences" >  "Appearance" > "Double-click on windows title bar to minimize"
        NSString *const MDAppleMiniaturizeOnDoubleClickKey = @"AppleMiniaturizeOnDoubleClick";
        NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
        [userDefaults addSuiteNamed:NSGlobalDomain];
        BOOL shouldMiniaturize = [[userDefaults objectForKey:MDAppleMiniaturizeOnDoubleClickKey] boolValue];
        if (shouldMiniaturize) {
            [[self window] miniaturize:self];
        }
    }
}

@end

@implementation INAppStoreWindow
@synthesize windowMenuTitle = _windowMenuTitle;
@synthesize centerFullScreenButton = _centerFullScreenButton;
@synthesize centerTrafficLightButtons = _centerTrafficLightButtons;
@synthesize hideTitleBarInFullScreen = _hideTitleBarInFullScreen;
#pragma mark -
#pragma mark Initialization

- (id)initWithContentRect:(NSRect)contentRect styleMask:(NSUInteger)aStyle backing:(NSBackingStoreType)bufferingType defer:(BOOL)flag
{
    if ((self = [super initWithContentRect:contentRect styleMask:aStyle backing:bufferingType defer:flag])) {
        [self _doInitialWindowSetup];
    }
    return self;
}

- (id)initWithContentRect:(NSRect)contentRect styleMask:(NSUInteger)aStyle backing:(NSBackingStoreType)bufferingType defer:(BOOL)flag screen:(NSScreen *)screen
{
    if ((self = [super initWithContentRect:contentRect styleMask:aStyle backing:bufferingType defer:flag screen:screen])) {
        [self _doInitialWindowSetup];
    }
    return self;
}

#pragma mark -
#pragma mark Memory Management

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    #if !__has_feature(objc_arc)
    [_titleBarView release];
	[_windowMenuTitle release];
    [super dealloc];    
    #endif
}

#pragma mark -
#pragma mark NSWindow Overrides

// Disable window titles

- (NSString*)title
{
    return @"";
}

- (void)setTitle:(NSString*)title
{
	self.windowMenuTitle = title;
	if ( ![self isExcludedFromWindowsMenu] )
		[NSApp changeWindowsItem:self title:self.windowMenuTitle filename:NO];
}

- (void)setRepresentedURL:(NSURL *)url
{
	// do nothing, don't want to show document icon in menu bar
}

- (void)makeKeyAndOrderFront:(id)sender
{
	[super makeKeyAndOrderFront:sender];
	if (![self isExcludedFromWindowsMenu]) {
		[NSApp addWindowsItem:self title:self.windowMenuTitle filename:NO];	
    }
}

- (void)becomeKeyWindow
{
    [super becomeKeyWindow];
    [_titleBarView setNeedsDisplay:YES];
}

- (void)resignKeyWindow
{
    [super resignKeyWindow];
    [_titleBarView setNeedsDisplay:YES];
}

- (void)orderFront:(id)sender
{
	[super orderFront:sender];
	if (![self isExcludedFromWindowsMenu]) {
		[NSApp addWindowsItem:self title:self.windowMenuTitle filename:NO];
    }
}

- (void)orderOut:(id)sender
{
	[super orderOut:sender];
	[NSApp removeWindowsItem:self];
}

#pragma mark -
#pragma mark Accessors

- (void)setTitleBarView:(NSView *)newTitleBarView
{
    if ((_titleBarView != newTitleBarView) && newTitleBarView) {
        [_titleBarView removeFromSuperview];
        #if __has_feature(objc_arc)
        _titleBarView = newTitleBarView;
        #else
        [_titleBarView release];
        _titleBarView = [newTitleBarView retain];
        #endif
        // Configure the view properties and add it as a subview of the theme frame
        NSView *themeFrame = [[self contentView] superview];
        NSView *firstSubview = [[themeFrame subviews] objectAtIndex:0];
        [_titleBarView setAutoresizingMask:(NSViewMinYMargin | NSViewWidthSizable)];
        [self _recalculateFrameForTitleBarView];
        [themeFrame addSubview:_titleBarView positioned:NSWindowBelow relativeTo:firstSubview];
        [self _layoutTrafficLightsAndContent];
        [self _displayWindowAndTitlebar];
    }
}

- (NSView *)titleBarView
{
    return _titleBarView;
}

- (void)setTitleBarHeight:(CGFloat)newTitleBarHeight 
{
	if (_titleBarHeight != newTitleBarHeight) {
        _cachedTitleBarHeight = newTitleBarHeight;
		_titleBarHeight = newTitleBarHeight;
		[self _recalculateFrameForTitleBarView];
		[self _layoutTrafficLightsAndContent];
		[self _displayWindowAndTitlebar];
	}
}

- (CGFloat)titleBarHeight
{
    return _titleBarHeight;
}

- (void)setShowsBaselineSeparator:(BOOL)showsBaselineSeparator
{
    if (_showsBaselineSeparator != showsBaselineSeparator) {
        _showsBaselineSeparator = showsBaselineSeparator;
        [self.titleBarView setNeedsDisplay:YES];
    }
}

- (BOOL)showsBaselineSeparator
{
    return _showsBaselineSeparator;
}

- (void)setTrafficLightButtonsLeftMargin:(CGFloat)newTrafficLightButtonsLeftMargin
{
	if (_trafficLightButtonsLeftMargin != newTrafficLightButtonsLeftMargin) {
		_trafficLightButtonsLeftMargin = newTrafficLightButtonsLeftMargin;
		[self _recalculateFrameForTitleBarView];
		[self _layoutTrafficLightsAndContent];
		[self _displayWindowAndTitlebar];
        [self _setupTrafficLightsTrackingArea];
	}
}

- (CGFloat)trafficLightButtonsLeftMargin
{
    return _trafficLightButtonsLeftMargin;
}


- (void)setFullScreenButtonRightMargin:(CGFloat)newFullScreenButtonRightMargin
{
	if (_fullScreenButtonRightMargin != newFullScreenButtonRightMargin) {
        _setFullScreenButtonRightMargin = YES;
		_fullScreenButtonRightMargin = newFullScreenButtonRightMargin;
		[self _recalculateFrameForTitleBarView];
		[self _layoutTrafficLightsAndContent];
		[self _displayWindowAndTitlebar];
	}
}

- (CGFloat)fullScreenButtonRightMargin
{
    return _fullScreenButtonRightMargin;
}

- (void)setCenterFullScreenButton:(BOOL)centerFullScreenButton{
    if( _centerFullScreenButton != centerFullScreenButton ) {
        _centerFullScreenButton = centerFullScreenButton;
        [self _layoutTrafficLightsAndContent];
    }
}

- (void)setCenterTrafficLightButtons:(BOOL)centerTrafficLightButtons
{
    if ( _centerTrafficLightButtons != centerTrafficLightButtons ) {
        _centerTrafficLightButtons = centerTrafficLightButtons;
        [self _layoutTrafficLightsAndContent];
        [self _setupTrafficLightsTrackingArea];
    }
}

#pragma mark -
#pragma mark Private

- (void)_doInitialWindowSetup
{
    _showsBaselineSeparator = YES;
    _centerTrafficLightButtons = YES;
    _titleBarHeight = [self _minimumTitlebarHeight];
	_trafficLightButtonsLeftMargin = [self _defaultTrafficLightLeftMargin];
    [self setMovableByWindowBackground:YES];
    
    /** -----------------------------------------
     - The window automatically does layout every time its moved or resized, which means that the traffic lights and content view get reset at the original positions, so we need to put them back in place
     - NSWindow is hardcoded to redraw the traffic lights in a specific rect, so when they are moved down, only part of the buttons get redrawn, causing graphical artifacts. Therefore, the window must be force redrawn every time it becomes key/resigns key
     ----------------------------------------- **/
    NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
    [nc addObserver:self selector:@selector(_layoutTrafficLightsAndContent) name:NSWindowDidResizeNotification object:self];
    [nc addObserver:self selector:@selector(_layoutTrafficLightsAndContent) name:NSWindowDidMoveNotification object:self];
    [nc addObserver:self selector:@selector(_displayWindowAndTitlebar) name:NSWindowDidResignKeyNotification object:self];
    [nc addObserver:self selector:@selector(_displayWindowAndTitlebar) name:NSWindowDidBecomeKeyNotification object:self];
    [nc addObserver:self selector:@selector(_setupTrafficLightsTrackingArea) name:NSWindowDidBecomeKeyNotification object:self];
    [nc addObserver:self selector:@selector(_displayWindowAndTitlebar) name:NSApplicationDidBecomeActiveNotification object:nil];
    [nc addObserver:self selector:@selector(_displayWindowAndTitlebar) name:NSApplicationDidResignActiveNotification object:nil];
    #if IN_COMPILING_LION
    if (IN_RUNNING_LION) {
        [nc addObserver:self selector:@selector(_setupTrafficLightsTrackingArea) name:NSWindowDidExitFullScreenNotification object:nil];
        [nc addObserver:self selector:@selector(windowWillEnterFullScreen:) name:NSWindowWillEnterFullScreenNotification object:nil];
        [nc addObserver:self selector:@selector(windowWillExitFullScreen:) name:NSWindowWillExitFullScreenNotification object:nil];
    }
    #endif
    [self _createTitlebarView];
    [self _layoutTrafficLightsAndContent];
    [self _setupTrafficLightsTrackingArea];
}

- (void)_layoutTrafficLightsAndContent
{
    NSButton *close = [self standardWindowButton:NSWindowCloseButton];
    NSButton *minimize = [self standardWindowButton:NSWindowMiniaturizeButton];
    NSButton *zoom = [self standardWindowButton:NSWindowZoomButton];
    
    // Set the frame of the window buttons
    NSRect closeFrame = [close frame];
    NSRect minimizeFrame = [minimize frame];
    NSRect zoomFrame = [zoom frame];
    NSRect titleBarFrame = [_titleBarView frame];
    CGFloat buttonOrigin = 0.0;
    if ( self.centerTrafficLightButtons ) {
        buttonOrigin = round(NSMidY(titleBarFrame) - INMidHeight(closeFrame));
    } else {
        buttonOrigin = NSMaxY(titleBarFrame) - NSHeight(closeFrame) - INButtonTopOffset;
    }
    closeFrame.origin.y = buttonOrigin;
    minimizeFrame.origin.y = buttonOrigin;
    zoomFrame.origin.y = buttonOrigin;
	closeFrame.origin.x = _trafficLightButtonsLeftMargin;
    minimizeFrame.origin.x = _trafficLightButtonsLeftMargin + [self _trafficLightSeparation];
    zoomFrame.origin.x = _trafficLightButtonsLeftMargin + [self _trafficLightSeparation] * 2;
    [close setFrame:closeFrame];
    [minimize setFrame:minimizeFrame];
    [zoom setFrame:zoomFrame];
    
    #if IN_COMPILING_LION
    // Set the frame of the FullScreen button in Lion if available
    if ( IN_RUNNING_LION ) {
        NSButton *fullScreen = [self standardWindowButton:NSWindowFullScreenButton];        
        if( fullScreen ) {
            NSRect fullScreenFrame = [fullScreen frame];
            if ( !_setFullScreenButtonRightMargin ) {
                self.fullScreenButtonRightMargin = NSWidth([_titleBarView frame]) - NSMaxX(fullScreen.frame);
            }
			fullScreenFrame.origin.x = NSWidth(titleBarFrame) - NSWidth(fullScreenFrame) - _fullScreenButtonRightMargin;
            if( self.centerFullScreenButton ) {
                fullScreenFrame.origin.y = round(NSMidY(titleBarFrame) - INMidHeight(fullScreenFrame));
            } else {
                fullScreenFrame.origin.y = NSMaxY(titleBarFrame) - NSHeight(fullScreenFrame) - INButtonTopOffset;
            }
            [fullScreen setFrame:fullScreenFrame];
        }
    }
    #endif
    
    // Reposition the content view
    NSView *contentView = [self contentView];    
    NSRect windowFrame = [self frame];
    NSRect newFrame = [contentView frame];
    CGFloat titleHeight = NSHeight(windowFrame) - NSHeight(newFrame);
    CGFloat extraHeight = _titleBarHeight - titleHeight;
    newFrame.size.height -= extraHeight;
    [contentView setFrame:newFrame];
    [contentView setNeedsDisplay:YES];
}

- (void)windowWillEnterFullScreen:(NSNotification *)notification 
{
    if (_hideTitleBarInFullScreen) {
        // Recalculate the views when entering from fullscreen
        _titleBarHeight = 0.0f;
		[self _recalculateFrameForTitleBarView];
		[self _layoutTrafficLightsAndContent];
		[self _displayWindowAndTitlebar];
        
        [self _hideTitleBarView:YES];
    }
}

- (void)windowWillExitFullScreen:(NSNotification *)notification 
{
    if (_hideTitleBarInFullScreen) {
        _titleBarHeight = _cachedTitleBarHeight;
		[self _recalculateFrameForTitleBarView];
		[self _layoutTrafficLightsAndContent];
		[self _displayWindowAndTitlebar];
        
        [self _hideTitleBarView:NO];
    }
}

- (void)_createTitlebarView
{
    // Create the title bar view
    #if __has_feature(objc_arc)
    self.titleBarView = [[INTitlebarView alloc] initWithFrame:NSZeroRect];
    #else
    self.titleBarView = [[[INTitlebarView alloc] initWithFrame:NSZeroRect] autorelease];
    #endif
}

- (void)_hideTitleBarView:(BOOL)hidden 
{
    [self.titleBarView setHidden:hidden];
}

// Solution for tracking area issue thanks to @Perspx (Alex Rozanski) <https://gist.github.com/972958>
- (void)_setupTrafficLightsTrackingArea
{
    [[[self contentView] superview] viewWillStartLiveResize];
    [[[self contentView] superview] viewDidEndLiveResize];
}

- (void)_recalculateFrameForTitleBarView
{
    NSView *themeFrame = [[self contentView] superview];
    NSRect themeFrameRect = [themeFrame frame];
    NSRect titleFrame = NSMakeRect(0.0, NSMaxY(themeFrameRect) - _titleBarHeight, NSWidth(themeFrameRect), _titleBarHeight);
    [_titleBarView setFrame:titleFrame];
}

- (CGFloat)_minimumTitlebarHeight
{
    static CGFloat minTitleHeight = 0.0;
    if ( !minTitleHeight ) {
        NSRect frameRect = [self frame];
        NSRect contentRect = [self contentRectForFrameRect:frameRect];
        minTitleHeight = NSHeight(frameRect) - NSHeight(contentRect);
    }
    return minTitleHeight;
}

- (CGFloat)_defaultTrafficLightLeftMargin
{
    static CGFloat trafficLightLeftMargin = 0.0;
    if ( !trafficLightLeftMargin ) {
        NSButton *close = [self standardWindowButton:NSWindowCloseButton];
        trafficLightLeftMargin = NSMinX(close.frame);
    }
    return trafficLightLeftMargin;
}

- (CGFloat)_trafficLightSeparation
{
    static CGFloat trafficLightSeparation = 0.0;
    if ( !trafficLightSeparation ) {
        NSButton *close = [self standardWindowButton:NSWindowCloseButton];
        NSButton *minimize = [self standardWindowButton:NSWindowMiniaturizeButton];
        trafficLightSeparation = NSMinX(minimize.frame) - NSMinX(close.frame);
    }
    return trafficLightSeparation;    
}

- (void)_displayWindowAndTitlebar
{
    // Redraw the window and titlebar
    [_titleBarView setNeedsDisplay:YES];
}
@end
