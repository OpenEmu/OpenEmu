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

#import "OELibrarySubviewController.h"
#import "OEROMImporter.h"

#import "OEDBGame.h"
@import OpenEmuKit;
#import "OEDBSmartCollection.h"

#import "OEGameCollectionViewController.h"
#import "OEMediaViewController.h"
#import "OEDBSavedGamesMedia.h"
#import "OEDBScreenshotsMedia.h"
#import "OEHomebrewViewController.h"

#import "OELibraryGamesViewController.h"

#import "OpenEmu-Swift.h"

#pragma mark - Exported variables
NSString * const OELastGridSizeKey        = @"lastGridSize";
NSString * const OELibraryStatesKey       = @"Library States";
NSString * const OELibraryLastCategoryKey = @"OELibraryLastCategoryKey";

typedef NS_ENUM(NSUInteger, OELibraryCategory) {
    OELibraryCategoryGames,
    OELibraryCategorySaveStates,
    OELibraryCategoryScreenshots,
    OELibraryCategoryHomebrew,

    OELibraryCategoryCount
};

#pragma mark - Imported variables

@interface OELibraryController ()

@property OELibraryCategory selectedCategory;
@property (nonatomic, readwrite) NSViewController *currentSubviewController;

// Library subview controllers.
@property (strong, readonly) OELibraryGamesViewController *libraryGamesViewController;
@property (strong, readonly) OEMediaViewController *saveStatesViewController;
@property (strong, readonly) OEMediaViewController *screenshotsViewController;
@property (strong, readonly) OEHomebrewViewController *homebrewViewController;

@end

@implementation OELibraryController

+ (void)initialize
{
    // Make sure not to reinitialize for subclassed objects
    if(self != [OELibraryController class]) return;

    [[NSUserDefaults standardUserDefaults] registerDefaults:@{ OELastGridSizeKey : @1.0f }];
}

- (NSString *)nibName
{
    return @"OELibraryController";
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    if([self database] == nil) [self setDatabase:[OELibraryDatabase defaultDatabase]];
    
    //set initial zoom value
    NSSlider *sizeSlider = self.toolbar.gridSizeSlider;
    CGFloat defaultZoomValue = [[NSUserDefaults standardUserDefaults] floatForKey:OELastGridSizeKey];
    [sizeSlider setFloatValue:defaultZoomValue];
    
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
    [self addChildViewController:_libraryGamesViewController];
    _libraryGamesViewController.database = self.database;
    // HACK: force NIB to load, so GameScannerViewController is initialized
    (void)_libraryGamesViewController.view;
    
    _saveStatesViewController = [[OEMediaViewController alloc] init];
    [self addChildViewController:_saveStatesViewController];
    _saveStatesViewController.database = self.database;
    _saveStatesViewController.representedObject = [OEDBSavedGamesMedia sharedDBSavedGamesMedia];
    
    _screenshotsViewController = [[OEMediaViewController alloc] init];
    [self addChildViewController:_screenshotsViewController];
    _screenshotsViewController.database = self.database;
    _screenshotsViewController.representedObject = [OEDBScreenshotsMedia sharedDBScreenshotsMedia];
    
    _homebrewViewController = [[OEHomebrewViewController alloc] init];
    [self addChildViewController:_homebrewViewController];
    _homebrewViewController.database = self.database;
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

- (IBAction)switchToView:(id)sender
{
    if(self.toolbar.viewModeSelector.selectedSegment == 0)
       [self switchToGridView:sender];
    if(self.toolbar.viewModeSelector.selectedSegment == 1)
       [self switchToListView:sender];
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

- (IBAction)decreaseGridSize:(id)sender
{
    if([self.currentSubviewController respondsToSelector:@selector(decreaseGridSize:)])
        [self.currentSubviewController performSelector:@selector(decreaseGridSize:) withObject:sender];
}

- (IBAction)increaseGridSize:(id)sender
{
    if([self.currentSubviewController respondsToSelector:@selector(increaseGridSize:)])
        [self.currentSubviewController performSelector:@selector(increaseGridSize:) withObject:sender];
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

- (IBAction)switchCategoryFromMenu:(id)sender
{
    self.toolbar.categorySelector.selectedSegment = [sender tag] - 100;
    [self switchCategory:nil];
}

- (NSViewController *)_subviewControllerForCategory:(OELibraryCategory)category
{
    NSViewController *viewController = nil;

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
    NSViewController *newViewController = [self _subviewControllerForCategory:category];
    NSViewController *currentSubviewController = self.currentSubviewController;
    
    newViewController.view.frame = self.view.bounds;
    newViewController.view.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
    
    if(currentSubviewController) {
        
        [self transitionFromViewController:currentSubviewController
                          toViewController:newViewController
                                   options:NSViewControllerTransitionNone
                         completionHandler:nil];
        
    } else {
        
        [self.view addSubview:newViewController.view];
    }
    
    self.currentSubviewController = newViewController;

    [self.view.window makeFirstResponder:newViewController.view];
}

#pragma mark - Validation

- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
    SEL action = [menuItem action];
    if(action == @selector(newCollectionFolder:)) return NO;

    const id currentSubviewController = [self currentSubviewController];

    if(action == @selector(startSelectedGame:))
    {
        if ([currentSubviewController conformsToProtocol:@protocol(OELibrarySubviewControllerGameSelection)])
        {
            __auto_type ctl = (id<OELibrarySubviewControllerGameSelection>)currentSubviewController;
            return ctl.selectedGames.count > 0;
        }

        return NO;
    }

    if(action == @selector(find:))
    {
        return [[[self toolbar] searchField] isEnabled];
    }

    if(action == @selector(switchCategoryFromMenu:))
    {
        if(self.toolbar.categorySelector.selectedSegment == menuItem.tag - 100)
            menuItem.state = NSControlStateValueOn;
        else
            menuItem.state = NSControlStateValueOff;
        return YES;
    }
    
    if(action == @selector(decreaseGridSize:) || action == @selector(increaseGridSize:))
    {
        return self.toolbar.gridSizeSlider.isEnabled;
    }
    
    NSSegmentedControl *viewModeSelector = self.toolbar.viewModeSelector;
    if(action == @selector(switchToGridView:)) {
        menuItem.state = viewModeSelector.selectedSegment == 0 ? NSControlStateValueOn : NSControlStateValueOff;
        return viewModeSelector.isEnabled;
    }
    if(action == @selector(switchToListView:)) {
        menuItem.state = viewModeSelector.selectedSegment == 1 ? NSControlStateValueOn : NSControlStateValueOff;
        return viewModeSelector.isEnabled;
    }
    
    return YES;
}

- (BOOL)validateToolbarItem:(NSToolbarItem *)item
{
    if ([self.currentSubviewController respondsToSelector:@selector(validateToolbarItem:)])
        return [[self currentSubviewController] validateToolbarItem:item];
    return NO;
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
         if(result == NSModalResponseOK)
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

    if([sender isKindOfClass:[OEDBGame class]]) {
        [gamesToStart addObject:sender];
    }
    else
    {
        NSAssert([self.currentSubviewController conformsToProtocol:@protocol(OELibrarySubviewControllerGameSelection)], @"Attempt to start a game from a view controller that doesn't announce selectedGames");
        
        __auto_type gs = (id<OELibrarySubviewControllerGameSelection>)self.currentSubviewController;
        [gamesToStart addObjectsFromArray:gs.selectedGames];
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

@end
