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

#import "OECollectionViewController.h"
#import "NSImage+OEDrawingAdditions.h"
#import "OEGameControlsBar.h"
#import "OEMainWindowController.h"
#import "OEGameViewController.h"
#import "OEGameDocument.h"

#import "OELibraryController.h"
#import "OEROMImporter.h"

#import "OETableHeaderCell.h"
#import "OEListViewDataSourceItem.h"
#import "OERatingCell.h"
#import "OEHorizontalSplitView.h"

#import "OECoverGridDataSourceItem.h"
#import "OECoverFlowDataSourceItem.h"
#import "OEBlankSlateView.h"

#import "OEDBSystem.h"
#import "OESystemPlugin.h"
#import "OEDBGame.h"
#import "OEDBRom.h"
#import "OEDBCollection.h"
#import "OEDBSmartCollection.h"
#import "OEDBSaveState.h"

#import "OEDBAllGamesCollection.h"

#import "OECenteredTextFieldCell.h"
#import "OELibraryDatabase.h"

#import "NSViewController+OEAdditions.h"
#import "OEHUDAlert+DefaultAlertsAdditions.h"
#import "NSURL+OELibraryAdditions.h"

#import "OESidebarController.h"
#import "OETableView.h"

#import "OEArrayController.h"

#import "OECollectionDebugWindowController.h"
#import "OEBackgroundNoisePattern.h"
#pragma mark - Public variables

NSString * const OELastGridSizeKey       = @"lastGridSize";
NSString * const OELastCollectionViewKey = @"lastCollectionView";

#pragma mark - Private variables

typedef NS_ENUM(NSInteger, OECollectionViewControllerViewTag) {
    OEBlankSlateTag = -1,
    OEGridViewTag   = 0,
    OEFlowViewTag   = 1,
    OEListViewTag   = 2
};

static const float OE_coverFlowHeightPercentage = 0.75;
static NSArray *OE_defaultSortDescriptors;

static const CGFloat defaultGridWidth = 143;
static const NSSize defaultGridSize = (NSSize){26+142, defaultGridWidth};
#pragma mark -

@interface OECollectionViewController ()
{    
    IBOutlet NSView *gridViewContainer;// gridview
    IBOutlet OEGridView *gridView;// scrollview for gridview

    IBOutlet OEHorizontalSplitView *flowlistViewContainer; // cover flow and simple list container
    IBOutlet IKImageFlowView *coverFlowView;
    IBOutlet OETableView *listView;
    IBOutlet OEBlankSlateView *blankSlateView;
    
    NSDate *_listViewSelectionChangeDate;
}

- (void)OE_managedObjectContextDidUpdate:(NSNotification *)notification;
- (void)OE_reloadData;

- (NSMenu *)OE_menuForItemsAtIndexes:(NSIndexSet *)indexes;
- (NSMenu *)OE_saveStateMenuForGame:(OEDBGame *)game;
- (NSMenu *)OE_ratingMenuForGames:(NSArray *)games;
- (NSMenu *)OE_collectionsMenuForGames:(NSArray *)games;

@end

@implementation OECollectionViewController
{
    int _selectedViewTag;
}
@synthesize libraryController, gamesController;

+ (void)initialize
{
    // Make sure not to reinitialize for subclassed objects
    if(self != [OECollectionViewController class]) return;
    
    // Indicators for list view
    NSImage *image = [NSImage imageNamed:@"list_indicators"];
    
    // unselected states
    [image setName:@"list_indicators_playing" forSubimageInRect:NSMakeRect(0, 32, 12, 12)];
    [image setName:@"list_indicators_missing" forSubimageInRect:NSMakeRect(0, 24, 12, 12)];
    [image setName:@"list_indicators_unplayed" forSubimageInRect:NSMakeRect(0, 12, 12, 12)];
    
    // selected states
    [image setName:@"list_indicators_playing_selected" forSubimageInRect:NSMakeRect(12, 32, 12, 12)];
    [image setName:@"list_indicators_missing_selected" forSubimageInRect:NSMakeRect(12, 24, 12, 12)];
    [image setName:@"list_indicators_unplayed_selected" forSubimageInRect:NSMakeRect(12, 12, 12, 12)];
    
    [[NSUserDefaults standardUserDefaults] registerDefaults:@{ OELastGridSizeKey : @1.0f }];
}

- (id)init
{
    self = [super init];
    if (self) {
        _selectedViewTag = -2;
    }
    return self;
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [listView unbind:@"selectionIndexes"];
    gamesController = nil;
}

#pragma mark -
#pragma mark View Controller Stuff
- (void)loadView
{
    [super loadView];
        
    // Set up games controller
    gamesController = [[OEArrayController alloc] init];
    [gamesController setAutomaticallyRearrangesObjects:YES];
    [gamesController setAutomaticallyPreparesContent:NO];
    [gamesController setUsesLazyFetching:NO];
    
    NSManagedObjectContext *context = [[OELibraryDatabase defaultDatabase] mainThreadContext];
    //[gamesController bind:@"managedObjectContext" toObject:context withKeyPath:@"" options:nil];

    OE_defaultSortDescriptors = @[[NSSortDescriptor sortDescriptorWithKey:@"cleanDisplayName" ascending:YES selector:@selector(caseInsensitiveCompare:)]];
    
    [gamesController setManagedObjectContext:context];
    [gamesController setEntityName:@"Game"];
    [gamesController setSortDescriptors:OE_defaultSortDescriptors];
    [gamesController setFetchPredicate:[NSPredicate predicateWithValue:NO]];
    [gamesController setAvoidsEmptySelection:NO];
    
    // Setup View
    [[self view] setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];

    // Set up GridView
    [gridView setDelegate:self];
    [gridView setDataSource:self];
    [gridView setDraggingDestinationDelegate:self];
    [gridView setCellSize:defaultGridSize];

    //set initial zoom value
    NSSlider *sizeSlider = [[self libraryController] toolbarSlider];
    [sizeSlider setContinuous:YES];
    
    // set up flow view
    [coverFlowView setDelegate:self];
    [coverFlowView setDataSource:self];
    [coverFlowView setCellsAlignOnBaseline:YES];
    [coverFlowView setCellBorderColor:[NSColor blueColor]];
    
    // Set up list view
    [listView setTarget:self];
    [listView setDelegate:self];
    [listView setDataSource:self];
    [listView setDoubleAction:@selector(tableViewWasDoubleClicked:)];
    [listView setRowSizeStyle:NSTableViewRowSizeStyleCustom];
    [listView setRowHeight:20.0];
    [listView bind:@"selectionIndexes" toObject:gamesController withKeyPath:@"selectionIndexes" options:@{}];

    // There's no natural order for status indicators, so we don't allow that column to be sorted
    OETableHeaderCell *romStatusHeaderCell = [[listView tableColumnWithIdentifier:@"listViewStatus"] headerCell];
    [romStatusHeaderCell setClickable:NO];

    [listView registerForDraggedTypes:[NSArray arrayWithObjects:NSFilenamesPboardType, nil]];

    for(NSTableColumn *aColumn in [listView tableColumns])
    {
        if([[aColumn dataCell] isKindOfClass:[OECenteredTextFieldCell class]])
            [[aColumn dataCell] setWidthInset:9];

        [[aColumn headerCell] setAlignment:[[aColumn dataCell] alignment]];
    }
    
    // Setup BlankSlate View
    [blankSlateView setDelegate:self];
    [blankSlateView setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];
    [blankSlateView registerForDraggedTypes:[NSArray arrayWithObjects:NSFilenamesPboardType, nil]];
    
    // Watch the main thread's managed object context for changes
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(OE_managedObjectContextDidUpdate:) name:NSManagedObjectContextDidSaveNotification object:context];

    [[NSUserDefaults standardUserDefaults] addObserver:self forKeyPath:OEDisplayGameTitle options:0 context:NULL];

    // If the view has been loaded after a collection has been set via -setRepresentedObject:, set the appropriate
    // fetch predicate to display the items in that collection via -OE_reloadData. Otherwise, the view shows an
    // empty collection until -setRepresentedObject: is received again
    if([self representedObject]) [self OE_reloadData];
}

