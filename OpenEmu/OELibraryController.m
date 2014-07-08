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

#import "OELibraryController.h"
#import "OELibraryDatabase.h"

#import "OESidebarItem.h"
#import "OESidebarController.h"
#import "OELibrarySplitView.h"

#import "OELibrarySubviewController.h"
#import "OEROMImporter.h"

#import "OEDBGame.h"
#import "OESystemPlugin.h"
#import "OEDBSmartCollection.h"

#import "NSViewController+OEAdditions.h"
#import "NSArray+OEAdditions.h"
#import "NSWindow+OEFullScreenAdditions.h"

#import "OEPreferencesController.h"

#import "OEBackgroundNoisePattern.h"

#import "OEGameCollectionViewController.h"
#import "OEMediaViewController.h"

#pragma mark - Exported variables
NSString * const OELastSidebarSelectionKey = @"lastSidebarSelection";

#pragma mark - Imported variables
extern NSString * const OESidebarSelectionDidChangeNotificationName;

#pragma mark - Private variables
static const CGFloat _OEToolbarHeight = 44;

@interface OELibraryController ()
- (void)OE_showFullscreen:(BOOL)fsFlag animated:(BOOL)animatedFlag;

@property NSMutableDictionary *subviewControllers;
- (NSViewController<OELibrarySubviewController> *)viewControllerWithClassName:(NSString *)className;
@end

@implementation OELibraryController
@synthesize cachedSnapshot = _cachedSnapshot;

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

#pragma mark - NSViewController stuff
- (NSString *)nibName
{
    return @"Library";
}

- (void)loadView
{
    [super loadView];

    [self setSubviewControllers:[NSMutableDictionary dictionary]];

    if([self database] == nil) [self setDatabase:[OELibraryDatabase defaultDatabase]];
    
    [[self sidebarController] view];
    
    // setup sidebar controller
    OESidebarController *sidebarCtrl = [self sidebarController];    
    [sidebarCtrl setDatabase:[self database]];

    [[self view] setPostsFrameChangedNotifications:YES];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(sidebarSelectionDidChange:) name:OESidebarSelectionDidChangeNotificationName object:nil];
    [[NSNotificationCenter defaultCenter] postNotificationName:OESidebarSelectionDidChangeNotificationName object:sidebarCtrl];
    
    [self updateToggleSidebarButtonState];

    // setup splitview
    OELibrarySplitView *splitView = [self mainSplitView];
    [splitView setDelegate:self];

    // setup tool tips
    [_toolbarSidebarButton setToolTip:OELocalizedString(@"Toggle Sidebar", @"Tooltip")];
    [_toolbarSidebarButton setToolTipStyle:OEToolTipStyleDefault];

    [_toolbarGridViewButton setToolTip:OELocalizedString(@"Switch To Grid View", @"Tooltip")];
    [_toolbarGridViewButton setToolTipStyle:OEToolTipStyleDefault];

    [_toolbarFlowViewButton setToolTip:OELocalizedString(@"Switch To Flow View", @"Tooltip")];
    [_toolbarFlowViewButton setToolTipStyle:OEToolTipStyleDefault];

    [_toolbarListViewButton setToolTip:OELocalizedString(@"Switch To List View", @"Tooltip")];
    [_toolbarListViewButton setToolTipStyle:OEToolTipStyleDefault];

    [_toolbarAddToSidebarButton setToolTip:OELocalizedString(@"New Collection", @"Tooltip")];
    [_toolbarAddToSidebarButton setToolTipStyle:OEToolTipStyleDefault];
}

- (void)viewDidAppear
{
    [super viewDidAppear];
    
    [self layoutToolbar];
    
    [[self sidebarController] reloadData];
}

- (void)viewWillDisappear
{
    [super viewWillDisappear];
    
    // Save Current State
    id lastState = [(id <OELibrarySubviewController>)[self currentViewController] encodeCurrentState];
    id itemID    = [[[self currentViewController] representedObject] sidebarID];
    [self OE_storeState:lastState forSidebarItemWithID:itemID];
    
    NSView *toolbarItemContainer = [[self toolbarSearchField] superview];
    
    [toolbarItemContainer setAutoresizingMask:NSViewWidthSizable];
}

