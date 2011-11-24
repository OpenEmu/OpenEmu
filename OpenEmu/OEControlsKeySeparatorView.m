//
//  OEControlsKeySeperatorView.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 13.11.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#import "OEControlsKeySeparatorView.h"

@implementation OEControlsKeySeparatorView

- (BOOL)isOpaque
{
    return FALSE;
}

- (void)drawRect:(NSRect)dirtyRect
{
    const NSColor* topColor = [NSColor colorWithDeviceRed:85/255.0 green:45/255 blue:0.0 alpha:1.0];
    const NSColor* bottomColor = [NSColor colorWithDeviceRed:1.0 green:1.0 blue:0.0 alpha:0.2];
    
    NSRect lineRect = self.bounds;
    lineRect.size.height = 1;
    lineRect.origin.y = self.bounds.size.height/2;
    
    [topColor setFill];
    NSRectFill(lineRect);
    
    lineRect.origin.y -= 1;
    [bottomColor setFill];
    NSRectFillUsingOperation(lineRect, NSCompositeSourceOver);
}

@end