- (NSString *)nibName
{
    return @"CollectionView";
}

#pragma mark - KVO
- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if([keyPath isEqualToString:OEDisplayGameTitle])
        [self setNeedsReloadVisible];
}

#pragma mark - OELibrarySubviewControllerProtocol Implementation
- (void)setRepresentedObject:(id<OECollectionViewItemProtocol>)representedObject
{
    NSAssert([representedObject conformsToProtocol:@protocol(OECollectionViewItemProtocol)], @"OECollectionViewController accepts OECollectionViewItemProtocol represented objects only");

    if(representedObject == [self representedObject])
    {
        return;
    }
    [super setRepresentedObject:representedObject];
    
    [[listView tableColumnWithIdentifier:@"listViewConsoleName"] setHidden:![representedObject shouldShowSystemColumnInListView]];

    [self OE_reloadData];
}

- (id <OECollectionViewItemProtocol>)representedObject
{
    return [super representedObject];
}

- (id)encodeCurrentState
{
    if(![self libraryController] || _selectedViewTag==OEBlankSlateTag)
        return nil;
    
    NSMutableData    *data  = [NSMutableData data];
    NSKeyedArchiver  *coder = [[NSKeyedArchiver alloc] initForWritingWithMutableData:data];
    NSSlider *sizeSlider    = [[self libraryController] toolbarSlider];
    
    [coder encodeInt:_selectedViewTag forKey:@"selectedView"];
    [coder encodeFloat:[sizeSlider floatValue] forKey:@"sliderValue"];
    [coder encodeObject:[self selectedIndexes] forKey:@"selectionIndexes"];
    if([listView headerState]) [coder encodeObject:[listView headerState] forKey:@"listViewHeaderState"];
    if([listView sortDescriptors]) [coder encodeObject:[listView sortDescriptors] forKey:@"listViewSortDescriptors"];
    if(_selectedViewTag == OEGridViewTag) [coder encodeRect:[[gridView enclosingScrollView] documentVisibleRect] forKey:@"gridViewVisibleRect"];
    
    [coder finishEncoding];
    
    return data;
}

- (void)restoreState:(id)state
{
    if([self libraryController] == nil) return;
    
    NSInteger     selectedViewTag;
    CGFloat       sliderValue;
    NSIndexSet   *selectionIndexes;
    NSDictionary *listViewHeaderState = nil;
    NSArray      *listViewSortDescriptors = nil;
    NSRect        gridViewVisibleRect = NSZeroRect;
    
    NSSlider     *sizeSlider     = [[self libraryController] toolbarSlider];
    NSTextField  *searchField    = [[self libraryController] toolbarSearchField];

    NSKeyedUnarchiver *coder = state ? [[NSKeyedUnarchiver alloc] initForReadingWithData:state] : nil;
    if(coder)
    {
        selectedViewTag         = [coder decodeIntForKey:@"selectedView"];
        sliderValue             = [coder decodeFloatForKey:@"sliderValue"];
        selectionIndexes        = [coder decodeObjectForKey:@"selectionIndexes"];
        listViewHeaderState     = [coder decodeObjectForKey:@"listViewHeaderState"];
        listViewSortDescriptors = [coder decodeObjectForKey:@"listViewSortDescriptors"];
        gridViewVisibleRect     = [coder decodeRectForKey:@"gridViewVisibleRect"];
        
        [coder finishDecoding];
                
        // Make sure selected view tag is valid
        if(selectedViewTag != OEListViewTag && selectedViewTag != OEListViewTag && selectedViewTag != OEFlowViewTag)
           selectedViewTag = OEGridViewTag;
        
        // Make sure slider value is valid
        if(sliderValue < [sizeSlider minValue] || sliderValue > [sizeSlider maxValue])
           sliderValue = [sizeSlider doubleValue];
    }
    else
    {
        NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
        
        selectedViewTag  = [userDefaults integerForKey:OELastCollectionViewKey];
        sliderValue      = [userDefaults floatForKey:OELastGridSizeKey];
        selectionIndexes = [NSIndexSet indexSet];
    }

    if(listViewSortDescriptors == nil)
        listViewSortDescriptors = OE_defaultSortDescriptors;

    [gamesController setSelectionIndexes:selectionIndexes];
    [listView setSortDescriptors:listViewSortDescriptors];
    [listView setHeaderState:listViewHeaderState];
    [self OE_switchToView:selectedViewTag];
    [sizeSlider setFloatValue:sliderValue];
    [self changeGridSize:sizeSlider];
    [searchField setStringValue:@""];
	[self search:searchField];

    if(selectedViewTag == OEGridViewTag)
    {
        //[gridView setSelectionIndexes:selectionIndexes];
        [gridView scrollRectToVisible:gridViewVisibleRect];
    }

    [self OE_updateBlankSlate];
}

#pragma mark -
- (NSArray *)selectedGames
{
    return [gamesController selectedObjects];
}

- (NSIndexSet *)selectedIndexes
{
    return [gamesController selectionIndexes];
}

#pragma mark -
#pragma mark View Selection
- (IBAction)switchToGridView:(id)sender
{
    [sender setState:NSOnState];
    [self OE_switchToView:OEGridViewTag];
}

- (IBAction)switchToFlowView:(id)sender
{
    [sender setState:NSOnState];
    [self OE_switchToView:OEFlowViewTag];
}

- (IBAction)switchToListView:(id)sender
{
    [sender setState:NSOnState];
    [self OE_switchToView:OEListViewTag];
}

- (void)OE_switchToView:(OECollectionViewControllerViewTag)tag
{
    // Set sort descriptors and reload data
    NSArray *sortDescriptors = nil;
    BOOL reloadListView = NO;
    switch (tag) {
        case OEGridViewTag:
            sortDescriptors = OE_defaultSortDescriptors;
            break;
        default:
            sortDescriptors = [listView sortDescriptors];
            reloadListView = YES;
            break;
    }
	
    [[self gamesController] setSortDescriptors:sortDescriptors];

    if(reloadListView)
        [listView reloadData];
    else
        [gridView reloadData];
    
    if(_selectedViewTag == tag && tag != OEBlankSlateTag) return;

    [self OE_setupToolbarStatesForViewTag:tag];
    [self OE_showView:tag];

    _selectedViewTag = tag;
}

- (void)OE_showView:(OECollectionViewControllerViewTag)tag
{
    NSView *view;
    float splitterPosition = -1;
    switch (tag) {
        case OEBlankSlateTag:
            view = blankSlateView;
            break;
        case OEGridViewTag:
            view = gridViewContainer;
            break;
        case OEFlowViewTag:
            view = flowlistViewContainer;
            splitterPosition = NSHeight([view frame]) * OE_coverFlowHeightPercentage;
            break;
        case OEListViewTag:
            view = flowlistViewContainer; //  TODO: fix splitter position here too
            splitterPosition = 0.0f;
            break;
    }
    
    // Set splitter position (makes the difference between flow and list view)
    if(splitterPosition != -1)
        [flowlistViewContainer setSplitterPosition:splitterPosition animated:NO];
    
    if([view superview] == [self view]) return;
    
    // Determine if we are about to replace the current first responder or one of its superviews
    id firstResponder = [[[self view] window] firstResponder];
    BOOL makeFirstResponder = [firstResponder isKindOfClass:[NSView class]] && [firstResponder isDescendantOf:[self view]];
    
    // Remove subviews
    while([[[self view] subviews] count] != 0)
    {
        NSView *currentSubview = [[[self view] subviews] objectAtIndex:0];
        [currentSubview removeFromSuperview];
    }
    
    // Add new subview
    [[self view] addSubview:view];
    [view setFrame:[[self view] bounds]];
    
    // restore first responder if necessary
    if(makeFirstResponder)
        [[[self view] window] makeFirstResponder:view];
}