#pragma mark - Toolbar

- (IBAction)toggleSidebar:(id)sender
{
    [[self mainSplitView] toggleSidebar];
}

- (void)updateToggleSidebarButtonState
{
    [[self toolbarSidebarButton] setState:([[self mainSplitView] isSidebarVisible] ? NSOnState : NSOffState)];
    [[self toolbarSidebarButton] display];
}

- (IBAction)switchToGridView:(id)sender
{
    if([[self currentViewController] respondsToSelector:@selector(switchToGridView:)])
       [[self currentViewController] performSelector:@selector(switchToGridView:) withObject:sender];
}

- (IBAction)switchToListView:(id)sender
{
    if([[self currentViewController] respondsToSelector:@selector(switchToListView:)])
        [[self currentViewController] performSelector:@selector(switchToListView:) withObject:sender];
}

- (IBAction)switchToFlowView:(id)sender
{
    if([[self currentViewController] respondsToSelector:@selector(switchToFlowView:)])
        [[self currentViewController] performSelector:@selector(switchToFlowView:) withObject:sender];
}

- (IBAction)search:(id)sender
{
    if([[self currentViewController] respondsToSelector:@selector(search:)])
        [[self currentViewController] performSelector:@selector(search:) withObject:sender];
}

- (IBAction)changeGridSize:(id)sender
{
    if([[self currentViewController] respondsToSelector:@selector(changeGridSize:)])
        [[self currentViewController] performSelector:@selector(changeGridSize:) withObject:sender];
}

- (IBAction)addCollectionAction:(id)sender
{
    [[self sidebarController] addCollectionAction:sender];
}

#pragma mark - FileMenu Actions

- (IBAction)newCollection:(id)sender
{
    [[self database] addNewCollection:nil];
    
    [[self sidebarController] reloadData];
}

- (IBAction)newSmartCollection:(id)sender
{
    [[self database] addNewSmartCollection:nil];
    
    [[self sidebarController] reloadData];
}

- (IBAction)newCollectionFolder:(id)sender
{
    [[self database] addNewCollectionFolder:nil];
    
    [[self sidebarController] reloadData];
}

- (IBAction)editSmartCollection:(id)sender
{
    NSLog(@"Edit smart collection: ");
}

#pragma mark - Edit Menu

- (IBAction)find:(id)sender
{
	[[[self view] window] makeFirstResponder:_toolbarSearchField];
}

#pragma mark - Menu Items

- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
    if([menuItem action] == @selector(newCollectionFolder:)) return NO;
    
    if([menuItem action] == @selector(editSmartCollection:))
        return [[[self sidebarController] selectedSidebarItem] isKindOfClass:[OEDBSmartCollection class]];

    const id currentViewController = [self currentViewController];

    if([menuItem action] == @selector(startGame:))
    {
        return [currentViewController isKindOfClass:[OEGameCollectionViewController class]] && [currentViewController respondsToSelector:@selector(selectedGames)] && [[currentViewController selectedGames] count] != 0;
    }

    if([menuItem action] == @selector(startSaveState:))
    {
        return [currentViewController isKindOfClass:[OEGameCollectionViewController class]] && [currentViewController respondsToSelector:@selector(selectedSaveStates)] && [[currentViewController selectedSaveStates] count] != 0;
    }

    if([menuItem action] == @selector(find:))
    {
        return [[self toolbarSearchField] isEnabled];
    }
    
    NSButton *button = nil;
    if([menuItem action] == @selector(switchToGridView:))
        button = [self toolbarGridViewButton];
    else if([menuItem action] == @selector(switchToFlowView:))
        button = [self toolbarFlowViewButton];
    else if([menuItem action] == @selector(switchToListView:))
        button = [self toolbarListViewButton];
    else return YES;
    
    [menuItem setState:[button state]];
    return [button isEnabled];
}

#pragma mark - Import

