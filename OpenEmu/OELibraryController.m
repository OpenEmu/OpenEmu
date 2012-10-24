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

NSString * const OESidebarVisibleKey = @"isSidebarVisible";
NSString * const OESidebarWidthKey = @"lastSidebarWidth";
NSString * const OELastCollectionSelectedKey = @"lastCollectionSelected";

extern NSString * const OESidebarSelectionDidChangeNotificationName;

@interface OELibraryController ()
- (void)OE_showFullscreen:(BOOL)fsFlag animated:(BOOL)animatedFlag;

@property NSMutableDictionary *subviewControllers;
- (NSViewController <OELibrarySubviewController>*)viewControllerWithClassName:(NSString*)className;
@end

@implementation OELibraryController
@synthesize database;
@synthesize sidebarChangesWindowSize;
@synthesize currentViewController;
@synthesize sidebarController, mainSplitView, mainContentPlaceholderView;
@synthesize toolbarFlowViewButton, toolbarGridViewButton, toolbarListViewButton;
@synthesize toolbarSearchField, toolbarSidebarButton, toolbarAddToSidebarButton, toolbarSlider;
@synthesize cachedSnapshot;
@synthesize delegate;
@synthesize subviewControllers;

+ (void)initialize
{
    if(self == [OELibraryController class])
    {
        [[NSUserDefaults standardUserDefaults] registerDefaults:@{ OESidebarVisibleKey : @YES }];
    }
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObject:self];
}

#pragma mark -
#pragma mark NSViewController stuff
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
    
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    
    // setup sidebar controller
    OESidebarController *sidebarCtrl = [self sidebarController];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(sidebarSelectionDidChange:) name:OESidebarSelectionDidChangeNotificationName object:sidebarCtrl];
    
    [sidebarCtrl setDatabase:[self database]];
    [self setSidebarChangesWindowSize:YES];

    // setup splitview
    OELibrarySplitView *splitView = [self mainSplitView];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(splitViewDidResizeSubviews:) name:OELibrarySplitViewDidResizeSubviewsNotification object:splitView];

    [splitView setMinWidth:[defaults doubleForKey:OESidebarMinWidth]];
    [splitView setMainViewMinWidth:[defaults doubleForKey:OEMainViewMinWidth]];
    [splitView setSidebarMaxWidth:[defaults doubleForKey:OESidebarMaxWidth]];
    [splitView adjustSubviews];
}

- (void)viewDidAppear
{
    [super viewDidAppear];
    
    [self layoutToolbarItems];
    
    [[self sidebarController] reloadData];
    
    
    NSUserDefaults *standardUserDefaults = [NSUserDefaults standardUserDefaults];
    
    CGFloat splitterPos = 0;
    if([self isSidebarVisible]) splitterPos = [standardUserDefaults doubleForKey:OESidebarWidthKey];
    
    OELibrarySplitView *splitView = [self mainSplitView];
    [splitView setResizesLeftView:YES];
    [splitView setSplitterPosition:splitterPos animated:NO];
    [splitView setResizesLeftView:NO];
}

- (void)viewWillDisappear
{
    [super viewWillDisappear];
    
    NSView *toolbarItemContainer = [[self toolbarSearchField] superview];
    
    [toolbarItemContainer setAutoresizingMask:NSViewWidthSizable];
}

#pragma mark -
#pragma mark Toolbar
- (IBAction)toggleSidebar:(id)sender
{
    NSUserDefaults *standardDefaults = [NSUserDefaults standardUserDefaults];
    OELibrarySplitView *mainSplit = [self mainSplitView];
    
    BOOL opening = ![self isSidebarVisible];
    
    CGFloat widthCorrection = 0;
    if(opening)
    {
        widthCorrection = [standardDefaults doubleForKey:OESidebarWidthKey];
    }
    else
    {
        CGFloat lastWidth = [mainSplit splitterPosition];
        [standardDefaults setDouble:lastWidth forKey:OESidebarWidthKey];
        widthCorrection = -lastWidth;
    }
    
    if([self sidebarChangesWindowSize])
    {
        NSWindow *window = [[self view] window];
        NSRect frameRect = [window frame];
        
        frameRect.origin.x -= widthCorrection;
        frameRect.size.width += widthCorrection;
        NSRect splitViewRect = [mainSplit frame];
        splitViewRect.size.width += widthCorrection;
        
        [mainSplit setResizesLeftView:YES];
        [window setFrame:frameRect display:YES animate:YES];
        [mainSplit setResizesLeftView:NO];
    }
    else
    {
        widthCorrection = MAX(widthCorrection, 0);
        [mainSplit setSplitterPosition:widthCorrection animated:YES];
    }
    
    if(!opening) [standardDefaults setDouble:abs(widthCorrection) forKey:OESidebarWidthKey];

    [self setSidebarVisible:opening];
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

#pragma mark FileMenu Actions
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
#pragma mark Edit Menu
- (IBAction)find:(id)sender
{
	[[[self view] window] makeFirstResponder:toolbarSearchField];
}
#pragma mark -
#pragma mark Menu Items
- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
    if([menuItem action] == @selector(newCollectionFolder:)) return NO;
    
    if([menuItem action] == @selector(editSmartCollection:))
        return [[[self sidebarController] selectedSidebarItem] isKindOfClass:[OEDBSmartCollection class]];
    
    if([menuItem action] == @selector(startGame:))
        return [[self currentViewController] respondsToSelector:@selector(selectedGames)] && [[[self currentViewController] selectedGames] count] != 0;
    
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

#pragma mark -
#pragma mark Import
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
    NSAssert([(id)[self currentViewController] respondsToSelector:@selector(selectedGames)], @"Attempt to start a game from a view controller that doesn't announc selectedGames");
    id selectedGame = [[(id <OELibrarySubviewController>)[self currentViewController] selectedGames] lastObject];
    NSAssert(selectedGame != nil, @"Attempt to start a game while the selection is empty");

    if([[self delegate] respondsToSelector:@selector(libraryController:didSelectGame:)])
        [[self delegate] libraryController:self didSelectGame:selectedGame];
}