- (void)OE_setupToolbarStatesForViewTag:(OECollectionViewControllerViewTag)tag
{
    switch (tag)
    {
        case OEGridViewTag:
            [[[self libraryController] toolbarGridViewButton] setState:NSOnState];
            [[[self libraryController] toolbarFlowViewButton] setState:NSOffState];
            [[[self libraryController] toolbarListViewButton] setState:NSOffState];
            [[[self libraryController] toolbarSlider] setEnabled:YES];
            break;
        case OEFlowViewTag:
            [[[self libraryController] toolbarGridViewButton] setState:NSOffState];
            [[[self libraryController] toolbarFlowViewButton] setState:NSOnState];
            [[[self libraryController] toolbarListViewButton] setState:NSOffState];
            [[[self libraryController] toolbarSlider] setEnabled:NO];
            break;
        case OEListViewTag:
            [[[self libraryController] toolbarGridViewButton] setState:NSOffState];
            [[[self libraryController] toolbarFlowViewButton] setState:NSOffState];
            [[[self libraryController] toolbarListViewButton] setState:NSOnState];
            [[[self libraryController] toolbarSlider] setEnabled:NO];
            break;
        case OEBlankSlateTag:
            [[[self libraryController] toolbarSlider] setEnabled:NO];
            break;
    }
}

- (void)OE_updateBlankSlate
{
    // We cannot use [[gamesController arrangedObjects] count] since that takes into account the filter predicate
    NSFetchRequest *fetchRequest = [gamesController defaultFetchRequest];
    [fetchRequest setFetchLimit:1];
    NSUInteger count = [[gamesController managedObjectContext] countForFetchRequest:fetchRequest error:NULL];

    if(count)
    {
        [self OE_switchToView:[self OE_currentViewTagByToolbarState]];
        
        [[[self libraryController] toolbarGridViewButton] setEnabled:YES];
        [[[self libraryController] toolbarFlowViewButton] setEnabled:YES];
        [[[self libraryController] toolbarListViewButton] setEnabled:YES];
        
        [[[self libraryController] toolbarSearchField] setEnabled:YES];
    }
    else
    {
        [self OE_switchToView:OEBlankSlateTag];
        
        [[[self libraryController] toolbarGridViewButton] setEnabled:NO];
        [[[self libraryController] toolbarFlowViewButton] setEnabled:NO];
        [[[self libraryController] toolbarListViewButton] setEnabled:NO];
        
        [[[self libraryController] toolbarSearchField] setEnabled:NO];
        [[[self libraryController] toolbarSlider] setEnabled:NO];
        
        if([[self representedObject] isKindOfClass:[OEDBSystem class]])
            [blankSlateView setRepresentedSystemPlugin:[(OEDBSystem*)[self representedObject] plugin]];
        else if([[self representedObject] respondsToSelector:@selector(collectionViewName)])
            [blankSlateView setRepresentedCollectionName:[[self representedObject] collectionViewName]];
    }
}

- (OECollectionViewControllerViewTag)OE_currentViewTagByToolbarState
{
    if([[[self libraryController] toolbarGridViewButton] state] == NSOnState)
        return OEGridViewTag;
    else if([[[self libraryController] toolbarFlowViewButton] state] == NSOnState)
        return OEFlowViewTag;
    else
        return OEListViewTag;
}
#pragma mark -
- (void)viewDidAppear
{
    [super viewDidAppear];
    [self OE_updateBlankSlate];
}

#pragma mark -
#pragma mark Toolbar Actions
- (IBAction)search:(id)sender
{
    NSString *searchTerm = [sender stringValue];
    NSArray *tokens = [searchTerm componentsSeparatedByCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
    
    NSMutableArray *predarray = [NSMutableArray array];
    for(NSString *token in tokens)
    {
        if(token.length > 0)
        {
            NSPredicate *predicate = [NSPredicate predicateWithFormat:@"displayName contains[cd] %@", token];
            [predarray addObject:predicate];
        }
    }
    NSPredicate *pred = [NSCompoundPredicate andPredicateWithSubpredicates:predarray];
    
    [gamesController setFilterPredicate:pred];
    
    [listView reloadData];
    [coverFlowView performSelectorOnMainThread:@selector(reloadData) withObject:nil waitUntilDone:NO];
    [gridView reloadData];
}

- (IBAction)changeGridSize:(id)sender
{
    float zoomValue = [sender floatValue];
    [gridView setCellSize:NSMakeSize(roundf(26+142*zoomValue), roundf(defaultGridWidth*zoomValue))];
    [[NSUserDefaults standardUserDefaults] setValue:[NSNumber numberWithFloat:zoomValue] forKey:OELastGridSizeKey];
}

#pragma mark - GridView Delegate
- (void)imageBrowserSelectionDidChange:(IKImageBrowserView *)aBrowser
{
    [gamesController setSelectionIndexes:[aBrowser selectionIndexes]];

    if([[NSUserDefaults standardUserDefaults] boolForKey:OEDebugCollectionView] && [[[self gamesController] selectedObjects] count])
    {
        [[OECollectionDebugWindowController sharedController] setRepresentedObject:[[[self gamesController] selectedObjects] objectAtIndex:0]];
        [[[OECollectionDebugWindowController sharedController] window] makeKeyAndOrderFront:self];
    }
}

- (void)imageBrowser:(IKImageBrowserView *)aBrowser removeItemsAtIndexes:(NSIndexSet *)indexes
{
    [self deleteSelectedGames:aBrowser];
}

- (void)imageBrowser:(IKImageBrowserView *)aBrowser cellWasDoubleClickedAtIndex:(NSUInteger)index
{
    [[self libraryController] startGame:self];
}

#pragma mark -
#pragma mark GridView DraggingDestinationDelegate
- (BOOL)performDragOperation:(id<NSDraggingInfo>)sender
{
    NSPasteboard *draggingPasteboard = [sender draggingPasteboard];
    NSImage      *draggingImage      = [[NSImage alloc] initWithPasteboard:draggingPasteboard];
    NSInteger     draggingOperation  = [gridView draggingOperation];
    
    if (draggingOperation == IKImageBrowserDropOn && draggingImage)
    {
        NSUInteger droppedIndex = [gridView indexAtLocationOfDroppedItem];
        OEDBGame  *droppedGame  = [[gridView cellForItemAtIndex:droppedIndex] representedItem];
        
        [droppedGame setBoxImageByImage:draggingImage];
        [[droppedGame managedObjectContext] save:nil];
        [self OE_reloadData];
    }
    else if (draggingOperation == IKImageBrowserDropBefore || draggingOperation == IKImageBrowserDropOn)
    {
        NSArray *files = [draggingPasteboard propertyListForType:NSFilenamesPboardType];
        OEROMImporter *romImporter = [[[self libraryController] database] importer];
        OEDBCollection *collection = [[self representedObject] isKindOfClass:[OEDBCollection class]] ? [self representedObject] : nil;
        [romImporter importItemsAtPaths:files intoCollectionWithID:[collection permanentID]];
    }
    else if (draggingOperation == IKImageBrowserDropNone)
    {
        [NSApp presentError:[NSError errorWithDomain:@"Error in performing drag operation." code:-1 userInfo:nil]];
    }
    
    gridView.draggingOperation = IKImageBrowserDropNone;
    return YES;
}

#pragma mark -
#pragma mark GridView DataSource
- (NSUInteger)numberOfItemsInImageBrowser:(IKImageBrowserView *)aBrowser
{
    return [[gamesController arrangedObjects] count];
}

- (id)imageBrowser:(IKImageBrowserView *)aBrowser itemAtIndex:(NSUInteger)index
{
    return [[gamesController arrangedObjects] objectAtIndex:index];
}

- (NSMenu *)gridView:(OEGridView *)gridView menuForItemsAtIndexes:(NSIndexSet *)indexes
{
    return [self OE_menuForItemsAtIndexes:indexes];
}

#pragma mark -
#pragma mark Context Menu
- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
    return [[self libraryController] validateMenuItem:menuItem];
}

