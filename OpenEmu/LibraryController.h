//
//  LibraryController.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 30.03.11.
//  Copyright 2011 Christoph Leimbrock. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class LibraryDatabase;
@class BackgroundColorView;
@class SidebarController;
@class CollectionViewController;
@class LibrarySplitView;

@class FullscreenWindow;
@class OEGameViewController;
#import "IKSGridView.h"
#import "OEUIDrawingUtils.h"

#import "IKSGridViewProtocols.h"
@interface LibraryController : NSWindowController <NSWindowDelegate, NSSplitViewDelegate> {
@private
    LibraryDatabase* database;
    OEGameViewController* gameViewController;
    
    IBOutlet LibrarySplitView	* mainSplitView;
    
    IBOutlet SidebarController	* sidebarController;
    IBOutlet CollectionViewController *collectionViewController;
    
    // Toolbar Items
    IBOutlet NSButton	* sidebarBtn;
    
    IBOutlet NSButton	* gridViewBtn;	IBOutlet NSMenuItem* gridViewMenuItem;
    IBOutlet NSButton	* flowViewBtn;	IBOutlet NSMenuItem* flowViewMenuItem;
    IBOutlet NSButton	* listViewBtn;	IBOutlet NSMenuItem* listViewMenuItem;
    
    // MenuItems
    IBOutlet NSMenuItem* editSmartCollectionMenuItem;
}
#pragma mark -
#pragma mark Toolbar Actions
- (IBAction)toggleSidebar:(id)sender;

#pragma mark -
#pragma mark Menu Item Actions
- (IBAction)toggleWindow:(id)sender;

- (IBAction)filemenu_launchGame:(id)sender;

- (IBAction)filemenu_newCollection:(id)sender;
- (IBAction)filemenu_newSmartCollection:(id)sender;
- (IBAction)filemenu_newCollectionFolder:(id)sender;

- (IBAction)filemenu_editSmartCollection:(id)sender;

- (IBAction)filemenu_addToLibrary:(id)sender;
#pragma mark -
#pragma mark Properties
@property (assign) IBOutlet LibrarySplitView* mainSplitView;
@property (retain, nonatomic) LibraryDatabase* database;
@end
