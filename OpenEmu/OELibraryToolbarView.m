//
//  OELibraryToolbarView.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 25.07.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "OELibraryToolbarView.h"

@implementation OELibraryToolbarView

- (void)drawRect:(NSRect)dirtyRect
{
    [[NSColor blackColor] setFill];
    NSRectFill(dirtyRect);
    
    if(NSMinY(dirtyRect) > 44) return;
    
    NSRect viewRect = [self bounds];
    viewRect.origin.y = NSMinY(viewRect);
    viewRect.size.height = 44.0;
    
    NSColor *topLineColor   = [NSColor colorWithDeviceWhite:0.32 alpha:1];
    NSColor *gradientTop    = [NSColor colorWithDeviceWhite:0.20 alpha:1];
    NSColor *gradientBottom = [NSColor colorWithDeviceWhite:0.15 alpha:1];
    
    // Draw top line
    NSRect lineRect = NSMakeRect(0, 43, viewRect.size.width, 1);
    [topLineColor setFill];
    NSRectFill(lineRect);
    
    // Draw Gradient
    viewRect.origin.y = 0;
    viewRect.size.height -= 1;
    NSGradient *backgroundGradient = [[NSGradient alloc] initWithStartingColor:gradientTop endingColor:gradientBottom];
    [backgroundGradient drawInRect:viewRect angle:-90.0];
}

- (BOOL)isOpaque
{
    return NO;
}

@end
