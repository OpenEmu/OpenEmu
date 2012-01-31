//
//  OEPopUpButton.m
//  OEPreferencesMockup
//
//  Created by Christoph Leimbrock on 04.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OEControlsPopupButton.h"
@implementation OEControlsPopupButton
@synthesize oemenu;

- (id)init{
    self = [super init];
    if (self) {}
    return self;
}
- (void)awakeFromNib{
    self.menu = [self menu];
}
- (void)dealloc{
    self.oemenu = nil;
    
    [super dealloc];
}
#pragma mark -
- (void)mouseDown:(NSEvent *)theEvent{
    BOOL outside = !NSPointInRect([theEvent locationInWindow], self.frame);
    if(outside || self.oemenu.isVisible){
        [self.oemenu closeMenuWithoutChanges:self];
        [[self window] makeFirstResponder:nil];
    } else {
        NSWindow* win = [self window];
        NSPoint location = [win convertBaseToScreen:self.frame.origin];
        
        location.y += (self.frame.size.height-self.oemenu.frame.size.height)/2;
        location.x += self.frame.size.width/2;
        [self.oemenu openAtPoint:location ofWindow:win];
    }
    [[self cell] setHighlighted:YES];
}
- (void)mouseUp:(NSEvent *)theEvent
{
    [[self cell] setHighlighted:NO]; 
}
#pragma mark -
- (void)setMenu:(NSMenu *)menu{
    [super setMenu:menu];
    self.oemenu = [[self menu] convertToOEMenu];
    self.oemenu.popupButton = self;
    self.oemenu.delegate = self;
    
    NSSize minSize = self.oemenu.minSize;
    minSize.width = self.frame.size.width;
    self.oemenu.minSize = minSize;
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
