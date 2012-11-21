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
#import "OECoverGridForegroundLayer.h"
#import "OECoverGridViewCell.h"

#import "OETableHeaderCell.h"
#import "OEListViewDataSourceItem.h"
#import "OERatingCell.h"
#import "OEHorizontalSplitView.h"

#import "OECoverGridDataSourceItem.h"
#import "OEGridBlankSlateView.h"

#import "OEDBSystem.h"
#import "OESystemPlugin.h"

#import "OEDBSaveState.h"

#import "OECenteredTextFieldCell.h"
#import "OELibraryDatabase.h"

#import "OEMenu.h"
#import "OEDBGame.h"
#import "OEDBRom.h"
#import "OEDBCollection.h"

#import "NSViewController+OEAdditions.h"
#import "OEHUDAlert+DefaultAlertsAdditions.h"
#import "NSURL+OELibraryAdditions.h"

#import "OESidebarController.h"
#import "OETableView.h"

NSString * const OELastGridSizeKey = @"lastGridSize";
NSString * const OELastCollectionViewKey = @"lastCollectionView";

#define     MainMenu_View_GridViewTag 301
#define     MainMenu_View_FlowViewTag 302
#define     MainMenu_View_ListViewTag 303

const int OE_GridViewTag = 0;
const int OE_FlowViewTag = 1;
const int OE_ListViewTag = 2;

static const float OE_coverFlowHeightPercentage = .75;


@interface OECollectionViewController ()
{    
    IBOutlet NSView *gridViewContainer;// gridview
    IBOutlet OEGridView *gridView;// scrollview for gridview
    
    IBOutlet OEHorizontalSplitView *flowlistViewContainer; // cover flow and simple list container
    IBOutlet IKImageFlowView *coverFlowView;
    IBOutlet NSTableView *listView;

    NSDate *_listViewSelectionChangeDate;
}

- (void)OE_managedObjectContextDidSave:(NSNotification *)notification;
- (void)OE_reloadData;

- (NSMenu *)OE_menuForItemsAtIndexes:(NSIndexSet *)indexes;
- (NSMenu *)OE_saveStateMenuForGame:(OEDBGame *)game;
- (NSMenu *)OE_ratingMenuForGames:(NSArray *)games;
- (NSMenu *)OE_collectionsMenuForGames:(NSArray *)games;

@end

@implementation OECollectionViewController
{
    BOOL _stateRewriteRequired;
    BOOL blankSlateVisible;
    int _selectedViewTag;
}
@synthesize libraryController, gamesController;
@synthesize emptyCollectionView, emptyConsoleView;

+ (void)initialize
{
    // Make sure not to reinitialize for subclassed objects
    if(self != [OECollectionViewController class]) return;
    
    // Indicators for list view
    NSImage *image = [NSImage imageNamed:@"list_indicators"];
    
    // unselected states
    [image setName:@"list_indicators_playing" forSubimageInRect:NSMakeRect(0, 24, 12, 12)];
    [image setName:@"list_indicators_missing" forSubimageInRect:NSMakeRect(0, 12, 12, 12)];
    [image setName:@"list_indicators_unplayed" forSubimageInRect:NSMakeRect(0, 0, 12, 12)];
    
    // selected states
    [image setName:@"list_indicators_playing_selected" forSubimageInRect:NSMakeRect(12, 24, 12, 12)];
    [image setName:@"list_indicators_missing_selected" forSubimageInRect:NSMakeRect(12, 12, 12, 12)];
    [image setName:@"list_indicators_unplayed_selected" forSubimageInRect:NSMakeRect(12, 0, 12, 12)];
    
    [[NSUserDefaults standardUserDefaults] registerDefaults:@{ OELastGridSizeKey : @1.0f }];
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    gamesController = nil;
}

