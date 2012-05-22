//
//  OESidebarOutlineButtonCell.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 30.01.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "OESidebarOutlineButtonCell.h"
#import "NSImage+OEDrawingAdditions.h"
@implementation OESidebarOutlineButtonCell
+ (void)initialize
{
    if(self != [OESidebarOutlineButtonCell class])
        return;
    
    NSImage *image = [NSImage imageNamed:@"sidebar_triangle"];
    [image setName:@"sidebar_triangle_closed" forSubimageInRect:NSMakeRect(0, 0, 9, 9)];
    [image setName:@"sidebar_triangle_open" forSubimageInRect:NSMakeRect(9, 0, 9, 9)];
}
- (id)init {
    self = [super init];
    if (self) {
    }
    return self;
}

- (void)drawWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
    [([self isHighlighted]?[NSColor colorWithDeviceWhite:0.4 alpha:1.0]:[NSColor colorWithDeviceWhite:0.6 alpha:1.0]) setFill];
    NSRect triangleRect = (NSRect){{cellFrame.origin.x+round((cellFrame.size.width-9)/2),cellFrame.origin.y+round((cellFrame.size.height-9)/2)},{9,9}};
    
    NSRectFill(triangleRect);
    NSImage *triangleImage = [self state]==NSOnState?[NSImage imageNamed:@"sidebar_triangle_open"]:[NSImage imageNamed:@"sidebar_triangle_closed"];
    [triangleImage drawInRect:triangleRect fromRect:NSZeroRect operation:NSCompositeDestinationAtop fraction:1 respectFlipped:YES hints:nil];
    
    NSRect shadowRect = triangleRect;
    shadowRect.origin.y -= 1;
    [triangleImage drawInRect:shadowRect fromRect:NSZeroRect operation:NSCompositeDestinationOver fraction:1 respectFlipped:YES hints:nil]; 
    
    [[NSColor colorWithDeviceWhite:0.19 alpha:1.0] setFill];
    NSRectFillUsingOperation(triangleRect, NSCompositeDestinationOver);
}

@end

