//
//  CoverGridSelectionLayer.m
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 02.05.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OECoverGridSelectionLayer.h"

#import "OEUIDrawingUtils.h"
#import "NSImage+OEDrawingAdditions.h"
@implementation OECoverGridSelectionLayer
@synthesize isInactive;
#pragma mark -
- (id)init{
    self = [super init];
    if (self) {
        self.needsDisplayOnBoundsChange = NO;
    }
    
    return self;
}

- (void)dealloc{
    [super dealloc];
}
#pragma mark -
- (void)drawInContext:(CGContextRef)ctx{	
	NSGraphicsContext *graphicsContext = [NSGraphicsContext graphicsContextWithGraphicsPort:ctx flipped:YES];
	[NSGraphicsContext saveGraphicsState];
	[NSGraphicsContext setCurrentContext:graphicsContext];
	[[NSGraphicsContext currentContext] setCompositingOperation:NSCompositeSourceOver];
	
	[NSGraphicsContext saveGraphicsState];
	NSBezierPath* path = [NSBezierPath bezierPathWithRoundedRect:NSInsetRect([self bounds], 2, 2) xRadius:5 yRadius:5];
	[path appendBezierPathWithRoundedRect:NSInsetRect([self bounds], 8, 8) xRadius:2 yRadius:2];
	[path setWindingRule:NSEvenOddWindingRule];
	[path addClip];
	
	NSShadow* shadow = [[[NSShadow alloc] init] autorelease];
	[shadow setShadowColor:[NSColor blackColor]];
	[shadow setShadowOffset:NSMakeSize(0, 2)];
	[shadow setShadowBlurRadius:3];
	[shadow set];
	
	NSColor* strokeColor;
	if(!self.isInactive){ // active
		strokeColor = [NSColor colorWithDeviceRed:0 green:0.125 blue:0.639 alpha:1.0];
	} else {				
		strokeColor = [NSColor colorWithDeviceWhite:0.10 alpha:1.0];
	}
	[strokeColor set];			
	[path fill];
	[NSGraphicsContext restoreGraphicsState];
	
	
	[NSGraphicsContext saveGraphicsState];
	path = [NSBezierPath bezierPathWithRoundedRect:NSInsetRect([self bounds], 3, 3) xRadius:4 yRadius:4];
	[path appendBezierPathWithRoundedRect:NSInsetRect([self bounds], 7, 7) xRadius:3 yRadius:3];
	[path setWindingRule:NSEvenOddWindingRule];
	[path addClip];
	
	NSColor* topColor;
	NSColor* bottomColor;
	if(!self.isInactive){ // active
		topColor = [NSColor colorWithDeviceRed:0.243 green:0.502 blue:0.871 alpha:1.0];
		bottomColor = [NSColor colorWithDeviceRed:0.078 green:0.322 blue:0.667 alpha:1.0];
	} else {
		topColor = [NSColor colorWithDeviceWhite:0.65 alpha:1.0];
		bottomColor = [NSColor colorWithDeviceWhite:0.44 alpha:1.0];
	}
	NSGradient* grad = [[[NSGradient alloc] initWithStartingColor:topColor endingColor:bottomColor] autorelease];
	[grad drawInBezierPath:path angle:90];
	[NSGraphicsContext restoreGraphicsState];
	
	
	[NSGraphicsContext saveGraphicsState];
	NSImage* image = !self.isInactive?[NSImage imageNamed:@"selector_ring_active"]:[NSImage imageNamed:@"selector_ring_inactive"];
	NSRect targetRect = [self bounds];
	[image drawInRect:targetRect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil leftBorder:14 rightBorder:14 topBorder:14 bottomBorder:14];
	
	[NSGraphicsContext restoreGraphicsState];
	
	
	[NSGraphicsContext restoreGraphicsState];
}

@end