- (NSMenu *)OE_menuForItemsAtIndexes:(NSIndexSet*)indexes
{
    NSMenu *menu = [[NSMenu alloc] init];
    NSMenuItem *menuItem;
    NSArray *games = [[gamesController arrangedObjects] objectsAtIndexes:indexes];
    
    if([indexes count] == 1)
    {
        NSInteger index = [indexes lastIndex];
        [menu addItemWithTitle:NSLocalizedString(@"Play Game", @"") action:@selector(startGame:) keyEquivalent:@""];
        OEDBGame  *game = [[gamesController arrangedObjects] objectAtIndex:index];
        
        // Create Save Game Menu
        menuItem = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Play Save Games", @"") action:NULL keyEquivalent:@""];
        [menuItem setSubmenu:[self OE_saveStateMenuForGame:game]];
        [menu addItem:menuItem];
        
        [menu addItem:[NSMenuItem separatorItem]];
        
        // Create Rating Item
        menuItem = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Rating", @"") action:NULL keyEquivalent:@""];
        [menuItem setSubmenu:[self OE_ratingMenuForGames:games]];
        [menu addItem:menuItem];    
        [menu addItemWithTitle:NSLocalizedString(@"Show In Finder", @"") action:@selector(showSelectedGamesInFinder:) keyEquivalent:@""];
        [menu addItem:[NSMenuItem separatorItem]];

        if([[game status] isEqualTo:@(OEDBGameStatusOK)])
            [menu addItemWithTitle:NSLocalizedString(@"Download Cover Art", @"") action:@selector(downloadCoverArt:) keyEquivalent:@""];
        if([[game status] isEqualTo:@(OEDBGameStatusProcessing)])
            [menu addItemWithTitle:NSLocalizedString(@"Cancel Cover Art Download", @"") action:@selector(cancelCoverArtDownload:) keyEquivalent:@""];

        [menu addItemWithTitle:NSLocalizedString(@"Add Cover Art From File…", @"") action:@selector(addCoverArtFromFile:) keyEquivalent:@""];
        [menu addItemWithTitle:NSLocalizedString(@"Consolidate Files…", @"") action:@selector(consolidateFiles:) keyEquivalent:@""];

        //[menu addItemWithTitle:@"Add Save File To Game…" action:@selector(addSaveStateFromFile:) keyEquivalent:@""];
        [menu addItem:[NSMenuItem separatorItem]];
        // Create Add to collection menu
        NSMenuItem *collectionMenuItem = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Add To Collection", @"") action:NULL keyEquivalent:@""];
        [collectionMenuItem setSubmenu:[self OE_collectionsMenuForGames:games]];
        [menu addItem:collectionMenuItem];
        [menu addItem:[NSMenuItem separatorItem]];
        [menu addItemWithTitle:NSLocalizedString(@"Rename Game", @"") action:@selector(renameSelectedGame:) keyEquivalent:@""];
        [menu addItemWithTitle:NSLocalizedString(@"Delete Game", @"") action:@selector(deleteSelectedGames:) keyEquivalent:@""];
    }
    else
    {
        if([[NSUserDefaults standardUserDefaults] boolForKey:OEForcePopoutGameWindowKey])
        {
            [menu addItemWithTitle:NSLocalizedString(@"Play Games (Caution)", @"") action:@selector(startGame:) keyEquivalent:@""];
        }
        
        // Create Rating Item
        menuItem = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Rating", @"") action:NULL keyEquivalent:@""];
        [menuItem setSubmenu:[self OE_ratingMenuForGames:games]];
        [menu addItem:menuItem];    
        [menu addItemWithTitle:NSLocalizedString(@"Show In Finder", @"") action:@selector(showSelectedGamesInFinder:) keyEquivalent:@""];
        [menu addItem:[NSMenuItem separatorItem]];

        [menu addItemWithTitle:NSLocalizedString(@"Download Cover Art", @"") action:@selector(downloadCoverArt:) keyEquivalent:@""];
        [menu addItemWithTitle:NSLocalizedString(@"Add Cover Art From File…", @"") action:@selector(addCoverArtFromFile:) keyEquivalent:@""];
        [menu addItemWithTitle:NSLocalizedString(@"Consolidate Files…", @"") action:@selector(consolidateFiles:) keyEquivalent:@""];

        [menu addItem:[NSMenuItem separatorItem]];
        // Create Add to collection menu
        NSMenuItem *collectionMenuItem = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Add To Collection", @"") action:NULL keyEquivalent:@""];
        [collectionMenuItem setSubmenu:[self OE_collectionsMenuForGames:games]];
        [menu addItem:collectionMenuItem];
        
        [menu addItem:[NSMenuItem separatorItem]];
        [menu addItemWithTitle:NSLocalizedString(@"Delete Games", @"") action:@selector(deleteSelectedGames:) keyEquivalent:@""];
    }
    
    [menu setAutoenablesItems:YES];
    return menu;
}

- (NSMenu *)OE_saveStateMenuForGame:(OEDBGame *)game
{
    NSMenu    *saveGamesMenu = [[NSMenu alloc] init];
    NSSet     *roms = [game roms];
    
    [roms enumerateObjectsUsingBlock:^(id obj, BOOL *stop) {
        [obj removeMissingStates];
        
        NSMenuItem  *item;
        NSArray     *saveStates = [obj normalSaveStatesByTimestampAscending:NO];
        for(OEDBSaveState *saveState in saveStates)
        {
            NSString *itemTitle = [saveState name];
            if(!itemTitle || [itemTitle isEqualToString:@""])
                itemTitle = [NSString stringWithFormat:@"%@", [saveState timestamp]];
            
            item = [[NSMenuItem alloc] initWithTitle:itemTitle action:@selector(startSelectedGameWithSaveState:) keyEquivalent:@""];
            [item setRepresentedObject:saveState];
            [saveGamesMenu addItem:item];

            if([[NSUserDefaults standardUserDefaults] boolForKey:OEGameControlsBarCanDeleteSaveStatesKey])
            {
                NSMenuItem *alternateItem = [[NSMenuItem alloc] initWithTitle:itemTitle action:@selector(deleteSaveState:) keyEquivalent:@""];
                [alternateItem setAlternate:YES];
                [alternateItem setKeyEquivalentModifierMask:NSAlternateKeyMask];
                [alternateItem setRepresentedObject:saveState];
                [saveGamesMenu addItem:alternateItem];
            }
        }
    }];
    
    if([[saveGamesMenu itemArray] count] == 0)
    {
        [saveGamesMenu addItemWithTitle:NSLocalizedString(@"No Save States available", @"") action:NULL keyEquivalent:@""];
        [(NSMenuItem*)[[saveGamesMenu itemArray] lastObject] setEnabled:NO];
    }
    
    return saveGamesMenu;
}

