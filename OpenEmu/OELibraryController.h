/*
 Copyright (c) 2011, OpenEmu Team
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the OpenEmu Team nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#import <Cocoa/Cocoa.h>

#import "OEUIDrawingUtils.h"
#import "OEMainWindowContentController.h"
#import "OELibraryDatabase.h"

extern NSString * const OESidebarVisibleKey;
extern NSString * const OESidebarWidthKey;
extern NSString * const OELastCollectionViewKey;

@class OELibraryDatabase;
@class OESidebarController;
@class OELibrarySplitView;

@protocol OELibraryControllerDelegate, OELibrarySubviewController;

@interface OELibraryController : NSViewController <OEMainWindowContentController>
- (void)layoutToolbarItems;
@property(unsafe_unretained) id<OELibraryControllerDelegate> delegate;

#pragma mark -
#pragma mark Toolbar Actions
- (IBAction)toggleSidebar:(id)sender;
- (IBAction)switchToGridView:(id)sender;
- (IBAction)switchToListView:(id)sender;
- (IBAction)switchToFlowView:(id)sender;
- (IBAction)search:(id)sender;
- (IBAction)changeGridSize:(id)sender;
- (IBAction)addCollectionAction:(id)sender;

#pragma mark -
#pragma mark Menu Item Actions
- (IBAction)newCollection:(id)sender;
- (IBAction)newSmartCollection:(id)sender;
- (IBAction)newCollectionFolder:(id)sender;

- (IBAction)editSmartCollection:(id)sender;
- (IBAction)addToLibrary:(id)sender;
- (IBAction)startGame:(id)sender;

#pragma mark -
#pragma mark Menu Items
- (BOOL)validateMenuItem:(NSMenuItem *)menuItem;

#pragma mark -
#pragma mark Properties
@property (nonatomic, getter=isSidebarVisible) BOOL sidebarVisible;
@property (nonatomic) BOOL sidebarChangesWindowSize;
@property (strong)    OELibraryDatabase *database;

@property (strong) IBOutlet OESidebarController         *sidebarController;
@property (strong) NSViewController <OELibrarySubviewController> *currentViewController;

@property (strong) IBOutlet OELibrarySplitView           *mainSplitView;
@property (strong) IBOutlet NSView                       *mainContentPlaceholderView;

@property (strong) IBOutlet NSButton      *toolbarSidebarButton;
@property (strong) IBOutlet NSButton      *toolbarGridViewButton;
@property (strong) IBOutlet NSButton      *toolbarFlowViewButton;
@property (strong) IBOutlet NSButton      *toolbarListViewButton;

@property (strong) IBOutlet NSButton       *toolbarAddToSidebarButton;
@property (strong) IBOutlet NSSearchField *toolbarSearchField;
@property (strong) IBOutlet NSSlider         *toolbarSlider;

@property (strong) NSBitmapImageRep *cachedSnapshot;
@end

@class OEDBGame, OEDBRom, OEDBSaveState;
@protocol OELibraryControllerDelegate <NSObject>
@optional
- (void)libraryController:(OELibraryController *)sender didSelectGame:(OEDBGame *)aGame;
- (void)libraryController:(OELibraryController *)sender didSelectRom:(OEDBRom *)aGame;
- (void)libraryController:(OELibraryController *)sender didSelectSaveState:(OEDBSaveState *)aSaveState;
@end
