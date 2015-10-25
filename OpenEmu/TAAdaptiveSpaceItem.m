//
//  TAAdaptiveSpaceItem.m
//  TAAdaptiveSpaceItem
//
//  Created by Timothy Armes on 17/02/2014.
//  Copyright (c) 2014 Timothy Armes. All rights reserved.
//

#import "TAAdaptiveSpaceItem.h"
#import "TAAdaptiveSpaceItemView.h"

@implementation TAAdaptiveSpaceItem

- (void)awakeFromNib
{
    TAAdaptiveSpaceItemView *adaptiveSpaceItemView = [[TAAdaptiveSpaceItemView alloc] initWithFrame:NSMakeRect(0, 0, 1, 1)];
    adaptiveSpaceItemView.adaptiveSpaceItem = self;
    self.view = adaptiveSpaceItemView;
}

- (NSString *)label
{
    return @"";
}

- (NSString *)paletteLabel
{
    return NSLocalizedString(@"Adaptive Space Item", @"Palette name when customising toolbar");
}

- (NSSize)minSize
{
    NSArray *items = [self.toolbar items];
    NSInteger index = [items indexOfObject:self];
    
    if (index != NSNotFound) {
        NSRect thisFrame = self.view.superview.frame;
        if (thisFrame.origin.x > 0) {
            
            CGFloat space = 0;
            if (items.count > index + 1) {
            
                NSToolbarItem *nextItem = [items objectAtIndex:index + 1];
                NSRect nextFrame = nextItem.view.superview.frame;
                NSRect toolbarFrame = nextItem.view.superview.superview.frame;
                
                space = (toolbarFrame.size.width - nextFrame.size.width) / 2 - thisFrame.origin.x - 6;
                if (space < 0)
                    space = 0;
            }
            
            NSSize size = [super minSize];
            return NSMakeSize(space, size.height);
        }
    }
    
    return [super minSize];
}

- (NSSize)maxSize
{
    NSSize size = [super maxSize];
    return NSMakeSize([self minSize].width, size.height);
}

- (void)updateWidth
{
    [self setMinSize:[self minSize]];
    [self setMaxSize:[self maxSize]];
}

@end
