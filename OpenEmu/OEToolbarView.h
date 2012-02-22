//
//  OEToolbarView.h
//  OEPreferencesMockup
//
//  Created by Christoph Leimbrock on 31.05.11.
//  Copyright 2011 none. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "INAppStoreWindow.h"
@class OEToolbarItem;
@interface OEToolbarView : INTitlebarView
{
@private
    NSMutableArray *items;
    
    OEToolbarItem *highlightedItem;
    OEToolbarItem *selectedItem;
    
    NSColor *_contentseparatorColor;
}

- (void)addItem:(OEToolbarItem*)item;
- (OEToolbarItem*)selectedItem;
- (NSUInteger)selectedItemIndex;
- (void)markItemAsSelected:(OEToolbarItem*)tbItem; // basically selects a toolbar item without invoking its actions

// FIXME: This is atrotious !
@property(strong) NSMutableArray *items;
@property(nonatomic, strong) NSColor *contentseparatorColor;
@end

@interface OEToolbarItem : NSObject
@property NSRect itemRect;

@property(copy) NSString *title;
@property(copy) NSImage *icon;
@property(unsafe_unretained) id target;
@property SEL action;
@end