- (IBAction)addToLibrary:(id)sender
{
    NSOpenPanel *openPanel = [NSOpenPanel openPanel];
    [openPanel setAllowsMultipleSelection:YES];
    [openPanel setCanChooseFiles:YES];
    [openPanel setCanCreateDirectories:NO];
    [openPanel setCanChooseDirectories:YES];
    
    NSWindow *win = [[self view] window];
    
    [openPanel beginSheetModalForWindow:win completionHandler:
     ^(NSInteger result)
     {
         if(result == NSFileHandlingPanelOKButton)
         {
             // exit our initial open panels completion handler
             //[self performSelector:@selector(startImportSheet:) withObject:[openPanel URLs] afterDelay:0.0];
             OEROMImporter *romImporter = [[self database] importer];
             [romImporter importItemsAtURLs:[openPanel URLs]];
         }
     }];
}

#pragma mark -

- (IBAction)startGame:(id)sender
{
    NSMutableArray *gamesToStart = [NSMutableArray new];

    if([sender isKindOfClass:[OEDBGame class]]) [gamesToStart addObject:sender];
    else
    {
        NSAssert([(id)[self currentViewController] respondsToSelector:@selector(selectedGames)], @"Attempt to start a game from a view controller that doesn't announc selectedGames");

        [gamesToStart addObjectsFromArray:[(id <OELibrarySubviewController>)[self currentViewController] selectedGames]];
    }

    NSAssert([gamesToStart count] > 0, @"Attempt to start a game while the selection is empty");

    if([[self delegate] respondsToSelector:@selector(libraryController:didSelectGame:)])
    {
        for(OEDBGame *game in gamesToStart) [[self delegate] libraryController:self didSelectGame:game];
    }
}

- (IBAction)startSaveState:(id)sender
{
    OEMediaViewController *media = (OEMediaViewController *)[self currentViewController];
    NSArray *statesToLaunch = [media selectedSaveStates];

    if([statesToLaunch count] != 1) return;

    if([[self delegate] respondsToSelector:@selector(libraryController:didSelectSaveState:)])
    {
        for(OEDBSaveState *state in statesToLaunch)
            [[self delegate] libraryController:self didSelectSaveState:state];
    }
}

- (void)startSelectedGameWithSaveState:(id)stateItem
{
    OEDBSaveState *saveState = [stateItem representedObject];
    
    NSAssert(saveState != nil, @"Attempt to start a save state without valid item");

    if([[self delegate] respondsToSelector:@selector(libraryController:didSelectSaveState:)])
        [[self delegate] libraryController:self didSelectSaveState:saveState];
}

#pragma mark - Sidebar Helpers

- (void)showViewController:(NSViewController<OELibrarySubviewController> *)nextViewController
{
    NSViewController <OELibrarySubviewController> *oldViewController = [self currentViewController];
    if(nextViewController == oldViewController) return;
    
    if([nextViewController respondsToSelector:@selector(setLibraryController:)])
        [nextViewController setLibraryController:self];
    
    [oldViewController viewWillDisappear];
    [nextViewController viewWillAppear];
    
    NSView *newView    = [nextViewController view];    
    if(oldViewController)
    {
        NSView *superView = [[oldViewController view] superview];
        NSView *oldView   = [oldViewController view];
        
        [newView setFrame:[oldView frame]];
        [newView setAutoresizingMask:[oldView autoresizingMask]];
        
        [superView replaceSubview:oldView with:newView];
    }
    else
    {
        NSView *mainContentView = [self mainContentPlaceholderView];
        [newView setFrame:[mainContentView bounds]];
        [mainContentView addSubview:newView];
    }
    [self setCurrentViewController:nextViewController];
    
    [nextViewController viewDidAppear];
    [oldViewController viewDidDisappear];
    if([oldViewController respondsToSelector:@selector(setLibraryController:)])
        [oldViewController setLibraryController:nil];
}

