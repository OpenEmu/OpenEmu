//
//  OEControlerImageView.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 25.09.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OEControlerImageView.h"

@protocol OEControlsButtonHighlightProtocol  <NSObject>
- (NSPoint)highlightPoint;
@end
@interface OEControlerImageView (Priavte)
- (void)_setup;
@end
@implementation OEControlerImageView
@synthesize image, overlayAlpha, ringAlpha, ringPosition;

- (void)_setup{
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(selectedButtonChanged:) name:@"OEControlsPreferencesSelectedButtonDidChange" object:nil];
	
	self.wantsLayer = YES;
	self.overlayAlpha = 0.0;
	self.ringPosition = NSZeroPoint;
	self.ringAlpha = 0.0;
	
	CABasicAnimation* anim = [CABasicAnimation animation];
	CABasicAnimation* ringAnimation = [CABasicAnimation animation];
	ringAnimation.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseOut];
	
	ringAnimation.delegate = self;
	
    [self setWantsLayer:YES];
    [self setAnimations: [NSDictionary dictionaryWithObjectsAndKeys:ringAnimation, @"ringAlpha", ringAnimation, @"ringPosition", anim, @"overlayAlpha",  nil]];

}

- (id)init{
    self = [super init];
    if (self) {
		[self _setup];
	}
    return self;
}

- (id)initWithCoder:(NSCoder *)coder {
    self = [super initWithCoder:coder];
    if (self) {
		[self _setup];
    }
    return self;
}

- (id)initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
		[self _setup];
    }
    return self;
}
- (void)dealloc {
    self.image = nil;
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	
    [super dealloc];
}
#pragma mark -
- (void)drawRect:(NSRect)dirtyRect{
	if(!self.image) return;
	
	NSRect targetRect;
	targetRect.size = self.image.size;
	targetRect.origin = NSMakePoint((self.frame.size.width-image.size.width)/2, 0);
	
	[self.image drawInRect:targetRect fromRect:NSZeroRect operation:NSCompositeCopy fraction:1.0 respectFlipped:NO hints:[NSDictionary dictionaryWithObject:[NSNumber numberWithInt:NSImageInterpolationNone] forKey:NSImageHintInterpolation]];

	if(self.overlayAlpha != 0.0){
		[NSGraphicsContext saveGraphicsState];
		NSRect rect = NSMakeRect(targetRect.origin.x+ringPosition.x-33, targetRect.origin.y+ringPosition.y-33, 66.0, 66.0);
		
		NSBezierPath* path = [NSBezierPath bezierPathWithRect:self.bounds];
		[path setWindingRule:NSEvenOddWindingRule];
		[path appendBezierPathWithOvalInRect:rect];
		[path setClip];
		
		[[NSColor colorWithDeviceWhite:0.0 alpha:self.overlayAlpha] setFill];
		NSRectFillUsingOperation(self.bounds, NSCompositeSourceAtop);
		[NSGraphicsContext restoreGraphicsState];
	}
	
	if(self.ringAlpha != 0.0){
		NSPoint highlightP = NSMakePoint(targetRect.origin.x+ringPosition.x-38, targetRect.origin.y+ringPosition.y-45);
		NSImage* highlightImage = [NSImage imageNamed:@"controls_highlight"]; 
		[highlightImage drawAtPoint:highlightP fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:self.ringAlpha];
	}
	
}
- (void)setImage:(NSImage*)img{
	self.ringPosition = NSZeroPoint;
	self.ringAlpha = 0.0;
	
	[img retain];
	[image release];
	
	image = img;
	
	[self setNeedsDisplay:YES];
}

- (void)selectedButtonChanged:(NSNotification*)notification{
	NSPoint newHighlightPoint;
	id <OEControlsButtonHighlightProtocol> obj = (id <OEControlsButtonHighlightProtocol>)[notification object];
	if(!obj || ![obj respondsToSelector:@selector(highlightPoint)]){
		newHighlightPoint = NSZeroPoint;
	} else {
		newHighlightPoint = [obj highlightPoint];
	}
	
	[NSAnimationContext beginGrouping];
	if(NSEqualPoints(newHighlightPoint, NSZeroPoint)){
		[[self animator] setRingAlpha:0.0];
		[[self animator] setOverlayAlpha:0.0];
	} else if(NSEqualPoints(self.ringPosition, NSZeroPoint)){
		self.ringPosition = newHighlightPoint;
		[[self animator] setRingAlpha:1.0];
		[[self animator] setOverlayAlpha:0.3];
	} else {
		[[self animator] setRingPosition:newHighlightPoint];
		[[self animator] setRingAlpha:1.0];
		[[self animator] setOverlayAlpha:0.3];
	}
	[NSAnimationContext endGrouping];
}

- (void)setOverlayAlpha:(float)_overlayAlpha{
	overlayAlpha = _overlayAlpha;
	[self setNeedsDisplay:YES];
}

- (void)setRingAlpha:(float)_ringAlpha{
	ringAlpha = _ringAlpha;
	[self setNeedsDisplay:YES];
}

- (void)setRingPosition:(NSPoint)_highlightedButtonPoint{
	ringPosition = _highlightedButtonPoint;
	[self setNeedsDisplay:YES];
}

#pragma mark -
- (void)animationDidStart:(CAAnimation *)anim{}
- (void)animationDidStop:(CAAnimation *)anim finished:(BOOL)flag{
	if(self.ringAlpha == 0.0) self.ringPosition=NSZeroPoint;
}
@end
