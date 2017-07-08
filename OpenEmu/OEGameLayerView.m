//
//  OEGameLayerView.m
//  OpenEmu
//
//  Created by Alexander Strange on 4/19/16.
//
//

#import "OEGameLayerView.h"
#import "NSColor+OEAdditions.h"

NSString * const OEScreenshotAspectRatioCorrectionDisabled = @"disableScreenshotAspectRatioCorrection";
NSString * const OEDefaultVideoFilterKey = @"videoFilter";

@interface CAContext : NSObject
+ (id)contextWithCGSConnection:(CAContextID)contextId options:(NSDictionary*)optionsDict;
@property(readonly) CAContextID contextId;
@property(retain) CALayer *layer;
@end

@interface CALayerHost : CALayer
@property CAContextID contextId;
@end

static NSString *const OEGameViewBackgroundColorKey = @"gameViewBackgroundColor";

/*!
 * @abstract View which hosts and resizes the helper app's game rendering.
 * @description
 * The WindowServer takes care of drawing in the game's image, so this does no actual work.
 * TODO: Make sure game resizes when changing from/to HiDPI.
 * TODO: Tell the remote side to change pixel size after (not during) live resize - NSViewLayerContentsRedrawCrossfade?
 * TODO: Tell the remote side to change pixel size during setBounds:.
 */
@implementation OEGameLayerView
{
    CALayerHost *_remoteLayer;
	NSTrackingArea    *_trackingArea;
	OEIntSize          _gameScreenSize;
	OEIntSize          _gameAspectSize;
}

- (BOOL)wantsLayer {
    return YES;
}

- (BOOL)wantsUpdateLayer {
    return YES;
}

- (BOOL)isOpaque {
    return YES;
}

- (BOOL)canDrawSubviewsIntoLayer {
    return NO;
}

- (NSViewLayerContentsRedrawPolicy)layerContentsRedrawPolicy {
    return NSViewLayerContentsRedrawNever;
}

- (CALayer *)makeBackingLayer {
    CALayer *layer = [super makeBackingLayer];

    NSString *backgroundColorName = [[NSUserDefaults standardUserDefaults] objectForKey:OEGameViewBackgroundColorKey];
    NSColor *backgroundColor = backgroundColorName ? [NSColor colorFromString:backgroundColorName] : [NSColor blackColor];
    
    layer.contentsGravity = kCAGravityResize;
    layer.backgroundColor = (__bridge CGColorRef)backgroundColor;

    if (_remoteContextID) [self updateTopLayer:layer withContextID:_remoteContextID];

    return layer;
}

- (void)updateLayer {
    // Probably don't need to do anything.
    // TODO: catch the bounds change here
}

- (void)updateTopLayer:(CALayer *)layer withContextID:(CAContextID)remoteContextID
{
    if (!_remoteLayer) {
        _remoteLayer = [CALayerHost new];

        [layer addSublayer:_remoteLayer];
    }

    _remoteLayer.contextId = remoteContextID;
}

- (void)setScreenSize:(OEIntSize)newScreenSize aspectSize:(OEIntSize)newAspectSize
{
	_gameScreenSize = newScreenSize;
	_gameAspectSize = newAspectSize;
}

#pragma mark - APIs

- (void)setRemoteContextID:(CAContextID)remoteContextID
{
    CALayer *layer = self.layer;

    _remoteContextID = remoteContextID;
    if (layer) [self updateTopLayer:layer withContextID:_remoteContextID];
}

- (NSSize)correctScreenSize:(OEIntSize)screenSize forAspectSize:(OEIntSize)aspectSize returnVertices:(BOOL)flag
{
	// calculate aspect ratio
	CGFloat wr = (CGFloat) aspectSize.width / screenSize.width;
	CGFloat hr = (CGFloat) aspectSize.height / screenSize.height;
	CGFloat ratio = MAX(hr, wr);
	NSSize scaled = NSMakeSize((wr / ratio), (hr / ratio));
	
	CGFloat halfw = scaled.width;
	CGFloat halfh = scaled.height;
	
	NSSize corrected;
	
	if(flag)
		corrected = NSMakeSize(halfw, halfh);
	else
		corrected = NSMakeSize(screenSize.width / halfh, screenSize.height / halfw);
	
	return corrected;
}

#pragma mark - NSResponder

- (void)keyDown:(NSEvent *)event
{
}

- (void)keyUp:(NSEvent *)event
{
}

