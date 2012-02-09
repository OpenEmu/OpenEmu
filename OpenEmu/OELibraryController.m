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

#import "OEHUDGameWindow.h"
#import "OEROMImporter.h"

#import "OEPlugin.h"
#import "OECorePlugin.h"
#import "OESystemPlugin.h"
#import "OECompositionPlugin.h"

#import "NSControl+OEAdditions.h"

#import "OEGameDocument.h"


#ifndef NSWindowWillEnterFullScreenNotification
NSString *const NSWindowWillEnterFullScreenNotification = @"OEWindowWillEnterFullScreenNotification";
NSString *const NSWindowWillExitFullScreenNotification = @"OEWindowWillExitFullScreenNotification";
#endif

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
@synthesize sidebarController, collectionViewController, mainSplitView;

@synthesize searchResults;

- (id)initWithWindowController:(OEMainWindowController*)windowController andDatabase:(OELibraryDatabase*)aDatabase
{
    if((self = [super initWithWindowController:windowController]))
    {
        [self setDatabase:aDatabase];
    }
    
    return self;
}

- (void)dealloc
{
    NSLog(@"Dealloc OELibraryController");
    
    [[NSNotificationCenter defaultCenter] removeObject:self];
    
    [self setCollectionViewController:nil];
    [self setMainSplitView:nil];
    [self setSidebarController:nil];
    [self setDatabase:nil];
    [self setRomImporter:nil];
    
    [super dealloc];
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
    
    [self setRomImporter:[[[OEROMImporter alloc] initWithDatabase:[self database]] autorelease]];
    
    [self setSearchResults:[[[NSMutableArray alloc] initWithCapacity:1] autorelease]];
    
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
    NSView *rightContentView = [splitView rightContentView];
    [rightContentView addSubview:[collectionVC view]];
    [[collectionVC view] setFrame:[rightContentView bounds]];
    
    [splitView adjustSubviews];
}

- (void)awakeFromNib
{
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowFullscreenEnter:) name:NSWindowWillEnterFullScreenNotification object:[[self windowController] window]];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowFullscreenExit:) name:NSWindowWillExitFullScreenNotification object:[[self windowController] window]];
}

- (void)viewDidAppear
{
    [super viewDidAppear];
    
    OEMainWindowController *windowController = [self windowController];
    NSView *toolbarItemContainer = [[windowController toolbarSearchField] superview];
    [toolbarItemContainer setAutoresizingMask:0];
    
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
    if([standardUserDefaults boolForKey:UDSidebarVisibleKey])
        splitterPos = [standardUserDefaults doubleForKey:UDSidebarWidthKey];
    
    OELibrarySplitView *splitView = [self mainSplitView];
    [splitView setResizesLeftView:YES];
    [splitView setSplitterPosition:splitterPos animated:NO];
    [splitView setResizesLeftView:NO];
}

- (void)viewWillDisappear
{
    [super viewWillDisappear];
    
    OEMainWindowController *windowController = [self windowController];
    NSView *toolbarItemContainer = [[windowController toolbarSearchField] superview];
    
    [toolbarItemContainer setAutoresizingMask:NSViewWidthSizable];
}

#pragma mark -
#pragma mark Toolbar Actions

