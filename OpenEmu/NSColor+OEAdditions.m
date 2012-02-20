//
//  NSColor+OEAdditions.m
//  OEGridViewDemo
//
//  Created by Faustino Osuna on 2/9/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "NSColor+OEAdditions.h"

@implementation NSColor (OEAdditions)

+ (NSColor *)colorWithCGColor:(CGColorRef)color
{
    const CGFloat *components = CGColorGetComponents(color);
    NSColorSpace *colorSpace = [[NSColorSpace alloc] initWithCGColorSpace:CGColorGetColorSpace(color)];
    NSColor *result = [NSColor colorWithColorSpace:colorSpace components:components count:CGColorGetNumberOfComponents(color)];

    return result;
}

- (CGColorRef)CGColor
{
    if([self isEqualTo:[NSColor blackColor]])
        return CGColorGetConstantColor(kCGColorBlack);
    if([self isEqualTo:[NSColor whiteColor]])
        return CGColorGetConstantColor(kCGColorWhite);
    if([self isEqualTo:[NSColor clearColor]])
        return CGColorGetConstantColor(kCGColorClear);

    NSColor *rgbColor = [self colorUsingColorSpaceName:NSCalibratedRGBColorSpace];
    CGFloat components[4];
    [rgbColor getComponents:components];

    CGColorSpaceRef theColorSpace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
    CGColorRef theColor = CGColorCreate(theColorSpace, components);
    CGColorSpaceRelease(theColorSpace);

    return (__bridge CGColorRef)CFBridgingRelease(theColor);
}

@end