- (NSMenu *)OE_ratingMenuForGames:(NSArray*)games
{
    NSMenu   *ratingMenu = [[NSMenu alloc] init];
    NSString *ratingLabel = @"★★★★★";
    
    for (NSInteger i=0; i<=5; i++) {
        NSMenuItem *ratingItem = [[NSMenuItem alloc] initWithTitle:[ratingLabel substringToIndex:i] action:@selector(setRatingForSelectedGames:) keyEquivalent:@""];
        [ratingItem setRepresentedObject:@(i)];
        if(i == 0)
            [ratingItem setTitle:NSLocalizedString(@"None", "")];
        [ratingMenu addItem:ratingItem];
    }
    
    BOOL valuesDiffer = NO;
    for(NSInteger i=0; i<[games count]; i++)
    {
        NSNumber   *gameRating = [(OEDBGame *)[games objectAtIndex:i] rating];
        NSInteger   itemIndex = [gameRating integerValue];
        NSMenuItem *item = [ratingMenu itemAtIndex:itemIndex];
        
        if(i==0)
            [item setState:NSOnState];
        else if([item state] != NSOnState)
        {
            valuesDiffer = YES;
            [item setState:NSMixedState];
        }
    }
    
    if(valuesDiffer)
    {
        NSNumber   *gameRating = [(OEDBGame *)[games objectAtIndex:0] rating];
        NSMenuItem *item = [ratingMenu itemAtIndex:[gameRating integerValue]];
        [item setState:NSMixedState];
    }
    
    return ratingMenu;
}

- (NSMenu *)OE_collectionsMenuForGames:(NSArray *)games
{
    NSMenu  *collectionMenu = [[NSMenu alloc] init];
    NSArray *collections = [[[self libraryController] database] collections];
    
    [collectionMenu addItemWithTitle:NSLocalizedString(@"New Collection from Selection", @"")
                              action:@selector(makeNewCollectionWithSelectedGames:)
                       keyEquivalent:@""];
    
    for(id collection in collections)
    {
        if([collection isMemberOfClass:[OEDBCollection class]] && collection != [self representedObject])
        {
            NSMenuItem *collectionMenuItem = [[NSMenuItem alloc] initWithTitle:[collection valueForKey:@"name"] action:@selector(addSelectedGamesToCollection:) keyEquivalent:@""];
            
            // TODO: might want to use managedObjectID instead
            [collectionMenuItem setRepresentedObject:collection];
            [collectionMenu addItem:collectionMenuItem];
        }
    }
    
    if([[collectionMenu itemArray] count]!=1)
        [collectionMenu insertItem:[NSMenuItem separatorItem] atIndex:1];
    
    
    return collectionMenu;
}

- (void)setRatingForSelectedGames:(id)sender
{
    NSArray *selectedGames = [self selectedGames];
    for(OEDBGame *game in selectedGames)
    {
        [game setRating:[sender representedObject]];
    }
    
    [self reloadDataIndexes:[self selectedIndexes]];
}

- (IBAction)showSelectedGamesInFinder:(id)sender
{
    NSArray *selectedGames = [self selectedGames];
    NSMutableArray *urls = [NSMutableArray array];
    
    [selectedGames enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        NSSet *roms = [obj roms];
        [roms enumerateObjectsUsingBlock:^(id obj, BOOL *stop) {
            // -[NSWorkspace activateFileViewerSelectingURLs:] does not like relative URLs, i.e., those with non-nil baseURL.
            // We need to make sure only absolute URLs are passed to that method.
            [urls addObject:[[obj URL] absoluteURL]];
        }];
    }];

    [[NSWorkspace sharedWorkspace] activateFileViewerSelectingURLs:urls];
}

- (void)deleteSaveState:(id)stateItem
{
    // TODO: localize and rephrase text
    OEDBSaveState *state = [stateItem representedObject];
    NSString *stateName = [state name];
    OEHUDAlert *alert = [OEHUDAlert deleteStateAlertWithStateName:stateName];
    
    NSUInteger result = [alert runModal];
    if(result)
        [state remove];
}

- (void)renameSelectedGame:(id)sender
{
    NSLog(@"renameSelectedGame: Not implemented yet.");
}

- (void)delete:(id)sender
{
    [self deleteSelectedGames:sender];
}
- (void)deleteBackward:(id)sender
{
    [self deleteSelectedGames:sender];
}

- (void)deleteBackwardByDecomposingPreviousCharacter:(id)sender
{
    [self deleteSelectedGames:sender];
}

- (void)deleteSelectedGames:(id)sender
{
    OECoreDataMainThreadAssertion();

    NSArray *selectedGames = [self selectedGames];
    BOOL multipleGames = ([selectedGames count]>1);

    // deleting from 'All Games', Smart Collections and consoles removes games from the library
    if([[self representedObject] isKindOfClass:[OEDBSmartCollection class]]
       || [self representedObject]==(id<OECollectionViewItemProtocol>)[OEDBAllGamesCollection sharedDBAllGamesCollection]
       || [[self representedObject] isKindOfClass:[OEDBSystem class]])
    {
        // delete games from library if user allows it
        if([[OEHUDAlert removeGamesFromLibraryAlert:multipleGames] runModal])
        {
            NSURL* romsFolderURL             = [[[self libraryController] database] romsFolderURL];
            __block BOOL romsAreInRomsFolder = NO;
            [selectedGames enumerateObjectsUsingBlock:^(OEDBGame *game, NSUInteger idx, BOOL *stopGames) {
                [[game roms] enumerateObjectsUsingBlock:^(OEDBRom *rom, BOOL *stopRoms) {
                    NSURL *romURL = [rom URL];
                    if(romURL != nil && [romURL isSubpathOfURL:romsFolderURL])
                    {
                        romsAreInRomsFolder = YES;
                        
                        *stopGames = YES;
                        *stopRoms = YES;
                    }
                }];
            }];
            
            BOOL deleteFiles = NO;
            if(romsAreInRomsFolder)
            {
                NSUInteger alertReturn = [[OEHUDAlert removeGameFilesFromLibraryAlert:multipleGames] runModal];
                deleteFiles = (alertReturn == NSAlertDefaultReturn);
            }
            
            DLog(@"deleteFiles: %d", deleteFiles);
            [selectedGames enumerateObjectsUsingBlock:^(OEDBGame *game, NSUInteger idx, BOOL *stopGames) {
                [game deleteByMovingFile:deleteFiles keepSaveStates:YES];
            }];

            NSManagedObjectContext *context = [[selectedGames lastObject] managedObjectContext];
            [context save:nil];

            NSRect visibleRect = [gridView visibleRect];
            [self OE_reloadData];
            [gridView scrollRectToVisible:visibleRect];
        }
    }
    // deletign from normal collections removes games from that collection
    else if([[self representedObject] isMemberOfClass:[OEDBCollection class]])
    {
        // remove games from collection if user allows it
        if([[OEHUDAlert removeGamesFromCollectionAlert] runModal])
        {
            OEDBCollection* collection = (OEDBCollection*)[self representedObject];
            [[collection mutableGames] minusSet:[NSSet setWithArray:selectedGames]];
            [collection save];
        }
        [self setNeedsReload];
    }
}

- (void)makeNewCollectionWithSelectedGames:(id)sender
{
    OECoreDataMainThreadAssertion();

    NSArray *selectedGames = [self selectedGames];
    OEDBCollection *collection = [[[self libraryController] sidebarController] addCollection:NO];
    [collection setGames:[NSSet setWithArray:selectedGames]];
    [collection save];
    [self setNeedsReload];
}

