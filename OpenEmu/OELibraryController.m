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

#import "OEButton.h"
#import "OESearchField.h"

#import "OELibrarySubviewController.h"
#import "OEROMImporter.h"

#import "OEDBGame.h"
#import "OESystemPlugin.h"
#import "OEDBSmartCollection.h"

#import "OEBackgroundNoisePattern.h"

#import "OEGameCollectionViewController.h"
#import "OEMediaViewController.h"
#import "OEDBSavedGamesMedia.h"
#import "OEDBScreenshotsMedia.h"
#import "OEHomebrewViewController.h"

#import "OELibraryGamesViewController.h"

#import "OpenEmu-Swift.h"

#pragma mark - Exported variables
NSString * const OELastSidebarSelectionKey = @"lastSidebarSelection";
NSString * const OELibraryStatesKey        = @"Library States";
NSString * const OELibraryLastCategoryKey = @"OELibraryLastCategoryKey";

typedef NS_ENUM(NSUInteger, OELibraryCategory) {
    OELibraryCategoryGames,
    OELibraryCategorySaveStates,
    OELibraryCategoryScreenshots,
    OELibraryCategoryHomebrew,

    OELibraryCategoryCount
};

#pragma mark - Imported variables
extern NSString * const OESidebarSelectionDidChangeNotificationName;

@interface OELibraryController ()

- (void)OE_showFullscreen:(BOOL)fsFlag animated:(BOOL)animatedFlag;

@property OELibraryCategory selectedCategory;
@property (nonatomic, readwrite) NSViewController <OELibrarySubviewController> *currentSubviewController;

// Library subview controllers.
@property (strong, readonly) OELibraryGamesViewController *libraryGamesViewController;
@property (strong, readonly) OEMediaViewController *saveStatesViewController;
@property (strong, readonly) OEMediaViewController *screenshotsViewController;
@property (strong, readonly) OEHomebrewViewController *homebrewViewController;

@end

@implementation OELibraryController
@synthesize cachedSnapshot = _cachedSnapshot;

- (NSString *)nibName
{
    return @"OELibraryController";
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    if([self database] == nil) [self setDatabase:[OELibraryDatabase defaultDatabase]];
    
    [self setUpCategoryViewControllers];
    
    _selectedCategory = [[NSUserDefaults standardUserDefaults] integerForKey:OELibraryLastCategoryKey];
    if(_selectedCategory >= OELibraryCategoryCount) {
        _selectedCategory = OELibraryCategoryGames;
    }
    
    [self _showSubviewControllerForCategory:_selectedCategory];
}

- (void)setUpCategoryViewControllers
{
    _libraryGamesViewController = [[OELibraryGamesViewController alloc] init];
    _libraryGamesViewController.libraryController = self;
    
    _saveStatesViewController = [[OEMediaViewController alloc] init];
    _saveStatesViewController.libraryController = self;
    _saveStatesViewController.representedObject = [OEDBSavedGamesMedia sharedDBSavedGamesMedia];
    
    _screenshotsViewController = [[OEMediaViewController alloc] init];
    _screenshotsViewController.libraryController = self;
    _screenshotsViewController.representedObject = [OEDBScreenshotsMedia sharedDBScreenshotsMedia];
    
    _homebrewViewController = [[OEHomebrewViewController alloc] init];
    _homebrewViewController.libraryController = self;
    
    [self addChildViewController:_libraryGamesViewController];
    [self addChildViewController:_saveStatesViewController];
    [self addChildViewController:_screenshotsViewController];
    [self addChildViewController:_homebrewViewController];
}

- (void)viewDidAppear
{
    [super viewDidAppear];

    [[[self toolbar] categorySelector] setSelectedSegment:_selectedCategory];
}

- (void)viewWillDisappear
{
    [super viewWillDisappear];
    
    NSView *toolbarItemContainer = [[[self toolbar] searchField] superview];
    [toolbarItemContainer setAutoresizingMask:NSViewWidthSizable];
}

#pragma mark - Toolbar

- (IBAction)addCollectionAction:(id)sender
{
    if([[self currentSubviewController] respondsToSelector:@selector(addCollectionAction:)])
        [[self currentSubviewController] performSelector:@selector(addCollectionAction:) withObject:sender];
}

- (IBAction)switchToGridView:(id)sender
{
    if([[self currentSubviewController] respondsToSelector:@selector(switchToGridView:)])
       [[self currentSubviewController] performSelector:@selector(switchToGridView:) withObject:sender];
}

- (IBAction)switchToListView:(id)sender
{
    if([[self currentSubviewController] respondsToSelector:@selector(switchToListView:)])
        [[self currentSubviewController] performSelector:@selector(switchToListView:) withObject:sender];
}

- (IBAction)search:(id)sender
{
    if([[self currentSubviewController] respondsToSelector:@selector(search:)])
        [[self currentSubviewController] performSelector:@selector(search:) withObject:sender];
}

- (IBAction)changeGridSize:(id)sender
{
    if([[self currentSubviewController] respondsToSelector:@selector(changeGridSize:)])
        [[self currentSubviewController] performSelector:@selector(changeGridSize:) withObject:sender];
}

