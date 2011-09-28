//
//  CoverGridGlossLayer.m
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 30.04.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OECoverGridGlossLayer.h"
#import "NSColor+IKSAdditions.h"

@implementation OECoverGridGlossLayer

- (id)init{
    self = [super init];
    if (self) {
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
	CGRect workingFrame = CGRectMake(0, 0+1, width, height-1);
	
	[upperGlossImage drawInRect:NSRectFromCGRect(workingFrame) fromRect:NSZeroRect operation:NSCompositeSourceOut fraction:1.0 respectFlipped:YES hints:nil];
	
	// Add topline color
	workingFrame = CGRectInset(self.bounds, 0, 1);
	workingFrame.size.height = 1;
	
	CGContextSetFillColorWithColor(ctx, [[NSColor colorWithDeviceWhite:1 alpha:0.27] CGColor]);
	CGContextFillRect(ctx, workingFrame);
	
	workingFrame = CGRectInset([self bounds], 0.5, 0.5);;
	CGContextSetStrokeColorWithColor(ctx, [[NSColor blackColor] CGColor]);
	CGContextStrokeRect(ctx, workingFrame);
	
	[NSGraphicsContext restoreGraphicsState];	
}

#pragma mark -
#pragma mark Helpers
- (NSImage*)glossImage{
	static NSImage* uppperGlossImage = nil;
	if(uppperGlossImage == nil){;
		uppperGlossImage = [NSImage imageNamed:@"box_gloss"];
	}	
	return uppperGlossImage;
}
@end
