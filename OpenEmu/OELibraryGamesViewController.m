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
@end

@implementation OELibraryGamesViewController
@synthesize libraryController = _libraryController;

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    self.sidebarVisualEffectView.material = NSVisualEffectMaterialSidebar;
    
    [self _assignLibraryController];

    NSNotificationCenter *noc = [NSNotificationCenter defaultCenter];
    [noc addObserver:self selector:@selector(_updateCollectionContentsFromSidebar:) name:OESidebarSelectionDidChangeNotificationName object:[self sidebarController]];

    NSView *collectionView = [[self collectionController] view];
    NSView *collectionViewContainer = [self collectionViewContainer];

    [collectionView setFrame:[collectionViewContainer bounds]];
    [collectionView setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];
    [collectionViewContainer addSubview:collectionView];

    [self addChildViewController:[self sidebarController]];
    [self addChildViewController:[self collectionController]];
    
    [self _updateCollectionContentsFromSidebar:nil];
}

- (void)viewWillAppear
{
    [super viewWillAppear];
    
    [self _setupToolbar];

    self.view.needsDisplay = YES;
}

- (void)viewDidAppear
{
    [super viewDidAppear];
    [self.collectionController updateBlankSlate];
}

- (void)_setupToolbar
{
    OELibraryController *libraryController = self.libraryController;
    OELibraryToolbar *toolbar = libraryController.toolbar;

    toolbar.gridSizeSlider.enabled = YES;
    toolbar.gridViewButton.enabled = YES;
    toolbar.listViewButton.enabled = YES;
    
    OECollectionViewControllerViewTag selectedViewTag = self.collectionController.selectedViewTag;
    BOOL setGridView = selectedViewTag == OEGridViewTag || selectedViewTag == OEBlankSlateTag;
    toolbar.gridViewButton.state = setGridView ? NSOnState : NSOffState;
    toolbar.listViewButton.state = !setGridView ? NSOnState : NSOffState;

    NSSearchField *field = toolbar.searchField;
    field.searchMenuTemplate = nil;
    field.enabled = NO;
    field.stringValue = @"";
}

#pragma mark - OELibrarySubviewController

- (NSArray*)selectedGames
{
    return [[self collectionController] selectedGames];
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
    [[self sidebarController] setDatabase:[_libraryController database]];
    [[self collectionController] setLibraryController:_libraryController];
    [[self gameScannerController] setLibraryController:_libraryController];
}

#pragma mark - Toolbar
- (IBAction)addCollectionAction:(id)sender
{
    [[self sidebarController] addCollectionAction:sender];
}

- (IBAction)switchToGridView:(id)sender
{
    [[self collectionController] switchToGridView:sender];
    
    // Update state of respective view menu items.
    NSMenu *viewMenu = [[[NSApp mainMenu] itemAtIndex:3] submenu];
    [[viewMenu itemWithTag:MainMenu_View_GridTag] setState:NSOnState];
    [[viewMenu itemWithTag:MainMenu_View_ListTag] setState:NSOffState];
}

- (IBAction)switchToListView:(id)sender
{
    [[self collectionController] switchToListView:sender];
    
    // Update state of respective view menu items.
    NSMenu *viewMenu = [[[NSApp mainMenu] itemAtIndex:3] submenu];
    [[viewMenu itemWithTag:MainMenu_View_GridTag] setState:NSOffState];
    [[viewMenu itemWithTag:MainMenu_View_ListTag] setState:NSOnState];
}

- (IBAction)search:(id)sender
{
    [[self collectionController] search:sender];
}

- (IBAction)changeGridSize:(id)sender
{
    [[self collectionController] changeGridSize:sender];
}

#pragma mark - Sidebar handling
- (void)_updateCollectionContentsFromSidebar:(id)sender
{
    id selectedItem = self.sidebarController.selectedSidebarItem;
    self.collectionController.representedObject = selectedItem;
    
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
        [alert addButtonWithTitle:NSLocalizedString(@"View Guide In Browser", @"")];
        [alert addButtonWithTitle:NSLocalizedString(@"Dismiss", @"")];
                
        [alert beginSheetModalForWindow:self.view.window completionHandler:^(NSModalResponse returnCode) {
            if(returnCode == NSAlertFirstButtonReturn) {
                NSURL *guideURL = [NSURL URLWithString:OECDBasedGamesUserGuideURLString];
                [[NSWorkspace sharedWorkspace] openURL:guideURL];
            }
        }];
    }
}

- (void)makeNewCollectionWithSelectedGames:(id)sender
{
    OECoreDataMainThreadAssertion();
    NSArray *selectedGames = [self selectedGames];
    OEDBCollection *collection = [[self sidebarController] addCollection:NO];
    [collection setGames:[NSSet setWithArray:selectedGames]];
    [collection save];

    [[self collectionController] setNeedsReload];
}

#pragma mark - Issue Resolving
- (IBAction)showIssuesView:(id)sender
{
    NSView *container  = [self collectionViewContainer];
    NSView *issuesView = [[self gameScannerController] view];

    if([[container subviews] containsObject:issuesView])
        return;

    [container addSubview:issuesView positioned:NSWindowAbove relativeTo:NULL];
    [issuesView setFrame:[container bounds]];
    
    // Disable toolbar controls.
    OELibraryToolbar *toolbar = [[self libraryController] toolbar];
    [[toolbar categorySelector] setEnabled:NO];
    [[toolbar gridViewButton] setEnabled:NO];
    [[toolbar listViewButton] setEnabled:NO];
    [[toolbar gridSizeSlider] setEnabled:NO];
    [[toolbar searchField] setEnabled:NO];
}
@end
