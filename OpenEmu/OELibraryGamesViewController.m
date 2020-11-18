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

#import "OEDBCollection.h"
#import "OEDBSystem+CoreDataProperties.h"

@import OpenEmuKit;

#import "OpenEmu-Swift.h"

#pragma mark - Public constants

NSString * const OESkipDiscGuideMessageKey = @"OESkipDiscGuideMessageKey";
NSNotificationName const OELibrarySplitViewResetSidebarNotification = @"OELibrarySplitViewResetSidebarNotification";

#pragma mark - Private constants

static const CGFloat _OESidebarMinWidth     = 105;
static const CGFloat _OESidebarDefaultWidth = 200;
static const CGFloat _OESidebarMaxWidth     = 450;
static const CGFloat _OEMainViewMinWidth    = 495;

#pragma mark - Private class members

@interface OELibraryGamesViewController()<OELibrarySubviewControllerGameSelection>
@property (nonatomic, readonly, nullable) OELibraryToolbar *toolbar;
@end

#pragma mark - Implementation

@implementation OELibraryGamesViewController
@synthesize database = _database;

- (OELibraryToolbar * _Nullable)toolbar
{
    NSToolbar *toolbar = self.view.window.toolbar;
    if ([toolbar isKindOfClass:OELibraryToolbar.class])
    {
        return (OELibraryToolbar *)toolbar;
    }
    
    return nil;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
        
    [self _assignDatabase];

    NSNotificationCenter *noc = [NSNotificationCenter defaultCenter];
    [noc addObserver:self selector:@selector(_updateCollectionContentsFromSidebar:) name:OESidebarSelectionDidChangeNotification object:[self sidebarController]];

    NSView *collectionView = [[self collectionController] view];
    NSView *collectionViewContainer = [self collectionViewContainer];

    [collectionView setFrame:[collectionViewContainer bounds]];
    [collectionView setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];
    [collectionViewContainer addSubview:collectionView];

    [self addChildViewController:self.sidebarController];
    [self addChildViewController:self.collectionController];
    [self addChildViewController:self.gameScannerController];
    
    [self _setupSplitView];
}

- (void)viewWillAppear
{
    [super viewWillAppear];
    
    [self _updateCollectionContentsFromSidebar:nil];

    self.view.needsDisplay = YES;
}

- (void)viewDidAppear
{
    [super viewDidAppear];
    
    [self _validateToolbarItems];
    
    [self.collectionController updateBlankSlate];
}

#pragma mark - Validation

- (void)_validateToolbarItems
{
    OELibraryToolbar *toolbar = self.toolbar;
    BOOL isGridView = self.collectionController.selectedViewTag == OEGridViewTag;
    BOOL isBlankSlate = self.collectionController.shouldShowBlankSlate;
    
    toolbar.viewModeSelector.enabled = !isBlankSlate;
    toolbar.viewModeSelector.selectedSegment = isGridView ? 0 : 1;
    
    toolbar.gridSizeSlider.enabled = isGridView && !isBlankSlate;
    toolbar.decreaseGridSizeButton.enabled = isGridView && !isBlankSlate;
    toolbar.increaseGridSizeButton.enabled = isGridView && !isBlankSlate;
    
    toolbar.searchField.enabled = !isBlankSlate;
    toolbar.searchField.searchMenuTemplate = nil;
    toolbar.searchField.stringValue = self.collectionController.currentSearchTerm ?: @"";
    
    toolbar.addButton.enabled = YES;
}

- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
    BOOL isGridView = self.collectionController.selectedViewTag == OEGridViewTag;
    BOOL isBlankSlate = self.collectionController.shouldShowBlankSlate;
    SEL action = menuItem.action;
    
    if (action == @selector(switchToGridView:))
    {
        menuItem.state = isGridView ? NSControlStateValueOn : NSControlStateValueOff;
        return !isBlankSlate;
    }
    
    if (action == @selector(switchToListView:))
    {
        menuItem.state = !isGridView ? NSControlStateValueOn : NSControlStateValueOff;
        return !isBlankSlate;
    }
    
    return YES;
}

#pragma mark - Split View

