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
#import "NSViewController+OEAdditions.h"
#import "OELibraryDatabase.h"
#import "OEDBSmartCollection.h"

#import "OESidebarController.h"
#import "OECollectionViewController.h"
#import "OELibrarySplitView.h"

#import "NSImage+OEDrawingAdditions.h"
#import "OEBackgroundColorView.h"

#import "OEGameView.h"

#import "OEROMImporter.h"

#import "OEPlugin.h"
#import "OECorePlugin.h"
#import "OESystemPlugin.h"
#import "OECompositionPlugin.h"

#import "NSControl+OEAdditions.h"

#import "OEGameDocument.h"

@interface OELibraryController ()

// spotlight search results.
@property(readwrite, retain) NSMutableArray *searchResults;

- (void)OE_showFullscreen:(BOOL)fsFlag animated:(BOOL)animatedFlag;

- (void)OE_setupMenu;
- (void)OE_setupToolbarItems;
@end

@implementation OELibraryController
@synthesize romImporter, sidebarChangesWindowSize;
@synthesize database;
@synthesize sidebarController, collectionViewController, mainSplitView, mainContentPlaceholderView;
@synthesize toolbarFlowViewButton, toolbarGridViewButton, toolbarListViewButton;
@synthesize toolbarSearchField, toolbarSidebarButton, toolbarAddToSidebarButton, toolbarSlider;
@synthesize delegate;

@synthesize searchResults;

- (id)initWithDatabase:(OELibraryDatabase *)aDatabase
{
    if((self = [super initWithNibName:@"Library" bundle:nil]))
    {
        [self setDatabase:aDatabase];
    }
    
    return self;
}

- (void)dealloc
{
    NSLog(@"Dealloc OELibraryController");
    
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
    
    if([self database] == nil) [self setDatabase:[OELibraryDatabase defaultDatabase]];
    
    [[self sidebarController] view];
    
    self.romImporter = [[OEROMImporter alloc] initWithDatabase:[self database]];
    self.searchResults = [[NSMutableArray alloc] initWithCapacity:1];
    
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    
    // setup sidebar controller
    OESidebarController *sidebarCtrl = [self sidebarController];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(sidebarSelectionDidChange:) name:@"SidebarSelectionChanged" object:sidebarCtrl];
    
    [sidebarCtrl setDatabase:[self database]];
    [self setSidebarChangesWindowSize:YES];
    
    // make sure view has been loaded already
    OECollectionViewController *collectionVC = [self collectionViewController];
    [collectionVC view];
    
    // Select first view
    [collectionVC setLibraryController:self];
    
    // setup splitview
    OELibrarySplitView *splitView = [self mainSplitView];
    [splitView setMinWidth:[defaults doubleForKey:UDSidebarMinWidth]];
    [splitView setMainViewMinWidth:[defaults doubleForKey:UDMainViewMinWidth]];
    [splitView setSidebarMaxWidth:[defaults doubleForKey:UDSidebarMaxWidth]];
    
    // add collection controller's view to splitview
    [collectionVC viewWillAppear];
    
    NSView *mainContentView = [self mainContentPlaceholderView];
    [mainContentView addSubview:[collectionVC view]];
    [[collectionVC view] setFrame:[mainContentView bounds]];
    
    [splitView adjustSubviews];
    
    [collectionVC viewDidAppear];
    
    [[self toolbarSidebarButton] setImage:[NSImage imageNamed:@"toolbar_sidebar_button_close"]];
    
    // Setup Toolbar Buttons
    [[self toolbarGridViewButton] setImage:[NSImage imageNamed:@"toolbar_view_button_grid"]];
    [[self toolbarFlowViewButton] setImage:[NSImage imageNamed:@"toolbar_view_button_flow"]];
    [[self toolbarListViewButton] setImage:[NSImage imageNamed:@"toolbar_view_button_list"]];
    
    [[self toolbarAddToSidebarButton] setImage:[NSImage imageNamed:@"toolbar_add_button"]];
}

