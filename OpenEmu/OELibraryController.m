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

#import "NSArray+OEAdditions.h"
#import "NSWindow+OEFullScreenAdditions.h"

#import "OEPreferencesController.h"

#import "OEBackgroundNoisePattern.h"

#import "OEGameCollectionViewController.h"
#import "OEMediaViewController.h"
#import "OEDBSavedGamesMedia.h"
#import "OEDBScreenshotsMedia.h"
#import "OEFeaturedGamesViewController.h"

#import "OELibraryGamesViewController.h"

#pragma mark - Exported variables
NSString * const OELastSidebarSelectionKey = @"lastSidebarSelection";
NSString * const OELibraryStatesKey        = @"Library States";

#pragma mark - Imported variables
extern NSString * const OESidebarSelectionDidChangeNotificationName;

@interface OELibraryController ()
- (void)OE_showFullscreen:(BOOL)fsFlag animated:(BOOL)animatedFlag;

@property (strong) NSViewController *mainViewController;
@property (strong) NSViewController *overlayChildController;
@end

@implementation OELibraryController
@synthesize cachedSnapshot = _cachedSnapshot;

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (NSString *)nibName
{
    return @"OELibraryController";
}

- (void)loadView
{
    [super loadView];

    if([self database] == nil) [self setDatabase:[OELibraryDatabase defaultDatabase]];

    OELibraryGamesViewController *controller = [[OELibraryGamesViewController alloc] init];
    [controller setLibraryController:self];

    NSView *subview = [controller view];
    [subview setFrame:[[self view] bounds]];
    [subview setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];
    [[self view] addSubview:subview];
    [self addChildViewController:controller];

    [self setMainViewController:controller];
}

- (void)viewDidAppear
{
    [super viewDidAppear];
    
    [[self sidebarController] reloadData];

    if([[[[self view] window] titlebarAccessoryViewControllers] count]) return;

    [self prepareToolbar];
}

- (void)viewWillDisappear
{
    [super viewWillDisappear];
    
    // Save Current State
//    id lastState = [[self currentViewController] encodeCurrentState];
//    id itemID    = [[[self currentViewController] representedObject] sidebarID];
//    [self OE_storeState:lastState forSidebarItemWithID:itemID];
    
    NSView *toolbarItemContainer = [[self toolbarSearchField] superview];
    [toolbarItemContainer setAutoresizingMask:NSViewWidthSizable];
}

- (id <OELibrarySubviewController>)currentViewController {
    if(self.overlayChildController)
        return (id <OELibrarySubviewController>)self.overlayChildController;

    return (id <OELibrarySubviewController>)self.mainViewController;
}

- (void)prepareToolbar {
    NSWindow *window = [[self view] window];
    
    while([[window titlebarAccessoryViewControllers] count])
        [window removeTitlebarAccessoryViewControllerAtIndex:0];

    NSTitlebarAccessoryViewController * leftViewController = [[NSTitlebarAccessoryViewController alloc] init];
    [leftViewController setLayoutAttribute:NSLayoutAttributeLeft];
    [leftViewController setView:[self leftToolbarView]];
    [window addTitlebarAccessoryViewController:leftViewController];

    NSTitlebarAccessoryViewController * rightViewController = [[NSTitlebarAccessoryViewController alloc] init];
    [rightViewController setLayoutAttribute:NSLayoutAttributeRight];
    [rightViewController setView:[self rightToolbarView]];
    [window addTitlebarAccessoryViewController:rightViewController];
}

