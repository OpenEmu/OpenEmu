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
@protocol OECollectionViewItemProtocol;
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
- (id)addCollection:(BOOL)isSmart;

- (void)selectItem:(id)item;
- (void)startEditingItem:(id)item;
- (void)expandCollections:(id)sender;
- (void)removeSelectedItemsOfOutlineView:(NSOutlineView*)outlineView;

- (void)willHide;
- (void)willShow;

- (id<OECollectionViewItemProtocol>)selectedCollection;

@property (retain, nonatomic) OESidebarOutlineView *view;
@property (strong) OELibraryDatabase *database;
@property (retain, readwrite) NSArray *groups;
@property (retain, readwrite) NSArray *systems, *collections;
@property (nonatomic, strong, readwrite) id editingItem;
@end