#pragma mark -
#pragma mark View Controller Stuff
- (void)loadView
{
    [super loadView];
        
    // Set up games controller
    gamesController = [[NSArrayController alloc] init];
    [gamesController setAutomaticallyRearrangesObjects:YES];
    [gamesController setAutomaticallyPreparesContent:YES];
    [gamesController setUsesLazyFetching:NO];
    
    NSManagedObjectContext *context = [[OELibraryDatabase defaultDatabase] managedObjectContext];
    //[gamesController bind:@"managedObjectContext" toObject:context withKeyPath:@"" options:nil];
    
    [gamesController setManagedObjectContext:context];
    [gamesController setEntityName:@"Game"];
    [gamesController setSortDescriptors:@[[NSSortDescriptor sortDescriptorWithKey:@"name" ascending:YES]]];
    [gamesController setFetchPredicate:[NSPredicate predicateWithValue:NO]];
    [gamesController setAvoidsEmptySelection:NO];
    [gamesController prepareContent];
    
    // Setup View
    [[self view] setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];

    // Set up GridView
    [gridView setItemSize:NSMakeSize(168, 193)];
    [gridView setMinimumColumnSpacing:22.0];
    [gridView setRowSpacing:29.0];
    [gridView setDelegate:self];
    [gridView registerForDraggedTypes:[NSArray arrayWithObjects:NSFilenamesPboardType, NSPasteboardTypePNG, NSPasteboardTypeTIFF, nil]];
    [gridView setDataSource:self];
    
    OECoverGridForegroundLayer *foregroundLayer = [[OECoverGridForegroundLayer alloc] init];
    [gridView setForegroundLayer:foregroundLayer];
    
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

    // There's no natural order for status indicators, so we don't allow that column to be sorted
    OETableHeaderCell *romStatusHeaderCell = [[listView tableColumnWithIdentifier:@"romStatus"] headerCell];
    [romStatusHeaderCell setClickable:NO];

    [listView registerForDraggedTypes:[NSArray arrayWithObjects:NSFilenamesPboardType, nil]];
    
    for(NSTableColumn *aColumn in [listView tableColumns])
        if([[aColumn dataCell] isKindOfClass:[OECenteredTextFieldCell class]])
            [[aColumn dataCell] setWidthInset:9];
        
    // Watch the main thread's managed object context for changes
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(OE_managedObjectContextDidSave:) name:NSManagedObjectContextDidSaveNotification object:nil];

    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(OE_emulationDidFinish:) name:OEGameViewControllerEmulationDidFinishNotification object:nil];

    // If the view has been loaded after a collection has been set via -setRepresentedObject:, set the appropriate
    // fetch predicate to display the items in that collection via -OE_reloadData. Otherwise, the view shows an
    // empty collection until -setRepresentedObject: is received again
    if([self representedObject]) [self OE_reloadData];
}

- (NSString *)nibName
{
    return @"CollectionView";
}
#pragma mark - OELibrarySubviewControllerProtocol Implementation
- (void)setRepresentedObject:(id<OECollectionViewItemProtocol>)representedObject
{
    NSAssert([representedObject conformsToProtocol:@protocol(OECollectionViewItemProtocol)], @"OECollectionViewController accepts OECollectionViewItemProtocol represented objects only");

    if(representedObject == [self representedObject]) return;
    [super setRepresentedObject:representedObject];

    [[listView tableColumnWithIdentifier:@"consoleName"] setHidden:![representedObject shouldShowSystemColumnInListView]];
    
    _stateRewriteRequired = YES;
    [self OE_reloadData];
}

- (id <OECollectionViewItemProtocol>)representedObject
{
    return [super representedObject];
}

- (id)encodeCurrentState
{
    if(!_stateRewriteRequired || ![self libraryController] || _selectedViewTag==-1)
        return nil;
    
    NSMutableData    *data  = [NSMutableData data];
    NSKeyedArchiver  *coder = [[NSKeyedArchiver alloc] initForWritingWithMutableData:data];
    NSSlider *sizeSlider    = [[self libraryController] toolbarSlider];
    NSString *searchString  = [[[self libraryController] toolbarSearchField] stringValue];
    
    [coder encodeInt:_selectedViewTag forKey:@"selectedView"];
    [coder encodeFloat:[sizeSlider floatValue] forKey:@"sliderValue"];
    [coder encodeObject:searchString forKey:@"searchString"];
    [coder encodeObject:[self selectedIndexes] forKey:@"selectionIndexes"];
    if([listView sortDescriptors]) [coder encodeObject:[listView sortDescriptors] forKey:@"listViewSortDescriptors"];
    
    [coder finishEncoding];
    
    return data;
}

- (void)restoreState:(id)state
{
    if([self libraryController] == nil) return;
    
    int selectedViewTag;
    float sliderValue;
    NSString   *searchString;
    NSIndexSet *selectionIndexes;
    NSArray    *listViewSortDescriptors = nil;
         
    NSKeyedUnarchiver *coder = state ? [[NSKeyedUnarchiver alloc] initForReadingWithData:state] : nil;
    if(coder)
    {
        selectedViewTag         = [coder decodeIntForKey:@"selectedView"];
        sliderValue             = [coder decodeFloatForKey:@"sliderValue"];
        searchString            = [coder decodeObjectForKey:@"searchString"];
        selectionIndexes        = [coder decodeObjectForKey:@"selectionIndexes"];
        listViewSortDescriptors = [coder decodeObjectForKey:@"listViewSortDescriptors"];
        
        [coder finishDecoding];
        // TODO: Validate decoded values
    }
    else
    {
        NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
        
        selectedViewTag  = [userDefaults integerForKey:OELastCollectionViewKey];
        sliderValue      = [userDefaults floatForKey:OELastGridSizeKey];
        searchString     = @"";
        selectionIndexes = [NSIndexSet indexSet];
    }
        
    NSSlider    *sizeSlider     = [[self libraryController] toolbarSlider];
    NSTextField *searchField    = [[self libraryController] toolbarSearchField];
    
    [self OE_setupToolbarStatesForViewTag:selectedViewTag];
    [sizeSlider setFloatValue:sliderValue];
    [searchField setStringValue:searchString];
    [listView setSortDescriptors:listViewSortDescriptors];

    if(selectedViewTag == OE_FlowViewTag || selectedViewTag == OE_ListViewTag)
    {
        [[self gamesController] setSortDescriptors:listViewSortDescriptors];
        [[self gamesController] rearrangeObjects];
        [listView reloadData];
    }

    [self OE_updateBlankSlate];
    
    _stateRewriteRequired = NO;
    // TODO: restore selection using selectionIndexes
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
    [[self gamesController] setSortDescriptors:@[[NSSortDescriptor sortDescriptorWithKey:@"name" ascending:YES]]];
    [[self gamesController] rearrangeObjects];
    [gridView reloadData];

    [self OE_switchToView:OE_GridViewTag];
}

