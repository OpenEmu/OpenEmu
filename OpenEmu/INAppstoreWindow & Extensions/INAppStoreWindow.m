//
//	INAppStoreWindow.m
//
//	Copyright (c) 2011-2014 Indragie Karunaratne. All rights reserved.
//	Copyright (c) 2014 Petroules Corporation. All rights reserved.
//
//	Licensed under the BSD 2-clause License. See LICENSE file distributed in the source
//	code of this project.
//

#import "INAppStoreWindow.h"
#import "INAppStoreWindowCompatibility.h"
#import "INWindowButton.h"

#import <Carbon/Carbon.h>
#import <objc/runtime.h>

const NSInteger kINAppStoreWindowSmallBottomBarHeight = 22;
const NSInteger kINAppStoreWindowLargeBottomBarHeight = 32;

static NSString * const INWindowBackgroundPatternOverlayLayer;
static NSString * const INWindowBackgroundPatternOverlayLayer2x;

/** Values chosen to match the defaults in OS X 10.9, which may change in future versions **/
const CGFloat INWindowDocumentIconButtonOriginY = 3.f;
const CGFloat INWindowDocumentVersionsButtonOriginY = 2.f;
const CGFloat INWindowDocumentVersionsDividerOriginY = 2.f;

/** Corner clipping radius **/
const CGFloat INCornerClipRadius = 4.0;

/** Padding used between traffic light/fullscreen buttons and title **/
const NSSize INTitleMargins = {8.0, 2.0};

/** Title offsets used when the document button is showing */
const NSSize INTitleDocumentButtonOffset = {4.0, 1.0};

/** X offset used when the document status label is showing */
const CGFloat INTitleDocumentStatusXOffset = -20.0;

/** X offset used when the versions button is showing */
const CGFloat INTitleVersionsButtonXOffset = -1.0;

NS_INLINE CGFloat INMidHeight(NSRect aRect) {
	return (aRect.size.height * (CGFloat) 0.5);
}

CF_RETURNS_RETAINED
NS_INLINE CGPathRef INCreateClippingPathWithRectAndRadius(NSRect rect, CGFloat radius) {
	CGMutablePathRef path = CGPathCreateMutable();
	CGPathMoveToPoint(path, NULL, NSMinX(rect), NSMinY(rect));
	CGPathAddLineToPoint(path, NULL, NSMinX(rect), NSMaxY(rect) - radius);
	CGPathAddArcToPoint(path, NULL, NSMinX(rect), NSMaxY(rect), NSMinX(rect) + radius, NSMaxY(rect), radius);
	CGPathAddLineToPoint(path, NULL, NSMaxX(rect) - radius, NSMaxY(rect));
	CGPathAddArcToPoint(path, NULL, NSMaxX(rect), NSMaxY(rect), NSMaxX(rect), NSMaxY(rect) - radius, radius);
	CGPathAddLineToPoint(path, NULL, NSMaxX(rect), NSMinY(rect));
	CGPathCloseSubpath(path);
	return path;
}

NS_INLINE void INApplyClippingPath(CGPathRef path, CGContextRef ctx) {
	CGContextAddPath(ctx, path);
	CGContextClip(ctx);
}

NS_INLINE void INApplyClippingPathInCurrentContext(CGPathRef path) {
	INApplyClippingPath(path, [[NSGraphicsContext currentContext] graphicsPort]);
}

@interface INAppStoreWindowDelegateProxy : NSProxy <NSWindowDelegate>
@property (nonatomic, assign) id <NSWindowDelegate> secondaryDelegate;
@end

@implementation INAppStoreWindowDelegateProxy

- (NSMethodSignature *)methodSignatureForSelector:(SEL)selector
{
	NSMethodSignature *signature = [[self.secondaryDelegate class] instanceMethodSignatureForSelector:selector];
	if (!signature) {
		signature = [super methodSignatureForSelector:selector];
	}
	return signature;
}

- (BOOL)respondsToSelector:(SEL)aSelector
{
	if ([self.secondaryDelegate respondsToSelector:aSelector]) {
		return YES;
	} else if (aSelector == @selector(window:willPositionSheet:usingRect:)) {
		return YES;
	}
	return NO;
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	if ([self.secondaryDelegate respondsToSelector:anInvocation.selector]) {
		[anInvocation invokeWithTarget:self.secondaryDelegate];
	}
}

- (NSRect)window:(INAppStoreWindow *)window willPositionSheet:(NSWindow *)sheet usingRect:(NSRect)rect
{
	// Somehow the forwarding machinery doesn't handle this.
	if ([self.secondaryDelegate respondsToSelector:_cmd]) {
		return [self.secondaryDelegate window:window willPositionSheet:sheet usingRect:rect];
	}
	rect.origin.y = NSHeight(window.frame) - window.titleBarHeight - window.toolbarHeight;
	return rect;
}