- (IBAction)toggleSidebar:(id)sender
{
    NSUserDefaults *standardDefaults = [NSUserDefaults standardUserDefaults];
    OELibrarySplitView *mainSplit = [self mainSplitView];
    
    BOOL opening = [mainSplit splitterPosition] == 0;
    CGFloat widthCorrection = 0;
    if(opening)
    {
        widthCorrection = [standardDefaults doubleForKey:UDSidebarWidthKey];
    }
    else
    {
        CGFloat lastWidth = [mainSplit splitterPosition];
        [standardDefaults setDouble:lastWidth forKey:UDSidebarWidthKey];
        widthCorrection = -1 * lastWidth;
    }
    
    if([self sidebarChangesWindowSize])
    {
        NSWindow *window = [[self windowController] window];
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
    
    [[[self windowController] toolbarSidebarButton] setImage:image];
    
    [standardDefaults setBool:opening forKey:UDSidebarVisibleKey];
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
    NSArray *selection = [[self collectionViewController] selectedGames];
    if(selection == nil)
    {
        DLog(@"No game. This should not be possible from UI (item disabled)");
        return;
    }
    
    OEDBGame             *selectedGame  = [selection lastObject];
    NSDocumentController *docController = [NSDocumentController sharedDocumentController];
    OEGameDocument       *document      = [[OEGameDocument alloc] initWithGame:selectedGame];
    
    if(document == nil) return;
    [docController addDocument:document];
    [document release];
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
    
    [supportedFileExtensions release];
    
    searchString = [searchString substringWithRange:NSMakeRange(0, [searchString length] - 4)];
    
    NSLog(@"SearchString: %@", searchString);
    
    MDQueryRef searchQuery = MDQueryCreate(kCFAllocatorDefault, (CFStringRef)searchString, NULL, NULL);
    
    if(searchQuery)
    {
        NSLog(@"Valid search query ref");
        
        [[self searchResults] removeAllObjects];
        
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(finalizeSearchResults:)
                                                     name:(NSString *)kMDQueryDidFinishNotification
                                                   object:(id)searchQuery];
        
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(updateSearchResults:)
                                                     name:(NSString *)kMDQueryProgressNotification
                                                   object:(id)searchQuery];
        
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(updateSearchResults:)
                                                     name:(NSString *)kMDQueryDidUpdateNotification
                                                   object:(id)searchQuery];
        
        MDQuerySetSearchScope(searchQuery, (CFArrayRef) [NSArray arrayWithObject:(NSString *) kMDQueryScopeComputer /*kMDQueryScopeComputer */], 0);
        
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
    
    MDQueryRef searchQuery = (MDQueryRef)[notification object];
    
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
        NSString *resultPath = (NSString *)MDItemCopyAttribute(resultItem, kMDItemPath);
        
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
            
            [resultDict release];
        }
        
        [resultPath release];
    }
}

- (void)finalizeSearchResults:(NSNotification *)notification
{
    MDQueryRef searchQuery = (MDQueryRef)[notification object];
    
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
        
        [[[self windowController] toolbarSidebarButton] setImage:image];
        [[[self windowController] toolbarSidebarButton] display];
    }
}

- (BOOL)sidebarChangesWindowSize
{
    return sidebarChangesWindowSize;
}

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
    OEMainWindowController *windowController = [self windowController];
    
    [[windowController toolbarFlowViewButton] setEnabled:YES];
    [[windowController toolbarFlowViewButton] setTarget:self andAction:@selector(switchToFlowView:)];
    
    [[windowController toolbarGridViewButton] setEnabled:YES];
    [[windowController toolbarGridViewButton] setTarget:self andAction:@selector(switchToGridView:)];
    
    [[windowController toolbarListViewButton] setEnabled:YES];
    [[windowController toolbarListViewButton] setTarget:self andAction:@selector(switchToListView:)];
    
    [[windowController toolbarSearchField] setEnabled:YES];
    [[windowController toolbarSearchField] setTarget:self andAction:@selector(search:)];
    
    [[windowController toolbarSlider] setEnabled:[[windowController toolbarGridViewButton] state]];
    [[windowController toolbarSlider] setTarget:self andAction:@selector(changeGridSize:)];
    
    [[windowController toolbarSidebarButton] setEnabled:YES];
    [[windowController toolbarSidebarButton] setTarget:self andAction:@selector(toggleSidebar:)];
    
    [[windowController toolbarAddToSidebarButton] setEnabled:YES];
    [[windowController toolbarAddToSidebarButton] setTarget:[self sidebarController] andAction:@selector(addCollectionAction:)];
}

- (void)layoutToolbarItems
{
    OEMainWindowController *windowController = [self windowController];
    OELibrarySplitView *splitView = [self mainSplitView];
    NSView *toolbarItemContainer = [[windowController toolbarSearchField] superview];
    
    CGFloat splitterPosition = [splitView splitterPosition];
    
    if(splitterPosition != 0) [[NSUserDefaults standardUserDefaults] setDouble:splitterPosition forKey:UDSidebarWidthKey];
    
    [toolbarItemContainer setFrame:NSMakeRect(splitterPosition, 0.0, NSWidth([[splitView rightContentView] frame]), 44.0)];
}

- (void)windowFullscreenExit:(NSWindow *)window
{
    [self setSidebarChangesWindowSize:YES];
}

- (void)windowFullscreenEnter:(NSWindow *)window
{
    [self setSidebarChangesWindowSize:NO];
}

@end
