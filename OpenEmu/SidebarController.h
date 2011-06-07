//
//  SidebarController.h
//  SidebarViewTest
//
//  Created by Christoph Leimbrock on 16.05.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>

@class LibraryDatabase, SidebarOutlineView;
@interface SidebarController : NSObject <NSOutlineViewDelegate, NSOutlineViewDataSource>{
@private
    IBOutlet SidebarOutlineView* sidebarView;
	
	NSArray* groups;
	LibraryDatabase* database;
@private	
	id editingItem;
}


- (void)reloadData;
- (void)addCollection:(BOOL)isSmart;

@property (retain, readwrite) LibraryDatabase* database;
@property (retain, readwrite) NSArray* groups;

@property (nonatomic, retain, readwrite) id editingItem;
@end
