//
//  NSColor+IKSAdditions.h
//  Sonora
//
//  Created by Indragie Karunaratne on 10-11-15.
//  Copyright 2010 PCWiz Computer. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>

/**
 NSColor category for converting NSColor<-->CGColor
 */

@interface NSColor (IKSAdditions)
/**
 Return CGColor representation of the NSColor in the RGB color space
 */
@property (readonly) CGColorRef CGColor;
/**
 Create new NSColor from a CGColorRef
 */
+ (NSColor*)colorWithCGColor:(CGColorRef)aColor;
@end
