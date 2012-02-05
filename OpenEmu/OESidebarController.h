//
//  SidebarController.h
//  SidebarViewTest
//
//  Created by Christoph Leimbrock on 16.05.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>

#import "OESidebarOutlineView.h"
@class OELibraryDatabase, OESidebarOutlineView;
@interface OESidebarController : NSViewController <NSOutlineViewDelegate, NSOutlineViewDataSource, OEDraggingDestinationDelegate>
{
@private
    NSArray *groups;
    OELibraryDatabase *database;
    
    NSArray *systems, *collections;
@private
    id editingItem;
}
- (IBAction)addCollectionAction:(id)sender;

- (void)setEnabled:(BOOL)enabled;

- (void)reloadData;
- (void)addCollection:(BOOL)isSmart;

- (void)selectItem:(id)item;
- (void)startEditingItem:(id)item;
- (void)expandCollections:(id)sender;
- (void)removeSelectedItemsOfOutlineView:(NSOutlineView*)outlineView;

- (void)willHide;
- (void)willShow;
@property (assign) OELibraryDatabase *database;
@property (retain, readwrite) NSArray *groups;
@property (retain, readwrite) NSArray *systems, *collections;

@property (nonatomic, retain, readwrite) id editingItem;
@end
