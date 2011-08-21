//
//  CollectionViewController.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 24.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "IKSGridView.h"
#import "IKSGridViewProtocols.h"
#import "IKImageFlowView.h"

#import "CollectionViewItemProtocol.h"

@class LibraryDatabase;
@class OEHorizontalSplitView;
@interface CollectionViewController : NSViewController <IKSGridViewDelegate, IKSGridViewDataSource, NSTableViewDelegate, NSTableViewDataSource>{
@private
    LibraryDatabase	* database;
    NSArrayController	* gamesController;
    
    id <NSObject, CollectionViewItemProtocol> collectionItem;

    IBOutlet NSView			* gridViewContainer;	// gridview
    IBOutlet IKSGridView		* gridView;			// scrollview for gridview
    
    IBOutlet OEHorizontalSplitView	* flowlistViewContainer; // cover flow and simple list container
  //  IBOutlet IKImageFlowView		* coverFlowView;
    IBOutlet NSTableView			* listView;
    
    IBOutlet NSButton	* gridViewBtn;	IBOutlet NSMenuItem* gridViewMenuItem;
    IBOutlet NSButton	* flowViewBtn;	IBOutlet NSMenuItem* flowViewMenuItem;
    IBOutlet NSButton	* listViewBtn;	IBOutlet NSMenuItem* listViewMenuItem;
    
    IBOutlet NSSlider		* sizeSlider;
    IBOutlet NSSearchField	* searchField;
    /*
     *
     IBOutlet NSButton	*dbg_btn_colors;
     IBOutlet NSTextField* dbg_dnd_width;
     IBOutlet NSTextField* dbg_dnd_opstart, *dbg_dnd_opend, *dbg_dnd_delay;
     IBOutlet NSPopUpButton* dbg_layer_state;
     *
     */
}

- (void)finishSetup;

#pragma mark -
- (NSArray*)selectedGames;
#pragma mark -
#pragma mark View Selection
- (IBAction)selectGridView:(id)sender;
- (IBAction)selectFlowView:(id)sender;
- (IBAction)selectListView:(id)sender;

#pragma mark -
#pragma mark Toolbar Actions
- (IBAction)search:(id)sender;
- (IBAction)changeGridSize:(id)sender;

#pragma mark -
#pragma mark "Notifications" (not really)
- (void)willHide;
- (void)willShow;
#pragma mark -
@property (retain, readwrite) LibraryDatabase* database;
@property (nonatomic, retain) id <NSObject, CollectionViewItemProtocol> collectionItem;
@end
