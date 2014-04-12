/*
 Copyright (c) 2012, OpenEmu Team
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 * Neither the name of the OpenEmu Team nor the
 names of its contributors may be used to endorse or promote products
 derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#import "OEAppStoreWindow.h"

@implementation OEAppStoreWindow
- (NSRect)contentRectForFrameRect:(NSRect)frameRect{
    NSRect rect = [super contentRectForFrameRect:frameRect];
    
    float adjustment = [self titleBarHeight] - 22;
    rect.size.height -= adjustment;
    
    return rect;
}

- (NSRect)frameRectForContentRect:(NSRect)contentRect
{
    NSRect rect = [super frameRectForContentRect:contentRect];
    
    float adjustment = [self titleBarHeight] - 22;
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