- (IBAction)switchToFlowView:(id)sender
{
    [[self gamesController] setSortDescriptors:[listView sortDescriptors]];
    [[self gamesController] rearrangeObjects];
    [listView reloadData];

    [self OE_switchToView:OE_FlowViewTag];
}

- (IBAction)switchToListView:(id)sender
{
    [[self gamesController] setSortDescriptors:[listView sortDescriptors]];
    [[self gamesController] rearrangeObjects];
    [listView reloadData];

    [self OE_switchToView:OE_ListViewTag];
}

- (void)OE_switchToView:(int)tag
{
    [self OE_setupToolbarStatesForViewTag:tag];
    if(_selectedViewTag == tag) return;
    
    [self OE_showView:tag];
    _stateRewriteRequired = YES;
    _selectedViewTag = tag;
}

- (void)OE_showView:(int)tag
{
    NSView *view;
    float splitterPosition = -1;
    switch (tag) {
        case -1:
            _selectedViewTag = -1;
        case OE_GridViewTag:
            view = gridViewContainer;
            break;
        case OE_FlowViewTag:
            view = flowlistViewContainer;
            splitterPosition = NSHeight([view frame]) * OE_coverFlowHeightPercentage;
            break;
        case OE_ListViewTag:
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

- (void)OE_setupToolbarStatesForViewTag:(int)tag
{
    switch (tag)
    {
        case 0:
            [[[self libraryController] toolbarGridViewButton] setState:NSOnState];
            [[[self libraryController] toolbarFlowViewButton] setState:NSOffState];
            [[[self libraryController] toolbarListViewButton] setState:NSOffState];
            break;
        case 1:
            [[[self libraryController] toolbarGridViewButton] setState:NSOffState];
            [[[self libraryController] toolbarFlowViewButton] setState:NSOnState];
            [[[self libraryController] toolbarListViewButton] setState:NSOffState];
            break;
        case 2:
            [[[self libraryController] toolbarGridViewButton] setState:NSOffState];
            [[[self libraryController] toolbarFlowViewButton] setState:NSOffState];
            [[[self libraryController] toolbarListViewButton] setState:NSOnState];
            break;
    }
    
    [[[self libraryController] toolbarSlider] setEnabled:(!blankSlateVisible && tag==0)];
}

- (void)OE_updateBlankSlate
{    
    NSUInteger count = [[gamesController arrangedObjects] count];
    if(count)
    {
        blankSlateVisible = NO;
        if([[[self libraryController] toolbarFlowViewButton] state] == NSOnState)
            [self switchToFlowView:self];
        else if([[[self libraryController] toolbarListViewButton] state] == NSOnState)
            [self switchToListView:self];
        else
            [self switchToGridView:self];
        
        [[[self libraryController] toolbarGridViewButton] setEnabled:YES];
        [[[self libraryController] toolbarFlowViewButton] setEnabled:YES];
        [[[self libraryController] toolbarListViewButton] setEnabled:YES];
        
        [[[self libraryController] toolbarSearchField] setEnabled:YES];
    }
    else
    {
        blankSlateVisible = YES;
        [self OE_showView:-1];
        
        [[[self libraryController] toolbarGridViewButton] setEnabled:NO];
        [[[self libraryController] toolbarFlowViewButton] setEnabled:NO];
        [[[self libraryController] toolbarListViewButton] setEnabled:NO];
        
        [[[self libraryController] toolbarSearchField] setEnabled:NO];
        [[[self libraryController] toolbarSlider] setEnabled:NO];
    }
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
    NSPredicate *pred = [[sender stringValue] isEqualToString:@""]?nil:[NSPredicate predicateWithFormat:@"name contains[cd] %@", [sender stringValue]];
    [gamesController setFilterPredicate:pred];
    
    [listView reloadData];
    [coverFlowView performSelectorOnMainThread:@selector(reloadData) withObject:nil waitUntilDone:NO];
    [gridView reloadData];
    
    _stateRewriteRequired = YES;
}

- (IBAction)changeGridSize:(id)sender
{
    float zoomValue = [sender floatValue];
    [gridView setItemSize:NSMakeSize(roundf(26+142*zoomValue), roundf(44+7+142*zoomValue))];
    
    [[NSUserDefaults standardUserDefaults] setValue:[NSNumber numberWithFloat:zoomValue] forKey:OELastGridSizeKey];
    _stateRewriteRequired = YES;
}

#pragma mark -
#pragma mark GridView Delegate
- (void)selectionChangedInGridView:(OEGridView *)view
{
    [gamesController setSelectionIndexes:[view selectionIndexes]];
    _stateRewriteRequired = YES;
}

- (NSDragOperation)gridView:(OEGridView *)gridView validateDrop:(id<NSDraggingInfo>)draggingInfo
{
    if (![[[draggingInfo draggingPasteboard] types] containsObject:NSFilenamesPboardType])
        return NSDragOperationNone;
    
    return NSDragOperationCopy;
}

- (BOOL)gridView:(OEGridView*)gridView acceptDrop:(id<NSDraggingInfo>)draggingInfo
{
    NSPasteboard *pboard = [draggingInfo draggingPasteboard];
    if (![[pboard types] containsObject:NSFilenamesPboardType])
        return NO;
    
    NSArray *files = [pboard propertyListForType:NSFilenamesPboardType];
    OEROMImporter *romImporter = [[[self libraryController] database] importer];
    [romImporter importItemsAtPaths:files];
    
    return YES;
}

#pragma mark -
#pragma mark Grid View DataSource
- (NSUInteger)numberOfItemsInGridView:(OEGridView *)view
{
    return [[gamesController arrangedObjects] count];
}

- (OEGridViewCell *)gridView:(OEGridView *)view cellForItemAtIndex:(NSUInteger)index
{
    if (index >= [[gamesController arrangedObjects] count]) return nil;
    
    OECoverGridViewCell *item = (OECoverGridViewCell *)[view cellForItemAtIndex:index makeIfNecessary:NO];
    
    if(item == nil) item = (OECoverGridViewCell *)[view dequeueReusableCell];
    if(item == nil) item = [[OECoverGridViewCell alloc] init];
    
    id <OECoverGridDataSourceItem> object = (id <OECoverGridDataSourceItem>)[[gamesController arrangedObjects] objectAtIndex:index];
    [item setTitle:[object gridTitle]];
    [item setRating:[object gridRating]];
    
    if([object hasImage])
    {
        [item setImageSize:[object actualGridImageSizeforSize:[view itemSize]]];
        [item setImage:[object gridImageWithSize:[gridView itemSize]]];
    }
    else
    {
        [item setImageSize:[gridView itemSize]];
        [item setImage:nil];
    }
    
    return item;
}

- (NSView *)viewForNoItemsInGridView:(OEGridView *)view
{
    if([[self representedObject] isKindOfClass:[OEDBSystem class]])
        return [[OEGridBlankSlateView alloc] initWithSystemPlugin:[(OEDBSystem*)[self representedObject] plugin]];
    
    if([[self representedObject] respondsToSelector:@selector(collectionViewName)])
        return [[OEGridBlankSlateView alloc] initWithCollectionName:[[self representedObject] collectionViewName]];
    
    return nil;
}

- (id<NSPasteboardWriting>)gridView:(OEGridView *)gridView pasteboardWriterForIndex:(NSInteger)index
{
    return [[gamesController arrangedObjects] objectAtIndex:index];
}

- (NSMenu*)gridView:(OEGridView *)gridView menuForItemsAtIndexes:(NSIndexSet*)indexes
{
    return [self OE_menuForItemsAtIndexes:indexes];
}
#pragma mark -
#pragma mark GridView Interaction
- (void)gridView:(OEGridView *)view doubleClickedCellForItemAtIndex:(NSUInteger)index
{
    [[self libraryController] startGame:self];
}

- (void)gridView:(OEGridView *)view didEndEditingCellForItemAtIndex:(NSUInteger)index
{
    OECoverGridViewCell *item = (OECoverGridViewCell *)[view cellForItemAtIndex:index makeIfNecessary:NO];
    if(!item)
        return;
    
    id <OECoverGridDataSourceItem> object = (id <OECoverGridDataSourceItem>)[[gamesController arrangedObjects] objectAtIndex:index];
    if(!object)
        return;
    
    [object setGridRating:[item rating]];
    [object setGridTitle:[item title]];
    [object setGridImage:[item image]];
}

#pragma mark -
#pragma mark Context Menu
- (NSMenu*)OE_menuForItemsAtIndexes:(NSIndexSet*)indexes
{
    NSMenu *menu = [[NSMenu alloc] init];
    NSMenuItem *menuItem;
    NSArray *games = [[gamesController arrangedObjects] objectsAtIndexes:indexes];
    
    if([indexes count] == 1)
    {
        NSInteger index = [indexes lastIndex];
        [menu addItemWithTitle:@"Play Game" action:@selector(startGame:) keyEquivalent:@""];
        OEDBGame  *game = [[gamesController arrangedObjects] objectAtIndex:index];
        
        // Create Save Game Menu
        menuItem = [[NSMenuItem alloc] initWithTitle:@"Play Save Games" action:NULL keyEquivalent:@""];
        [menuItem setSubmenu:[self OE_saveStateMenuForGame:game]];
        [menu addItem:menuItem];
        
        [menu addItem:[NSMenuItem separatorItem]];
        
        // Create Rating Item
        menuItem = [[NSMenuItem alloc] initWithTitle:@"Rating" action:NULL keyEquivalent:@""];
        [menuItem setSubmenu:[self OE_ratingMenuForGames:games]];
        [menu addItem:menuItem];    
        [menu addItemWithTitle:@"Show In Finder" action:@selector(showSelectedGamesInFinder:) keyEquivalent:@""];
        [menu addItem:[NSMenuItem separatorItem]];

        // Temporarily disable Get Game Info from Archive.vg per issue #322. This should be eventually enabled in a later version.
        // See the corresponding menu item a few lines below.
//        [menu addItemWithTitle:@"Get Game Info From Archive.vg" action:@selector(getGameInfoFromArchive:) keyEquivalent:@""];

        [menu addItemWithTitle:@"Get Cover Art From Archive.vg" action:@selector(getCoverFromArchive:) keyEquivalent:@""];
        [menu addItem:[NSMenuItem separatorItem]];
        [menu addItemWithTitle:@"Add Cover Art From File..." action:@selector(addCoverArtFromFile:) keyEquivalent:@""];
//        [menu addItemWithTitle:@"Add Save File To Game..." action:@selector(addSaveStateFromFile:) keyEquivalent:@""];
        [menu addItem:[NSMenuItem separatorItem]];
        // Create Add to collection menu
        NSMenuItem *collectionMenuItem = [[NSMenuItem alloc] initWithTitle:@"Add To Collection" action:NULL keyEquivalent:@""];
        [collectionMenuItem setSubmenu:[self OE_collectionsMenuForGames:games]];
        [menu addItem:collectionMenuItem];
        [menu addItem:[NSMenuItem separatorItem]];
        [menu addItemWithTitle:@"Rename Game" action:@selector(renameSelectedGame:) keyEquivalent:@""];
        [menu addItemWithTitle:@"Delete Game" action:@selector(deleteSelectedGames:) keyEquivalent:@""];        
    }
    else
    {
        if([[NSUserDefaults standardUserDefaults] boolForKey:OEForcePopoutGameWindowKey])
        {
            [menu addItemWithTitle:@"Play Games (Caution)" action:@selector(startGame:) keyEquivalent:@""];
        }
        
        // Create Rating Item
        menuItem = [[NSMenuItem alloc] initWithTitle:@"Rating" action:NULL keyEquivalent:@""];
        [menuItem setSubmenu:[self OE_ratingMenuForGames:games]];
        [menu addItem:menuItem];    
        [menu addItemWithTitle:@"Show In Finder" action:@selector(showSelectedGamesInFinder:) keyEquivalent:@""];
        
        [menu addItem:[NSMenuItem separatorItem]];

        // Temporarily disable Get Game Info from Archive.vg per issue #322. This should be eventually enabled in a later version.
        // See the corresponding menu item a few lines above.
//        [menu addItemWithTitle:@"Get Game Info From Archive.vg" action:@selector(getGameInfoFromArchive:) keyEquivalent:@""];

        [menu addItemWithTitle:@"Get Cover Art From Archive.vg" action:@selector(getCoverFromArchive:) keyEquivalent:@""];
        
        [menu addItem:[NSMenuItem separatorItem]];
        // Create Add to collection menu
        NSMenuItem *collectionMenuItem = [[NSMenuItem alloc] initWithTitle:@"Add To Collection" action:NULL keyEquivalent:@""];
        [collectionMenuItem setSubmenu:[self OE_collectionsMenuForGames:games]];
        [menu addItem:collectionMenuItem];
        
        [menu addItem:[NSMenuItem separatorItem]];
        [menu addItemWithTitle:@"Delete Games" action:@selector(deleteSelectedGames:) keyEquivalent:@""];        
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
        [saveGamesMenu addItemWithTitle:@"No Save States available" action:NULL keyEquivalent:@""];
        [(NSMenuItem*)[[saveGamesMenu itemArray] lastObject] setEnabled:NO];
    }
    
    return saveGamesMenu;
}

- (NSMenu*)OE_ratingMenuForGames:(NSArray*)games
{
    NSMenu   *ratingMenu = [[NSMenu alloc] init];
    NSString *ratingLabel = @"★★★★★";
    
    for (NSInteger i=0; i<=5; i++) {
        NSMenuItem *ratingItem = [[NSMenuItem alloc] initWithTitle:[ratingLabel substringToIndex:i] action:@selector(setRatingForSelectedGames:) keyEquivalent:@""];
        [ratingItem setRepresentedObject:[NSNumber numberWithInt:i]];
        if(i==0)
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
    
    [collectionMenu addItemWithTitle:@"New Collection from Selection" action:@selector(makeNewCollectionWithSelectedGames:) keyEquivalent:@""];
    
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

- (void)showSelectedGamesInFinder:(id)sender
{
    NSArray *selectedGames = [self selectedGames];
    NSMutableArray *urls = [NSMutableArray array];
    
    [selectedGames enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        NSSet *roms = [obj roms];
        [roms enumerateObjectsUsingBlock:^(id obj, BOOL *stop) {
            [urls addObject:[obj URL]];
        }];    
    }];
    
    [[NSWorkspace sharedWorkspace] activateFileViewerSelectingURLs:urls];
}

- (void)deleteSaveState:(id)stateItem
{
    // TODO: localize and rephrase text
    id state = [stateItem representedObject];
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
    NSArray *selectedGames = [self selectedGames];
    
    if([[self representedObject] isKindOfClass:[OEDBCollection class]])
    {
        if([[OEHUDAlert removeGamesFromCollectionAlert] runModal])
        {
            OEDBCollection* collection = (OEDBCollection*)[self representedObject];
            [[collection mutableGames] minusSet:[NSSet setWithArray:selectedGames]];
            [[collection managedObjectContext] save:nil];
        }
        [self setNeedsReload];
    }
    else if([[OEHUDAlert removeGamesFromLibraryAlert:[selectedGames count]>1] runModal])
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
        
        NSUInteger alertReturn = NSAlertAlternateReturn;
        if(!romsAreInRomsFolder || (alertReturn=[[OEHUDAlert removeGameFilesFromLibraryAlert:[selectedGames count]>1] runModal]))
        {
            NSManagedObjectContext *moc = [[selectedGames lastObject] managedObjectContext];
            [selectedGames enumerateObjectsUsingBlock:^(OEDBGame *game, NSUInteger idx, BOOL *stopGames) {
                [game deleteByMovingFile:alertReturn==NSAlertDefaultReturn keepSaveStates:YES];
            }];
            [moc save:nil];
            
            [self setNeedsReload];
        }
    }
}

- (void)makeNewCollectionWithSelectedGames:(id)sender
{
    NSArray *selectedGames = [self selectedGames];
    id collection = [[[self libraryController] sidebarController] addCollection:NO];
    [collection setGames:[NSSet setWithArray:selectedGames]];
    
    [self setNeedsReload];
}

- (void)addSelectedGamesToCollection:(id)sender
{
    id collection;
    if(![sender isKindOfClass:[OEDBCollection class]])
    {
        collection = [sender representedObject];
    }
    
    NSArray *selectedGames = [self selectedGames];
    [[collection mutableGames] addObjectsFromArray:selectedGames];
}

- (void)getGameInfoFromArchive:(id)sender
{
    NSArray *selectedGames = [self selectedGames];
    [selectedGames enumerateObjectsUsingBlock:^(OEDBGame *obj, NSUInteger idx, BOOL *stop) {
        [obj setNeedsInfoSyncWithArchiveVG];
    }];
    
    [self reloadDataIndexes:[self selectedIndexes]];
}

- (void)getCoverFromArchive:(id)sender
{
    NSArray *selectedGames = [self selectedGames];
    [selectedGames enumerateObjectsUsingBlock:^(OEDBGame *obj, NSUInteger idx, BOOL *stop) {
        [obj setNeedsCoverSyncWithArchiveVG];
    }];
    
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
    
    if([openPanel runModal] != NSFileHandlingPanelOKButton)
        return;
    
    NSURL   *imageURL       = [openPanel URL];
    NSArray *selectedGames  = [self selectedGames];
    
    [selectedGames enumerateObjectsUsingBlock:^(OEDBGame *obj, NSUInteger idx, BOOL *stop) {
        [obj setBoxImageByURL:imageURL];
    }];
    
    [self reloadDataIndexes:[self selectedIndexes]];
}

- (void)addSaveStateFromFile:(id)sender
{
    NSLog(@"addCoverArtFromFile: Not implemented yet.");
}
#pragma mark -
#pragma mark NSTableView DataSource
- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView
{
    if( aTableView == listView )
    {
        return [[gamesController arrangedObjects] count];
    }
    
    return 0;
}

- (id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex
{
    
    if( aTableView == listView )
    {
        id <OEListViewDataSourceItem> obj = [[gamesController arrangedObjects] objectAtIndex:rowIndex];//(id <ListViewDataSourceItem>)[context objectWithID:objID];
        if(![obj isKindOfClass:[OEDBGame class]]) return nil;
        
        NSString *colIdent = [aTableColumn identifier];
        id result = nil;
        if([colIdent isEqualToString:@"romStatus"])
        {
            BOOL selected = [[listView selectedRowIndexes] containsIndex:rowIndex];
            result = [obj listViewStatusWithSelected:selected playing:[self OE_isGameOpen:(OEDBGame *)obj]];
        }
        else if([colIdent isEqualToString:@"romName"])
        {
            result = [obj listViewTitle];
        } 
        else if([colIdent isEqualToString:@"romRating"])
        {
            result = [obj listViewRating];
        } 
        else if([colIdent isEqualToString:@"romLastPlayed"])
        {
            result = [obj listViewLastPlayed];
        }
        else if([colIdent isEqualToString:@"consoleName"])
        {
            result = [obj listViewConsoleName];
        } 
        else if(colIdent == nil)
        {
            result = obj;
        }
        return result;
    }
    
    return nil;
}

- (void)tableView:(NSTableView *)aTableView setObjectValue:(id)anObject forTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex
{
    
    if( aTableView == listView)
    {
        id <OEListViewDataSourceItem> obj = [[gamesController arrangedObjects] objectAtIndex:rowIndex];
        NSString *columnIdentifier = [aTableColumn identifier];
        if([columnIdentifier isEqualToString:@"romRating"])
        {
            [obj setListViewRating:anObject];
        } else if([columnIdentifier isEqualToString:@"romName"])
        {
            if([anObject isKindOfClass:[NSAttributedString class]])
                anObject = [anObject string];
            
            [obj setListViewTitle:anObject];
        }
        [self reloadDataIndexes:[NSIndexSet indexSetWithIndex:rowIndex]];
    }
}

- (void)tableView:(NSTableView *)tableView sortDescriptorsDidChange:(NSArray *)oldDescriptors
{
    if(tableView == listView)
    {
        [[self gamesController] setSortDescriptors:[listView sortDescriptors]];
        [[self gamesController] rearrangeObjects];
        [listView reloadData];
        _stateRewriteRequired = YES;
    }
}

- (BOOL)OE_isGameOpen:(OEDBGame *)game
{
    BOOL open = NO;
    for(id openDocument in [[NSDocumentController sharedDocumentController] documents])
    {
        if(![openDocument isKindOfClass:[OEGameDocument class]]) continue;

        OEGameDocument *doc = openDocument;
        if([[[[doc gameViewController] rom] game] isEqual:game])
        {
            open = YES;
            break;
        }
    }

    return open;
}

#pragma mark -
#pragma mark TableView Drag and Drop 
- (BOOL)tableView:(NSTableView *)aTableView acceptDrop:(id < NSDraggingInfo >)info row:(NSInteger)row dropOperation:(NSTableViewDropOperation)operation
{
    
    if( aTableView == listView && operation==NSTableViewDropAbove)
        return YES;
    
    return NO;
}

- (NSDragOperation)tableView:(NSTableView *)aTableView validateDrop:(id < NSDraggingInfo >)info proposedRow:(NSInteger)row proposedDropOperation:(NSTableViewDropOperation)operation
{
    
    if( aTableView == listView && operation==NSTableViewDropAbove)
        return NSDragOperationGeneric;
    
    return NSDragOperationNone;
    
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
        if([aCell isKindOfClass:[NSTextFieldCell class]])
        {
            NSDictionary *attr = (@{
                                  NSFontAttributeName            : [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:0 weight:5 size:11.0],
                                  NSForegroundColorAttributeName : [NSColor colorWithDeviceWhite:1.0 alpha:1.0],
                                  });
            
            [aCell setAttributedStringValue:[[NSAttributedString alloc] initWithString:[aCell stringValue] attributes:attr]];
        }
        
        if(![aCell isKindOfClass:[OERatingCell class]])
            [aCell setHighlighted:NO];
    }
    
}

- (BOOL)tableView:(NSTableView *)aTableView shouldEditTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex
{
    if( aTableView == listView )
    {
        if([[aTableColumn identifier] isEqualToString:@"romRating"]) return NO;
        
        return YES;
    }
    
    return NO;
}

- (BOOL)selectionShouldChangeInTableView:(NSTableView *)aTableView
{
    
    if( aTableView == listView )
    {
        return YES;
    }
    
    
    return YES;
}

 - (CGFloat)tableView:(NSTableView *)aTableView heightOfRow:(NSInteger)row
{
    if( aTableView == listView )
    {
        return 17.0;
    }
    
    return 0.0;
}

- (BOOL)tableView:(NSTableView *)aTableView shouldSelectRow:(NSInteger)rowIndex
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
    NSTableView *aTableView = [aNotification object];
    
    if( aTableView == listView )
    {
        _listViewSelectionChangeDate = [NSDate date];

        [gamesController setSelectionIndexes:[aTableView selectedRowIndexes]];
        
        NSIndexSet *selectedIndexes = [listView selectedRowIndexes];
        if([selectedIndexes count] > 0)
        {
            [coverFlowView setSelectedIndex:[selectedIndexes firstIndex]];
        
            [selectedIndexes enumerateIndexesUsingBlock:^(NSUInteger idx, BOOL *stop) {
                [listView setNeedsDisplayInRect:[listView rectOfRow:idx]];
            }];
        }
    }
}

- (BOOL)tableView:(NSTableView *)tableView shouldTrackCell:(NSCell *)cell forTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    if( tableView == listView && [[tableColumn identifier] isEqualToString:@"romRating"] )
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

#pragma mark - Notifications
- (void)OE_emulationDidFinish:(NSNotification *)notification
{
    OEDBRom *rom = [[notification userInfo] objectForKey:OEGameViewControllerROMKey];
    if(!rom) return;

    NSUInteger rowIndex = [[gamesController arrangedObjects] indexOfObject:[rom game]];
    if(rowIndex == NSNotFound) return;

    NSInteger columnIndex = [listView columnWithIdentifier:@"romStatus"];
    NSAssert(columnIndex != -1, @"We should have a column identified by 'romStatus' in the library list view");

    [listView reloadDataForRowIndexes:[NSIndexSet indexSetWithIndex:rowIndex] columnIndexes:[NSIndexSet indexSetWithIndex:columnIndex]];

    // If we ever implement indicators in the grid view, we may want to use -setNeedsReloadIndexes:
}

#pragma mark -
#pragma mark Private
#define reloadDelay 0.1
- (void)OE_managedObjectContextDidSave:(NSNotification *)notification
{
    NSPredicate *predicateForGame = [NSPredicate predicateWithFormat:@"entity = %@", [NSEntityDescription entityForName:@"Game" inManagedObjectContext:[notification object]]];
    NSSet *insertedObjects        = [[[notification userInfo] objectForKey:NSInsertedObjectsKey] filteredSetUsingPredicate:predicateForGame];
    NSSet *deletedObjects         = [[[notification userInfo] objectForKey:NSDeletedObjectsKey] filteredSetUsingPredicate:predicateForGame];
    NSSet *updatedObjects         = [[[notification userInfo] objectForKey:NSUpdatedObjectsKey] filteredSetUsingPredicate:predicateForGame];
    
    if((insertedObjects && [insertedObjects count]) || (deletedObjects && [deletedObjects count]))
    {
        [self performSelector:@selector(noteNumbersChanged) onThread:[NSThread mainThread] withObject:nil waitUntilDone:YES];
    }
    else if(updatedObjects && [updatedObjects count])
    {
        // Nothing was removed or added, just updated so just update the visible items
        [self performSelector:@selector(setNeedsReloadVisible) onThread:[NSThread mainThread] withObject:nil waitUntilDone:YES];
    }
}

- (void)noteNumbersChanged
{
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(updateViews) object:nil];
    [self performSelector:@selector(updateViews) withObject:nil afterDelay:reloadDelay];
}

