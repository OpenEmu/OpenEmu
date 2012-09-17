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
const CGFloat INCornerClipRadius = 4.0;
const CGFloat INButtonTopOffset = 3.0;

NS_INLINE CGFloat INMidHeight(NSRect aRect){
    return (aRect.size.height * (CGFloat)0.5);
}

static inline CGPathRef createClippingPathWithRectAndRadius(NSRect rect, CGFloat radius)
{
    CGMutablePathRef path = CGPathCreateMutable();
    CGPathMoveToPoint(path, NULL, NSMinX(rect), NSMinY(rect));
    CGPathAddLineToPoint(path, NULL, NSMinX(rect), NSMaxY(rect)-radius);
    CGPathAddArcToPoint(path, NULL, NSMinX(rect), NSMaxY(rect), NSMinX(rect)+radius, NSMaxY(rect), radius);
    CGPathAddLineToPoint(path, NULL, NSMaxX(rect)-radius, NSMaxY(rect));
    CGPathAddArcToPoint(path, NULL,  NSMaxX(rect), NSMaxY(rect), NSMaxX(rect), NSMaxY(rect)-radius, radius);
    CGPathAddLineToPoint(path, NULL, NSMaxX(rect), NSMinY(rect));
    CGPathCloseSubpath(path);
    return path;
}

static inline CGGradientRef createGradientWithColors(NSColor *startingColor, NSColor *endingColor)
{
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceGray();
    CGFloat startingComponents[2];
    [startingColor getWhite:&startingComponents[0] alpha:&startingComponents[1]];
    
    CGFloat endingComponents[2];
    [endingColor getWhite:&endingComponents[0] alpha:&endingComponents[1]];
    
    CGFloat compontents[4] = {
        startingComponents[0],
        startingComponents[1],
        endingComponents[0],
        endingComponents[1],
    };
    
    CGFloat locations[2] = {
        0.0f,
        1.0f,
    };
    
    CGGradientRef gradient = 
    CGGradientCreateWithColorComponents(colorSpace, 
                                        (const CGFloat *)&compontents, 
                                        (const CGFloat *)&locations, 2);
    CGColorSpaceRelease(colorSpace);
    return gradient;
}

@interface INAppStoreWindowDelegateProxy : NSProxy <NSWindowDelegate>
@property (nonatomic, assign) id<NSWindowDelegate> secondaryDelegate;
@end

@implementation INAppStoreWindowDelegateProxy
@synthesize secondaryDelegate = _secondaryDelegate;

- (NSMethodSignature *)methodSignatureForSelector:(SEL)selector
{
    NSMethodSignature *signature = [[self.secondaryDelegate class] instanceMethodSignatureForSelector:selector];
    NSAssert(signature != nil, @"The method signature(%@) should not be nil becuase of the respondsToSelector: check", NSStringFromSelector(selector));
    return signature;
}

- (BOOL)respondsToSelector:(SEL)aSelector
{
    if ([self.secondaryDelegate respondsToSelector:aSelector]) {
        return YES;
    } else if ([NSStringFromSelector(aSelector) isEqualToString:@"window:willPositionSheet:usingRect:"]) {
        //TODO: not sure if there is a better way to do this check
        return YES;
    }
    return NO;
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
    if ([self.secondaryDelegate respondsToSelector:[anInvocation selector]]) {
        [anInvocation invokeWithTarget:self.secondaryDelegate];
    }
}

- (NSRect)window:(INAppStoreWindow *)window willPositionSheet:(NSWindow *)sheet usingRect:(NSRect)rect
{
    rect.origin.y = NSHeight(window.frame)-window.titleBarHeight;
    return rect;
}
@end

@interface INAppStoreWindow ()
- (void)_doInitialWindowSetup;
- (void)_createTitlebarView;
- (void)_setupTrafficLightsTrackingArea;
- (void)_recalculateFrameForTitleBarContainer;
- (void)_repositionContentView;
- (void)_layoutTrafficLightsAndContent;
- (CGFloat)_minimumTitlebarHeight;
- (void)_displayWindowAndTitlebar;
- (void)_hideTitleBarView:(BOOL)hidden;
- (CGFloat)_defaultTrafficLightLeftMargin;
- (CGFloat)_trafficLightSeparation;
@end

@implementation INTitlebarView

