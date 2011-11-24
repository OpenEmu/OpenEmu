//
//  OESidebarScrollView.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 24.11.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#import "OESidebarScrollView.h"

@implementation OESidebarScrollView
- (void)drawRect:(NSRect)dirtyRect
{
    [super drawRect:dirtyRect];
    
    [[NSColor colorWithDeviceWhite:0.19 alpha:1.0] setFill];
    NSRectFill([self bounds]);
    [[NSColor blackColor] setFill];
    NSRect blackBorderLine = [self bounds];
    
    blackBorderLine.origin.x += blackBorderLine.size.width-1;
    blackBorderLine.size.width = 1;
    
    NSRectFill(blackBorderLine);
}

@end
