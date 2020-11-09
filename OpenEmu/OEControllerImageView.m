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

#import "OEControllerImageView.h"

@import QuartzCore;

#import "OpenEmu-Swift.h"

#define OverlayAlphaON  0.5
#define OverlayAlphaOFF 0.0
#define RingRadius 37.0

NSString *const OEControlsDisableMouseSelection = @"controlsDisableMouseSelection";
NSString *const OEControlsDisableMouseDeactivation = @"controlsDisableMouseDeactivation";
NSString *const OEDebugDrawControllerMaskKey = @"drawControllerMask";

@protocol OEControlsButtonHighlightProtocol  <NSObject>
- (NSPoint)highlightPoint;
@end

@interface OEControllerImageView () <CAAnimationDelegate>
- (void)OE_commonControllerImageViewInit;
- (void)OE_setHighlightPoint:(NSPoint)value animated:(BOOL)animated;
- (NSPoint)OE_highlightPointForKey:(NSString *)aKey;
- (NSString *)OE_keyForHighlightPointClosestToPoint:(NSPoint)aPoint;
@end

@implementation OEControllerImageView
@synthesize image, overlayAlpha, ringAlpha, ringPosition;
@synthesize imageMask, keyPositions, selectedKey;
@synthesize target, action;

- (void)OE_commonControllerImageViewInit
{
    [self setWantsLayer:YES];
    [self setOverlayAlpha:OverlayAlphaOFF];
    [self setRingPosition:NSZeroPoint];
    [self setRingAlpha:0.0];
    
    CABasicAnimation *anim = [CABasicAnimation animation];
    CABasicAnimation *ringAnimation = [CABasicAnimation animation];
    ringAnimation.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseOut];
    ringAnimation.delegate = self;
    
    [self setWantsLayer:YES];
    [self setAnimations:@{
                          @"ringAlpha" : ringAnimation,
                          @"ringPosition" : ringAnimation,
                          @"overlayAlpha" : anim
                          }];
}

- (id)initWithCoder:(NSCoder *)coder 
{
    if((self = [super initWithCoder:coder]))
    {
        [self OE_commonControllerImageViewInit];
    }
    return self;
}

- (id)initWithFrame:(NSRect)frame 
{
    if((self = [super initWithFrame:frame]))
    {
        [self OE_commonControllerImageViewInit];
    }
    return self;
}

#pragma mark -

- (void)drawRect:(NSRect)dirtyRect
{
    if(![self image]) return;
    
    NSRect targetRect;
    targetRect.size = [self image].size;
    targetRect.origin = NSMakePoint(([self frame].size.width-image.size.width)/2, 0);
    
    [[self image] drawInRect:targetRect fromRect:NSZeroRect operation:NSCompositingOperationCopy fraction:1.0 respectFlipped:NO hints:@{ NSImageHintInterpolation: @(NSImageInterpolationNone) }];
    
    if([[NSUserDefaults standardUserDefaults] boolForKey:OEDebugDrawControllerMaskKey])
        [[self imageMask] drawInRect:targetRect fromRect:NSZeroRect operation:NSCompositingOperationSourceOver fraction:1.0 respectFlipped:NO hints:@{ NSImageHintInterpolation: @(NSImageInterpolationNone) }];

    
    if([self overlayAlpha] != OverlayAlphaOFF)
    {
        [NSGraphicsContext saveGraphicsState];
        NSRect rect = NSMakeRect(targetRect.origin.x+ringPosition.x-33, targetRect.origin.y+ringPosition.y-33, 66.0, 66.0);
        
        NSBezierPath *path = [NSBezierPath bezierPathWithRect:[self bounds]];
        [path setWindingRule:NSWindingRuleEvenOdd];
        [path appendBezierPathWithOvalInRect:rect];
        [path setClip];
        
        [[NSColor colorWithDeviceWhite:0.0 alpha:[self overlayAlpha]] setFill];
        NSRectFillUsingOperation([self bounds], NSCompositingOperationSourceAtop);
        [NSGraphicsContext restoreGraphicsState];
    }
    
    if([self ringAlpha] != 0.0)
    {
        NSPoint highlightP = NSMakePoint(targetRect.origin.x+ringPosition.x-38, targetRect.origin.y+ringPosition.y-45);
        NSImage *highlightImage = [NSImage imageNamed:@"controls_highlight"]; 
        [highlightImage drawAtPoint:highlightP fromRect:NSZeroRect operation:NSCompositingOperationSourceOver fraction:[self ringAlpha]];
    }
    
}

- (void)setImage:(NSImage *)img
{
    [self setRingPosition:NSZeroPoint];
    [self setRingAlpha:0.0];
    
    image = img;
    
    [self setNeedsDisplay:YES];
}

- (void)OE_setHighlightPoint:(CGPoint)value animated:(BOOL)animated;
{
    if(animated && ![NSThread isMainThread])
    {
        [self performSelectorOnMainThread:@selector(OE_animatedSetHighlightPointOnMainThread:) withObject:[NSValue valueWithPoint:value] waitUntilDone:NO];
        return;
    }
    
    OEControllerImageView *animator = animated ? [self animator] : self;
    
    if(animated) [NSAnimationContext beginGrouping];
    
    if(NSEqualPoints(value, NSZeroPoint))
    {
        [animator setRingAlpha:0.0];
        [animator setOverlayAlpha:OverlayAlphaOFF];
    } 
    else if(NSEqualPoints([self ringPosition], NSZeroPoint))
    {
        [self setRingPosition:value];
        [animator setRingAlpha:1.0];
        [animator setOverlayAlpha:OverlayAlphaON];
    } 
    else 
    {
        [animator setRingPosition:value];
        [animator setRingAlpha:1.0];
        [animator setOverlayAlpha:OverlayAlphaON];
    }
    
    if(animated) [NSAnimationContext endGrouping];
}

