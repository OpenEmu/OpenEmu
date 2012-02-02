//
//  OENewMenu.h
//  OpenEmu
//
//  Created by Carl Leimbrock on 28.11.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#import <AppKit/AppKit.h>
#import "NSWindow+OECustomWindow.h"
@class OEPopupButton;
@class OENewMenu;
@protocol OENewMenuDelegate <NSObject>
@optional
- (void)menuDidShow:(OENewMenu*)men;
- (void)menuDidHide:(OENewMenu*)men;
- (void)menuDidSelect:(OENewMenu*)men;
- (void)menuDidCancel:(OENewMenu*)men;
@end
@interface OENewMenu : NSWindow <OECustomWindow, NSTableViewDelegate, NSTableViewDataSource>
{
    NSTableView *upperTableView;
    NSTableView *mainTableView;
    NSTableView *lowerTableView;
    
    id _localMonitor;
    BOOL visible;
    BOOL closing;
}

- (void)openAtPoint:(NSPoint)p ofWindow:(NSWindow*)win;
- (void)closeMenuWithoutChanges:(id)sender;
- (void)closeMenu;
#pragma mark -
- (NSArray *)itemArray;
#pragma mark -
- (void)highlightItemAtPoint:(NSPoint)p;
- (id)itemAtPoint:(NSPoint)p;
@property int itemsAboveScroller, itemsBelowScroller;
@property (getter=isAlernate) BOOL alernate;

@property (retain) NSMenu *menu;
@property (retain) OENewMenu *supermenu;
@property (retain) OENewMenu *activeSubmenu;
@property (retain) OEPopupButton *popupButton;
@property (retain) NSMenuItem *highlightedItem;

@property (assign) id <OENewMenuDelegate> delegate;
@end

@interface NSMenu (OEOENewMenuAdditions)
- (OENewMenu*)convertToOENewMenu;
@end

