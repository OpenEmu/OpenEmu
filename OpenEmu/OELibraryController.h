//
//  LibraryController.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 30.03.11.
//  Copyright 2011 Christoph Leimbrock. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#import "IKSGridView.h"
#import "OEUIDrawingUtils.h"

#import "IKSGridViewProtocols.h"
#import "OEMainWindowContentController.h"
@class OELibraryDatabase;
@class BackgroundColorView;
@class OESidebarController;
@class OECollectionViewController;
@class OELibrarySplitView;
@class OEROMImporter;
@class FullscreenWindow;
@interface OELibraryController : OEMainWindowContentController 
{
@private
    OEROMImporter* romImporter;
    
    // Toolbar Items
    BOOL sidebarChangesWindowSize;
}
- (void)layoutToolbarItems;
- (id)initWithWindowController:(OEMainWindowController*)windowController andDatabase:(OELibraryDatabase*)database;

#pragma mark -
#pragma mark Toolbar Actions
- (IBAction)toggleSidebar:(id)sender;
- (IBAction)switchToGridView:(id)sender;
- (IBAction)switchToListView:(id)sender;
- (IBAction)switchToFlowView:(id)sender;
- (IBAction)search:(id)sender;

#pragma mark -
#pragma mark Menu Item Actions
- (IBAction)filemenu_newCollection:(id)sender;
- (IBAction)filemenu_newSmartCollection:(id)sender;
- (IBAction)filemenu_newCollectionFolder:(id)sender;

- (IBAction)filemenu_editSmartCollection:(id)sender;
- (IBAction)filemenu_addToLibrary:(id)sender;
- (IBAction)controlsmenu_startGame:(id)sender;
#pragma mark -
#pragma mark Menu Items
- (BOOL)validateMenuItem:(NSMenuItem *)menuItem;
- (void)menuItemAction:(id)sender;
#pragma mark -
#pragma mark Properties
@property (assign, nonatomic) BOOL sidebarChangesWindowSize;
@property (retain) OEROMImporter* romImporter;
@property (retain) OELibraryDatabase* database;

@property (retain) IBOutlet OESidebarController* sidebarController;
@property (retain) IBOutlet OECollectionViewController *collectionViewController;
@property (retain) IBOutlet OELibrarySplitView* mainSplitView;
@end
