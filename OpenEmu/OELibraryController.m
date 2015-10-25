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

typedef NS_ENUM(NSInteger, OELibraryCategory) {
    OELibraryCategoryGames,
    OELibraryCategorySaveStates,
    OELibraryCategoryScreenshots,
    OELibraryCategoryHomebrew
};

#pragma mark - Imported variables
extern NSString * const OESidebarSelectionDidChangeNotificationName;

@interface OELibraryController ()
- (void)OE_showFullscreen:(BOOL)fsFlag animated:(BOOL)animatedFlag;

@property OELibraryCategory selectedCategory;
@property (strong) NSViewController *contentViewController;
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

    _selectedCategory = OELibraryCategoryGames;

    [self setContentViewController:controller];
}

- (void)viewDidAppear
{
    [super viewDidAppear];

    NSWindow *window = [[self view] window];
    [window setToolbar:[self toolbar]];
}

- (void)viewWillDisappear
{
    [super viewWillDisappear];
    
    // Save Current State
//    id lastState = [[self currentViewController] encodeCurrentState];
//    id itemID    = [[[self currentViewController] representedObject] sidebarID];
//    [self OE_storeState:lastState forSidebarItemWithID:itemID];
    
    NSView *toolbarItemContainer = [[[self toolbar] searchField] superview];
    [toolbarItemContainer setAutoresizingMask:NSViewWidthSizable];
}

- (id <OELibrarySubviewController>)currentViewController {
    return (id <OELibrarySubviewController>)self.contentViewController;
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
        CGFloat zoomValue = [[[self toolbar] gridSizeSlider] floatValue];

        [[[self toolbar] gridSizeSlider] setFloatValue:zoomValue+zoomChange];
        [self changeGridSize:[[self toolbar] gridSizeSlider]];
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
	[[[self view] window] makeFirstResponder:[[self toolbar] searchField]];
}

#pragma mark - Categories
- (IBAction)switchCategory:(id)sender
{
    OELibraryCategory category = [[[self toolbar] viewSelector] selectedSegment];

    if(category == _selectedCategory) {
        return;
    }

    NSViewController<OELibrarySubviewController> *newViewController = nil;

    switch (category) {
        case OELibraryCategoryGames:
            NSLog(@"games");
            newViewController = [[OELibraryGamesViewController alloc] init];
            break;
        case OELibraryCategorySaveStates:
            NSLog(@"save states");
            newViewController = [[OEMediaViewController alloc] init];
            [newViewController setRepresentedObject:[OEDBSavedGamesMedia sharedDBSavedGamesMedia]];
            break;
        case OELibraryCategoryScreenshots:
            NSLog(@"screenshots");
            newViewController = [[OEMediaViewController alloc] init];
            [newViewController setRepresentedObject:[OEDBScreenshotsMedia sharedDBScreenshotsMedia]];
            break;
        case OELibraryCategoryHomebrew:
            NSLog(@"homebrew");
            newViewController = [[OEFeaturedGamesViewController alloc] init];
            break;
        default:
            break;
    }

    [self _showViewController:newViewController];
    _selectedCategory = category;
}

- (void)_showViewController:(NSViewController<OELibrarySubviewController>*)newViewController {
    NSViewController *currentViewController = [self contentViewController];
    if(currentViewController)
    {
        [[currentViewController view] removeFromSuperview];
        [currentViewController removeFromParentViewController];
        [self setContentViewController:nil];
    }

    [newViewController setLibraryController:self];

    NSView *newView = [newViewController view];
    [newView setFrame:[[self view] bounds]];
    [newView setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];
    [[self view] addSubview:newView];

    [self addChildViewController:newViewController];
    [self setContentViewController:newViewController];

    [[[self view] window] makeFirstResponder:newView];
}

#pragma mark -
- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
    SEL action = [menuItem action];
    if(action == @selector(newCollectionFolder:)) return NO;

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
        return [[[self toolbar] searchField] isEnabled];
    }

    NSButton *button = nil;
    if(action == @selector(switchToGridView:))
        button = [[self toolbar] gridViewButton];
    else if(action == @selector(switchToListView:))
        button = [[self toolbar] listViewButton];
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

#pragma mark - Private
- (void)OE_showFullscreen:(BOOL)fsFlag animated:(BOOL)animatedFlag
{
    [NSApp setPresentationOptions:(fsFlag ? NSApplicationPresentationAutoHideDock | NSApplicationPresentationAutoHideMenuBar : NSApplicationPresentationDefault)];
}

@end
