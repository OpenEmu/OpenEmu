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
@implementation OEPopupButton
@synthesize oemenu, dontOpenMenuOnNextMouseUp;

- (id)init{
    self = [super init];
    if (self) {}
    return self;
}
- (void)awakeFromNib{
    self.menu = [self menu];
}
- (void)dealloc{
    [self setOEMenu:nil];
    
    [super dealloc];
}
#pragma mark -
- (void)mouseDown:(NSEvent *)theEvent{
    BOOL outside = !NSPointInRect([theEvent locationInWindow], [self frame]);
    if(outside || [self oemenu].isVisible){
        [[self oemenu] closeMenuWithoutChanges:self];
        [[self window] makeFirstResponder:nil];
    }
    [[self cell] setHighlighted:YES];
}
- (void)mouseUp:(NSEvent *)theEvent
{
    if([self dontOpenMenuOnNextMouseUp])
    {
        [self setDontOpenMenuOnNextMouseUp:NO];
        return;
    }
    
    BOOL outside = !NSPointInRect([theEvent locationInWindow], [self frame]);
    if(!outside && ![[self oemenu] isVisible]){
        NSWindow *win = [self window];
        NSPoint location = [win convertBaseToScreen:[self frame].origin];
        
        location.y += ([self frame].size.height-[self oemenu].frame.size.height)/2;
        location.x += [self frame].size.width/2;
        [[self oemenu] openAtPoint:location ofWindow:win];
    }
    
    [[self cell] setHighlighted:NO]; 
}
#pragma mark -
- (void)setMenu:(NSMenu *)menu{
    [super setMenu:menu];
    [self setOEMenu:[[self menu] convertToOEMenu]];
    [[self oemenu] setPopupButton:self];
    [[self oemenu] setDelegate:self];
    
    NSSize minSize = [[self oemenu] minSize];
    minSize.width = [self frame].size.width;
    [[self oemenu] setMinSize:minSize];
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
