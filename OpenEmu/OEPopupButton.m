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

#import "OEPopupButton.h"

#define GapBetweenOpenMenuAndPopupButton 3
#define MenuContentBorder 4
@implementation OEPopupButton
@synthesize oemenu;
@synthesize menuOpenDate;
- (id)init{
    self = [super init];
    if (self) {}
    return self;
}
- (void)awakeFromNib{
    self.menu = [self menu];
}
#pragma mark -
- (void)mouseDown:(NSEvent *)theEvent{
    NSRect boundsInWindowCoord = [self convertRect:[self bounds] toView:nil];
    BOOL outside = !NSPointInRect([theEvent locationInWindow], boundsInWindowCoord);
    if(outside || [self oemenu].isVisible)
    {
        [[self oemenu] closeMenuWithoutChanges:self];
        [[self window] makeFirstResponder:nil];
    }
    else
    {
        NSSize minSize = [self frame].size;
        minSize.width = [self frame].size.width+2*(GapBetweenOpenMenuAndPopupButton);
        [[self oemenu] setMinSize:minSize];

        NSRect rect = boundsInWindowCoord;
        rect.origin.y += 2;
        
        [[self oemenu] openOnEdge:OENoEdge ofRect:rect ofWindow:[self window]];
        [[self window] makeFirstResponder:nil];

        [self setMenuOpenDate:[NSDate date]];
    }
}
- (void)mouseDragged:(NSEvent *)theEvent
{
    [[self oemenu] menuMouseDragged:theEvent];
}

- (void)mouseUp:(NSEvent *)theEvent
{
    if([[self menuOpenDate] timeIntervalSinceNow] < -0.1) 
        [[self oemenu] menuMouseUp:theEvent];
}
#pragma mark -
- (void)setMenu:(NSMenu *)menu{
    [super setMenu:menu];
    [self setOEMenu:[[self menu] convertToOEMenu]];
    [[self oemenu] setPopupButton:self];
    [[self oemenu] setDelegate:self];
}

- (NSString*)stringValue{
    return [selectedItem title];
}

- (id)objectValue{
    return [super objectValue];
}

- (NSString *)itemTitleAtIndex:(NSInteger)index{
    return [super itemTitleAtIndex:index];
}
- (NSInteger)selectedTag{
    return [[super selectedItem] tag];
}
#pragma mark - OEMenuDelegate
- (void)menuDidShow:(OEMenu *)men{
    [self setNeedsDisplay];
}
- (void)menuDidHide:(OEMenu *)men{
    [self setNeedsDisplay];
}
@end