- (void)viewDidAppear
{
    [super viewDidAppear];
    
    [self OE_setupMenu];
    [self OE_setupToolbarItems];
    [self layoutToolbarItems];
    
    [[self collectionViewController] willShow];
    
    [[self sidebarController] reloadData];
    
    // Restore last selected collection item
    NSUserDefaults *standardUserDefaults = [NSUserDefaults standardUserDefaults];
    id collectionViewName = [standardUserDefaults valueForKey:UDLastCollectionSelectedKey];
    id collectionItem = nil;
    
    // Look for the collection item
    if(collectionViewName != nil && [collectionViewName isKindOfClass:[NSString class]])
    {
        NSPredicate *filterCollectionViewNamePredicate = [NSPredicate predicateWithFormat:@"collectionViewName == %@", collectionViewName];
        
        collectionItem = ([[[[self sidebarController] systems] filteredArrayUsingPredicate:filterCollectionViewNamePredicate] lastObject]
                          ? : [[[[self sidebarController] collections] filteredArrayUsingPredicate:filterCollectionViewNamePredicate] lastObject]);
    }
    
    // Select the found collection item, or select the first item by default
    if(collectionItem != nil) [[self sidebarController] selectItem:collectionItem];
    
    [[self sidebarController] outlineViewSelectionDidChange:nil];
    
    CGFloat splitterPos = 0;
    if([self isSidebarVisible]) splitterPos = [standardUserDefaults doubleForKey:UDSidebarWidthKey];
    
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
#pragma mark Toolbar Actions

- (IBAction)toggleSidebar:(id)sender
{
    NSUserDefaults *standardDefaults = [NSUserDefaults standardUserDefaults];
    OELibrarySplitView *mainSplit = [self mainSplitView];
    
    BOOL opening = ![self isSidebarVisible];
    
    CGFloat widthCorrection = 0;
    if(opening)
    {
        widthCorrection = [standardDefaults doubleForKey:UDSidebarWidthKey];
    }
    else
    {
        CGFloat lastWidth = [mainSplit splitterPosition];
        [standardDefaults setDouble:lastWidth forKey:UDSidebarWidthKey];
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
    
    if(!opening) [standardDefaults setDouble:abs(widthCorrection) forKey:UDSidebarWidthKey];
    
    NSImage *image = [NSImage imageNamed:
                      ([self sidebarChangesWindowSize] == opening
                       ? @"toolbar_sidebar_button_close"
                       : @"toolbar_sidebar_button_open")];
    
    [[self toolbarSidebarButton] setImage:image];
    
    [self setSidebarVisible:opening];
}

- (IBAction)switchToGridView:(id)sender
{
    [[self collectionViewController] selectGridView:sender];
}

- (IBAction)switchToListView:(id)sender
{
    [[self collectionViewController] selectListView:sender];
}

- (IBAction)switchToFlowView:(id)sender
{
    [[self collectionViewController] selectFlowView:sender];
}

- (IBAction)search:(id)sender
{
    [[self collectionViewController] search:sender];
}

- (IBAction)changeGridSize:(id)sender
{
    [[self collectionViewController] changeGridSize:sender];
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

#pragma mark -
#pragma mark Menu Items

- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
    if([menuItem action] == @selector(newCollectionFolder:)) return NO;
    
    if([menuItem action] == @selector(editSmartCollection:))
        return [[[self sidebarController] selectedCollection] isKindOfClass:[OEDBSmartCollection class]];
    
    if([menuItem action] == @selector(startGame:))
        return [[[self collectionViewController] selectedGames] count] != 0;
    
    return YES;
}

- (void)menuItemAction:(id)sender
{
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
             [romImporter setErrorBehaviour:OEImportErrorAskUser];
             [romImporter importROMsAtURLs:[openPanel URLs] inBackground:YES error:nil];
         }
     }];
}

#pragma mark -
- (IBAction)startGame:(id)sender
{
    OEDBGame *selectedGame = [[[self collectionViewController] selectedGames] lastObject];
    
    NSAssert(selectedGame != nil, @"Attempt to start a game while the selection is empty");
    
    if([[self delegate] respondsToSelector:@selector(libraryController:didSelectGame:)])
        [[self delegate] libraryController:self didSelectGame:selectedGame];
}

- (void)startSelectedGameWithSaveState:(id)stateItem
{
    OEDBSaveState *saveState = [stateItem representedObject];
    
    NSAssert(saveState != nil, @"Attempt to start a save state without valid item");

    if([[self delegate] respondsToSelector:@selector(libraryController:didSelectGame:)])
        [[self delegate] libraryController:self didSelectSaveState:saveState];
}