- (void)_setupSplitView
{
    [NSUserDefaults.standardUserDefaults registerDefaults:@{
        @"lastSidebarWidth": @(_OESidebarDefaultWidth),
    }];
    
    NSSplitView *librarySplitView = (NSSplitView *)self.view;
    NSView *sidebar = librarySplitView.arrangedSubviews[0];
    NSView *gridView = librarySplitView.arrangedSubviews[1];
    
    [librarySplitView addConstraints:@[
        [sidebar.widthAnchor constraintGreaterThanOrEqualToConstant:_OESidebarMinWidth],
        [sidebar.widthAnchor constraintLessThanOrEqualToConstant:_OESidebarMaxWidth],
        [gridView.widthAnchor constraintGreaterThanOrEqualToConstant:_OEMainViewMinWidth]
    ]];
    
    if (![NSUserDefaults.standardUserDefaults objectForKey:@"NSSplitView Subview Frames libraryGamesSplitView"]) {
        /* read the old property key (OE 2.2.1 and prior) */
        CGFloat dividerPosition = [NSUserDefaults.standardUserDefaults doubleForKey:@"lastSidebarWidth"];
        [librarySplitView setPosition:dividerPosition ofDividerAtIndex:0];
    }
    librarySplitView.autosaveName = @"libraryGamesSplitView";
    
    [NSNotificationCenter.defaultCenter addObserver:self selector:@selector(resetSidebar) name:OELibrarySplitViewResetSidebarNotification object:nil];
    
}

- (void)resetSidebar
{
    NSSplitView *librarySplitView = (NSSplitView *)self.view;
    [librarySplitView setPosition:_OESidebarDefaultWidth ofDividerAtIndex:0];
}

#pragma mark - OELibrarySubviewController

- (NSArray<OEDBGame *> *)selectedGames
{
    return [[self collectionController] selectedGames];
}

- (OELibraryDatabase *)database
{
    return _database;
}

- (void)setDatabase:(OELibraryDatabase *)database
{
    _database = database;
    [self _assignDatabase];
}

- (void)_assignDatabase
{
    [[self sidebarController] setDatabase:_database];
    [[self collectionController] setDatabase:_database];
}

#pragma mark - Toolbar
- (IBAction)addCollectionAction:(id)sender
{
    [[self sidebarController] addCollectionAction:sender];
}

- (IBAction)switchToGridView:(id)sender
{
    [self.collectionController showGridView];
    [self _validateToolbarItems];
}

- (IBAction)switchToListView:(id)sender
{
    [self.collectionController showListView];
    [self _validateToolbarItems];
}

- (IBAction)search:(id)sender
{
    [self.collectionController performSearch:[sender stringValue]];
}

- (IBAction)changeGridSize:(id)sender
{
    NSSlider *slider = self.toolbar.gridSizeSlider;
    [self.collectionController zoomGridViewWithValue:slider.doubleValue];
}

- (IBAction)decreaseGridSize:(id)sender
{
    NSSlider *slider = self.toolbar.gridSizeSlider;
    slider.doubleValue -= [sender tag] == 199 ? 0.25 : 0.5;
    [self.collectionController zoomGridViewWithValue:slider.doubleValue];
}

- (IBAction)increaseGridSize:(id)sender
{
    NSSlider *slider = self.toolbar.gridSizeSlider;
    slider.doubleValue += [sender tag] == 199 ? 0.25 : 0.5;
    [self.collectionController zoomGridViewWithValue:slider.doubleValue];
}

#pragma mark - Sidebar handling

- (void)_updateCollectionContentsFromSidebar:(id)sender
{
    id selectedItem = self.sidebarController.selectedSidebarItem;
    self.collectionController.representedObject = selectedItem;
    [self _validateToolbarItems];
    
    // For empty collections of disc-based games, display an alert to compel the user to read the disc-importing guide.
    if ([selectedItem isKindOfClass:[OEDBSystem class]] &&
        ((OEDBSystem *)selectedItem).plugin.supportsDiscsWithDescriptorFile &&
        ((OEDBSystem *)selectedItem).games.count == 0 &&
        ![[NSUserDefaults standardUserDefaults] boolForKey:OESkipDiscGuideMessageKey])
    {
        
        NSAlert *alert = [[NSAlert alloc] init];
        
        alert.messageText = NSLocalizedString(@"Have you read the guide?", @"");
        alert.informativeText = NSLocalizedString(@"Disc-based games have special requirements. Please read the disc importing guide.", @"");
        alert.alertStyle = NSAlertStyleInformational;
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

- (void)makeNewCollectionWithSelectedGames:(id)sender
{
    OECoreDataMainThreadAssertion();
    NSArray<OEDBGame *> *selectedGames = [self selectedGames];
    OEDBCollection *collection = [[self sidebarController] addCollection:NO];
    [collection setGames:[NSSet setWithArray:selectedGames]];
    [collection save];

    [[self collectionController] setNeedsReload];
    [self _validateToolbarItems];
}

#pragma mark - Issue Resolving
- (IBAction)showIssuesView:(id)sender
{
    [self presentViewControllerAsSheet:self.gameScannerController];
}

@end
