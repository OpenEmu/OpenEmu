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
#import "OEControlsViewController.h"
#import "OEControlsSetupView.h"
#import "NSImage+OEDrawingAdditions.h"
#define OverlayAlphaON  0.5
#define OverlayAlphaOFF 0.0
#define RingRadius 37.0
@protocol OEControlsButtonHighlightProtocol  <NSObject>
- (NSPoint)highlightPoint;
@end
@interface OEControllerImageView (Priavte)
- (void)_setup;
@end
@implementation OEControllerImageView
@synthesize image, overlayAlpha, ringAlpha, ringPosition;
@synthesize controlsViewController;

- (void)_setup
{
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(selectedButtonChanged:) name:@"OEControlsPreferencesSelectedButtonDidChange" object:nil];
    
    [self setWantsLayer:YES];
    self.overlayAlpha = OverlayAlphaOFF;
    [self setRingPosition:NSZeroPoint];
    [self setRingAlpha:0.0];
    
    CABasicAnimation *anim = [CABasicAnimation animation];
    CABasicAnimation *ringAnimation = [CABasicAnimation animation];
    ringAnimation.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseOut];
    ringAnimation.delegate = self;
    
    [self setWantsLayer:YES];
    [self setAnimations: [NSDictionary dictionaryWithObjectsAndKeys:ringAnimation, @"ringAlpha", ringAnimation, @"ringPosition", anim, @"overlayAlpha",  nil]];
    
}

- (id)init
{
    self = [super init];
    if (self) 
    {
        [self _setup];
    }
    return self;
}

- (id)initWithCoder:(NSCoder *)coder 
{
    self = [super initWithCoder:coder];
    if (self) {
        [self _setup];
    }
    return self;
}