- (void)drawNoiseWithOpacity:(CGFloat)opacity
{
    static CGImageRef noiseImageRef = nil;
    static dispatch_once_t oncePredicate;
    dispatch_once(&oncePredicate, ^{
        NSUInteger width = 124, height = width;
        NSUInteger size = width*height;
        char *rgba = (char *)malloc(size); srand(120);
        for(NSUInteger i=0; i < size; ++i){rgba[i] = rand()%256;}
        CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceGray();
        CGContextRef bitmapContext =
        CGBitmapContextCreate(rgba, width, height, 8, width, colorSpace, kCGImageAlphaNone);
        CFRelease(colorSpace);
        noiseImageRef = CGBitmapContextCreateImage(bitmapContext);
        CFRelease(bitmapContext);
        free(rgba);
    });

    CGContextRef context = [[NSGraphicsContext currentContext] graphicsPort];
    CGContextSaveGState(context);
    CGContextSetAlpha(context, opacity);
    CGContextSetBlendMode(context, kCGBlendModeScreen);

    if ( [[self window] respondsToSelector:@selector(backingScaleFactor)] ) {
        CGFloat scaleFactor = [[self window] backingScaleFactor];
        CGContextScaleCTM(context, 1/scaleFactor, 1/scaleFactor);
    }

    CGRect imageRect = (CGRect){CGPointZero, CGImageGetWidth(noiseImageRef), CGImageGetHeight(noiseImageRef)};
    CGContextDrawTiledImage(context, imageRect, noiseImageRef);
    CGContextRestoreGState(context);
}