- (void)OE_fetchGames
{
    NSError *error = nil;
    BOOL ok = [gamesController fetchWithRequest:nil merge:NO error:&error];
    if(!ok)
    {
        NSLog(@"Error while fetching: %@", error);
        return;
    }
    [self OE_updateBlankSlate];
}

- (void)updateViews
{
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(updateViews) object:nil];

    [self OE_fetchGames];
    [gridView noteNumberOfCellsChanged];
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
    [gridView reloadCellsAtIndexes:indexSet];
    [listView reloadDataForRowIndexes:indexSet
                        columnIndexes:[listView columnIndexesInRect:[listView visibleRect]]];
    [indexSet enumerateIndexesUsingBlock:^(NSUInteger idx, BOOL *stop) {
        [coverFlowView reloadCellDataAtIndex:idx];
    }];
}

- (void)_reloadVisibleData
{
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(_reloadVisibleData) object:nil];
    if(!gamesController) return;
    [gamesController rearrangeObjects];
    [gridView reloadCellsAtIndexes:[gridView indexesForVisibleCells]];
    [listView reloadDataForRowIndexes:[NSIndexSet indexSetWithIndexesInRange:[listView rowsInRect:[listView visibleRect]]]
                        columnIndexes:[listView columnIndexesInRect:[listView visibleRect]]];
    [coverFlowView performSelectorOnMainThread:@selector(reloadData) withObject:nil waitUntilDone:NO];
}

- (void)OE_reloadData
{
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(OE_reloadData) object:nil];
    if(!gamesController) return;
    
    NSPredicate *pred = [self representedObject]?[[self representedObject] predicate]:[NSPredicate predicateWithValue:NO];
    [gamesController setFetchPredicate:pred];
    
    NSError *error = nil;
    BOOL ok = [gamesController fetchWithRequest:nil merge:NO error:&error];
    if(!ok)
    {
        NSLog(@"Error while fetching: %@", error);
        return;
    }
    
    [gridView reloadData];
    [listView reloadData];
    [coverFlowView performSelectorOnMainThread:@selector(reloadData) withObject:nil waitUntilDone:NO];
}

@end
