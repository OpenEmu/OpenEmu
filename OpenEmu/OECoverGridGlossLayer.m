//
//  CoverGridGlossLayer.m
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 30.04.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OECoverGridGlossLayer.h"


@implementation OECoverGridGlossLayer

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
#pragma mark Drawing
- (void)drawInContext:(CGContextRef)ctx{
	NSGraphicsContext *graphicsContext = [NSGraphicsContext graphicsContextWithGraphicsPort:ctx flipped:YES];
	[NSGraphicsContext saveGraphicsState];
	[NSGraphicsContext setCurrentContext:graphicsContext];
	[[NSGraphicsContext currentContext] setCompositingOperation:NSCompositeSourceOver];
	
	NSImage* upperGlossImage = [self glossImage];
	
	float width = self.bounds.size.width;
	float height = width*0.6442;
	NSRect workingFrame = CGRectMake(0, 0+1, width, height-1);
	[upperGlossImage drawInRect:workingFrame fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil];
	
	// Add topline color
	workingFrame = NSInsetRect(self.bounds, 0, 1);
	workingFrame.size.height = 1;
	NSBezierPath* topHighlightLine = [NSBezierPath bezierPathWithRect:workingFrame];
	[[NSColor colorWithDeviceWhite:1 alpha:0.27] set];
	[topHighlightLine fill];
	
	// Draw black stroke
	workingFrame = [self bounds];
	
	[[NSColor blackColor] setStroke];
	NSBezierPath* borderPath = [NSBezierPath bezierPathWithRect:NSInsetRect(workingFrame, 0.5, 0.5)];
	[borderPath setLineWidth:1.0];
	[borderPath stroke];
	
	[NSGraphicsContext restoreGraphicsState];	
}

#pragma mark -
#pragma mark Helpers
- (NSImage*)glossImage{
	static NSImage* upperGlossImage = nil;
	if(upperGlossImage == nil){
		upperGlossImage = [NSImage imageNamed:@"box_gloss"];
	}	
	return upperGlossImage;
}
@end
