/*
 Copyright (c) 2011, OpenEmu Team
 
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

#import "OESetupAssistantScrollView.h"

@implementation OESetupAssistantScrollView

- (void)awakeFromNib{
    NSView *containerView = [[NSView alloc] initWithFrame:[self frame]];
    
    NSView *superView = [self superview];
    [superView replaceSubview:self with:containerView];
    [containerView addSubview:self];
    
    OESetupAssistantScrollBorderView *borderView = [[OESetupAssistantScrollBorderView alloc] initWithFrame:(NSRect){{0,0},[self frame].size}];
    [containerView addSubview:borderView positioned:NSWindowAbove relativeTo:self];
    [self setFrame:NSInsetRect(containerView.bounds, 2, 2)];    
}
@end

@implementation OESetupAssistantScrollBorderView
- (void)drawRect:(NSRect)dirtyRect
{
    if(NSMinX(dirtyRect) < 5 || NSMaxX(dirtyRect) > [self bounds].size.width-5
       || NSMinY(dirtyRect) < 5 || NSMaxY(dirtyRect) > [self bounds].size.height-5)
    {
        NSImage *img = [NSImage imageNamed:@"installer_scrollview_box"];
        [img drawInRect:[self bounds] fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil leftBorder:5 rightBorder:5 topBorder:5 bottomBorder:5];
    }
}
@end