- (void)magnifyWithEvent:(NSEvent*)event {

    if(![[[self toolbar] gridSizeSlider] isEnabled])
        return;

    if(![[self currentSubviewController] respondsToSelector:@selector(changeGridSize:)])
        return;

    CGFloat zoomChange = [event magnification];
    CGFloat zoomValue = [[[self toolbar] gridSizeSlider] floatValue];

    [[[self toolbar] gridSizeSlider] setFloatValue:zoomValue+zoomChange];
    [self changeGridSize:[[self toolbar] gridSizeSlider]];
}


#pragma mark - FileMenu Actions

- (IBAction)newCollection:(id)sender
{
    if([[self currentSubviewController] respondsToSelector:@selector(addCollectionAction:)])
        [[self currentSubviewController] performSelector:@selector(addCollectionAction:) withObject:sender];
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
    OELibraryCategory category = [[[self toolbar] categorySelector] selectedSegment];

    if(category == _selectedCategory) {
        return;
    }

    [self _showSubviewControllerForCategory:category];

    _selectedCategory = category;
    
    [[NSUserDefaults standardUserDefaults] setInteger:_selectedCategory forKey:OELibraryLastCategoryKey];
}

- (NSViewController<OELibrarySubviewController> *)_subviewControllerForCategory:(OELibraryCategory)category
{
    NSViewController<OELibrarySubviewController> *viewController = nil;

    switch (category) {
        case OELibraryCategoryGames:
            return self.libraryGamesViewController;
        case OELibraryCategorySaveStates:
            return self.saveStatesViewController;
        case OELibraryCategoryScreenshots:
            return self.screenshotsViewController;
        case OELibraryCategoryHomebrew:
            return self.homebrewViewController;
        default:
            @throw [NSException exceptionWithName:NSInvalidArgumentException
                                           reason:@"Unrecognized category."
                                         userInfo:nil];
            break;
    }
    return viewController;
}

- (void)_showSubviewControllerForCategory:(OELibraryCategory)category
{
    const CGFloat crossfadeDuration = 0.35;
    
    NSViewController <OELibrarySubviewController> *newViewController = [self _subviewControllerForCategory:category];
    NSViewController <OELibrarySubviewController> *currentSubviewController = self.currentSubviewController;
    
    newViewController.view.frame = self.view.bounds;
    newViewController.view.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
    
    if(currentSubviewController) {
        
        [NSAnimationContext beginGrouping];
        
        [[NSAnimationContext currentContext] setDuration:crossfadeDuration];
        
        [self transitionFromViewController:currentSubviewController
                          toViewController:newViewController
                                   options:NSViewControllerTransitionCrossfade
                         completionHandler:nil];
        
        [NSAnimationContext endGrouping];
        
    } else {
        
        [self.view addSubview:newViewController.view];
    }
    
    self.currentSubviewController = newViewController;

    [self.view.window makeFirstResponder:newViewController.view];
}

#pragma mark -

- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
    SEL action = [menuItem action];
    if(action == @selector(newCollectionFolder:)) return NO;

    const id currentSubviewController = [self currentSubviewController];

    if(action == @selector(startSelectedGame:))
    {
        return ([currentSubviewController isKindOfClass:[OELibraryGamesViewController class]] && [currentSubviewController respondsToSelector:@selector(selectedGames)] && [[currentSubviewController selectedGames] count] != 0) ||
            ([currentSubviewController isKindOfClass:[OEMediaViewController class]] && [[currentSubviewController representedObject] isKindOfClass:[OEDBSavedGamesMedia class]] && [[currentSubviewController selectedSaveStates] count] != 0);
    }

    if(action == @selector(startSaveState:))
    {
        return [currentSubviewController isKindOfClass:[OEGameCollectionViewController class]] && [currentSubviewController respondsToSelector:@selector(selectedSaveStates)] && [[currentSubviewController selectedSaveStates] count] != 0;
    }

    if(action == @selector(find:))
    {
        return [[[self toolbar] searchField] isEnabled];
    }

    OEButton *button = nil;
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
- (IBAction)startSelectedGame:(id)sender
{
    NSMutableArray *gamesToStart = [NSMutableArray new];

    if([sender isKindOfClass:[OEDBGame class]]){
        [gamesToStart addObject:sender];
    }
    else
    {
        NSAssert([(id)[self currentSubviewController] respondsToSelector:@selector(selectedGames)], @"Attempt to start a game from a view controller that doesn't announc selectedGames");

        [gamesToStart addObjectsFromArray:[(id <OELibrarySubviewController>)[self currentSubviewController] selectedGames]];
    }

    NSAssert([gamesToStart count] > 0, @"Attempt to start a game while the selection is empty");

    if([[self delegate] respondsToSelector:@selector(libraryController:didSelectGame:)])
    {
        for(OEDBGame *game in gamesToStart) [[self delegate] libraryController:self didSelectGame:game];
    }
}

- (void)startGame:(OEDBGame*)game
{
    [[self delegate] libraryController:self didSelectGame:game];
}

- (IBAction)startSaveState:(id)sender
{
    OEMediaViewController *media = (OEMediaViewController *)[self currentSubviewController];
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
