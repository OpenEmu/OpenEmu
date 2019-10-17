/*
 Copyright (c) 2017, OpenEmu Team
 
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

#import "OEGameLayerView.h"
#import "NSColor+OEAdditions.h"
@import QuartzCore;

@interface CAContext : NSObject
+ (id)contextWithCGSConnection:(CAContextID)contextId options:(NSDictionary*)optionsDict;
@property(readonly) CAContextID contextId;
@property(retain) CALayer *layer;
@end

@interface CALayerHost : CALayer
@property CAContextID contextId;
@end

static NSString *const OEGameViewBackgroundColorKey = @"gameViewBackgroundColor";

@interface OEGameLayerView () <CALayerDelegate>
@end

/// View which hosts and resizes the helper app's game rendering.
/// TODO: If this was tvOS, we'd set a preferred frame rate here. Can we do that?
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

- (NSViewLayerContentsRedrawPolicy)layerContentsRedrawPolicy {
    return NSViewLayerContentsRedrawBeforeViewResize;
}

- (CALayer *)makeBackingLayer {
    CALayer *layer = [super makeBackingLayer];
    
    NSString *backgroundColorName = [[NSUserDefaults standardUserDefaults] objectForKey:OEGameViewBackgroundColorKey];
    NSColor *backgroundColor = backgroundColorName ? [NSColor colorFromString:backgroundColorName] : [NSColor blackColor];
    
    layer.contentsGravity = kCAGravityResize;
    layer.backgroundColor = (__bridge CGColorRef)backgroundColor;
    
    if (_remoteContextID)
    {
        [self updateTopLayer:layer withContextID:_remoteContextID];
    }
    return layer;
}

- (void)updateLayer {
    [super updateLayer];
    [self.delegate gameView:self updateBounds:self.bounds];
}

- (void)updateTopLayer:(CALayer *)layer withContextID:(CAContextID)remoteContextID
{
    if (!_remoteLayer) {
        _remoteLayer = [CALayerHost new];
        
        [layer addSublayer:_remoteLayer];
    }
    
    _remoteLayer.contextId = remoteContextID;
    _remoteLayer.delegate = self;
    [self updateLayer];
}

- (void)setScreenSize:(OEIntSize)newScreenSize aspectSize:(OEIntSize)newAspectSize
{
    _gameScreenSize = newScreenSize;
    _gameAspectSize = newAspectSize;
}

- (void)viewWillMoveToWindow:(NSWindow *)newWindow {
    [super viewWillMoveToWindow:newWindow];
}

#pragma mark - APIs

- (void)setRemoteContextID:(CAContextID)remoteContextID
{
    CALayer *layer = self.layer;
    
    _remoteContextID = remoteContextID;
    if (layer)
    {
        [self updateTopLayer:layer withContextID:_remoteContextID];
    }
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
        corrected = _gameScreenSize.width <= aspectSize.width ?
        NSMakeSize(screenSize.width / halfh, screenSize.height / halfw) :
        NSMakeSize(screenSize.width * halfw, screenSize.height * halfh);
    
    return corrected;
}

#pragma mark - NSResponder

- (BOOL)acceptsFirstMouse:(NSEvent *)theEvent
{
    // By default, AppKit tries to set the child window containing this view as its main & key window
    // upon first mouse. Since our child window shouldn’t behave like a window, we make its parent
    // window (the visible window from the user point of view) main and key.
    // See https://github.com/OpenEmu/OpenEmu/issues/365
    NSWindow *mainWindow = [[self window] parentWindow];
    if(mainWindow)
    {
        [mainWindow makeMainWindow];
        [mainWindow makeKeyWindow];
        return NO;
    }
    
    return [super acceptsFirstMouse:theEvent];
}

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

- (BOOL)layer:(CALayer *)layer shouldInheritContentsScale:(CGFloat)newScale fromWindow:(NSWindow *)window {
    [[self delegate] gameView:self updateBackingScaleFactor:newScale];
    return YES;
}
@end
