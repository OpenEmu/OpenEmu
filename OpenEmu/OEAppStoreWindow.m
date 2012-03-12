//
//  OEAppStoreWindow.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 03.03.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "OEAppStoreWindow.h"

@implementation OEAppStoreWindow
- (NSRect)contentRectForFrameRect:(NSRect)frameRect{
    NSRect rect = [super contentRectForFrameRect:frameRect];
    
    float adjustment = self.titleBarHeight - 22;
    rect.size.height -= adjustment;
    
    return rect;
}

- (NSRect)frameRectForContentRect:(NSRect)contentRect
{
    NSRect rect = [super frameRectForContentRect:contentRect];
    
    float adjustment = self.titleBarHeight - 22;
    rect.size.height += adjustment;
    
    return rect;
}

#pragma mark -
#pragma mark Toolbar dragging fix
- (void)mouseDown:(NSEvent *)event
{
    if([event locationInWindow].y <= [self frame].size.height - [self titleBarHeight])
    {
        return [super mouseDown:event];
    }
    
	NSPoint originalMouseLocation = [self convertBaseToScreen:[event locationInWindow]];
	NSPoint originalFrameOrigin = [self frame].origin;
    while (YES)
	{
        NSEvent *newEvent = [self nextEventMatchingMask:(NSLeftMouseDraggedMask | NSLeftMouseUpMask)];
        if ([newEvent type] == NSLeftMouseUp)
			break;
		
		NSPoint newMouseLocation = [self convertBaseToScreen:[newEvent locationInWindow]];		
        NSPoint newFrameOrigin = originalFrameOrigin;
        
        newFrameOrigin.x += (newMouseLocation.x - originalMouseLocation.x);
        newFrameOrigin.y += (newMouseLocation.y - originalMouseLocation.y);
			
        [self setFrameOrigin:newFrameOrigin];
	}
}

@end