- (void)sidebarSelectionDidChange:(NSNotification *)notification
{    
    // Save Current State
    id lastState = [(id <OELibrarySubviewController>)[self currentViewController] encodeCurrentState];
    id itemID    = [[[self currentViewController] representedObject] sidebarID];
    [self OE_storeState:lastState forSidebarItemWithID:itemID];

    // Set new item
    id<OESidebarItem> selectedItem = [[self sidebarController] selectedSidebarItem];
    if(selectedItem == nil)
    {
        DLog(@"nothing selected, this should not be possible, try to enable a random system from the library and select it");
        return;
    }

    NSString *viewControllerClasName = [selectedItem viewControllerClassName];
    NSViewController <OELibrarySubviewController> *viewController = [self viewControllerWithClassName:viewControllerClasName];

    // Make sure the view is loaded (-showViewController:) before setting the represented object.
    // This prepares the view controller (via its -loadView implementation) so that it can effectively act upon receiving a new
    // represented object
    [self showViewController:viewController];
    [viewController setRepresentedObject:selectedItem];

    itemID = [selectedItem sidebarID];
    [viewController restoreState:[self OE_storedStateForSidebarItemWithID:itemID]];
    
    // Try to select the current system in the controls pref pane
    // if sidebarID is not a system identifier the preference pane will handle it
    NSDictionary *userInfo = @{
                               OEPreferencesUserInfoPanelNameKey : @"Controls",
                               OEPreferencesUserInfoSystemIdentifierKey : itemID,
                               };
    [[NSNotificationCenter defaultCenter] postNotificationName:OEPreferencesSetupPaneNotificationName object:nil userInfo:userInfo];
}

- (void)OE_storeState:(id)state forSidebarItemWithID:(NSString *)itemID
{
    if(!itemID || !state) return;
    
    NSUserDefaults      *standardUserDefaults = [NSUserDefaults standardUserDefaults];
    NSDictionary        *libraryStates        = [standardUserDefaults valueForKey:@"Library States"];
    NSMutableDictionary *mutableLibraryStates = libraryStates ? [libraryStates mutableCopy] : [NSMutableDictionary dictionary];
    
    [mutableLibraryStates setObject:state forKey:itemID];
    [standardUserDefaults setObject:mutableLibraryStates forKey:@"Library States"];
    [standardUserDefaults synchronize];
}

- (id)OE_storedStateForSidebarItemWithID:(NSString *)itemID
{
    if(!itemID) return nil;
    
    NSUserDefaults      *standardUserDefaults = [NSUserDefaults standardUserDefaults];
    NSDictionary        *libraryStates        = [standardUserDefaults valueForKey:@"Library States"];
   
    return [libraryStates objectForKey:itemID];
}

#pragma mark - OELibrarySplitViewDelegate

- (void)librarySplitViewDidToggleSidebar:(NSNotification *)notification
{
    [self layoutToolbar];
}

- (void)splitViewDidResizeSubviews:(NSNotification *)notification
{
    [self layoutToolbar];
}

#pragma mark - Private

- (void)OE_showFullscreen:(BOOL)fsFlag animated:(BOOL)animatedFlag
{
    [NSApp setPresentationOptions:(fsFlag ? NSApplicationPresentationAutoHideDock | NSApplicationPresentationAutoHideToolbar : NSApplicationPresentationDefault)];
}

- (NSViewController<OELibrarySubviewController> *)viewControllerWithClassName:(NSString *)className
{
    if(![_subviewControllers valueForKey:className])
    {
        Class viewControllerClass = NSClassFromString(className);
        if(viewControllerClass)
        {
            NSViewController <OELibrarySubviewController>*viewController = [[viewControllerClass alloc] init];
            [_subviewControllers setObject:viewController forKey:className];
        }
    }
    return [_subviewControllers valueForKey:className];
}

#pragma mark -

- (void)layoutToolbar
{
    CGFloat splitterPosition = [[self mainSplitView] splitterPosition];
    NSView *toolbarItemContainer = [[self toolbarSearchField] superview];

    NSRect toolbarItemContainerFrame =
    {
        .origin.x = splitterPosition,
        .origin.y = 0,
        .size.width = NSWidth([[toolbarItemContainer superview] bounds]) - splitterPosition,
        .size.height = _OEToolbarHeight
    };

    [toolbarItemContainer setFrame:toolbarItemContainerFrame];
}

@end