- (void)addSelectedGamesToCollection:(id)sender
{
    OECoreDataMainThreadAssertion();

    OEDBCollection *collection;
    if(![sender isKindOfClass:[OEDBCollection class]])
    {
        collection = [sender representedObject];
    }
    
    NSArray *selectedGames = [self selectedGames];
    [[collection mutableGames] addObjectsFromArray:selectedGames];
    [collection save];

    [self setNeedsReload];
}

- (void)downloadCoverArt:(id)sender
{
    [[self selectedGames] makeObjectsPerformSelector:@selector(requestCoverDownload)];
    [(OEDBGame*)[[self selectedGames] lastObject] save];
    [self reloadDataIndexes:[self selectedIndexes]];
}


- (void)cancelCoverArtDownload:(id)sender
{
    [[self selectedGames] makeObjectsPerformSelector:@selector(cancelCoverDownload)];
    [(OEDBGame*)[[self selectedGames] lastObject] save];
    [self reloadDataIndexes:[self selectedIndexes]];
}

- (void)addCoverArtFromFile:(id)sender
{
    NSOpenPanel *openPanel = [NSOpenPanel openPanel];

    [openPanel setAllowsMultipleSelection:NO];
    [openPanel setCanChooseDirectories:NO];
    [openPanel setCanChooseFiles:YES];
    NSArray *imageTypes = [NSImage imageFileTypes];
    [openPanel setAllowedFileTypes:imageTypes];

    [openPanel beginWithCompletionHandler:^(NSInteger result) {
        if(result != NSFileHandlingPanelOKButton)
            return;

        NSArray *selectedGames = [self selectedGames];
        [selectedGames makeObjectsPerformSelector:@selector(setBoxImageByURL:) withObject:[openPanel URL]];
        NSManagedObjectContext *context = [[selectedGames lastObject] managedObjectContext];
        [context save:nil];

        [self reloadDataIndexes:[self selectedIndexes]];
    }];
}

- (void)addSaveStateFromFile:(id)sender
{
    NSLog(@"addCoverArtFromFile: Not implemented yet.");
}

- (void)consolidateFiles:(id)sender
{
    TODO("rewrite this part");
    // TODO: rewrite to escape threading hell
    dispatch_async(dispatch_get_main_queue(), ^{
        NSArray *games = [self selectedGames];
        if([games count] == 0) return;

        OEHUDAlert  *alert = [[OEHUDAlert alloc] init];
        [alert setHeadlineText:@""];
        [alert setMessageText:NSLocalizedString(@"Consolidating will copy all of the selected games into the OpenEmu Library folder.\n\nThis cannot be undone.", @"")];
        [alert setDefaultButtonTitle:NSLocalizedString(@"Consolidate", @"")];
        [alert setAlternateButtonTitle:NSLocalizedString(@"Cancel", @"")];
        if([alert runModal] != NSAlertDefaultReturn) return;

        alert = [[OEHUDAlert alloc] init];
        [alert setShowsProgressbar:YES];
        [alert setProgress:0.0];
        [alert setHeadlineText:NSLocalizedString(@"Copying Game Files…", @"")];
        [alert setTitle:NSLocalizedString(@"", @"")];
        [alert setShowsProgressbar:YES];
        [alert setDefaultButtonTitle:nil];
        [alert setMessageText:nil];

        NSMutableArray *gameIDs = [NSMutableArray arrayWithCapacity:[games count]];
        [games enumerateObjectsUsingBlock:^(OEDBGame *game, NSUInteger idx, BOOL *stop) {
            NSManagedObjectID *object = [game permanentID];
            [gameIDs addObject:object];
        }];

        __block NSInteger alertResult = -1;
        dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1.0 * NSEC_PER_SEC));
        dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
        NSManagedObjectContext *c = nil;
        // register objects in queue context, oehudalert's modal session will block it from fetching the objects later…
        __block NSMutableArray *threadSafeGames = [NSMutableArray arrayWithCapacity:[gameIDs count]];
        dispatch_sync(queue, ^{
            for (NSUInteger i=0; i<[gameIDs count]; i++) {
                NSManagedObjectID *objectID = [gameIDs objectAtIndex:i];
                [threadSafeGames addObject:[OEDBGame objectWithID:objectID inContext:c]];
            }
        });

        dispatch_after(popTime, queue, ^{
            __block NSError *error = nil;
            for (NSUInteger i=0; i<[threadSafeGames count]; i++) {
                if(alertResult != -1) break;

                OEDBGame *aGame = [threadSafeGames objectAtIndex:i];
                NSSet *roms = [aGame roms];
                for(OEDBRom *rom in roms)
                {
                    if(alertResult != -1) break;

                    NSURL *url = [rom URL];
                    if([url checkResourceIsReachableAndReturnError:nil] && ![url isSubpathOfURL:[[rom libraryDatabase] romsFolderURL]])
                    {
                        BOOL romFileLocked = NO;
                        if([[[[NSFileManager defaultManager] attributesOfItemAtPath:[url path] error:nil] objectForKey:NSFileImmutable] boolValue])
                        {
                            romFileLocked = YES;
                            [[NSFileManager defaultManager] setAttributes:@{ NSFileImmutable: @(FALSE) } ofItemAtPath:[url path] error:nil];
                        }

                        NSString *fullName  = [url lastPathComponent];
                        NSString *extension = [fullName pathExtension];
                        NSString *baseName  = [fullName stringByDeletingPathExtension];

                        NSURL *unsortedFolder = [[rom libraryDatabase] romsFolderURLForSystem:[aGame system]];
                        NSURL *romURL         = [unsortedFolder URLByAppendingPathComponent:fullName];
                        romURL = [romURL uniqueURLUsingBlock:^NSURL *(NSInteger triesCount) {
                            NSString *newName = [NSString stringWithFormat:@"%@ %ld.%@", baseName, triesCount, extension];
                            return [unsortedFolder URLByAppendingPathComponent:newName];
                        }];

                        if([[NSFileManager defaultManager] copyItemAtURL:url toURL:romURL error:&error] && (alertResult == -1))
                        {
                            TODO("rewrite this part");
                            [alert performBlockInModalSession:^{
                                /*
                                OEDBRom *rom = [OEDBRom objectWithID:objectID inLibrary:database];
                                NSString *location = [rom location];
                                NSFetchRequest *fetchRequest = [NSFetchRequest fetchRequestWithEntityName:[OEDBRom entityName]];
                                NSPredicate *predicate = [NSPredicate predicateWithFormat:@"location = %@", location];
                                [fetchRequest setPredicate:predicate];
                                NSArray *roms = [[rom libraryDatabase] executeFetchRequest:fetchRequest error:nil];
                                [roms enumerateObjectsUsingBlock:^(OEDBRom *obj, NSUInteger idx, BOOL *stop) {
                                    [obj setURL:romURL];
                                }];
                                [rom setURL:romURL];

                                [[rom managedObjectContext] save:nil];
                                 */
                            }];
                        }
                        else if(error != nil) break;

                        if(romFileLocked)
                            [[NSFileManager defaultManager] setAttributes:@{ NSFileImmutable: @(YES) } ofItemAtPath:[url path] error:nil];
                    }
                }

                [alert performBlockInModalSession:^{
                    [alert setProgress:(float)(i+1)/[games count]];
                }];

                if(error != nil)
                    break;
            }

            if(error != nil)
            {
                OEAlertCompletionHandler originalCompletionHandler = [alert callbackHandler];
                [alert setCallbackHandler:^(OEHUDAlert *alert, NSUInteger result){
                    NSString *messageText = [error localizedDescription];
                    OEHUDAlert *errorAlert = [OEHUDAlert alertWithMessageText:messageText defaultButton:@"OK" alternateButton:@""];
                    [errorAlert setTitle:@"Consolidating files failed."];
                    [errorAlert runModal];
                    
                    if(originalCompletionHandler) originalCompletionHandler(alert, result);
                }];
            }
            
            [alert closeWithResult:NSAlertDefaultReturn];
            
        });
        [alert setDefaultButtonTitle:@"Stop"];
        alertResult = [alert runModal];
    });
}
#pragma mark -
#pragma mark NSTableView DataSource

- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView
{
    if(aTableView == listView)
        return [[gamesController arrangedObjects] count];

    return 0;
}

- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)rowIndex
{
    if(tableView != listView) return nil;

    NSObject<OEListViewDataSourceItem> *item = [[gamesController arrangedObjects] objectAtIndex:rowIndex];
    NSString *columnId                       = [tableColumn identifier];
    id result                                = nil;

    if(columnId == nil)                                               result = item;
    else if([columnId isEqualToString:@"listViewStatus"])             result = ([[listView selectedRowIndexes] containsIndex:rowIndex] ? [item listViewSelectedStatus] : [item listViewStatus]);
    else if([item respondsToSelector:NSSelectorFromString(columnId)]) result = [item valueForKey:columnId];

    return result;
}

- (void)tableView:(NSTableView *)aTableView setObjectValue:(id)anObject forTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex
{
    if( aTableView == listView)
    {
        id <OEListViewDataSourceItem> obj = [[gamesController arrangedObjects] objectAtIndex:rowIndex];
        NSString *columnIdentifier = [aTableColumn identifier];
        if([columnIdentifier isEqualToString:@"listViewRating"])
        {
            [obj setListViewRating:anObject];
        } else if([columnIdentifier isEqualToString:@"listViewTitle"])
        {
            if([anObject isKindOfClass:[NSAttributedString class]])
                anObject = [anObject string];
            
            [obj setListViewTitle:anObject];

            // Search results may no longer be valid, reload
            [self OE_reloadData];
        }
        else return;
        
        if([obj isKindOfClass:[OEDBItem class]])
        {
            TODO("Save HERE");
        }
    }
}

- (void)tableView:(NSTableView *)tableView sortDescriptorsDidChange:(NSArray *)oldDescriptors
{
    if(tableView != listView) return;

    if([[listView sortDescriptors] count] > 0)
    {
        // Make sure we do not accumulate sort descriptors and `listViewTitle` is the secondary
        // sort descriptor provided it's not the main sort descriptor
        NSSortDescriptor *mainSortDescriptor = [[listView sortDescriptors] objectAtIndex:0];

        if(![[mainSortDescriptor key] isEqualToString:@"listViewTitle"])
        {
            [listView setSortDescriptors:(@[
                                          mainSortDescriptor,
                                          [NSSortDescriptor sortDescriptorWithKey:@"listViewTitle" ascending:YES selector:@selector(localizedCaseInsensitiveCompare:)],
                                          ])];
        }
    }

    [gamesController setSortDescriptors:[listView sortDescriptors]];
    [listView reloadData];

    // If we send -reloadData to `coverFlowView`, it changes the selected index to an index that doesn't match
    // either the previous selected index or the new selected index as defined by `gamesController`. We need to
    // remember the actual new selected index, wait for `coverFlowView` to reload its data and then restore the
    // correct selection.
    if([[gamesController selectionIndexes] count] == 1)
    {
        NSInteger selectedRow = [[gamesController selectionIndexes] firstIndex];
        [coverFlowView reloadData];
        [coverFlowView setSelectedIndex:(int)selectedRow];
    }
    else [coverFlowView reloadData];

}

#pragma mark -
#pragma mark TableView Drag and Drop 
- (BOOL)tableView:(NSTableView *)aTableView acceptDrop:(id < NSDraggingInfo >)info row:(NSInteger)row dropOperation:(NSTableViewDropOperation)operation
{
    NSPasteboard *pboard = [info draggingPasteboard];
    if (![[pboard types] containsObject:NSFilenamesPboardType])
        return NO;

    NSArray *files = [pboard propertyListForType:NSFilenamesPboardType];
    OEROMImporter *romImporter = [[[self libraryController] database] importer];
    OEDBCollection *collection = [[self representedObject] isKindOfClass:[OEDBCollection class]] ? [self representedObject] : nil;
    [romImporter importItemsAtPaths:files intoCollectionWithID:[collection permanentID]];
    
    return YES;
}

- (NSDragOperation)tableView:(NSTableView *)aTableView validateDrop:(id < NSDraggingInfo >)info proposedRow:(NSInteger)row proposedDropOperation:(NSTableViewDropOperation)operation
{
    if (![[[info draggingPasteboard] types] containsObject:NSFilenamesPboardType])
        return NSDragOperationNone;

    return NSDragOperationCopy;
}


- (BOOL)tableView:(NSTableView *)aTableView writeRowsWithIndexes:(NSIndexSet *)rowIndexes toPasteboard:(NSPasteboard *)pboard
{
    if( aTableView == listView )
    {
        [rowIndexes enumerateIndexesUsingBlock:^(NSUInteger idx, BOOL *stop) 
         {
             id <OEListViewDataSourceItem> obj = [[gamesController arrangedObjects] objectAtIndex:idx];
             [pboard writeObjects:[NSArray arrayWithObject:obj]];
         }];
        
        return YES;
    }

    return NO;
}

#pragma mark -
#pragma mark NSTableView Delegate
- (void)tableView:(NSTableView *)aTableView willDisplayCell:(id)aCell forTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex
{
    if(aTableView == listView)
    {
        if(![aCell isKindOfClass:[OERatingCell class]]) [aCell setHighlighted:NO];
    }
}

- (BOOL)tableView:(NSTableView *)tableView shouldEditTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)rowIndex
{
    return (tableView == listView && [[tableColumn identifier] isEqualToString:@"listViewTitle"]);
}

- (BOOL)selectionShouldChangeInTableView:(NSTableView *)aTableView
{
    if( aTableView == listView )
    {
        return YES;
    }
    return YES;
}

- (void)tableViewSelectionIsChanging:(NSNotification *)notification
{
    NSTableView *tableView = [notification object];

    // We use _listViewSelectionChangeDate to make sure the rating cell tracks the mouse only
    // if a row selection changed some time ago. Since -tableView:shouldTrackCell:forTableColumn:row:
    // is sent *before* -tableViewSelectionDidChange:, we need to make sure that the rating cell
    // does not track the mouse until the selection has changed and we have been able to assign
    // the proper date to _listViewSelectionChangeDate.
    if(tableView == listView) _listViewSelectionChangeDate = [NSDate distantFuture];
}

- (void)tableViewSelectionDidChange:(NSNotification *)aNotification
{
    if([aNotification object] != listView) return;
    
    _listViewSelectionChangeDate = [NSDate date];

    if([[listView selectedRowIndexes] count] == 1) [coverFlowView setSelectedIndex:(int)[[listView selectedRowIndexes] firstIndex]];
    
    if([[NSUserDefaults standardUserDefaults] boolForKey:OEDebugCollectionView] && [[[self gamesController] selectedObjects] count])
    {
        [[OECollectionDebugWindowController sharedController] setRepresentedObject:[[[self gamesController] selectedObjects] objectAtIndex:0]];
        [[[OECollectionDebugWindowController sharedController] window] makeKeyAndOrderFront:self];
    }
}

- (BOOL)tableView:(NSTableView *)tableView shouldTrackCell:(NSCell *)cell forTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    if(tableView == listView && [[tableColumn identifier] isEqualToString:@"listViewRating"])
    {
        // We only track the rating cell in selected rows...
        if(![[listView selectedRowIndexes] containsIndex:row]) return NO;

        // ...if we know when the last selection change happened...
        if(!_listViewSelectionChangeDate) return NO;

        // ...and the selection happened a while ago, where 'a while' is the standard double click interval.
        // This means that the user has to click a row to select it, wait the standard double click internval
        // and then click the rating cell to change it. See issue #294.
        return [_listViewSelectionChangeDate timeIntervalSinceNow] < -[NSEvent doubleClickInterval];

    }
    return NO;
}