- (void)drawRect:(NSRect)dirtyRect
{
    INAppStoreWindow *window = (INAppStoreWindow *)[self window];
    BOOL drawsAsMainWindow = ([window isMainWindow] && [[NSApplication sharedApplication] isActive]);
    
    NSRect drawingRect = [self bounds];
    if ( window.titleBarDrawingBlock ) {
        CGPathRef clippingPath = createClippingPathWithRectAndRadius(drawingRect, INCornerClipRadius);
        window.titleBarDrawingBlock(drawsAsMainWindow, NSRectToCGRect(drawingRect), clippingPath);
        CGPathRelease(clippingPath);
    } else {
        CGContextRef context = [[NSGraphicsContext currentContext] graphicsPort];        
        
        NSColor *startColor = nil;
        NSColor *endColor = nil;
        if (IN_RUNNING_LION) {
            startColor = drawsAsMainWindow ? IN_COLOR_MAIN_START_L : IN_COLOR_NOTMAIN_START_L;
            endColor = drawsAsMainWindow ? IN_COLOR_MAIN_END_L : IN_COLOR_NOTMAIN_END_L;
        } else {
            startColor = drawsAsMainWindow ? IN_COLOR_MAIN_START : IN_COLOR_NOTMAIN_START;
            endColor = drawsAsMainWindow ? IN_COLOR_MAIN_END : IN_COLOR_NOTMAIN_END;
        }
        
        NSRect clippingRect = drawingRect;
        #if IN_COMPILING_LION
        if((([window styleMask] & NSFullScreenWindowMask) == NSFullScreenWindowMask)){
            [[NSColor blackColor] setFill];
            [[NSBezierPath bezierPathWithRect:self.bounds] fill];
        }
        #endif
        clippingRect.size.height -= 1;        
        CGPathRef clippingPath = createClippingPathWithRectAndRadius(clippingRect, INCornerClipRadius);
        CGContextAddPath(context, clippingPath);
        CGContextClip(context);
        CGPathRelease(clippingPath);
        
        CGGradientRef gradient = createGradientWithColors(startColor, endColor);
        CGContextDrawLinearGradient(context, gradient, CGPointMake(NSMidX(drawingRect), NSMinY(drawingRect)), 
                                    CGPointMake(NSMidX(drawingRect), NSMaxY(drawingRect)), 0);
        CGGradientRelease(gradient);

        if ([window showsBaselineSeparator]) {
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
        
        if (IN_RUNNING_LION && drawsAsMainWindow) {
            CGPathRef noiseClippingPath = 
            createClippingPathWithRectAndRadius(NSInsetRect(drawingRect, 1, 1), INCornerClipRadius);
            CGContextAddPath(context, noiseClippingPath);
            CGContextClip(context);
            CGPathRelease(noiseClippingPath);
            
            [self drawNoiseWithOpacity:0.1];
        }        
    }
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

@interface INTitlebarContainer : NSView
@end

@implementation INTitlebarContainer
- (void)mouseDragged:(NSEvent *)theEvent
{
    NSWindow *window = [self window];
    NSPoint where =  [window convertBaseToScreen:[theEvent locationInWindow]];
    NSPoint origin = [window frame].origin;
    while ((theEvent = [NSApp nextEventMatchingMask:NSLeftMouseDownMask | NSLeftMouseDraggedMask | NSLeftMouseUpMask untilDate:[NSDate distantFuture] inMode:NSEventTrackingRunLoopMode dequeue:YES]) && ([theEvent type] != NSLeftMouseUp)) {
        @autoreleasepool {
            NSPoint now = [window convertBaseToScreen:[theEvent locationInWindow]];
            origin.x += now.x - where.x;
            origin.y += now.y - where.y;
            [window setFrameOrigin:origin];
            where = now;
        }
    }
}
@end

@implementation INAppStoreWindow{
    CGFloat _cachedTitleBarHeight;  
    BOOL _setFullScreenButtonRightMargin;
    INAppStoreWindowDelegateProxy *_delegateProxy;
    INTitlebarContainer *_titleBarContainer;
}

@synthesize titleBarView = _titleBarView;
@synthesize titleBarHeight = _titleBarHeight;
@synthesize centerFullScreenButton = _centerFullScreenButton;
@synthesize centerTrafficLightButtons = _centerTrafficLightButtons;
@synthesize hideTitleBarInFullScreen = _hideTitleBarInFullScreen;
@synthesize titleBarDrawingBlock = _titleBarDrawingBlock;
@synthesize showsBaselineSeparator = _showsBaselineSeparator;
@synthesize fullScreenButtonRightMargin = _fullScreenButtonRightMargin;
@synthesize trafficLightButtonsLeftMargin = _trafficLightButtonsLeftMargin;

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
//    [self setDelegate:nil];
    #if !__has_feature(objc_arc)
//    [_delegateProxy release];
    [_titleBarView release];
    [super dealloc];    
    #endif
}

#pragma mark -
#pragma mark NSWindow Overrides

- (void)becomeKeyWindow
{
    [super becomeKeyWindow];
    [self _updateTitlebarView];
    [self _layoutTrafficLightsAndContent];
    [self _setupTrafficLightsTrackingArea];
}

- (void)resignKeyWindow
{
    [super resignKeyWindow];
    [self _updateTitlebarView];
    [self _layoutTrafficLightsAndContent];
}

- (void)becomeMainWindow
{
    [super becomeMainWindow];
    [self _updateTitlebarView];
}

- (void)resignMainWindow
{
    [super resignMainWindow];
    [self _updateTitlebarView];
}

- (void)setContentView:(NSView *)aView
{
    [super setContentView:aView];
    [self _repositionContentView];
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
        [_titleBarView setFrame:[_titleBarContainer bounds]];
        [_titleBarView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
        [_titleBarContainer addSubview:_titleBarView];
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
		_titleBarHeight = _cachedTitleBarHeight;
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

- (void)setDelegate:(id<NSWindowDelegate>)anObject
{
    [_delegateProxy setSecondaryDelegate:anObject];
    [super setDelegate:_delegateProxy];    
}

- (id<NSWindowDelegate>)delegate
{
    return [_delegateProxy secondaryDelegate];
}

#pragma mark -
#pragma mark Private

- (void)_doInitialWindowSetup
{
    _showsBaselineSeparator = YES;
    _centerTrafficLightButtons = YES;
    _titleBarHeight = [self _minimumTitlebarHeight];
	_trafficLightButtonsLeftMargin = [self _defaultTrafficLightLeftMargin];
    _delegateProxy = [INAppStoreWindowDelegateProxy alloc];
    
    /** -----------------------------------------
     - The window automatically does layout every time its moved or resized, which means that the traffic lights and content view get reset at the original positions, so we need to put them back in place
     - NSWindow is hardcoded to redraw the traffic lights in a specific rect, so when they are moved down, only part of the buttons get redrawn, causing graphical artifacts. Therefore, the window must be force redrawn every time it becomes key/resigns key
     ----------------------------------------- **/
    NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
    [nc addObserver:self selector:@selector(_layoutTrafficLightsAndContent) name:NSWindowDidResizeNotification object:self];
    [nc addObserver:self selector:@selector(_layoutTrafficLightsAndContent) name:NSWindowDidMoveNotification object:self];
    [nc addObserver:self selector:@selector(_layoutTrafficLightsAndContent) name:NSWindowDidEndSheetNotification object:self];

    [nc addObserver:self selector:@selector(_updateTitlebarView) name:NSApplicationDidBecomeActiveNotification object:nil];
    [nc addObserver:self selector:@selector(_updateTitlebarView) name:NSApplicationDidResignActiveNotification object:nil];
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
    // Reposition/resize the title bar view as needed
    [self _recalculateFrameForTitleBarContainer];
    
    NSButton *close = [self standardWindowButton:NSWindowCloseButton];
    NSButton *minimize = [self standardWindowButton:NSWindowMiniaturizeButton];
    NSButton *zoom = [self standardWindowButton:NSWindowZoomButton];
    
    // Set the frame of the window buttons
    NSRect closeFrame = [close frame];
    NSRect minimizeFrame = [minimize frame];
    NSRect zoomFrame = [zoom frame];
    NSRect titleBarFrame = [_titleBarContainer frame];
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
                self.fullScreenButtonRightMargin = NSWidth([_titleBarContainer frame]) - NSMaxX(fullScreen.frame);
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
    
    [self _repositionContentView];
}

- (void)windowWillEnterFullScreen:(NSNotification *)notification 
{
    if (_hideTitleBarInFullScreen) {
        // Recalculate the views when entering from fullscreen
        _titleBarHeight = 0.0f;
		[self _layoutTrafficLightsAndContent];
		[self _displayWindowAndTitlebar];
        
        [self _hideTitleBarView:YES];
    }
}

- (void)windowWillExitFullScreen:(NSNotification *)notification 
{
    if (_hideTitleBarInFullScreen) {
        _titleBarHeight = _cachedTitleBarHeight;
		[self _layoutTrafficLightsAndContent];
		[self _displayWindowAndTitlebar];
        
        [self _hideTitleBarView:NO];
    }
}

- (void)_createTitlebarView
{
    // Create the title bar view
    INTitlebarContainer *container = [[INTitlebarContainer alloc] initWithFrame:NSZeroRect];
    // Configure the view properties and add it as a subview of the theme frame
    NSView *themeFrame = [[self contentView] superview];
    NSView *firstSubview = [[themeFrame subviews] objectAtIndex:0];
    [self _recalculateFrameForTitleBarContainer];
    [themeFrame addSubview:container positioned:NSWindowBelow relativeTo:firstSubview];
    #if __has_feature(objc_arc)
    _titleBarContainer = container;
    self.titleBarView = [[INTitlebarView alloc] initWithFrame:NSZeroRect];
    #else
    _titleBarContainer = [container autorelease];
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

- (void)_recalculateFrameForTitleBarContainer
{
    NSView *themeFrame = [[self contentView] superview];
    NSRect themeFrameRect = [themeFrame frame];
    NSRect titleFrame = NSMakeRect(0.0, NSMaxY(themeFrameRect) - _titleBarHeight, NSWidth(themeFrameRect), _titleBarHeight);
    [_titleBarContainer setFrame:titleFrame];
}

- (void)_repositionContentView
{
    NSView *contentView = [self contentView];
    NSRect windowFrame = [self frame];
    NSRect newFrame = [contentView frame];
    CGFloat titleHeight = NSHeight(windowFrame) - NSHeight(newFrame);
    CGFloat extraHeight = _titleBarHeight - titleHeight;
    newFrame.size.height -= extraHeight;
    [contentView setFrame:newFrame];
    [contentView setNeedsDisplay:YES];
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


- (void)_updateTitlebarView
{
    [_titleBarView setNeedsDisplay:YES];

    // "validate" any controls in the titlebar view
    BOOL isMainWindowAndActive = ([self isMainWindow] && [[NSApplication sharedApplication] isActive]);
    for (NSView *childView in [_titleBarView subviews]) {
        if ([childView isKindOfClass:[NSControl class]]) {
            [(NSControl *)childView setEnabled:isMainWindowAndActive];
        }
    }
}

@end
