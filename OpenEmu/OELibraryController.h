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

@import Cocoa;

#import "OEMainWindowContentController.h"
#import "OELibrarySplitView.h"

@class OELibraryDatabase;
@class OESidebarController;
@class OELibrarySplitView;
@class OELibraryToolbar;
@class OEDBGame;

extern NSString * const OELibraryStatesKey;
extern NSString * const OELibraryLastCategoryKey;

@protocol OELibraryControllerDelegate, OELibrarySubviewController;
@interface OELibraryController : NSViewController <OEMainWindowContentController>

@property(unsafe_unretained) id<OELibraryControllerDelegate> delegate;

- (void)startSelectedGameWithSaveState:(id)stateItem;

#pragma mark - Toolbar Actions
- (IBAction)switchToGridView:(id)sender;
- (IBAction)switchToListView:(id)sender;
- (IBAction)search:(id)sender;
- (IBAction)changeGridSize:(id)sender;
- (IBAction)addCollectionAction:(id)sender;

#pragma mark - Menu Item Actions
- (IBAction)newCollection:(id)sender;
- (IBAction)newSmartCollection:(id)sender;
- (IBAction)newCollectionFolder:(id)sender;

#pragma mark - Menu Item Actions
- (IBAction)editSmartCollection:(id)sender;
- (IBAction)addToLibrary:(id)sender;
- (void)startGame:(OEDBGame*)game;
- (IBAction)startSelectedGame:(id)sender;
- (IBAction)startSaveState:(id)sender;

#pragma mark - Custom Views
- (IBAction)switchCategory:(id)sender;

#pragma mark - Menu Items
- (BOOL)validateMenuItem:(NSMenuItem *)menuItem;

#pragma mark - Properties
@property (strong) OELibraryDatabase *database;
@property (nonatomic, readonly) NSViewController <OELibrarySubviewController> *currentSubviewController;
@property (strong) IBOutlet OELibraryToolbar *toolbar;
@end

@class OEDBGame, OEDBRom, OEDBSaveState;
@protocol OELibraryControllerDelegate <NSObject>
@optional
- (void)libraryController:(OELibraryController *)sender didSelectGame:(OEDBGame *)aGame;
- (void)libraryController:(OELibraryController *)sender didSelectRom:(OEDBRom *)aGame;
- (void)libraryController:(OELibraryController *)sender didSelectSaveState:(OEDBSaveState *)aSaveState;
@end