- (void)OE_animatedSetHighlightPointOnMainThread:(NSValue*)value
{
    CGPoint p = [value pointValue];
    [self OE_setHighlightPoint:p animated:YES];
}

- (void)setOverlayAlpha:(CGFloat)value
{
    overlayAlpha = value;
    [self setNeedsDisplay:YES];
}

- (void)setRingAlpha:(CGFloat)value
{
    ringAlpha = value;
    [self setNeedsDisplay:YES];
}

- (void)setRingPosition:(NSPoint)value
{
    ringPosition = value;
    [self setNeedsDisplay:YES];
}

- (void)setSelectedKey:(NSString *)value 
{ 
    [self setSelectedKey:value animated:NO]; 
}

- (void)setSelectedKey:(NSString *)value animated:(BOOL)animated;
{
    if([keyPositions objectForKey:value] == nil)
        value = nil;
    
    if(selectedKey != value)
    {
        selectedKey = [value copy];
        
        [self OE_setHighlightPoint:[self OE_highlightPointForKey:selectedKey] animated:animated];
    }
}

#define NSDistanceBetweenPoints(p1, p2) sqrt((p1.x-p2.x)*(p1.x-p2.x)+(p2.y-p1.y)*(p2.y-p1.y))
#define NSAddPoints(p1, p2) (NSPoint){p1.x+p2.x, p1.y+p2.y}
#define NSSubtractPoints(p1 ,p2) (NSPoint){p1.x-p2.x, p1.y-p2.y}

- (NSPoint)OE_highlightPointForKey:(NSString *)aKey;
{
    NSValue *value = [[self keyPositions] objectForKey:aKey];
    
    return value != nil ? [value pointValue] : NSZeroPoint;
}

- (NSString *)OE_keyForHighlightPointClosestToPoint:(NSPoint)aPoint;
{
    NSString *ret      = nil;
    CGFloat   distance = CGFLOAT_MAX;
    
    for(NSString *key in keyPositions)
    {
        NSPoint compared = [[keyPositions objectForKey:key] pointValue];
        
        CGFloat current = NSDistanceBetweenPoints(aPoint, compared);
        
        if(current < distance)
        {
            distance = current;
            ret      = key;
        }
    }
    
    return ret;
}

#pragma mark -
#pragma mark Interaction

- (void)mouseDown:(NSEvent *)theEvent
{
    NSUserDefaults *standardUserDefaults = [NSUserDefaults standardUserDefaults];
    BOOL allowDeactivationByMouse        = ![standardUserDefaults boolForKey:OEControlsDisableMouseDeactivation];
    BOOL allowSwitchingByMouse           = ![standardUserDefaults boolForKey:OEControlsDisableMouseSelection];
    
    if(!allowDeactivationByMouse && !allowSwitchingByMouse) return;
    
    NSString *selected = [self selectedKey];

    NSPoint event_location = [theEvent locationInWindow];
    NSPoint local_event_location = [self convertPoint:event_location fromView:nil];
    NSPoint ringLocation = [self ringPositionInView];
    
    NSRect targetRect;
    targetRect.size = [[self image] size];
    targetRect.origin = NSMakePoint(([self bounds].size.width - image.size.width) / 2, 0);
    
    if(allowSwitchingByMouse)
    {
        NSImage *maskImage = [self imageMask];
        BOOL selectAButton = [maskImage hitTestRect:(NSRect){local_event_location, {0,0}} withImageDestinationRect:targetRect context:nil hints:nil flipped:NO];
        
        if(selectAButton)
        {
            NSPoint locationOnController = NSSubtractPoints(local_event_location, targetRect.origin);
            selected = [self OE_keyForHighlightPointClosestToPoint:locationOnController];
        }
    }
    
    if(selected == [self selectedKey] && !NSEqualPoints(ringLocation, NSZeroPoint))
    {
        CGFloat distance = NSDistanceBetweenPoints(local_event_location, ringLocation);
        if(allowDeactivationByMouse && distance > RingRadius && NSPointInRect(local_event_location, [self bounds]))
            selected = nil;
    }
    
    if(selected != selectedKey)
    {
        [self setSelectedKey:selected animated:YES];
        if([self action] != nil) [NSApp sendAction:[self action] to:[self target] from:self];
    }
}
       
- (NSPoint)ringPositionInView
{
    NSPoint ringLocation = [self ringPosition];
    if(NSEqualPoints(ringLocation, NSZeroPoint) || NSEqualPoints(ringLocation, (NSPoint){0,0}))
        return NSZeroPoint;
    
    NSPoint offset = (NSPoint){([self frame].size.width-image.size.width)/2, 0};
    return NSAddPoints(offset, ringLocation);
}

#pragma mark -

- (void)animationDidStop:(CAAnimation *)anim finished:(BOOL)flag
{
    if([self ringAlpha] == 0.0) [self setRingPosition:NSZeroPoint];
}

@end