- (BOOL)isKindOfClass:(Class)aClass
{
	if (self.secondaryDelegate) {
		return [self.secondaryDelegate isKindOfClass:aClass];
	}
	return NO;
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
- (void)_setTitleBarViewHidden:(BOOL)hidden;
- (CGFloat)_defaultTrafficLightLeftMargin;
- (CGFloat)_defaultTrafficLightSeparation;
- (NSRect)_contentViewFrame;
- (NSButton *)_closeButtonToLayout;
- (NSButton *)_minimizeButtonToLayout;
- (NSButton *)_zoomButtonToLayout;
- (NSButton *)_fullScreenButtonToLayout;
@end

@implementation INWindowBackgroundView

/*!
 Color used to draw the noise pattern over the window's title bar gradient.
 */
+ (NSColor *)windowPatternOverlayColor
{
	static NSColor *noiseColor = nil;

	static dispatch_once_t oncePredicate;
	dispatch_once(&oncePredicate, ^{
		NSBitmapImageRep *rep = nil;

		NSData *layerInBase64 = nil;
	#if __MAC_OS_X_VERSION_MAX_ALLOWED >= __MAC_10_9 // initWithBase64EncodedString:options: is available in OS X v10.9 and later
		layerInBase64 = [[NSData alloc] initWithBase64EncodedString:INWindowBackgroundPatternOverlayLayer options:NSDataBase64DecodingIgnoreUnknownCharacters];
	#else
		layerInBase64 = [[NSData alloc] initWithBase64Encoding:INWindowBackgroundPatternOverlayLayer];
	#endif
		rep = [[NSBitmapImageRep alloc] initWithData:layerInBase64];

		NSImage *image = [[NSImage alloc] initWithSize:rep.size];
		[image addRepresentation:rep];

		NSData *layer2xInBase64 = nil;
	#if __MAC_OS_X_VERSION_MAX_ALLOWED >= __MAC_10_9 // Same as above
		layer2xInBase64 = [[NSData alloc] initWithBase64EncodedString:INWindowBackgroundPatternOverlayLayer2x options:NSDataBase64DecodingIgnoreUnknownCharacters];
	#else
		layer2xInBase64 = [[NSData alloc] initWithBase64Encoding:INWindowBackgroundPatternOverlayLayer2x];
	#endif
		[image addRepresentation:[[NSBitmapImageRep alloc] initWithData:layer2xInBase64]];

		noiseColor = [NSColor colorWithPatternImage:image];
	});

	return noiseColor;
}

- (void)drawWindowPatternOverlayColorInRect:(NSRect)rect forEdge:(NSRectEdge)edge
{
	NSAssert(edge == NSMinYEdge || edge == NSMaxYEdge, @"edge must be NSMinYEdge or NSMaxYEdge");

	// known constants used by OS X
	const CGFloat opacity = 0.3;
	const CGSize patternPhase = CGSizeMake(edge == NSMaxYEdge ? -5 : 0, self.window.frame.size.height);

	CGContextRef context = [[NSGraphicsContext currentContext] graphicsPort];
	CGContextSaveGState(context);
	CGContextSetAlpha(context, opacity);
	CGContextSetBlendMode(context, kCGBlendModeOverlay);

	CGContextSetPatternPhase(context, patternPhase);
	[[INWindowBackgroundView windowPatternOverlayColor] setFill];
	CGContextFillRect(context, NSRectToCGRect(rect));

	CGContextRestoreGState(context);
}

- (void)drawSeparatorInRect:(NSRect)separatorFrame forEdge:(NSRectEdge)edge
{
	NSAssert(edge == NSMinYEdge || edge == NSMaxYEdge, @"edge must be NSMinYEdge or NSMaxYEdge");

	CGFloat multiplier = 0;
	switch (edge) {
		case NSMinYEdge:
			multiplier = -1;
			break;
		case NSMaxYEdge:
			multiplier = 1;
			break;
	}

	INAppStoreWindow *window = (INAppStoreWindow *)self.window;
	BOOL drawsAsMainWindow = (window.isMainWindow && [NSApplication sharedApplication].isActive);

	NSColor *bottomColor = drawsAsMainWindow ? window.baselineSeparatorColor : window.inactiveBaselineSeparatorColor;

	bottomColor = bottomColor ? bottomColor : [INAppStoreWindow defaultBaselineSeparatorColor:drawsAsMainWindow];

	[bottomColor set];
	NSRectFill(separatorFrame);

	if (INRunningLion()) {
		separatorFrame.origin.y += (separatorFrame.size.height * multiplier);
		separatorFrame.size.height = 1.0;
		[[NSColor colorWithDeviceWhite:1.0 alpha:0.12] setFill];
		[[NSBezierPath bezierPathWithRect:separatorFrame] fill];
	}
}

- (void)drawBackgroundGradientInRect:(NSRect)drawingRect forEdge:(NSRectEdge)edge
{
	NSAssert(edge == NSMinYEdge || edge == NSMaxYEdge, @"edge must be NSMinYEdge or NSMaxYEdge");

	INAppStoreWindow *window = (INAppStoreWindow *)self.window;
	BOOL drawsAsMainWindow = (window.isMainWindow && [NSApplication sharedApplication].isActive);

	NSGradient *gradient = nil;
	if (edge == NSMaxYEdge) {
		gradient = drawsAsMainWindow ? window.titleBarGradient : window.inactiveTitleBarGradient;
		gradient = gradient ? gradient : [INAppStoreWindow defaultTitleBarGradient:drawsAsMainWindow];
	} else if (edge == NSMinYEdge) {
		gradient = drawsAsMainWindow ? window.bottomBarGradient : window.inactiveBottomBarGradient;
		gradient = gradient ? gradient : [INAppStoreWindow defaultBottomBarGradient:drawsAsMainWindow];
	}

	[gradient drawInRect:drawingRect angle:self.isFlipped ? -90 : 90];
}

- (void)drawWindowBackgroundLayersInRect:(NSRect)drawingRect forEdge:(NSRectEdge)drawingEdge showsSeparator:(BOOL)showsSeparator clippingPath:(CGPathRef)clippingPath
{
	INAppStoreWindow *window = (INAppStoreWindow *)self.window;

	if (clippingPath) {
		INApplyClippingPathInCurrentContext(clippingPath);
	}

	if ((window.styleMask & NSTexturedBackgroundWindowMask) == NSTexturedBackgroundWindowMask) {
		// If this is a textured window, we can draw the real background gradient and noise pattern
		CGFloat contentBorderThickness = 0;
		if (drawingEdge == NSMaxYEdge) {
			contentBorderThickness = window.titleBarHeight;
			if (((window.styleMask & NSFullScreenWindowMask) != NSFullScreenWindowMask)) {
				contentBorderThickness -= window._minimumTitlebarHeight;
			}
		} else if (drawingEdge == NSMinYEdge) {
			contentBorderThickness = window.bottomBarHeight;
		}

		[window setAutorecalculatesContentBorderThickness:NO forEdge:drawingEdge];
		[window setContentBorderThickness:contentBorderThickness forEdge:drawingEdge];

		// Technically the noise should be drawn over the separator but we can't do that here
		if (showsSeparator) {
			[self drawSeparatorInRect:[self baselineSeparatorFrameForRect:drawingRect edge:drawingEdge] forEdge:drawingEdge];
		}
	} else {
		// Not textured, we have to fake the background gradient and noise pattern
		[self drawBackgroundGradientInRect:drawingRect forEdge:drawingEdge];

		if (showsSeparator) {
			[self drawSeparatorInRect:[self baselineSeparatorFrameForRect:drawingRect edge:drawingEdge] forEdge:drawingEdge];
		}

		if (INRunningLion() && !INRunningYosemite() && window.drawsTitlePatternOverlay) {
			[self drawWindowPatternOverlayColorInRect:drawingRect forEdge:drawingEdge];
		}
	}
}

- (NSRect)baselineSeparatorFrameForRect:(NSRect)rect edge:(NSRectEdge)edge
{
	NSAssert(edge == NSMinYEdge || edge == NSMaxYEdge, @"edge must be NSMinYEdge or NSMaxYEdge");
	return NSMakeRect(0, edge == NSMaxYEdge ? NSMinY(rect) : NSMaxY(rect) - 1, NSWidth(rect), 1);
}

@end

@implementation INTitlebarView

- (void)drawWindowBorderAccentLineInRect:(NSRect)rect
{
	CGContextRef context = [[NSGraphicsContext currentContext] graphicsPort];
	CGContextSaveGState(context);
	CGContextSetAlpha(context, 0.4);
	CGContextSetBlendMode(context, kCGBlendModeScreen);

	[[NSColor whiteColor] setFill];
	CGContextFillRect(context, NSRectToCGRect(rect));

	CGContextRestoreGState(context);
}

- (NSRect)windowBorderAccentLineFrameForRect:(NSRect)rect
{
	return NSMakeRect(0, NSMaxY(rect) - 1, NSWidth(rect), 1);
}

- (void)drawBackgroundGradientInRect:(NSRect)drawingRect forEdge:(NSRectEdge)edge
{
	[super drawBackgroundGradientInRect:drawingRect forEdge:edge];
	[self drawWindowBorderAccentLineInRect:[self windowBorderAccentLineFrameForRect:drawingRect]];
}

- (void)drawRect:(NSRect)dirtyRect
{
	INAppStoreWindow *window = (INAppStoreWindow *)self.window;
	BOOL drawsAsMainWindow = (window.isMainWindow && [NSApplication sharedApplication].isActive);

	// Start by filling the title bar area with black in fullscreen mode to match native apps
	// Custom title bar drawing blocks can simply override this by not applying the clipping path
	if (((window.styleMask & NSFullScreenWindowMask) == NSFullScreenWindowMask)) {
		[[NSColor blackColor] setFill];
		[[NSBezierPath bezierPathWithRect:self.bounds] fill];
	}

	NSRect drawingRect = self.bounds;
	drawingRect.origin.y -= window.toolbarHeight;
	drawingRect.size.height += window.toolbarHeight;

	CGPathRef clippingPath = INCreateClippingPathWithRectAndRadius(drawingRect, INCornerClipRadius);

	[self drawWindowBackgroundLayersInRect:drawingRect forEdge:NSMaxYEdge showsSeparator:window.showsBaselineSeparator clippingPath:clippingPath];
	if (window.titleBarDrawingBlock) {
		window.titleBarDrawingBlock(drawsAsMainWindow, NSRectToCGRect(drawingRect), CGRectMaxYEdge, clippingPath);
	}

	CGPathRelease(clippingPath);

	if (window.showsTitle && ((window.styleMask & NSFullScreenWindowMask) == 0 || window.showsTitleInFullscreen)) {
		NSRect titleTextRect;
		NSDictionary *titleTextStyles = nil;
		[self getTitleFrame:&titleTextRect textAttributes:&titleTextStyles forWindow:window];

		if (titleTextStyles) {
			if (window.verticallyCenterTitle) {
				titleTextRect.origin.y = floor(NSMidY(drawingRect) - (NSHeight(titleTextRect) / 2.f) + 1);
			}

			[window.title drawInRect:titleTextRect withAttributes:titleTextStyles];
		} else {
			[self drawNativeWindowTitleInRect:titleTextRect];
		}
	}
}

- (void)getNativeTitleTextInfo:(out HIThemeTextInfo *)titleTextInfo
{
	INAppStoreWindow *window = (INAppStoreWindow *)self.window;
	BOOL drawsAsMainWindow = (window.isMainWindow && [NSApplication sharedApplication].isActive);
	titleTextInfo->version = 1;
	titleTextInfo->state = drawsAsMainWindow ? kThemeStateActive : kThemeStateUnavailableInactive;
	titleTextInfo->fontID = kThemeWindowTitleFont;
	titleTextInfo->horizontalFlushness = kHIThemeTextHorizontalFlushCenter;
	titleTextInfo->verticalFlushness = kHIThemeTextVerticalFlushCenter;
	titleTextInfo->options = kHIThemeTextBoxOptionEngraved;
	titleTextInfo->truncationPosition = kHIThemeTextTruncationDefault;
	titleTextInfo->truncationMaxLines = 1;
}

- (void)drawNativeWindowTitleInRect:(NSRect)titleTextRect
{
	INAppStoreWindow *window = (INAppStoreWindow *)self.window;

	HIThemeTextInfo titleTextInfo;
	[self getNativeTitleTextInfo:&titleTextInfo];

	// Let kHIThemeTextVerticalFlushCenter handle the vertical alignment instead of manually calculating it
	titleTextRect.size.height = window.verticallyCenterTitle ? self.bounds.size.height : window._minimumTitlebarHeight;
	titleTextRect.origin.y = self.isFlipped ? 0 : self.bounds.size.height - titleTextRect.size.height;

	HIThemeDrawTextBox((__bridge CFTypeRef)(window.title), &titleTextRect, &titleTextInfo, [[NSGraphicsContext currentContext] graphicsPort], self.isFlipped ? kHIThemeOrientationNormal : kHIThemeOrientationInverted);
}

- (void)getTitleFrame:(out NSRect *)frame textAttributes:(out NSDictionary **)attributes forWindow:(in INAppStoreWindow *)window
{
	BOOL drawsAsMainWindow = (window.isMainWindow && [NSApplication sharedApplication].isActive);
	BOOL isUsingCustomTitleDrawing = window.titleTextShadow
								  || window.inactiveTitleTextShadow
								  || window.titleTextColor
								  || window.inactiveTitleTextColor
								  || window.titleFont;

	NSSize titleSize;
	NSRect titleTextRect;
	if (isUsingCustomTitleDrawing)
	{
		NSShadow *titleTextShadow = drawsAsMainWindow ? window.titleTextShadow : window.inactiveTitleTextShadow;
		if (titleTextShadow == nil) {
			titleTextShadow = [[NSShadow alloc] init];
			titleTextShadow.shadowBlurRadius = 0.0;
			titleTextShadow.shadowOffset = NSMakeSize(0, -1);
			titleTextShadow.shadowColor = [NSColor colorWithDeviceWhite:1.0 alpha:0.5];
		}

		NSColor *titleTextColor = drawsAsMainWindow ? window.titleTextColor : window.inactiveTitleTextColor;
		titleTextColor = titleTextColor ? titleTextColor : [INAppStoreWindow defaultTitleTextColor:drawsAsMainWindow];

		NSFont *titleFont = window.titleFont ?: [NSFont titleBarFontOfSize:[NSFont systemFontSizeForControlSize:NSRegularControlSize]];

		NSMutableParagraphStyle *titleParagraphStyle = [[NSParagraphStyle defaultParagraphStyle] mutableCopy];
		[titleParagraphStyle setLineBreakMode:NSLineBreakByTruncatingTail];
		NSDictionary *titleTextStyles = @{NSFontAttributeName : titleFont,
										  NSForegroundColorAttributeName : titleTextColor,
										  NSShadowAttributeName : titleTextShadow,
										  NSParagraphStyleAttributeName : titleParagraphStyle};
		titleSize = [window.title sizeWithAttributes:titleTextStyles];

		if (attributes) {
			*attributes = titleTextStyles;
		}
	}
	else
	{
		HIThemeTextInfo titleTextInfo;
		[self getNativeTitleTextInfo:&titleTextInfo];
		HIThemeGetTextDimensions((__bridge CFTypeRef)(window.title), 0, &titleTextInfo, &titleSize.width, &titleSize.height, NULL);
	}

	titleTextRect.size = titleSize;

	NSButton *docIconButton = [window standardWindowButton:NSWindowDocumentIconButton];
	NSButton *versionsButton = [window standardWindowButton:NSWindowDocumentVersionsButton];
	NSButton *closeButton = window._closeButtonToLayout;
	NSButton *minimizeButton = window._minimizeButtonToLayout;
	NSButton *zoomButton = window._zoomButtonToLayout;
	if (docIconButton) {
		NSRect docIconButtonFrame = [self convertRect:docIconButton.frame fromView:docIconButton.superview];
		titleTextRect.origin.x = NSMaxX(docIconButtonFrame) + INTitleDocumentButtonOffset.width;
		titleTextRect.origin.y = NSMidY(docIconButtonFrame) - titleSize.height / 2 + INTitleDocumentButtonOffset.height;
	} else if (versionsButton) {
		NSRect versionsButtonFrame = [self convertRect:versionsButton.frame fromView:versionsButton.superview];
		titleTextRect.origin.x = NSMinX(versionsButtonFrame) - titleSize.width + INTitleVersionsButtonXOffset;

		NSDocument *document = (NSDocument *)[(NSWindowController *)self.window.windowController document];
		if (document.hasUnautosavedChanges || document.isDocumentEdited) {
			titleTextRect.origin.x += INTitleDocumentStatusXOffset;
		}
	} else if (closeButton || minimizeButton || zoomButton) {
		CGFloat closeMaxX = NSMaxX(closeButton.frame);
		CGFloat minimizeMaxX = NSMaxX(minimizeButton.frame);
		CGFloat zoomMaxX = NSMaxX(zoomButton.frame);

		CGFloat adjustedX = MAX(MAX(closeMaxX, minimizeMaxX), zoomMaxX) + INTitleMargins.width;
		CGFloat proposedX = NSMidX(self.bounds) - titleSize.width / 2;

		titleTextRect.origin.x = (proposedX < adjustedX) ? adjustedX : proposedX;
	} else {
		titleTextRect.origin.x = NSMidX(self.bounds) - titleSize.width / 2;
	}

	if (versionsButton) {
		NSRect versionsButtonFrame = [self convertRect:versionsButton.frame fromView:versionsButton.superview];

		NSTextField *titleDivider = window.titleDivider;
		if (titleDivider) {
			versionsButtonFrame = [self convertRect:titleDivider.frame fromView:titleDivider.superview];
		}

		if (NSMaxX(titleTextRect) > NSMinX(versionsButtonFrame)) {
			titleTextRect.size.width = NSMinX(versionsButtonFrame) - NSMinX(titleTextRect) - INTitleMargins.width;
		}
	}

	NSButton *fullScreenButton = window._fullScreenButtonToLayout;
	if (fullScreenButton) {
		CGFloat fullScreenX = fullScreenButton.frame.origin.x;
		CGFloat maxTitleX = NSMaxX(titleTextRect);
		if ((fullScreenX - INTitleMargins.width) < NSMaxX(titleTextRect)) {
			titleTextRect.size.width = titleTextRect.size.width - (maxTitleX - fullScreenX) - INTitleMargins.width;
		}
	}

	titleTextRect.origin.y = NSMaxY(self.bounds) - titleSize.height - INTitleMargins.height;

	if (frame) {
		*frame = titleTextRect;
	}
}

- (void)mouseUp:(NSEvent *)theEvent
{
	if (theEvent.clickCount == 2) {
		// Get settings from "System Preferences" >	 "Appearance" > "Double-click on windows title bar to minimize"
		NSString *const MDAppleMiniaturizeOnDoubleClickKey = @"AppleMiniaturizeOnDoubleClick";
		NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
		BOOL shouldMiniaturize = [[userDefaults objectForKey:MDAppleMiniaturizeOnDoubleClickKey] boolValue];
		if (shouldMiniaturize) {
			[self.window performMiniaturize:self];
		}
	}
}

@end

@implementation INBottomBarView

- (void)drawRect:(NSRect)dirtyRect
{
	INAppStoreWindow *window = (INAppStoreWindow *)self.window;
	BOOL drawsAsMainWindow = (window.isMainWindow && [NSApplication sharedApplication].isActive);

	NSRect drawingRect = self.bounds;

	[self drawWindowBackgroundLayersInRect:drawingRect forEdge:NSMinYEdge showsSeparator:window.showsBottomBarSeparator clippingPath:NULL];
	if (window.bottomBarDrawingBlock) {
		window.bottomBarDrawingBlock(drawsAsMainWindow, NSRectToCGRect(drawingRect), CGRectMinYEdge, NULL);
	}
}

@end

@interface INMovableByBackgroundContainerView : NSView
@property (nonatomic) CGFloat mouseDragDetectionThreshold;
@end

@implementation INMovableByBackgroundContainerView
- (instancetype)initWithFrame:(NSRect)frameRect
{
	self = [super initWithFrame:frameRect];
	if (self) {
		_mouseDragDetectionThreshold = 1;
	}
	return self;
}

- (void)mouseDragged:(NSEvent *)theEvent
{
	NSWindow *window = self.window;
	if (window.isMovableByWindowBackground || (window.styleMask & NSFullScreenWindowMask) == NSFullScreenWindowMask) {
		[super mouseDragged:theEvent];
		return;
	}

	NSPoint where = [window convertBaseToScreen:theEvent.locationInWindow];
	NSPoint origin = window.frame.origin;
	CGFloat deltaX = 0.0;
	CGFloat deltaY = 0.0;
	while ((theEvent = [NSApp nextEventMatchingMask:NSLeftMouseDownMask | NSLeftMouseDraggedMask | NSLeftMouseUpMask untilDate:[NSDate distantFuture] inMode:NSEventTrackingRunLoopMode dequeue:YES]) && (theEvent.type != NSLeftMouseUp)) {
		@autoreleasepool {
			NSPoint now = [window convertBaseToScreen:theEvent.locationInWindow];
			deltaX += now.x - where.x;
			deltaY += now.y - where.y;
			if (fabs(deltaX) >= _mouseDragDetectionThreshold || fabs(deltaY) >= _mouseDragDetectionThreshold) {
				// This part is only called if drag occurs on container view!
				origin.x += deltaX;
				origin.y += deltaY;
				window.frameOrigin = origin;
				deltaX = 0.0;
				deltaY = 0.0;
			}
			where = now; // this should be inside above if but doing that results in jittering while moving the window...
		}
	}
}
@end

@interface INAppStoreWindowContentView : NSView
@end

@implementation INAppStoreWindowContentView

- (void)setFrame:(NSRect)frameRect
{
	frameRect = [(INAppStoreWindow *)self.window _contentViewFrame];
	super.frame = frameRect;
}

- (void)setFrameSize:(NSSize)newSize
{
	newSize = [(INAppStoreWindow *)self.window _contentViewFrame].size;
	super.frameSize = newSize;
}

@end

@implementation INAppStoreWindow {
	CGFloat _cachedTitleBarHeight;
	BOOL _setFullScreenButtonRightMargin;
	BOOL _preventWindowFrameChange;
	INAppStoreWindowDelegateProxy *_delegateProxy;
	INMovableByBackgroundContainerView *_titleBarContainer;
	INMovableByBackgroundContainerView *_bottomBarContainer;
}

@synthesize titleBarView = _titleBarView;
@synthesize titleBarHeight = _titleBarHeight;
@synthesize bottomBarView = _bottomBarView;
@synthesize bottomBarHeight = _bottomBarHeight;
@synthesize centerFullScreenButton = _centerFullScreenButton;
@synthesize centerTrafficLightButtons = _centerTrafficLightButtons;
@synthesize verticalTrafficLightButtons = _verticalTrafficLightButtons;
@synthesize hideTitleBarInFullScreen = _hideTitleBarInFullScreen;
@synthesize titleBarDrawingBlock = _titleBarDrawingBlock;
@synthesize bottomBarDrawingBlock = _bottomBarDrawingBlock;
@synthesize showsBaselineSeparator = _showsBaselineSeparator;
@synthesize showsBottomBarSeparator = _showsBottomBarSeparator;
@synthesize fullScreenButtonRightMargin = _fullScreenButtonRightMargin;
@synthesize trafficLightButtonsLeftMargin = _trafficLightButtonsLeftMargin;
@synthesize titleBarGradient = _titleBarGradient;
@synthesize bottomBarGradient = _bottomBarGradient;
@synthesize baselineSeparatorColor = _baselineSeparatorColor;
@synthesize inactiveTitleBarGradient = _inactiveTitleBarGradient;
@synthesize inactiveBottomBarGradient = _inactiveBottomBarGradient;
@synthesize inactiveBaselineSeparatorColor = _inactiveBaselineSeparatorColor;
@synthesize showsDocumentProxyIcon = _showsDocumentProxyIcon;

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

- (void)setRepresentedURL:(NSURL *)url
{
	super.representedURL = url;
	if (_showsDocumentProxyIcon == NO) {
		[self standardWindowButton:NSWindowDocumentIconButton].image = nil;
	}
}

#pragma mark -
#pragma mark Memory Management

- (void)dealloc
{
	[[NSNotificationCenter defaultCenter] removeObserver:self];
}

#pragma mark -
#pragma mark NSWindow Overrides

- (void)becomeKeyWindow
{
	[super becomeKeyWindow];
	[self _updateTitlebarView];
	[self _updateBottomBarView];
	[self _layoutTrafficLightsAndContent];
	[self _setupTrafficLightsTrackingArea];
}

- (void)resignKeyWindow
{
	[super resignKeyWindow];
	[self _updateTitlebarView];
	[self _updateBottomBarView];
	[self _layoutTrafficLightsAndContent];
}

- (void)becomeMainWindow
{
	[super becomeMainWindow];
	[self _updateTitlebarView];
	[self _updateBottomBarView];
}

- (void)resignMainWindow
{
	[super resignMainWindow];
	[self _updateTitlebarView];
	[self _updateBottomBarView];
}

- (void)setContentView:(NSView *)aView
{
	// Remove performance-optimized content view class when changing content views
	NSView *oldView = self.contentView;
	if (oldView && object_getClass(oldView) == [INAppStoreWindowContentView class]) {
		object_setClass(oldView, [NSView class]);
	}

	super.contentView = aView;

	// Swap in performance-optimized content view class
	if (aView && object_getClass(aView) == [NSView class]) {
		object_setClass(aView, [INAppStoreWindowContentView class]);
	}

	[self _repositionContentView];
}

- (void)setTitle:(NSString *)aString
{
	super.title = aString;
	[self _layoutTrafficLightsAndContent];
	[self _displayWindowAndTitlebar];
}

- (void)setMaxSize:(NSSize)size
{
	super.maxSize = size;
	[self _layoutTrafficLightsAndContent];
}

- (void)setMinSize:(NSSize)size
{
	super.minSize = size;
	[self _layoutTrafficLightsAndContent];
}

- (void)setToolbar:(NSToolbar *)toolbar
{
	self.titleBarHeight = self._minimumTitlebarHeight;
	super.toolbar = toolbar;
}

- (IBAction)toggleToolbarShown:(id)sender
{
	[super toggleToolbarShown:sender];
	[self _repositionContentView];
}

#pragma mark -
#pragma mark Accessors

- (void)setTitleBarView:(NSView *)newTitleBarView
{
	if ((_titleBarView != newTitleBarView) && newTitleBarView) {
		[_titleBarView removeFromSuperview];
		_titleBarView = newTitleBarView;
		_titleBarView.frame = _titleBarContainer.bounds;
		_titleBarView.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
		[_titleBarContainer addSubview:_titleBarView];
	}
}

- (void)setTitleBarHeight:(CGFloat)newTitleBarHeight
{
	[self setTitleBarHeight:newTitleBarHeight adjustWindowFrame:YES];
}

- (void)setTitleBarHeight:(CGFloat)newTitleBarHeight adjustWindowFrame:(BOOL)adjustWindowFrame
{
	if (_titleBarHeight != newTitleBarHeight) {
		if (self.toolbar) {
			newTitleBarHeight = self._minimumTitlebarHeight;
		}

		NSRect windowFrame = self.frame;
		if (adjustWindowFrame)
		{
			windowFrame.origin.y -= newTitleBarHeight - _titleBarHeight;
			windowFrame.size.height += newTitleBarHeight - _titleBarHeight;
		}

		_cachedTitleBarHeight = newTitleBarHeight;
		_titleBarHeight = _cachedTitleBarHeight;

		[self setFrame:windowFrame display:YES];

		[self _layoutTrafficLightsAndContent];
		[self _displayWindowAndTitlebar];

		if ((self.styleMask & NSTexturedBackgroundWindowMask) == NSTexturedBackgroundWindowMask) {
			[self.contentView displayIfNeeded];
		}
	}
}

- (void)setBottomBarView:(NSView *)bottomBarView
{
	if ((_bottomBarView != bottomBarView) && bottomBarView) {
		[_bottomBarView removeFromSuperview];
		_bottomBarView = bottomBarView;
		_bottomBarView.frame = _bottomBarContainer.bounds;
		_bottomBarView.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
		[_bottomBarContainer addSubview:_bottomBarView];
	}
}

- (void)setBottomBarHeight:(CGFloat)bottomBarHeight
{
	if (_bottomBarHeight != bottomBarHeight) {
		_bottomBarHeight = bottomBarHeight;

		[self _layoutTrafficLightsAndContent];

		if ((self.styleMask & NSTexturedBackgroundWindowMask) == NSTexturedBackgroundWindowMask) {
			[self.contentView displayIfNeeded];
		}
	}
}

- (void)setShowsBaselineSeparator:(BOOL)showsBaselineSeparator
{
	if (_showsBaselineSeparator != showsBaselineSeparator) {
		_showsBaselineSeparator = showsBaselineSeparator;
		[self.titleBarView setNeedsDisplay:YES];
	}
}

- (void)setShowsBottomBarSeparator:(BOOL)showsBottomBarSeparator
{
	if (_showsBottomBarSeparator != showsBottomBarSeparator) {
		_showsBottomBarSeparator = showsBottomBarSeparator;
		[self.bottomBarView setNeedsDisplay:YES];
	}
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

- (void)setFullScreenButtonRightMargin:(CGFloat)newFullScreenButtonRightMargin
{
	if (_fullScreenButtonRightMargin != newFullScreenButtonRightMargin) {
		_setFullScreenButtonRightMargin = YES;
		_fullScreenButtonRightMargin = newFullScreenButtonRightMargin;
		[self _layoutTrafficLightsAndContent];
		[self _displayWindowAndTitlebar];
	}
}

- (void)setShowsTitle:(BOOL)showsTitle
{
	if (_showsTitle != showsTitle) {
		_showsTitle = showsTitle;
		[self _displayWindowAndTitlebar];
	}
}

- (void)setShowsDocumentProxyIcon:(BOOL)showsDocumentProxyIcon
{
	if (_showsDocumentProxyIcon != showsDocumentProxyIcon) {
		_showsDocumentProxyIcon = showsDocumentProxyIcon;
		[self _displayWindowAndTitlebar];
	}
}

- (void)setCenterFullScreenButton:(BOOL)centerFullScreenButton
{
	if (_centerFullScreenButton != centerFullScreenButton) {
		_centerFullScreenButton = centerFullScreenButton;
		[self _layoutTrafficLightsAndContent];
	}
}

- (void)setCenterTrafficLightButtons:(BOOL)centerTrafficLightButtons
{
	if (_centerTrafficLightButtons != centerTrafficLightButtons) {
		_centerTrafficLightButtons = centerTrafficLightButtons;
		[self _layoutTrafficLightsAndContent];
		[self _setupTrafficLightsTrackingArea];
	}
}

- (void)setVerticalTrafficLightButtons:(BOOL)verticalTrafficLightButtons
{
	if (_verticalTrafficLightButtons != verticalTrafficLightButtons) {
		_verticalTrafficLightButtons = verticalTrafficLightButtons;
		[self _layoutTrafficLightsAndContent];
		[self _setupTrafficLightsTrackingArea];
	}
}

- (void)setVerticallyCenterTitle:(BOOL)verticallyCenterTitle
{
	if (_verticallyCenterTitle != verticallyCenterTitle) {
		_verticallyCenterTitle = verticallyCenterTitle;
		[self _displayWindowAndTitlebar];
	}
}

- (void)setTrafficLightSeparation:(CGFloat)trafficLightSeparation
{
	if (_trafficLightSeparation != trafficLightSeparation) {
		_trafficLightSeparation = trafficLightSeparation;
		[self _layoutTrafficLightsAndContent];
		[self _setupTrafficLightsTrackingArea];
	}
}

- (void)setMouseDragDetectionThreshold:(CGFloat)mouseDragDetectionThreshold
{
	_titleBarContainer.mouseDragDetectionThreshold = mouseDragDetectionThreshold;
}

- (CGFloat)mouseDragDetectionThreshold
{
	return _titleBarContainer.mouseDragDetectionThreshold;
}

- (void)setDelegate:(id <NSWindowDelegate>)anObject
{
	_delegateProxy.secondaryDelegate = anObject;
	super.delegate = nil;
	super.delegate = _delegateProxy;
}

- (id <NSWindowDelegate>)delegate
{
	return _delegateProxy.secondaryDelegate;
}

- (void)setCloseButton:(INWindowButton *)closeButton
{
	if (_closeButton != closeButton) {
		[_closeButton removeFromSuperview];
		_closeButton = closeButton;
		if (_closeButton) {
			_closeButton.target = self;
			_closeButton.action = @selector(performClose:);
			_closeButton.frameOrigin = [self standardWindowButton:NSWindowCloseButton].frame.origin;
			[_closeButton.cell accessibilitySetOverrideValue:NSAccessibilityCloseButtonSubrole forAttribute:NSAccessibilitySubroleAttribute];
			[_closeButton.cell accessibilitySetOverrideValue:NSAccessibilityRoleDescription(NSAccessibilityButtonRole, NSAccessibilityCloseButtonSubrole) forAttribute:NSAccessibilityRoleDescriptionAttribute];
			[self.themeFrameView addSubview:_closeButton];
		}
	}
}

- (void)setMinimizeButton:(INWindowButton *)minimizeButton
{
	if (_minimizeButton != minimizeButton) {
		[_minimizeButton removeFromSuperview];
		_minimizeButton = minimizeButton;
		if (_minimizeButton) {
			_minimizeButton.target = self;
			_minimizeButton.action = @selector(performMiniaturize:);
			_minimizeButton.frameOrigin = [self standardWindowButton:NSWindowMiniaturizeButton].frame.origin;
			[_minimizeButton.cell accessibilitySetOverrideValue:NSAccessibilityMinimizeButtonSubrole forAttribute:NSAccessibilitySubroleAttribute];
			[_minimizeButton.cell accessibilitySetOverrideValue:NSAccessibilityRoleDescription(NSAccessibilityButtonRole, NSAccessibilityMinimizeButtonSubrole) forAttribute:NSAccessibilityRoleDescriptionAttribute];
			[self.themeFrameView addSubview:_minimizeButton];
		}
	}
}

- (void)setZoomButton:(INWindowButton *)zoomButton
{
	if (_zoomButton != zoomButton) {
		[_zoomButton removeFromSuperview];
		_zoomButton = zoomButton;
		if (_zoomButton) {
			_zoomButton.target = self;
			_zoomButton.action = @selector(performZoom:);
			_zoomButton.frameOrigin = [self standardWindowButton:NSWindowZoomButton].frame.origin;
			[_zoomButton.cell accessibilitySetOverrideValue:NSAccessibilityZoomButtonSubrole forAttribute:NSAccessibilitySubroleAttribute];
			[_zoomButton.cell accessibilitySetOverrideValue:NSAccessibilityRoleDescription(NSAccessibilityButtonRole, NSAccessibilityZoomButtonSubrole) forAttribute:NSAccessibilityRoleDescriptionAttribute];
			[self.themeFrameView addSubview:_zoomButton];
		}
	}
}

- (void)setFullScreenButton:(INWindowButton *)fullScreenButton
{
	if (_fullScreenButton != fullScreenButton) {
		[_fullScreenButton removeFromSuperview];
		_fullScreenButton = fullScreenButton;
		if (_fullScreenButton) {
			_fullScreenButton.target = self;
			_fullScreenButton.action = @selector(toggleFullScreen:);
			_fullScreenButton.frameOrigin = [self standardWindowButton:NSWindowFullScreenButton].frame.origin;
			[_fullScreenButton.cell accessibilitySetOverrideValue:NSAccessibilityFullScreenButtonSubrole forAttribute:NSAccessibilitySubroleAttribute];
			[_fullScreenButton.cell accessibilitySetOverrideValue:NSAccessibilityRoleDescription(NSAccessibilityButtonRole, NSAccessibilityFullScreenButtonSubrole) forAttribute:NSAccessibilityRoleDescriptionAttribute];
			[self.themeFrameView addSubview:_fullScreenButton];
		}
	}
}

- (NSTextField *)titleDivider
{
	for (NSTextField *divider in self.themeFrameView.subviews) {
		if ([divider isKindOfClass:[NSTextField class]]) {
			if ([divider.stringValue isEqualToString:@"\u2014"]) {
				return divider;
			}
		}
	}

	return nil;
}

- (void)setStyleMask:(NSUInteger)styleMask
{
	_preventWindowFrameChange = YES;

	// Prevent drawing artifacts when turning off NSTexturedBackgroundWindowMask before
	// exiting from full screen and then resizing the title bar; the problem is that internally
	// the content border is still set to the previous value, which confuses the system
	if (((self.styleMask & NSTexturedBackgroundWindowMask) == NSTexturedBackgroundWindowMask) &&
			((styleMask & NSTexturedBackgroundWindowMask) != NSTexturedBackgroundWindowMask)) {
		[self setContentBorderThickness:0 forEdge:NSMaxYEdge];
		[self setAutorecalculatesContentBorderThickness:YES forEdge:NSMaxYEdge];
	}

	super.styleMask = styleMask;
	[self _displayWindowAndTitlebar];
	[self.contentView display]; // force display, the view doesn't think it needs it, but it does
	_preventWindowFrameChange = NO;
}

- (void)setFrame:(NSRect)frameRect display:(BOOL)flag
{
	if (!_preventWindowFrameChange)
		[super setFrame:frameRect display:flag];
}

- (void)setFrame:(NSRect)frameRect display:(BOOL)displayFlag animate:(BOOL)animateFlag
{
	if (!_preventWindowFrameChange)
		[super setFrame:frameRect display:displayFlag animate:animateFlag];
}

- (CGFloat)toolbarHeight
{
	CGFloat toolbarHeight = 0.0;
	NSToolbar *toolbar = self.toolbar;
	if (toolbar.isVisible) {
		NSRect expectedContentFrame = [NSWindow contentRectForFrameRect:self.frame
															  styleMask:self.styleMask];
		toolbarHeight = NSHeight(expectedContentFrame) -
						NSHeight([self contentRectForFrameRect:self.frame]);
	}
	return toolbarHeight;
}

#pragma mark -
#pragma mark Private

- (void)_doInitialWindowSetup
{
	_showsBaselineSeparator = YES;
	_showsBottomBarSeparator = YES;
	_centerTrafficLightButtons = YES;
	_titleBarHeight = self._minimumTitlebarHeight;
	_cachedTitleBarHeight = _titleBarHeight;
	_trafficLightButtonsLeftMargin = self._defaultTrafficLightLeftMargin;
	_delegateProxy = [INAppStoreWindowDelegateProxy alloc];
	_trafficLightButtonsTopMargin = 3.f;
	_fullScreenButtonTopMargin = 3.f;
	_trafficLightSeparation = self._defaultTrafficLightSeparation;
	_drawsTitlePatternOverlay = YES;
	super.delegate = _delegateProxy;

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

	[nc addObserver:self selector:@selector(_updateBottomBarView) name:NSApplicationDidBecomeActiveNotification object:nil];
	[nc addObserver:self selector:@selector(_updateBottomBarView) name:NSApplicationDidResignActiveNotification object:nil];

	if (INRunningLion()) {
		[nc addObserver:self selector:@selector(windowDidExitFullScreen:) name:NSWindowDidExitFullScreenNotification object:self];
		[nc addObserver:self selector:@selector(windowWillEnterFullScreen:) name:NSWindowWillEnterFullScreenNotification object:self];
		[nc addObserver:self selector:@selector(windowWillExitFullScreen:) name:NSWindowWillExitFullScreenNotification object:self];
	}

	[self _createTitlebarView];
	[self _createBottomBarView];
	[self _layoutTrafficLightsAndContent];
	[self _setupTrafficLightsTrackingArea];
}

- (NSButton *)_windowButtonToLayout:(NSWindowButton)defaultButtonType orUserProvided:(NSButton *)userButton
{
	NSButton *defaultButton = [self standardWindowButton:defaultButtonType];
	if (userButton) {
		[defaultButton setHidden:YES];
		defaultButton = userButton;
	} else if (defaultButton.superview != self.themeFrameView) {
		[defaultButton setHidden:NO];
	}
	return defaultButton;
}

- (NSButton *)_closeButtonToLayout
{
	return [self _windowButtonToLayout:NSWindowCloseButton orUserProvided:self.closeButton];
}

- (NSButton *)_minimizeButtonToLayout
{
	return [self _windowButtonToLayout:NSWindowMiniaturizeButton orUserProvided:self.minimizeButton];
}

- (NSButton *)_zoomButtonToLayout
{
	return [self _windowButtonToLayout:NSWindowZoomButton orUserProvided:self.zoomButton];
}

- (NSButton *)_fullScreenButtonToLayout
{
	return [self _windowButtonToLayout:NSWindowFullScreenButton orUserProvided:self.fullScreenButton];
}

- (void)_layoutTrafficLightsAndContent
{
	// Reposition/resize the title bar view as needed
	[self _recalculateFrameForTitleBarContainer];
	[self _recalculateFrameForBottomBarContainer];
	NSButton *close = self._closeButtonToLayout;
	NSButton *minimize = self._minimizeButtonToLayout;
	NSButton *zoom = self._zoomButtonToLayout;

	// Set the frame of the window buttons
	NSRect closeFrame = close.frame;
	NSRect minimizeFrame = minimize.frame;
	NSRect zoomFrame = zoom.frame;
	NSRect titleBarFrame = _titleBarContainer.frame;
	CGFloat buttonOrigin = 0.0;
	if (!self.verticalTrafficLightButtons) {
		if (self.centerTrafficLightButtons) {
			buttonOrigin = round(NSMidY(titleBarFrame) - INMidHeight(closeFrame));
		} else {
			buttonOrigin = NSMaxY(titleBarFrame) - NSHeight(closeFrame) - self.trafficLightButtonsTopMargin;
		}
		closeFrame.origin.y = buttonOrigin;
		minimizeFrame.origin.y = buttonOrigin;
		zoomFrame.origin.y = buttonOrigin;
		closeFrame.origin.x = self.trafficLightButtonsLeftMargin;
		minimizeFrame.origin.x = NSMaxX(closeFrame) + self.trafficLightSeparation;
		zoomFrame.origin.x = NSMaxX(minimizeFrame) + self.trafficLightSeparation;
	} else {
		CGFloat groupHeight = NSHeight(closeFrame) + NSHeight(minimizeFrame) + NSHeight(zoomFrame) + 2.f * (self.trafficLightSeparation - 2.f);
		if (self.centerTrafficLightButtons) {
			buttonOrigin = round(NSMidY(titleBarFrame) - groupHeight / 2.f);
		} else {
			buttonOrigin = NSMaxY(titleBarFrame) - groupHeight - self.trafficLightButtonsTopMargin;
		}
		closeFrame.origin.x = self.trafficLightButtonsLeftMargin;
		minimizeFrame.origin.x = self.trafficLightButtonsLeftMargin;
		zoomFrame.origin.x = self.trafficLightButtonsLeftMargin;
		zoomFrame.origin.y = buttonOrigin;
		minimizeFrame.origin.y = NSMaxY(zoomFrame) + self.trafficLightSeparation - 2.f;
		closeFrame.origin.y = NSMaxY(minimizeFrame) + self.trafficLightSeparation - 2.f;
	}
	close.frame = closeFrame;
	minimize.frame = minimizeFrame;
	zoom.frame = zoomFrame;

	NSButton *docIconButton = [self standardWindowButton:NSWindowDocumentIconButton];
	if (docIconButton) {
		NSRect docButtonIconFrame = docIconButton.frame;

		if (self.verticallyCenterTitle) {
			docButtonIconFrame.origin.y = floor(NSMidY(titleBarFrame) - INMidHeight(docButtonIconFrame));
		} else {
			docButtonIconFrame.origin.y = NSMaxY(titleBarFrame) - NSHeight(docButtonIconFrame) - INWindowDocumentIconButtonOriginY;
		}

		docIconButton.frame = docButtonIconFrame;
	}

	// Set the frame of the FullScreen button in Lion if available
	if (INRunningLion()) {
		NSButton *fullScreen = self._fullScreenButtonToLayout;
		if (fullScreen) {
			NSRect fullScreenFrame = fullScreen.frame;
			if (!_setFullScreenButtonRightMargin) {
				self.fullScreenButtonRightMargin = NSWidth(_titleBarContainer.frame) - NSMaxX(fullScreen.frame);
			}
			fullScreenFrame.origin.x = NSWidth(titleBarFrame) - NSWidth(fullScreenFrame) - _fullScreenButtonRightMargin;
			if (self.centerFullScreenButton) {
				fullScreenFrame.origin.y = floor(NSMidY(titleBarFrame) - INMidHeight(fullScreenFrame));
			} else {
				fullScreenFrame.origin.y = NSMaxY(titleBarFrame) - NSHeight(fullScreenFrame) - self.fullScreenButtonTopMargin;
			}
			fullScreen.frame = fullScreenFrame;
		}

		NSButton *versionsButton = [self standardWindowButton:NSWindowDocumentVersionsButton];
		if (versionsButton) {
			NSRect versionsButtonFrame = versionsButton.frame;

			if (self.verticallyCenterTitle) {
				versionsButtonFrame.origin.y = floor(NSMidY(titleBarFrame) - INMidHeight(versionsButtonFrame));
			} else {
				versionsButtonFrame.origin.y = NSMaxY(titleBarFrame) - NSHeight(versionsButtonFrame) - INWindowDocumentVersionsButtonOriginY;
			}

			versionsButton.frame = versionsButtonFrame;

			// Also ensure that the title font is set
			if (self.titleFont) {
				versionsButton.font = self.titleFont;
			}
		}

		for (id subview in self.themeFrameView.subviews) {
			if ([subview isKindOfClass:[NSTextField class]]) {
				NSTextField *textField = (NSTextField *) subview;
				NSRect textFieldFrame = textField.frame;

				if (self.verticallyCenterTitle) {
					textFieldFrame.origin.y = round(NSMidY(titleBarFrame) - INMidHeight(textFieldFrame));
				} else {
					textFieldFrame.origin.y = NSMaxY(titleBarFrame) - NSHeight(textFieldFrame) - INWindowDocumentVersionsDividerOriginY;
				}

				textField.frame = textFieldFrame;

				// Also ensure that the font is set
				if (self.titleFont) {
					textField.font = self.titleFont;
				}
			}
		}
	}

	[self _repositionContentView];
}

- (void)undoManagerDidCloseUndoGroupNotification:(NSNotification *)notification
{
	[self _displayWindowAndTitlebar];
}

- (void)windowWillEnterFullScreen:(NSNotification *)notification
{
	if (_hideTitleBarInFullScreen) {
		// Recalculate the views when entering from fullscreen
		_titleBarHeight = 0.0f;
		[self _layoutTrafficLightsAndContent];
		[self _displayWindowAndTitlebar];

		[self _setTitleBarViewHidden:YES];
	}
}

- (void)windowWillExitFullScreen:(NSNotification *)notification
{
	if (_hideTitleBarInFullScreen) {
		_titleBarHeight = _cachedTitleBarHeight;
		[self _layoutTrafficLightsAndContent];
		[self _displayWindowAndTitlebar];

		[self _setTitleBarViewHidden:NO];
	}
}

- (void)windowDidExitFullScreen:(NSNotification *)notification
{
	[self _layoutTrafficLightsAndContent];
	[self _setupTrafficLightsTrackingArea];
}

- (NSView *)themeFrameView
{
	return [self.contentView superview];
}

- (void)_createTitlebarView
{
	// Create the title bar view
	INMovableByBackgroundContainerView *container = [[INMovableByBackgroundContainerView alloc] initWithFrame:NSZeroRect];
	// Configure the view properties and add it as a subview of the theme frame
	NSView *firstSubview = self.themeFrameView.subviews.firstObject;
	[self _recalculateFrameForTitleBarContainer];
	[self.themeFrameView addSubview:container positioned:NSWindowBelow relativeTo:firstSubview];
	_titleBarContainer = container;
	self.titleBarView = [[INTitlebarView alloc] initWithFrame:NSZeroRect];
}

- (void)_createBottomBarView
{
	// Create the bottom bar view
	INMovableByBackgroundContainerView *container = [[INMovableByBackgroundContainerView alloc] initWithFrame:NSZeroRect];
	// Configure the view properties and add it as a subview of the theme frame
	NSView *firstSubview = self.themeFrameView.subviews.firstObject;
	[self _recalculateFrameForBottomBarContainer];
	[self.themeFrameView addSubview:container positioned:NSWindowBelow relativeTo:firstSubview];
	_bottomBarContainer = container;
	self.bottomBarView = [[INBottomBarView alloc] initWithFrame:NSZeroRect];
}

- (void)_setTitleBarViewHidden:(BOOL)hidden
{
	self.titleBarView.hidden = hidden;
}

// Solution for tracking area issue thanks to @Perspx (Alex Rozanski) <https://gist.github.com/972958>
- (void)_setupTrafficLightsTrackingArea
{
	[self.themeFrameView viewWillStartLiveResize];
	[self.themeFrameView viewDidEndLiveResize];
}

- (void)_recalculateFrameForTitleBarContainer
{
	NSRect themeFrameRect = self.themeFrameView.frame;
	NSRect titleFrame = NSMakeRect(0.0, NSMaxY(themeFrameRect) - _titleBarHeight, NSWidth(themeFrameRect), _titleBarHeight);
	_titleBarContainer.frame = titleFrame;
}

- (void)_recalculateFrameForBottomBarContainer
{
	_bottomBarContainer.frame = NSMakeRect(0, 0, NSWidth(self.themeFrameView.frame), _bottomBarHeight);
}

- (NSRect)_contentViewFrame
{
	NSRect windowFrame = self.frame;
	NSRect contentRect = [self contentRectForFrameRect:windowFrame];

	if ((self.styleMask & NSFullScreenWindowMask) == NSFullScreenWindowMask) {
		if (self.hideTitleBarInFullScreen) {
			contentRect.size.height = NSHeight(windowFrame);
		} else {
			contentRect.size.height = NSHeight(windowFrame) - self.titleBarHeight - self.toolbarHeight;
		}
	} else {
		contentRect.size.height = NSHeight(windowFrame) - self.titleBarHeight - self.toolbarHeight;
	}
	contentRect.origin = NSZeroPoint;

	contentRect.origin.y += self.bottomBarHeight;
	contentRect.size.height -= self.bottomBarHeight;

	return contentRect;
}

- (void)_repositionContentView
{
	NSView *contentView = self.contentView;
	NSRect newFrame = self._contentViewFrame;

	if (!NSEqualRects(contentView.frame, newFrame)) {
		contentView.frame = newFrame;
		[contentView setNeedsDisplay:YES];
	}
}

- (CGFloat)_minimumTitlebarHeight
{
	static CGFloat minTitleHeight = 0.0;
	if (!minTitleHeight) {
		NSRect frameRect = self.frame;
		NSRect contentRect = [self contentRectForFrameRect:frameRect];
		minTitleHeight = NSHeight(frameRect) - NSHeight(contentRect);
	}
	return minTitleHeight;
}

- (CGFloat)_defaultTrafficLightLeftMargin
{
	static CGFloat trafficLightLeftMargin = 0.0;
	if (!trafficLightLeftMargin) {
		NSButton *close = self._closeButtonToLayout;
		trafficLightLeftMargin = NSMinX(close.frame);
	}
	return trafficLightLeftMargin;
}

- (CGFloat)_defaultTrafficLightSeparation
{
	static CGFloat trafficLightSeparation = 0.0;
	if (!trafficLightSeparation) {
		NSButton *close = self._closeButtonToLayout;
		NSButton *minimize = self._minimizeButtonToLayout;
		trafficLightSeparation = NSMinX(minimize.frame) - NSMaxX(close.frame);
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
	BOOL isMainWindowAndActive = (self.isMainWindow && [NSApplication sharedApplication].isActive);
	for (NSView *childView in _titleBarView.subviews) {
		if ([childView isKindOfClass:[NSControl class]]) {
			[(NSControl *)childView setEnabled:isMainWindowAndActive];
		}
	}
}

- (void)_updateBottomBarView
{
	[_bottomBarView setNeedsDisplay:YES];
}

+ (NSGradient *)defaultTitleBarGradient:(BOOL)drawsAsMainWindow
{
	if (INRunningLion()) {
		// Lion, Mountain Lion, Mavericks: real, 100% accurate system colors
		if (drawsAsMainWindow) {
			return [[NSGradient alloc] initWithColors:@[[NSColor colorWithDeviceWhite:180. / 0xff alpha:1],
														[NSColor colorWithDeviceWhite:195. / 0xff alpha:1],
														[NSColor colorWithDeviceWhite:210. / 0xff alpha:1],
														[NSColor colorWithDeviceWhite:235. / 0xff alpha:1]]
										  atLocations:(const CGFloat[]){0.0, 0.25, 0.5, 1.0}
										   colorSpace:[NSColorSpace deviceRGBColorSpace]];
		} else {
			return [[NSGradient alloc] initWithColors:@[[NSColor colorWithDeviceWhite:225. / 0xff alpha:1],
														[NSColor colorWithDeviceWhite:250. / 0xff alpha:1]]
										  atLocations:(const CGFloat[]){0.0, 1.0}
										   colorSpace:[NSColorSpace deviceRGBColorSpace]];
		}
	} else {
		// Leopard, Snow Leopard: approximations
		if (drawsAsMainWindow) {
			return [[NSGradient alloc] initWithColors:@[[NSColor colorWithDeviceWhite:0.659 alpha:1],
														[NSColor colorWithDeviceWhite:0.812 alpha:1]]
										  atLocations:(const CGFloat[]){0.0, 1.0}
										   colorSpace:[NSColorSpace deviceRGBColorSpace]];
		} else {
			return [[NSGradient alloc] initWithColors:@[[NSColor colorWithDeviceWhite:0.851 alpha:1],
														[NSColor colorWithDeviceWhite:0.929 alpha:1]]
										  atLocations:(const CGFloat[]){0.0, 1.0}
										   colorSpace:[NSColorSpace deviceRGBColorSpace]];
		}
	}
}

+ (NSGradient *)defaultBottomBarGradient:(BOOL)drawsAsMainWindow
{
	if (drawsAsMainWindow) {
		return [[NSGradient alloc] initWithColors:@[[NSColor colorWithDeviceWhite:180. / 0xff alpha:1],
													[NSColor colorWithDeviceWhite:210. / 0xff alpha:1]]
									  atLocations:(const CGFloat[]){0.0, 1.0}
									   colorSpace:[NSColorSpace deviceRGBColorSpace]];
	} else {
		return [[NSGradient alloc] initWithColors:@[[NSColor colorWithDeviceWhite:225. / 0xff alpha:1],
													[NSColor colorWithDeviceWhite:235. / 0xff alpha:1]]
									  atLocations:(const CGFloat[]){0.0, 1.0}
									   colorSpace:[NSColorSpace deviceRGBColorSpace]];
	}
}

+ (NSColor *)defaultBaselineSeparatorColor:(BOOL)drawsAsMainWindow
{
	if (INRunningLion())
		return drawsAsMainWindow ? [NSColor colorWithDeviceWhite:104. / 0xff alpha:1.0] : [NSColor colorWithDeviceWhite:167. / 0xff alpha:1.0];
	else
		return drawsAsMainWindow ? [NSColor colorWithDeviceWhite:0.318 alpha:1.0] : [NSColor colorWithDeviceWhite:0.600 alpha:1.0];
}

+ (NSColor *)defaultTitleTextColor:(BOOL)drawsAsMainWindow
{
	return drawsAsMainWindow ? [NSColor colorWithDeviceWhite:56.0/255.0 alpha:1.0] : [NSColor colorWithDeviceWhite:56.0/255.0 alpha:0.5];
}

@end

static NSString * const INWindowBackgroundPatternOverlayLayer = @"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";

static NSString * const INWindowBackgroundPatternOverlayLayer2x = @"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NgZ+IJCQ+M9lcAt9g2CBAJMZcs45U0+J1zol9dfdVTustfaumjnHNt++559//t/tdrs588wzN998883m119/3Zx99tnz/ueff24uvPDCzVlnnbX5999/5/P333+/+eOPPzb//PPP5pdfftkYP//88+bSSy+dfueff/7m/fffn7annXba5pxzztn8/vvv0/biiy/enH766fOZz0UXXTRjnXfeeZu//vpr89tvv20uueSSzddffz3zWffMRxz4rrrqqs0ZZ5wx5+W2Jn5YvfN77733pu2XX34533GC99xzz92YE8+FL+w//vjjBl7vy7Jsvv32281XX321ufXWWzc4f/TRR5MTfvK/++67myuvvHL67Nu3b4OD+LT87rvvTvGin3n6mqfBTz/9NDGZ84w7feF48803NxdccMG88pEP/k8//fRUDcSBH19YPdMBDs8//PDDfBYXZlzoVx2tX3bZZZOv55tvvnlqypaW7C+//PKJjeaff/45uTdXX331jKP+7PSGusFIOz74wvHGG29MLNdee+3EDiuuLnY4/P333xM/LF988cXUU3/R1lCXK664YuaklTX6wCmPvHSD5eOPP566ygMfP/2jruonpnx0iudMMv4HFnWOk3Xv+OkXMa3ByV8cOqsvO/bqBZPelh928+wNOMTTL2Lp02LRUO94p7GamDNgw5s++tfzwYMHp6bWYaE9XfQJfb3jjzdc8sJFL/lhg2F74MCBFYFPPvlkgr3xxhuns6QFZyg5oTkLqJEAuemmmyaAW2655VRxrQFgMxuIEK6mIqYGEAtgQgaOLQKKJae7/IBrKjFsJGISm2jiGBqVP0E0g/wJDE+N3CYRVwHZyEkHPviJa04hxVQUMTzjJ6e1a665Zh54eBDVHHw4wa55YFVsMfFQKLnEp6FYCmsj10jyiINL62LaBLRzx0MMMfGQQz5xPOPHhrY4ynvdddfNzYTPZ599Nu1t/rjHQ/1hctEZHtjkxcOFHy3M0w0Xl3yw15BsHAJ4iC+G+PQUQyyHGxuNb03DehaPj7rjyR7W66+/ftbc/+BMB3WTI1sY1AceMdRBTHiLTxfPfDzD5l2P0qhDhr7iw2ydxt7VGy510q/e8RWHZnRRB/vKnY+awGJPwK6u3g2bFmfzhrwGXvLQhq1eEws+3M3L5ZB3ibOXt1qrpRh8POO4feSRR1bABARa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";
