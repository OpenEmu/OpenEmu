//
//  NSColor+OEAdditions.h
//  OEGridViewDemo
//
//  Created by Faustino Osuna on 2/9/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <AppKit/AppKit.h>

@interface NSColor (OEAdditions)

+ (NSColor *)colorWithCGColor:(CGColorRef)color;
- (CGColorRef)CGColor;

@end