#pragma mark -
#pragma mark Spotlight Importing

- (void)discoverRoms
{
    NSMutableArray *supportedFileExtensions = [[OESystemPlugin supportedTypeExtensions] mutableCopy];
    
    // We skip common types by default.
    NSArray *commonTypes = [NSArray arrayWithObjects:@"bin", @"zip", @"elf", nil];
    
    [supportedFileExtensions removeObjectsInArray:commonTypes];
    
    //NSLog(@"Supported search Extensions are: %@", supportedFileExtensions);
    
    NSString *searchString = @"";
    for(NSString *extension in supportedFileExtensions)
    {
        searchString = [searchString stringByAppendingFormat:@"(kMDItemDisplayName == *.%@)", extension, nil];
        searchString = [searchString stringByAppendingString:@" || "];
    }
    
    
    searchString = [searchString substringWithRange:NSMakeRange(0, [searchString length] - 4)];
    
    NSLog(@"SearchString: %@", searchString);
    
    MDQueryRef searchQuery = MDQueryCreate(kCFAllocatorDefault, (__bridge CFStringRef)searchString, NULL, NULL);
    
    if(searchQuery)
    {
        NSLog(@"Valid search query ref");
        
        [[self searchResults] removeAllObjects];
        
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(finalizeSearchResults:)
                                                     name:(NSString*)kMDQueryDidFinishNotification
                                                   object:(__bridge id)searchQuery];
        
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(updateSearchResults:)
                                                     name:(NSString*)kMDQueryProgressNotification
                                                   object:(__bridge id)searchQuery];
        
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(updateSearchResults:)
                                                     name:(NSString*)kMDQueryDidUpdateNotification
                                                   object:(__bridge id)searchQuery];
        
        MDQuerySetSearchScope(searchQuery, (__bridge CFArrayRef) [NSArray arrayWithObject:(NSString*) kMDQueryScopeComputer /*kMDQueryScopeComputer */], 0);
        
        if(MDQueryExecute(searchQuery, kMDQueryWantsUpdates))
            NSLog(@"Searching for importable roms");
        else
        {
            CFRelease(searchQuery);
            searchQuery = nil;
            // leave this log message in...
            NSLog(@"MDQuery failed to start.");
        }
        
    }
    else
        NSLog(@"Invalid Search Query");
}

- (void)updateSearchResults:(NSNotification *)notification
{
    NSLog(@"updateSearchResults:");
    
    MDQueryRef searchQuery = (__bridge MDQueryRef)[notification object];
    
    // If you're going to have the same array for every iteration,
    // don't allocate it inside the loop !
    NSArray *excludedPaths = [NSArray arrayWithObjects:
                              @"System",
                              @"Library",
                              @"Developer",
                              @"Volumes",
                              @"Applications",
                              @"bin",
                              @"cores",
                              @"dev",
                              @"etc",
                              @"home",
                              @"net",
                              @"sbin",
                              @"private",
                              @"tmp",
                              @"usr",
                              @"var",
                              nil];
    
    // assume the latest result is the last index?
    for(CFIndex index = 0, limit = MDQueryGetResultCount(searchQuery); index < limit; index++)
    {
        MDItemRef resultItem = (MDItemRef)MDQueryGetResultAtIndex(searchQuery, index);
        NSString *resultPath = (__bridge_transfer NSString *)MDItemCopyAttribute(resultItem, kMDItemPath);
        
        // Nothing in common
        if([[resultPath pathComponents] firstObjectCommonWithArray:excludedPaths] == nil)
        {
            NSDictionary *resultDict = [[NSDictionary alloc] initWithObjectsAndKeys:
                                        resultPath, @"Path",
                                        [[resultPath lastPathComponent] stringByDeletingPathExtension], @"Name",
                                        nil];
            
            if(![[self searchResults] containsObject:resultDict])
            {
                [[self searchResults] addObject:resultDict];
                
                NSLog(@"Result Path: %@", resultPath);
            }
        }
    }
}

- (void)finalizeSearchResults:(NSNotification *)notification
{
    MDQueryRef searchQuery = (__bridge MDQueryRef)[notification object];    
    NSLog(@"Finished searching, found: %lu items", MDQueryGetResultCount(searchQuery));
    
    if(MDQueryGetResultCount(searchQuery))
    {
        [self importInBackground];
        
        //MDQueryStop(searchQuery);
        //CFRelease(MDQueryStop);
    }
    
    //CFRelease(MDQueryStop);
    //CFRelease(searchQuery);
}