- (id)initWithFrame:(NSRect)frame 
{
    self = [super initWithFrame:frame];
    if (self) 
    {
        [self _setup];
    }
    return self;
}
- (void)dealloc 
{
    [self setImage:nil];
    [self setControlsViewController:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    
    [super dealloc];
}
#pragma mark -
- (void)drawRect:(NSRect)dirtyRect
{
    if(![self image]) return;
    
    NSRect targetRect;
    targetRect.size = [self image].size;
    targetRect.origin = NSMakePoint(([self frame].size.width-image.size.width)/2, 0);
    
    [self.image drawInRect:targetRect fromRect:NSZeroRect operation:NSCompositeCopy fraction:1.0 respectFlipped:NO hints:NoInterpol];
    
    if([[NSUserDefaults standardUserDefaults] boolForKey:UDDebugDrawControllerMaskKey])
        [[[self controlsViewController] controllerImageMask] drawInRect:targetRect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:NO hints:NoInterpol];

    
    if([self overlayAlpha] != OverlayAlphaOFF)
    {
        [NSGraphicsContext saveGraphicsState];
        NSRect rect = NSMakeRect(targetRect.origin.x+ringPosition.x-33, targetRect.origin.y+ringPosition.y-33, 66.0, 66.0);
        
        NSBezierPath *path = [NSBezierPath bezierPathWithRect:[self bounds]];
        [path setWindingRule:NSEvenOddWindingRule];
        [path appendBezierPathWithOvalInRect:rect];
        [path setClip];
        
        [[NSColor colorWithDeviceWhite:0.0 alpha:[self overlayAlpha]] setFill];
        NSRectFillUsingOperation([self bounds], NSCompositeSourceAtop);
        [NSGraphicsContext restoreGraphicsState];
    }
    
    if([self ringAlpha] != 0.0)
    {
        NSPoint highlightP = NSMakePoint(targetRect.origin.x+ringPosition.x-38, targetRect.origin.y+ringPosition.y-45);
        NSImage *highlightImage = [NSImage imageNamed:@"controls_highlight"]; 
        [highlightImage drawAtPoint:highlightP fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:[self ringAlpha]];
    }
    
}
- (void)setImage:(NSImage*)img
{
    [self setRingPosition:NSZeroPoint];
    [self setRingAlpha:0.0];
    
    [img retain];
    [image release];
    
    image = img;
    
    [self setNeedsDisplay:YES];
}

- (void)selectedButtonChanged:(NSNotification*)notification
{
    NSPoint newHighlightPoint;
    id <OEControlsButtonHighlightProtocol> selectedButton = (id <OEControlsButtonHighlightProtocol>)[notification object];
    if(!selectedButton || ![selectedButton respondsToSelector:@selector(highlightPoint)])
    {
        newHighlightPoint = NSZeroPoint;
    } 
    else 
    {
        newHighlightPoint = [selectedButton highlightPoint];
    }
    
    [NSAnimationContext beginGrouping];
    if(NSEqualPoints(newHighlightPoint, NSZeroPoint))
    {
        [[self animator] setRingAlpha:0.0];
        [[self animator] setOverlayAlpha:OverlayAlphaOFF];
    } 
    else if(NSEqualPoints([self ringPosition], NSZeroPoint))
    {
        [self setRingPosition:newHighlightPoint];
        [[self animator] setRingAlpha:1.0];
        [[self animator] setOverlayAlpha:OverlayAlphaON];
    } 
    else 
    {
        [[self animator] setRingPosition:newHighlightPoint];
        [[self animator] setRingAlpha:1.0];
        [[self animator] setOverlayAlpha:OverlayAlphaON];
    }
    [NSAnimationContext endGrouping];
}

- (void)setOverlayAlpha:(float)_overlayAlpha
{
    overlayAlpha = _overlayAlpha;
    [self setNeedsDisplay:YES];
}

- (void)setRingAlpha:(float)_ringAlpha
{
    ringAlpha = _ringAlpha;
    [self setNeedsDisplay:YES];
}

- (void)setRingPosition:(NSPoint)_highlightedButtonPoint
{
    ringPosition = _highlightedButtonPoint;
    [self setNeedsDisplay:YES];
}
#pragma mark -
#pragma mark Interaction
#define NSDistanceBetweenPoints(p1, p2) sqrt((p1.x-p2.x)*(p1.x-p2.x)+(p2.y-p1.y)*(p2.y-p1.y))
#define NSAddPoints(p1, p2) (NSPoint){p1.x+p2.x, p1.y+p2.y}
#define NSSubtractPoints(p1 ,p2) (NSPoint){p1.x-p2.x, p1.y-p2.y}
- (void)mouseDown:(NSEvent *)theEvent
{
    NSUserDefaults *standardUserDefaults = [NSUserDefaults standardUserDefaults];
    BOOL allowDeactivationByMouse = ![standardUserDefaults boolForKey:UDControlsDisableMouseDeactivation];
    BOOL allowSwitchingByMouse = ![standardUserDefaults boolForKey:UDControlsDisableMouseDeactivation];
    
    if(!allowDeactivationByMouse && !allowSwitchingByMouse) return;    
    id controlToSelect = [[self controlsViewController] selectedControl];
    
    NSPoint event_location = [theEvent locationInWindow];
    NSPoint local_event_location = [self convertPoint:event_location fromView:nil];
    NSPoint ringLocation = [self ringPositionInView];
    
    NSRect targetRect;
    targetRect.size = self.image.size;
    targetRect.origin = NSMakePoint(([self frame].size.width-image.size.width)/2, 0);
    
    
    if(allowSwitchingByMouse)
    {
        NSImage *maskImage = [[self controlsViewController] controllerImageMask];
        BOOL selectAButton = [maskImage hitTestRect:(NSRect){local_event_location, {0,0}} withImageDestinationRect:targetRect context:nil hints:nil flipped:NO];
        if(selectAButton)
        {
            OEControlsSetupView *controlsSetupView = (OEControlsSetupView*)[[self controlsViewController] view];
            NSPoint locationOnController = NSSubtractPoints(local_event_location, targetRect.origin);
            controlToSelect = [controlsSetupView controllerButtonClosestTo:locationOnController];
        } 
        else
            controlToSelect = [[self controlsViewController] selectedControl];
    
    }
    if(controlToSelect == [[self controlsViewController] selectedControl] && !NSEqualPoints(ringLocation, NSZeroPoint))
    {
        float distance = NSDistanceBetweenPoints(local_event_location, ringLocation);
        if(distance > RingRadius && [[self image] hitTestRect:(NSRect){local_event_location, {0,0}} withImageDestinationRect:targetRect context:nil hints:nil flipped:NO])
            controlToSelect = nil;
    }
    
    if(controlToSelect != [[self controlsViewController] selectedControl])
    {
        [controlToSelect setState:NSOnState];
       [[self controlsViewController] selectInputControl:controlToSelect];
    }
}
       
- (NSPoint)ringPositionInView{
    NSPoint ringLocation = [self ringPosition];
    if(NSEqualPoints(ringLocation, NSZeroPoint) || NSEqualPoints(ringLocation, (NSPoint){0,0}))
        return NSZeroPoint;
    
    NSPoint offset = (NSPoint){([self frame].size.width-image.size.width)/2, 0};
    return NSAddPoints(offset, ringLocation);
    
}
#pragma mark -
- (void)animationDidStart:(CAAnimation *)anim{}
- (void)animationDidStop:(CAAnimation *)anim finished:(BOOL)flag
{
    if([self ringAlpha] == 0.0) [self setRingPosition:NSZeroPoint];
}
@end