#pragma mark - NSTableView Type Select
- (NSString*)tableView:(NSTableView *)tableView typeSelectStringForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    if([[tableColumn identifier] isEqualToString:@"listViewTitle"])
    {
        return [self tableView:tableView objectValueForTableColumn:tableColumn row:row];
    }
    return @"";
}

#pragma mark -
#pragma mark NSTableView Interaction
- (void)tableViewWasDoubleClicked:(id)sender{
    NSAssert(sender == listView, @"Sorry, but we're accepting listView senders only at this time");

    NSInteger row = [listView clickedRow];
    if(row == -1) return;
    
    id game = [self tableView:sender objectValueForTableColumn:nil row:row];
    if(!game) return;

    [[self libraryController] startGame:game];
}
#pragma mark -
#pragma mark OETableView Menu
- (NSMenu *)tableView:(OETableView*)tableView menuForItemsAtIndexes:(NSIndexSet*)indexes
{
    return [self OE_menuForItemsAtIndexes:indexes];
}

#pragma mark -
#pragma mark ImageFlow Data Source
- (NSUInteger)numberOfItemsInImageFlow:(IKImageFlowView *)aBrowser
{
    return [[gamesController arrangedObjects] count];
}

- (id)imageFlow:(id)aFlowLayer itemAtIndex:(int)index
{
    return [[gamesController arrangedObjects] objectAtIndex:index];
}

#pragma mark -
#pragma mark ImageFlow Delegates
- (NSUInteger)imageFlow:(IKImageFlowView *)browser writeItemsAtIndexes:(NSIndexSet *)indexes toPasteboard:(NSPasteboard *)pasteboard{ return 0; }
- (void)imageFlow:(IKImageFlowView *)sender removeItemsAtIndexes:(NSIndexSet *)indexes
{}
- (void)imageFlow:(IKImageFlowView *)sender cellWasDoubleClickedAtIndex:(NSInteger)index
{
    [[self libraryController] startGame:self];
}

- (void)imageFlow:(IKImageFlowView *)sender didSelectItemAtIndex:(NSInteger)index
{    
    [listView selectRowIndexes:[NSIndexSet indexSetWithIndex:[sender selectedIndex]] byExtendingSelection:NO];
    [listView scrollRowToVisible:index];
}

#pragma mark -
#pragma mark Blank Slate Delegate
- (NSDragOperation)blankSlateView:(OEBlankSlateView *)blankSlateView validateDrop:(id<NSDraggingInfo>)draggingInfo
{
    if (![[[draggingInfo draggingPasteboard] types] containsObject:NSFilenamesPboardType])
        return NSDragOperationNone;
    
    return NSDragOperationCopy;
}

- (BOOL)blankSlateView:(OEBlankSlateView*)blankSlateView acceptDrop:(id<NSDraggingInfo>)draggingInfo
{
    NSPasteboard *pboard = [draggingInfo draggingPasteboard];
    if (![[pboard types] containsObject:NSFilenamesPboardType])
        return NO;
    
    NSArray *files = [pboard propertyListForType:NSFilenamesPboardType];
    OEROMImporter *romImporter = [[[self libraryController] database] importer];
    OEDBCollection *collection = [[self representedObject] isKindOfClass:[OEDBCollection class]] ? [self representedObject] : nil;
    [romImporter importItemsAtPaths:files intoCollectionWithID:[collection permanentID]];
    
    return YES;
}

#pragma mark -
#pragma mark Private
#define reloadDelay 0.5
- (void)OE_managedObjectContextDidUpdate:(NSNotification *)notification
{
    [self performSelector:@selector(noteNumbersChanged) onThread:[NSThread mainThread] withObject:nil waitUntilDone:NO];
}

- (void)noteNumbersChanged
{
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(updateViews) object:nil];
    [self performSelector:@selector(updateViews) withObject:nil afterDelay:reloadDelay];
}

- (void)OE_fetchGames
{
    OECoreDataMainThreadAssertion();

    NSError *error = nil;
    BOOL ok = [gamesController fetchWithRequest:nil merge:NO error:&error];

    if(!ok)
        NSLog(@"Error while fetching");
    else
        [self OE_updateBlankSlate];
}

- (void)updateViews
{
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(updateViews) object:nil];
    [self OE_fetchGames];
    [listView noteNumberOfRowsChanged];
    [self setNeedsReloadVisible];
}

- (void)setNeedsReload
{
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(OE_reloadData) object:nil];
    [self performSelector:@selector(OE_reloadData) withObject:nil afterDelay:reloadDelay];
}

- (void)setNeedsReloadVisible
{
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(_reloadVisibleData) object:nil];
    [self performSelector:@selector(_reloadVisibleData) withObject:nil afterDelay:reloadDelay];
}

- (void)setNeedsReloadIndexes:(NSIndexSet *)indexes
{
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(_reloadVisibleData) object:indexes];
    [self performSelector:@selector(reloadDataIndexes:) withObject:indexes afterDelay:reloadDelay];
}

- (void)reloadDataIndexes:(NSIndexSet *)indexSet
{
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(reloadDataIndexes:) object:nil];
    if(!gamesController) return;
    [gamesController rearrangeObjects];
    [gridView performSelectorOnMainThread:@selector(reloadData) withObject:Nil waitUntilDone:NO];
    //[gridView reloadCellsAtIndexes:indexSet];
    [listView reloadDataForRowIndexes:indexSet
                        columnIndexes:[listView columnIndexesInRect:[listView visibleRect]]];
    [indexSet enumerateIndexesUsingBlock:^(NSUInteger idx, BOOL *stop) {
        [coverFlowView reloadCellDataAtIndex:(int)idx];
    }];
}

- (void)_reloadVisibleData
{
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(_reloadVisibleData) object:nil];
    if(!gamesController) return;
    [gamesController rearrangeObjects];
    [gridView performSelectorOnMainThread:@selector(reloadData) withObject:Nil waitUntilDone:NO];
    //[gridView reloadCellsAtIndexes:[gridView indexesForVisibleCells]];
    [listView reloadDataForRowIndexes:[NSIndexSet indexSetWithIndexesInRange:[listView rowsInRect:[listView visibleRect]]]
                        columnIndexes:[listView columnIndexesInRect:[listView visibleRect]]];
    [coverFlowView performSelectorOnMainThread:@selector(reloadData) withObject:nil waitUntilDone:NO];
}

- (void)OE_reloadData
{
    OECoreDataMainThreadAssertion();

    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(OE_reloadData) object:nil];
    if(!gamesController) return;
    
    NSPredicate *pred = [self representedObject]?[[self representedObject] fetchPredicate]:[NSPredicate predicateWithValue:NO];
    [gamesController setFetchPredicate:pred];
    [gamesController setLimit:[[self representedObject] fetchLimit]];
    [gamesController setFetchSortDescriptors:[[self representedObject] fetchSortDescriptors]];
    __block BOOL ok;

    DLog(@"%@", [[[gamesController managedObjectContext] userInfo] valueForKey:@"name"]);
    ok = [gamesController fetchWithRequest:nil merge:NO error:nil];

    if(!ok)
    {
        NSLog(@"Error while fetching");
        return;
    }

    [gridView reloadData];
    [listView reloadData];
    [coverFlowView performSelectorOnMainThread:@selector(reloadData) withObject:nil waitUntilDone:NO];

    [self OE_updateBlankSlate];
}

@end