- (void)importInBackground;
{
    NSLog(@"importInBackground");
    
    [[self romImporter] importROMsAtPaths:[[self searchResults] valueForKey:@"Path"] inBackground:YES error:nil];;
}

#pragma mark -
#pragma mark Sidebar Helpers

- (void)sidebarSelectionDidChange:(NSNotification *)notification
{
    [[self collectionViewController] setCollectionItem:[[notification userInfo] objectForKey:@"selectedCollection"]];
}

#pragma mark -
#pragma mark Properties

- (void)setSidebarChangesWindowSize:(BOOL)flag
{
    flag = !!flag;
    
    if(sidebarChangesWindowSize != flag)
    {
        sidebarChangesWindowSize = flag;
        
        NSImage *image = [NSImage imageNamed:
                          (flag == ([[self mainSplitView] splitterPosition] == 0)
                           ? @"toolbar_sidebar_button_open"
                           : @"toolbar_sidebar_button_close")];
        
        [[self toolbarSidebarButton] setImage:image];
        [[self toolbarSidebarButton] display];
    }
}

- (BOOL)sidebarChangesWindowSize
{
    return sidebarChangesWindowSize;
}

- (BOOL)isSidebarVisible              { return [[NSUserDefaults standardUserDefaults] boolForKey:UDSidebarVisibleKey];    }
- (void)setSidebarVisible:(BOOL)value { [[NSUserDefaults standardUserDefaults] setBool:value forKey:UDSidebarVisibleKey]; }

#pragma mark -
#pragma mark Private

- (void)OE_showFullscreen:(BOOL)fsFlag animated:(BOOL)animatedFlag
{
    [self setSidebarChangesWindowSize:!fsFlag];
    [[self mainSplitView] setDrawsWindowResizer:!fsFlag];
    
    [NSApp setPresentationOptions:(fsFlag ? NSApplicationPresentationAutoHideDock | NSApplicationPresentationAutoHideToolbar : NSApplicationPresentationDefault)];
}

#pragma mark -

- (void)OE_setupMenu
{
}

- (void)OE_setupToolbarItems
{
    [[self toolbarSlider] setEnabled:[[self toolbarGridViewButton] state]];
}

- (void)layoutToolbarItems
{
    OELibrarySplitView *splitView = [self mainSplitView];
    NSView *toolbarItemContainer = [[self toolbarSearchField] superview];
    
    CGFloat splitterPosition = [splitView splitterPosition];
    
    if(splitterPosition != 0) [[NSUserDefaults standardUserDefaults] setDouble:splitterPosition forKey:UDSidebarWidthKey];
    
    [toolbarItemContainer setFrame:NSMakeRect(splitterPosition, 0.0, NSWidth([[toolbarItemContainer superview] bounds]) - splitterPosition, 44.0)];
}

@end

@implementation OELibraryToolbarView

- (void)drawRect:(NSRect)dirtyRect
{
    [[NSColor blackColor] setFill];
    NSRectFill(dirtyRect);
    
    if(NSMinY(dirtyRect) > 44) return;
    
    NSRect viewRect = [self bounds];
    viewRect.origin.y = NSMinY(viewRect);
    viewRect.size.height = 44.0;
    
    NSColor *topLineColor   = [NSColor colorWithDeviceWhite:0.32 alpha:1];
    NSColor *gradientTop    = [NSColor colorWithDeviceWhite:0.20 alpha:1];
    NSColor *gradientBottom = [NSColor colorWithDeviceWhite:0.15 alpha:1];
    
    // Draw top line
    NSRect lineRect = NSMakeRect(0, 43, viewRect.size.width, 1);
    [topLineColor setFill];
    NSRectFill(lineRect);
    
    // Draw Gradient
    viewRect.origin.y = 0;
    viewRect.size.height -= 1;
    NSGradient *backgroundGradient = [[NSGradient alloc] initWithStartingColor:gradientTop endingColor:gradientBottom];
    [backgroundGradient drawInRect:viewRect angle:-90.0];
}

- (BOOL)isOpaque
{
    return NO;
}

@end
