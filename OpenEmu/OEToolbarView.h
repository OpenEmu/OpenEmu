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
@interface OEToolbarView : INTitlebarView {
@private
    NSMutableArray* items;
	
	OEToolbarItem* highlightedItem;
	OEToolbarItem* selectedItem;
	
	NSColor* _contentSeperatorColor;
}

- (void)addItem:(OEToolbarItem*)item;
- (OEToolbarItem*)selectedItem;
- (void)markItemAsSelected:(OEToolbarItem*)tbItem; // basically selects a toolbar item without invoking its actions

@property (retain, readwrite) NSMutableArray* items;
@property (nonatomic, retain) NSColor* contentSeperatorColor;
@end

@interface OEToolbarItem : NSObject {
@private
	NSString* title;
	NSImage* icon;
	
	id target;
	SEL action;
	
	NSRect itemRect;
}
@property (readwrite) NSRect itemRect;

@property (copy, readwrite) NSString* title;
@property (copy, readwrite) NSImage* icon;
@property (assign, readwrite) id target;
@property (readwrite) SEL action;
@end