- (void)startSelectedGameWithSaveState:(id)stateItem
{
    OEDBSaveState *saveState = [stateItem representedObject];
    
    NSAssert(saveState != nil, @"Attempt to start a save state without valid item");

    if([[self delegate] respondsToSelector:@selector(libraryController:didSelectSaveState:)])
        [[self delegate] libraryController:self didSelectSaveState:saveState];
}

#pragma mark -
#pragma mark Sidebar Helpers
- (void)showViewController:(NSViewController <OELibrarySubviewController>*)nextViewController
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
    
    NSString *viewControllerClasName = [selectedItem viewControllerClassName];
    NSViewController <OELibrarySubviewController> *viewController = [self viewControllerWithClassName:viewControllerClasName];
    [viewController setRepresentedObject:selectedItem];

    [self showViewController:viewController];

    itemID = [selectedItem sidebarID];
    [viewController restoreState:[self OE_storedStateForSidebarItemWithID:itemID]];
}

- (void)OE_storeState:(id)state forSidebarItemWithID:(NSString*)itemID
{
    if(!itemID || !state) return;
    
    NSUserDefaults      *standardUserDefaults = [NSUserDefaults standardUserDefaults];
    NSDictionary        *libraryStates        = [standardUserDefaults valueForKey:@"Library States"];
    NSMutableDictionary *mutableLibraryStates = libraryStates ? [libraryStates mutableCopy] : [NSMutableDictionary dictionary];
    
    [mutableLibraryStates setObject:state forKey:itemID];
    [standardUserDefaults setObject:mutableLibraryStates forKey:@"Library States"];
}

- (id)OE_storedStateForSidebarItemWithID:(NSString*)itemID
{
    if(!itemID) return nil;
    
    NSUserDefaults      *standardUserDefaults = [NSUserDefaults standardUserDefaults];
    NSDictionary        *libraryStates        = [standardUserDefaults valueForKey:@"Library States"];
    
    return [libraryStates objectForKey:itemID];
}


#pragma mark - Split view helpers

- (void)splitViewDidResizeSubviews:(NSNotification *)notification
{
    [self layoutToolbarItems];
}


#pragma mark -
#pragma mark Properties
- (void)setSidebarChangesWindowSize:(BOOL)flag
{
    flag = !!flag;
    
    if(sidebarChangesWindowSize != flag)
    {
        sidebarChangesWindowSize = flag;
        
        [[self toolbarSidebarButton] setState:(flag == ([[self mainSplitView] splitterPosition] == 0) ? NSOffState : NSOnState)];
        [[self toolbarSidebarButton] display];
    }
}

- (BOOL)sidebarChangesWindowSize
{
    return sidebarChangesWindowSize;
}

- (BOOL)isSidebarVisible
{
    return [[NSUserDefaults standardUserDefaults] boolForKey:OESidebarVisibleKey];
}
- (void)setSidebarVisible:(BOOL)value
{
    [[NSUserDefaults standardUserDefaults] setBool:value forKey:OESidebarVisibleKey];
}

#pragma mark -
#pragma mark Private
- (void)OE_showFullscreen:(BOOL)fsFlag animated:(BOOL)animatedFlag
{
    [self setSidebarChangesWindowSize:!fsFlag];
    [[self mainSplitView] setDrawsWindowResizer:!fsFlag];
    
    [NSApp setPresentationOptions:(fsFlag ? NSApplicationPresentationAutoHideDock | NSApplicationPresentationAutoHideToolbar : NSApplicationPresentationDefault)];
}

- (NSViewController <OELibrarySubviewController>*)viewControllerWithClassName:(NSString*)className
{
    if(![subviewControllers valueForKey:className])
    {
        Class viewControllerClass = NSClassFromString(className);
        if(viewControllerClass)
        {
            NSViewController <OELibrarySubviewController>*viewController = [[viewControllerClass alloc] init];
            [subviewControllers setObject:viewController forKey:className];
        }
    }
    return [subviewControllers valueForKey:className];
}
#pragma mark -
- (void)layoutToolbarItems
{
    OELibrarySplitView *splitView = [self mainSplitView];
    NSView *toolbarItemContainer = [[self toolbarSearchField] superview];
    
    CGFloat splitterPosition = [splitView splitterPosition];
    
    if(splitterPosition != 0) [[NSUserDefaults standardUserDefaults] setDouble:splitterPosition forKey:OESidebarWidthKey];
    
    [toolbarItemContainer setFrame:NSMakeRect(splitterPosition, 0.0, NSWidth([[toolbarItemContainer superview] bounds]) - splitterPosition, 44.0)];
}
@end

