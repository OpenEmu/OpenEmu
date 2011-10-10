//
//  LibraryController.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 30.03.11.
//  Copyright 2011 Christoph Leimbrock. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class OELibraryDatabase;
@class BackgroundColorView;
@class OESidebarController;
@class OECollectionViewController;
@class OELibrarySplitView;

@class FullscreenWindow;
@class OENewGameDocument;
#import "IKSGridView.h"
#import "OEUIDrawingUtils.h"

#import "IKSGridViewProtocols.h"
@protocol OENewGameDocumentDelegateProtocol;
@interface OELibraryController : NSWindowController <NSWindowDelegate, NSSplitViewDelegate, OENewGameDocumentDelegateProtocol> {
@private
    OELibraryDatabase* database;
    OENewGameDocument* gameDocument;
    
    IBOutlet NSWindow* libraryWindow;
    
    IBOutlet OELibrarySplitView	* mainSplitView;
    
    IBOutlet OESidebarController	* sidebarController;
    IBOutlet OECollectionViewController *collectionViewController;
    
    // Toolbar Items
    IBOutlet NSButton	* sidebarBtn;
	BOOL sidebarChangesWindowSize;
    
    IBOutlet NSButton	* gridViewBtn;	IBOutlet NSMenuItem* gridViewMenuItem;
    IBOutlet NSButton	* flowViewBtn;	IBOutlet NSMenuItem* flowViewMenuItem;
    IBOutlet NSButton	* listViewBtn;	IBOutlet NSMenuItem* listViewMenuItem;
    
    // MenuItems
    IBOutlet NSMenuItem* editSmartCollectionMenuItem;
    
    // Import Sheet
    IBOutlet NSWindow* importSheet;
    IBOutlet NSProgressIndicator* importProgress;
    IBOutlet NSTextField* importCurrentItem;
    BOOL cancelImport;
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
- (void) startImportSheet:(NSArray*)URLs;
- (IBAction) cancelImport:(id)sender;

- (IBAction)controlsmenu_startGame:(id)sender;

- (IBAction) switchToGridView:(id)sender;
- (IBAction) switchToListView:(id)sender;
- (IBAction) switchToFlowView:(id)sender;

// import finished
- (void) startImportSheet:(NSArray*)URLs;
- (void)sheetDidEnd:(NSWindow *)sheet returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo;


#pragma mark -
#pragma mark Properties
@property (assign, nonatomic) BOOL sidebarChangesWindowSize;
@property (assign) IBOutlet OELibrarySplitView* mainSplitView;
@property (retain, nonatomic) OELibraryDatabase* database;
@property (readwrite, assign) BOOL cancelImport;
@end
