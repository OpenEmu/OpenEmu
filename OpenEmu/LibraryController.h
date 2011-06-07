//
//  LibraryController.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 30.03.11.
//  Copyright 2011 Christoph Leimbrock. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class LibraryDatabase;
@class OEBackgroundColorView;
@class SidebarController;
@class LibrarySplitView;
@class IKSGridView;

@class IKImageFlowView;

@class FullscreenWindow;
#import "IKSGridView.h"
#import "OEUIDrawingUtils.h"

#import "IKSGridViewProtocols.h"
@interface LibraryController : NSWindowController <NSTableViewDelegate, NSTableViewDataSource, NSSplitViewDelegate,IKSGridViewDelegate, IKSGridViewDataSource> {
@private
	// Main
	IBOutlet LibrarySplitView	* mainSplitView;
	
	// Sidebar
	IBOutlet SidebarController	* sidebarController;
	
	IBOutlet NSSearchField		* searchField;
	
	IBOutlet NSButton	* addCollectionBtn;
	
	IBOutlet NSButton	* sidebarBtn;

	IBOutlet NSButton	* gridViewBtn;
	IBOutlet NSButton	* flowViewBtn;
	IBOutlet NSButton	* listViewBtn;
	
	IBOutlet NSSlider	* sizeSlider;
	
	// Content
	IBOutlet NSView			* gameViewContainer;	// right side of splitview
	IBOutlet NSView			* gridViewContainer;	// gridview
	IBOutlet IKSGridView	* gridView;				// scrollview for gridview
	
	IBOutlet LibrarySplitView		* flowlistViewContainer; // cover flow and simple list container
	IBOutlet IKImageFlowView		* coverFlowView;
	IBOutlet NSTableView			* listView;
	
	// Random Stuff
//	FullscreenWindow* fsWindow;	// Invisible window, used to animate fullscreen
	
	LibraryDatabase* database; // Database to be used with this library
	
	BOOL allRomsArranged;
	
	NSArrayController			* romsController;
		
	IBOutlet NSButton	*dbg_btn_colors;
	IBOutlet NSTextField* dbg_dnd_width;
	IBOutlet NSTextField* dbg_dnd_opstart, *dbg_dnd_opend, *dbg_dnd_delay;
	IBOutlet NSPopUpButton* dbg_layer_state;
}
- (void)reloadData;
#pragma mark -
#pragma mark Toolbar Actions
- (IBAction)addCollection:(id)sender;
- (IBAction)toggleSidebar:(id)sender;
- (IBAction)selectView:(id)sender;
- (IBAction)search:(id)sender;
- (IBAction)changeViewSize:(id)sender;

#pragma mark -
#pragma mark Debug Actions
- (IBAction)debug_toggleGridFrame:(id)sender;
- (IBAction)debug_setArrowsPosition:(id)sender;
- (IBAction)debug_applyValues:(id)sender;
- (IBAction)debug_layerState:(id)sender;
#pragma mark -
#pragma mark Properties
@property (assign) IBOutlet LibrarySplitView* mainSplitView;

// Toolbar:
@property (assign) IBOutlet NSSearchField* searchField;

@property (assign) IBOutlet NSButton* addCollectionBtn;

@property (assign) IBOutlet NSButton* gridViewBtn;
@property (assign) IBOutlet NSButton* flowViewBtn;
@property (assign) IBOutlet NSButton* listViewBtn;

@property (assign) IBOutlet NSSlider* sizeSlider;

// Views
@property (assign) IBOutlet NSView* gameViewContainer;

@property (assign) IBOutlet LibrarySplitView* flowlistViewContainer;
@property (assign) IBOutlet IKImageFlowView	* coverFlowView;
@property (assign) IBOutlet NSTableView		* listView;

@property (assign) IBOutlet IKSGridView* gridView;
@property (assign) IBOutlet NSView* gridViewContainer;

@property (retain, nonatomic) LibraryDatabase* database;
@end
