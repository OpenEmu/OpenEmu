//
//  NSImage+OEHighlight.m
//  OEPreferencesMockup
//
//  Created by Christoph Leimbrock on 01.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import "NSImage+OEHighlight.h"


@implementation NSImage (NSImage_OEHighlight)

- (NSImage*)imageForHighlight
{
    NSImage *highlightImage = [[[NSImage alloc] initWithSize:self.size] autorelease];
    
    [highlightImage lockFocus];
    
    [self drawInRect:NSMakeRect(0, 0, self.size.width, self.size.height) fromRect:NSZeroRect operation:NSCompositeCopy fraction:1.0];
    
    [[NSColor colorWithDeviceWhite:0.0 alpha:0.4] setFill];
    NSRectFillUsingOperation(NSMakeRect(0, 0, self.size.width, self.size.height), NSCompositeSourceAtop);
    
    [highlightImage unlockFocus];
    
    return highlightImage;
}
@end