#pragma mark - Toolbar
- (IBAction)addCollectionAction:(id)sender
{
    if([[self currentViewController] respondsToSelector:@selector(addCollectionAction:)])
        [[self currentViewController] performSelector:@selector(addCollectionAction:) withObject:sender];
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

- (void)magnifyWithEvent:(NSEvent*)event {
    if([[self currentViewController] respondsToSelector:@selector(changeGridSize:)])
    {
        CGFloat zoomChange = [event magnification];
        CGFloat zoomValue = [[self toolbarSlider] floatValue];

        [[self toolbarSlider] setFloatValue:zoomValue+zoomChange];
        [self changeGridSize:[self toolbarSlider]];
    }
}


#pragma mark - FileMenu Actions
- (IBAction)newCollection:(id)sender
{
    if([[self currentViewController] respondsToSelector:@selector(addCollectionAction:)])
        [[self currentViewController] performSelector:@selector(addCollectionAction:) withObject:sender];
}

- (IBAction)newSmartCollection:(id)sender
{
    // TODO: implement
}

- (IBAction)newCollectionFolder:(id)sender
{
    // TODO: implement
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

#pragma mark - Overlay Views
- (void)_clearToolbarButtons {
    [[self savestateButton] setState:NSOffState];
    [[self screenshotsButton] setState:NSOffState];
    [[self featuredGamesButton] setState:NSOffState];
}

- (IBAction)toggleSaveStateView:(id)sender
{
    if([[self savestateButton] state] != NSOnState){
        [self _removeCurrentOverlayController];
        [self _clearToolbarButtons];
        return;
    }


    OEMediaViewController *controller = [[OEMediaViewController alloc] init];
    [controller setLibraryController:self];
    [controller view];
    [controller setRepresentedObject:[OEDBSavedGamesMedia sharedDBSavedGamesMedia]];

    [self _showOverlayController:controller];

    [self _clearToolbarButtons];
    [[self savestateButton] setState:NSOnState];
}

- (IBAction)toggleScreenshotView:(id)sender
{
    if([[self screenshotsButton] state] != NSOnState){
        [self _removeCurrentOverlayController];
        [self _clearToolbarButtons];
        return;
    }

    OEMediaViewController *controller = [[OEMediaViewController alloc] init];
    [controller setLibraryController:self];
    [controller setRepresentedObject:[OEDBScreenshotsMedia sharedDBScreenshotsMedia]];

    [self _showOverlayController:controller];


    [self _clearToolbarButtons];
    [[self screenshotsButton] setState:NSOnState];
}

- (IBAction)toggleHomebrewView:(id)sender
{
    if([[self featuredGamesButton] state] != NSOnState){
        [self _removeCurrentOverlayController];
        [self _clearToolbarButtons];
        return;
    }

    OEFeaturedGamesViewController *controller = [[OEFeaturedGamesViewController alloc] init];
    [controller setLibraryController:self];

    [self _showOverlayController:controller];

    [self _clearToolbarButtons];
    [[self featuredGamesButton] setState:NSOnState];
}

- (void)_showOverlayController:(NSViewController<OELibrarySubviewController>*)newViewController
{
    [self _removeCurrentOverlayController];

    NSView *newView = [newViewController view];
    [newView setFrame:[[self view] bounds]];
    [newView setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];
    [[self view] addSubview:newView];
    [self addChildViewController:newViewController];

    [self setOverlayChildController:newViewController];
}

- (void)_removeCurrentOverlayController
{
    NSViewController *currentViewController = [self overlayChildController];
    if(!currentViewController) return;

    [[currentViewController view] removeFromSuperview];
    [currentViewController removeFromParentViewController];

    [self setOverlayChildController:nil];
}

#pragma mark -
- (void)showViewController:(NSViewController<OELibrarySubviewController> *)nextViewController
{}

#pragma mark - Controlling Sidebar
- (BOOL)OE_isSiderbarVisible
{
    return [[self mainSplitView] isSidebarVisible];
}
- (IBAction)showSidebar:(id)sender
{
    if([self OE_isSiderbarVisible]) return;

    [self toggleSidebar:sender];
}

- (IBAction)hideSidebar:(id)sender
{
    if(![self OE_isSiderbarVisible]) return;

    [self toggleSidebar:sender];
}

- (IBAction)toggleSidebar:(id)sender
{
    [[self mainSplitView] toggleSidebar];
}

#pragma mark -
- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
    SEL action = [menuItem action];

    if(action == @selector(toggleSidebar:))
    {
        NSString *title = nil;
        if([self OE_isSiderbarVisible]) {
            title = NSLocalizedString(@"Hide Sidebar", @"Hide Sidebar menu item");
        } else {
            title = NSLocalizedString(@"Show Sidebar", @"Show sidebar menu item");
        }
        [menuItem setTitle:title];
        return YES;
    }

    if(action == @selector(newCollectionFolder:)) return NO;
    
    if(action == @selector(editSmartCollection:))
        return [[[self sidebarController] selectedSidebarItem] isKindOfClass:[OEDBSmartCollection class]];

    const id currentViewController = [self currentViewController];

    if(action == @selector(startGame:))
    {
        return [currentViewController isKindOfClass:[OEGameCollectionViewController class]] && [currentViewController respondsToSelector:@selector(selectedGames)] && [[currentViewController selectedGames] count] != 0;
    }

    if(action == @selector(startSaveState:))
    {
        return [currentViewController isKindOfClass:[OEGameCollectionViewController class]] && [currentViewController respondsToSelector:@selector(selectedSaveStates)] && [[currentViewController selectedSaveStates] count] != 0;
    }

    if(action == @selector(find:))
    {
        return [[self toolbarSearchField] isEnabled];
    }

    NSButton *button = nil;
    if(action == @selector(switchToGridView:))
        button = [self toolbarGridViewButton];
    else if(action == @selector(switchToListView:))
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

    if([sender isKindOfClass:[OEDBGame class]]){
        [gamesToStart addObject:sender];
    }
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
/*
- (void)showViewController:(NSViewController<OELibrarySubviewController> *)nextViewController
{
    NSViewController <OELibrarySubviewController> *oldViewController = [self currentViewController];
    if(nextViewController == oldViewController) return;
    
    if([nextViewController respondsToSelector:@selector(setLibraryController:)])
        [nextViewController setLibraryController:self];

    NSView *newView = [nextViewController view];
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
    NSDictionary        *libraryStates        = [standardUserDefaults valueForKey:OELibraryStatesKey];
    NSMutableDictionary *mutableLibraryStates = libraryStates ? [libraryStates mutableCopy] : [NSMutableDictionary dictionary];
    
    [mutableLibraryStates setObject:state forKey:itemID];
    [standardUserDefaults setObject:mutableLibraryStates forKey:OELibraryStatesKey];
}

- (id)OE_storedStateForSidebarItemWithID:(NSString *)itemID
{
    if(!itemID) return nil;
    
    NSUserDefaults      *standardUserDefaults = [NSUserDefaults standardUserDefaults];
    NSDictionary        *libraryStates        = [standardUserDefaults valueForKey:OELibraryStatesKey];
   
    return [libraryStates objectForKey:itemID];
}
#pragma mark - OELibrarySplitViewDelegate
- (void)librarySplitViewDidToggleSidebar:(NSNotification *)notification
{
    NSView *sidebarView = [[self sidebarController] view];
    if(![[self mainSplitView] isSidebarVisible])
    {
//        [[sidebarView window] makeFirstResponder:[[[self mainSplitView] subviews] lastObject]];
    }
}

- (void)splitViewDidResizeSubviews:(NSNotification *)notification
{
 }
 */

#pragma mark - Private
- (void)OE_showFullscreen:(BOOL)fsFlag animated:(BOOL)animatedFlag
{
    [NSApp setPresentationOptions:(fsFlag ? NSApplicationPresentationAutoHideDock | NSApplicationPresentationAutoHideMenuBar : NSApplicationPresentationDefault)];
}

@end
