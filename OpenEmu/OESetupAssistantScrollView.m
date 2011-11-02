//
//  OESetupAssistantScrollView.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 26.10.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OESetupAssistantScrollView.h"

@implementation OESetupAssistantScrollView

- (void)awakeFromNib{
    NSView* containerView = [[NSView alloc] initWithFrame:[self frame]];
    
    NSView* superView = [self superview];
    [superView replaceSubview:self with:containerView];
    [containerView addSubview:self];
    
    OESetupAssistantScrollBorderView* borderView = [[OESetupAssistantScrollBorderView alloc] initWithFrame:(NSRect){{0,0},self.frame.size}];
    [containerView addSubview:borderView positioned:NSWindowAbove relativeTo:self];
    [borderView release];
    [containerView release];
    [self setFrame:NSInsetRect(containerView.bounds, 2, 2)];    
}
@end

@implementation OESetupAssistantScrollBorderView
- (void)drawRect:(NSRect)dirtyRect
{
    if(NSMinX(dirtyRect) < 5 || NSMaxX(dirtyRect) > self.bounds.size.width-5
       || NSMinY(dirtyRect) < 5 || NSMaxY(dirtyRect) > self.bounds.size.height-5)
    {
        NSImage* img = [NSImage imageNamed:@"installer_scrollview_box"];
        [img drawInRect:self.bounds fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil leftBorder:5 rightBorder:5 topBorder:5 bottomBorder:5];
    }
}
@end