- (void)updateTrackingAreas
{
	[super updateTrackingAreas];
	
	[self removeTrackingArea:_trackingArea];
	_trackingArea = [[NSTrackingArea alloc] initWithRect:[self bounds] options:NSTrackingMouseEnteredAndExited | NSTrackingMouseMoved | NSTrackingActiveInKeyWindow owner:self userInfo:nil];
	[self addTrackingArea:_trackingArea];
}

- (void)setAspectSize:(OEIntSize)newAspectSize
{
	DLog(@"Set aspectsize to: %@", NSStringFromOEIntSize(newAspectSize));
	_gameAspectSize = newAspectSize;
}

- (OEEvent *)OE_mouseEventWithEvent:(NSEvent *)anEvent;
{
	CGRect  frame    = [self frame];
	CGPoint location = [anEvent locationInWindow];
	location = [self convertPoint:location fromView:nil];
	location.y = frame.size.height - location.y;
	NSSize screenSize = [self correctScreenSize:_gameScreenSize forAspectSize:_gameAspectSize returnVertices:NO];
	
	CGRect screenRect = { .size.width = screenSize.width, .size.height = screenSize.height };
	
	CGFloat scale = MIN(CGRectGetWidth(frame)  / CGRectGetWidth(screenRect),
						CGRectGetHeight(frame) / CGRectGetHeight(screenRect));
	
	screenRect.size.width  *= scale;
	screenRect.size.height *= scale;
	screenRect.origin.x     = CGRectGetMidX(frame) - CGRectGetWidth(screenRect)  / 2;
	screenRect.origin.y     = CGRectGetMidY(frame) - CGRectGetHeight(screenRect) / 2;
	
	location.x -= screenRect.origin.x;
	location.y -= screenRect.origin.y;
	
	OEIntPoint point = {
		.x = MAX(0, MIN(round(location.x * screenSize.width  / CGRectGetWidth(screenRect)) , screenSize.width )),
		.y = MAX(0, MIN(round(location.y * screenSize.height / CGRectGetHeight(screenRect)), screenSize.height))
	};
	
	return (id)[OEEvent eventWithMouseEvent:anEvent withLocationInGameView:point];
}

- (void)mouseDown:(NSEvent *)theEvent;
{
	[[self delegate] gameView:self didReceiveMouseEvent:[self OE_mouseEventWithEvent:theEvent]];
}

- (void)rightMouseDown:(NSEvent *)theEvent;
{
	[[self delegate] gameView:self didReceiveMouseEvent:[self OE_mouseEventWithEvent:theEvent]];
}

- (void)otherMouseDown:(NSEvent *)theEvent;
{
	[[self delegate] gameView:self didReceiveMouseEvent:[self OE_mouseEventWithEvent:theEvent]];
}

- (void)mouseUp:(NSEvent *)theEvent;
{
	[[self delegate] gameView:self didReceiveMouseEvent:[self OE_mouseEventWithEvent:theEvent]];
}

- (void)rightMouseUp:(NSEvent *)theEvent;
{
	[[self delegate] gameView:self didReceiveMouseEvent:[self OE_mouseEventWithEvent:theEvent]];
}

- (void)otherMouseUp:(NSEvent *)theEvent;
{
	[[self delegate] gameView:self didReceiveMouseEvent:[self OE_mouseEventWithEvent:theEvent]];
}

- (void)mouseMoved:(NSEvent *)theEvent;
{
	[[self delegate] gameView:self didReceiveMouseEvent:[self OE_mouseEventWithEvent:theEvent]];
}

- (void)mouseDragged:(NSEvent *)theEvent;
{
	[[self delegate] gameView:self didReceiveMouseEvent:[self OE_mouseEventWithEvent:theEvent]];
}

- (void)scrollWheel:(NSEvent *)theEvent;
{
	[[self delegate] gameView:self didReceiveMouseEvent:[self OE_mouseEventWithEvent:theEvent]];
}

- (void)rightMouseDragged:(NSEvent *)theEvent;
{
	[[self delegate] gameView:self didReceiveMouseEvent:[self OE_mouseEventWithEvent:theEvent]];
}

- (void)otherMouseDragged:(NSEvent *)theEvent;
{
	[[self delegate] gameView:self didReceiveMouseEvent:[self OE_mouseEventWithEvent:theEvent]];
}

- (void)mouseEntered:(NSEvent *)theEvent;
{
	[[self delegate] gameView:self didReceiveMouseEvent:[self OE_mouseEventWithEvent:theEvent]];
}

- (void)mouseExited:(NSEvent *)theEvent;
{
	[[self delegate] gameView:self didReceiveMouseEvent:[self OE_mouseEventWithEvent:theEvent]];
}


@end
