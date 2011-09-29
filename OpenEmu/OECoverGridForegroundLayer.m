//
//  CoverGridForegroundLayer.m
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 04.04.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OECoverGridForegroundLayer.h"


@implementation OECoverGridForegroundLayer

- (id<CAAction>)actionForKey:(NSString *)event{
	return nil;
}

- (void)drawInContext:(CGContextRef)context
{	
//	CGContextScaleCTM(context, 1, -1);
//	CGContextTranslateCTM(context, 0, -[self bounds].size.height);
	
	//retreive visible rect
	NSRect visibleRect = [self bounds];
	
	// set up "shadow" gradient
	CGGradientRef gradient;
	CGColorSpaceRef colorspace;
	CGFloat locations[2] = { 0.0, 1.0 };
	CGFloat components[8] = { 0.0, 0.0, 0.0, 0.4,  // Start color
							  0.0, 0.0, 0.0, 0.0 }; // End color
	// Get colorspace
	colorspace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
	// create gradient
	gradient = CGGradientCreateWithColorComponents (colorspace, components, locations, 2);
	
	
	// define start and end of gradient for lower shadow (-> 6 pixels high)
	CGPoint startPoint = CGPointMake(0, 0);
	CGPoint endPoint = CGPointMake(0, 8);
	// draw shadow
	CGContextDrawLinearGradient (context, gradient, startPoint, endPoint, 0);
	
	// change points for upper shadow (six pixels, but from top now)
	startPoint.y = visibleRect.size.height;
	endPoint.y = visibleRect.size.height-8;
	// draw shadow
	CGContextDrawLinearGradient (context, gradient, startPoint, endPoint, 0);
	
	// clean up
	CGGradientRelease(gradient);
	CGColorSpaceRelease(colorspace);
}

@end
