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


#import "OESidebarOutlineButtonCell.h"
#import "OESidebarOutlineView.h"
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
    
    if([controlView isKindOfClass:[OESidebarOutlineView class]] && [(OESidebarOutlineView*)controlView isDrawingAboveDropHighlight])
        [[(OESidebarOutlineView*)controlView dropBackgroundColor] setFill];
    else
        [[NSColor colorWithDeviceWhite:(63/255.0) alpha:1.0] setFill];
    NSRectFillUsingOperation(triangleRect, NSCompositeDestinationOver);
}

@end

