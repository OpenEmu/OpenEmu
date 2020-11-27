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
@property OESidebarController *sidebarController;
@property OEGameCollectionViewController *collectionController;
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
    
    [self OE_setupSplitView];
    
    [self _assignDatabase];
    
    NSNotificationCenter *noc = [NSNotificationCenter defaultCenter];
    [noc addObserver:self selector:@selector(_updateCollectionContentsFromSidebar:) name:OESidebarSelectionDidChangeNotification object:[self sidebarController]];
}

- (void)viewWillAppear
{
    [super viewWillAppear];
    
    [self _setupSplitViewAutosave];
    [self _updateCollectionContentsFromSidebar:nil];
    
    self.view.needsDisplay = YES;
    
    if (@available(macOS 11.0, *)) {
        self.view.window.titlebarSeparatorStyle = NSTitlebarSeparatorStyleAutomatic;
    }
}

- (void)viewDidAppear
{
    [super viewDidAppear];
    
    [self.collectionController updateBlankSlate];
}

- (void)viewWillDisppear
{
    [super viewWillDisappear];
    
    if (@available(macOS 11.0, *)) {
        self.view.window.titlebarSeparatorStyle = NSTitlebarSeparatorStyleLine;
    }
}

#pragma mark - Validation

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

- (void)OE_setupSplitView
{
    _sidebarController = [[OESidebarController alloc] init];
    _collectionController = [[OEGameCollectionViewController alloc] init];
    
    self.splitView.translatesAutoresizingMaskIntoConstraints = NO;
    
    NSSplitViewItem *sidebarItem = [NSSplitViewItem sidebarWithViewController:_sidebarController];
    sidebarItem.minimumThickness = _OESidebarMinWidth;
    sidebarItem.maximumThickness = _OESidebarMaxWidth;
    sidebarItem.canCollapse = NO;
    if (@available(macOS 11.0, *)) {
        sidebarItem.titlebarSeparatorStyle = NSTitlebarSeparatorStyleAutomatic;
    }
    [self addSplitViewItem:sidebarItem];
    
    NSSplitViewItem *collectionItem = [NSSplitViewItem splitViewItemWithViewController:_collectionController];
    collectionItem.minimumThickness = _OEMainViewMinWidth;
    if (@available(macOS 11.0, *)) {
        collectionItem.titlebarSeparatorStyle = NSTitlebarSeparatorStyleLine;
    }
    [self addSplitViewItem:collectionItem];
}

- (void)_setupSplitViewAutosave
{
    NSSplitView *librarySplitView = self.splitView;
    if (librarySplitView.autosaveName && ![librarySplitView.autosaveName isEqual:@""])
        return;
    
    [NSUserDefaults.standardUserDefaults registerDefaults:@{
        @"lastSidebarWidth": @(_OESidebarDefaultWidth),
    }];
    
    if (![NSUserDefaults.standardUserDefaults objectForKey:@"NSSplitView Subview Frames OELibraryGamesSplitView"]) {
        /* read the old property key (OE 2.2.1 and prior) */
        CGFloat dividerPosition = [NSUserDefaults.standardUserDefaults doubleForKey:@"lastSidebarWidth"];
        [librarySplitView setPosition:dividerPosition ofDividerAtIndex:0];
    }
    librarySplitView.autosaveName = @"OELibraryGamesSplitView";
    
    [NSNotificationCenter.defaultCenter addObserver:self selector:@selector(resetSidebar) name:OELibrarySplitViewResetSidebarNotification object:nil];
}

- (void)resetSidebar
{
    [self.splitView setPosition:_OESidebarDefaultWidth ofDividerAtIndex:0];
}

#pragma mark - OELibrarySubviewController

- (NSArray<OEDBGame *> *)selectedGames
{
    return self.collectionController.selectedGames;
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
    self.sidebarController.database = _database;
    self.collectionController.database = _database;
}

#pragma mark - Toolbar
- (IBAction)addCollectionAction:(id)sender
{
    [[self sidebarController] addCollectionAction:sender];
}

- (IBAction)switchToGridView:(id)sender
{
    [self.collectionController showGridView];
}

- (IBAction)switchToListView:(id)sender
{
    [self.collectionController showListView];
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
    
    // For empty collections of disc-based games, display an alert to compel the user to read the disc-importing guide.
    if ([selectedItem isKindOfClass:[OEDBSystem class]] &&
        ((OEDBSystem *)selectedItem).plugin.supportsDiscsWithDescriptorFile &&
        ((OEDBSystem *)selectedItem).games.count == 0 &&
        ![NSUserDefaults.standardUserDefaults boolForKey:OESkipDiscGuideMessageKey])
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
                [NSWorkspace.sharedWorkspace openURL:guideURL];
            }
        }];
    }
}

- (void)makeNewCollectionWithSelectedGames:(id)sender
{
    OECoreDataMainThreadAssertion();
    NSArray<OEDBGame *> *selectedGames = self.selectedGames;
    OEDBCollection *collection = [self.sidebarController addCollection:NO];
    collection.games = [NSSet setWithArray:selectedGames];
    [collection save];

    [self.collectionController setNeedsReload];
}

@end
