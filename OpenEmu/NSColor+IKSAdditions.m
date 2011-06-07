//
//  NSColor+IKSAdditions.m
//  Sonora
//
//  Created by Indragie Karunaratne on 10-11-15.
//  Copyright 2010 PCWiz Computer. All rights reserved.
//

#import "NSColor+IKSAdditions.h"

@implementation NSColor (IKSAdditions)

- (CGColorRef)CGColor{
	NSColor *colorRGB = [self colorUsingColorSpaceName:NSCalibratedRGBColorSpace];
	CGFloat components[4];
	[colorRGB getRed:&components[0] green:&components[1] blue:&components[2] alpha:&components[3]];
	CGColorSpaceRef theColorSpace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
	CGColorRef theColor = CGColorCreate(theColorSpace, components);
	CGColorSpaceRelease(theColorSpace);
	return (CGColorRef)[(id)theColor autorelease];
}

+ (NSColor*)colorWithCGColor:(CGColorRef)aColor{
	const CGFloat *components = CGColorGetComponents(aColor);
	CGFloat red = components[0];
	CGFloat green = components[1];
	CGFloat blue = components[2];
	CGFloat alpha = components[3];
	return [self colorWithDeviceRed:red green:green blue:blue alpha:alpha];
}
@end
