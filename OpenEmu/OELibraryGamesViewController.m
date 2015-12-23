/*
 Copyright (c) 2015, OpenEmu Team

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

#import "OELibraryGamesViewController.h"

#import "OEGameCollectionViewController.h"
#import "OESidebarController.h"
#import "OEGameScannerViewController.h"
#import "OEButton.h"
#import "OESearchField.h"
#import "OELibrarySplitView.h"
#import "OELibraryController.h"

#import "OEDBCollection.h"
#import "OEDBSystem.h"

#import "OESystemPlugin.h"

#import "OpenEmu-Swift.h"

#define MainMenu_View_GridTag      301
#define MainMenu_View_ListTag      303

NSString * const OESkipDiscGuideMessageKey = @"OESkipDiscGuideMessageKey";

@interface OELibraryGamesViewController ()
@property (nonatomic, weak) IBOutlet NSVisualEffectView *sidebarVisualEffectView;
@property BOOL issueResolverVisible;

- (NSString*)_stateStorageKeyForCollection:(id)representedObject;
@end

@implementation OELibraryGamesViewController
@synthesize libraryController = _libraryController;
@synthesize controlsToolbar;

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    self.sidebarVisualEffectView.material = NSVisualEffectMaterialSidebar;
    
    [self _assignLibraryController];

    NSNotificationCenter *noc = [NSNotificationCenter defaultCenter];
    [noc addObserver:self selector:@selector(_updateCollectionContentsFromSidebar:) name:OESidebarSelectionDidChangeNotificationName object:self.sidebarController];
    [noc addObserver:self selector:@selector(_hideIssueResolver:) name:OEGameScannerViewShouldHideNotificationName object:self.gameScannerController];

    NSView *collectionView = (self.collectionController).view;
    NSView *collectionViewContainer = self.collectionViewContainer;

    collectionView.frame = collectionViewContainer.bounds;
    collectionView.autoresizingMask = NSViewWidthSizable|NSViewHeightSizable;
    [collectionViewContainer addSubview:collectionView];

    [self.collectionController bind:@"controlsToolbar" toObject:self withKeyPath:@"controlsToolbar" options:nil];

    [self addChildViewController:self.sidebarController];
    [self addChildViewController:self.collectionController];
    
    [self _updateCollectionContentsFromSidebar:nil];
}

- (void)viewWillAppear
{
    [super viewWillAppear];

    self.view.needsDisplay = YES;
}

#pragma mark - OELibrarySubviewController

- (NSArray*)selectedGames
{
    return [self.collectionController selectedGames];
}

- (OELibraryController *)libraryController
{
    return _libraryController;
}

- (void)setLibraryController:(OELibraryController *)libraryController
{
    _libraryController = libraryController;
    [self _assignLibraryController];
}

- (void)_assignLibraryController
{
    (self.sidebarController).database = _libraryController.database;
    (self.collectionController).libraryController = _libraryController;
    self.gameScannerController.libraryController = _libraryController;
}

#pragma mark - Toolbar
- (IBAction)addCollectionAction:(id)sender
{
    [self.sidebarController addCollectionAction:sender];
}

- (IBAction)switchToGridView:(id)sender
{
    [self.collectionController switchToGridView:sender];
    
    // Update state of respective view menu items.
    NSMenu *viewMenu = [NSApp.mainMenu itemAtIndex:3].submenu;
    [viewMenu itemWithTag:MainMenu_View_GridTag].state = NSOnState;
    [viewMenu itemWithTag:MainMenu_View_ListTag].state = NSOffState;
}

- (IBAction)switchToListView:(id)sender
{
    [self.collectionController switchToListView:sender];
    
    // Update state of respective view menu items.
    NSMenu *viewMenu = [NSApp.mainMenu itemAtIndex:3].submenu;
    [viewMenu itemWithTag:MainMenu_View_GridTag].state = NSOffState;
    [viewMenu itemWithTag:MainMenu_View_ListTag].state = NSOnState;
}

- (IBAction)search:(id)sender
{
    [self.collectionController search:sender];
}

- (IBAction)changeGridSize:(id)sender
{
    [self.collectionController changeGridSize:sender];
}

#pragma mark - Sidebar handling
- (NSString*)_stateStorageKeyForCollection:(id)representedObject
{
    NSString *objectID = [(id <OESidebarItem>)representedObject sidebarID];
    return [@"Collection_" stringByAppendingString:objectID];
}

- (void)_updateCollectionContentsFromSidebar:(id)sender
{
    if(self.issueResolverVisible){
        [self _hideIssueResolver:nil];
    }

    if(self.collectionController.representedObject) {
        NSString *key = [self _stateStorageKeyForCollection:self.collectionController.representedObject];
        [self.collectionController storeStateWithKey:key];
    }

    id selectedItem = self.sidebarController.selectedSidebarItem;
    self.collectionController.representedObject = selectedItem;
    NSString *key = [self _stateStorageKeyForCollection:selectedItem];
    [self.collectionController restoreStateWithKey:key];
    [self.collectionController updateToolbar];

    // For empty collections of disc-based games, display an alert to compel the user to read the disc-importing guide.
    if ([selectedItem isKindOfClass:[OEDBSystem class]] &&
        ((OEDBSystem *)selectedItem).plugin.supportsDiscs &&
        ((OEDBSystem *)selectedItem).games.count == 0 &&
        ![[NSUserDefaults standardUserDefaults] boolForKey:OESkipDiscGuideMessageKey])
    {
        
        NSAlert *alert = [[NSAlert alloc] init];
        
        alert.messageText = NSLocalizedString(@"Have you read the guide?", @"");
        alert.informativeText = NSLocalizedString(@"Disc-based games have special requirements. Please read the disc importing guide.", @"");
        alert.alertStyle = NSInformationalAlertStyle;
        [alert addButtonWithTitle:NSLocalizedString(@"View Guide in Browser", @"")];
        [alert addButtonWithTitle:NSLocalizedString(@"Dismiss", @"")];
                
        [alert beginSheetModalForWindow:self.view.window completionHandler:^(NSModalResponse returnCode) {
            if(returnCode == NSAlertFirstButtonReturn) {
                NSURL *guideURL = [NSURL URLWithString:OECDBasedGamesUserGuideURLString];
                [[NSWorkspace sharedWorkspace] openURL:guideURL];
            }
        }];
    }
}

- (void)_hideIssueResolver:(id)sender
{
    if(!self.issueResolverVisible) return;

    [self.gameScannerController.view removeFromSuperview];

    self.libraryController.toolbar.categorySelector.enabled = YES;
    self.issueResolverVisible = NO;
    [self.collectionController bind:@"controlsToolbar" toObject:self withKeyPath:@"controlsToolbar" options:nil];
}

- (void)makeNewCollectionWithSelectedGames:(id)sender
{
    OECoreDataMainThreadAssertion();
    NSArray *selectedGames = self.selectedGames;
    OEDBCollection *collection = [self.sidebarController addCollection:NO];
    collection.games = [NSSet setWithArray:selectedGames];
    [collection save];

    [self.collectionController setNeedsReload];
}

#pragma mark - Issue Resolving
- (IBAction)showIssuesView:(id)sender
{
    if(self.issueResolverVisible) return;
    self.issueResolverVisible = YES;

    NSView *container  = self.collectionViewContainer;
    NSView *issuesView = self.gameScannerController.view;

    [container addSubview:issuesView positioned:NSWindowAbove relativeTo:NULL];
    issuesView.frame = container.bounds;
    
    // Disable toolbar controls.
    [self.collectionController unbind:@"controlsToolbar"];
    self.collectionController.controlsToolbar = NO;

    OELibraryToolbar *toolbar = self.libraryController.toolbar;
    toolbar.categorySelector.enabled = NO;
    toolbar.gridViewButton.enabled = NO;
    toolbar.listViewButton.enabled = NO;
    toolbar.gridSizeSlider.enabled = NO;
    toolbar.searchField.enabled = NO;
}
@end
