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

#import "OEHUDAlert+DefaultAlertsAdditions.h"
#import "NSURL+OELibraryAdditions.h"

#import "OESidebarController.h"
#import "OETableView.h"

#import "OEArrayController.h"

#import "OECollectionDebugWindowController.h"
#import "OEBackgroundNoisePattern.h"

#import "OEGridGameCell.h"
#pragma mark - Public variables

NSString * const OELastGridSizeKey       = @"lastGridSize";
NSString * const OELastCollectionViewKey = @"lastCollectionView";

static const float OE_coverFlowHeightPercentage = 0.75;

@interface OECollectionViewController ()
{
    IBOutlet NSView *gridViewContainer;// gridview

    IBOutlet OEHorizontalSplitView *flowlistViewContainer; // cover flow and simple list container
    IBOutlet OEBlankSlateView *blankSlateView;
}

- (void)OE_managedObjectContextDidUpdate:(NSNotification *)notification;
@end

@implementation OECollectionViewController
{
    int _selectedViewTag;
}
@synthesize libraryController, listView=listView, coverFlowView=coverFlowView;

+ (void)initialize
{
    // Make sure not to reinitialize for subclassed objects
    if(self != [OECollectionViewController class]) return;

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
}

#pragma mark -
#pragma mark View Controller Stuff
- (void)loadView
{
    [super loadView];

    // Setup View
    [[self view] setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];

    // Set up GridView
    [_gridView setCellClass:[OEGridGameCell class]];
    [_gridView setDelegate:self];
    [_gridView setDataSource:self];
    [_gridView setDraggingDestinationDelegate:self];
    [_gridView setCellSize:defaultGridSize];

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
    NSManagedObjectContext *context = [[OELibraryDatabase defaultDatabase] mainThreadContext];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(OE_managedObjectContextDidUpdate:) name:NSManagedObjectContextDidSaveNotification object:context];

    [[NSUserDefaults standardUserDefaults] addObserver:self forKeyPath:OEDisplayGameTitle options:0 context:NULL];

    // If the view has been loaded after a collection has been set via -setRepresentedObject:, set the appropriate
    // fetch predicate to display the items in that collection via -OE_reloadData. Otherwise, the view shows an
    // empty collection until -setRepresentedObject: is received again
    if([self representedObject]) [self reloadData];
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
- (void)setRepresentedObject:(id<OEGameCollectionViewItemProtocol>)representedObject
{
    NSAssert([representedObject conformsToProtocol:@protocol(OEGameCollectionViewItemProtocol)], @"OECollectionViewController accepts OECollectionViewItemProtocol represented objects only");

    if(representedObject == [self representedObject])
    {
        return;
    }
    [super setRepresentedObject:representedObject];

    [[listView tableColumnWithIdentifier:@"listViewConsoleName"] setHidden:![representedObject shouldShowSystemColumnInListView]];

    [self reloadData];
}

- (id <OEGameCollectionViewItemProtocol>)representedObject
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
    if(_selectedViewTag == OEGridViewTag) [coder encodeRect:[[_gridView enclosingScrollView] documentVisibleRect] forKey:@"gridViewVisibleRect"];

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

    FIXME("restore default sort descriptors");
    if(listViewSortDescriptors == nil)
        ;//        listViewSortDescriptors = OE_defaultSortDescriptors;

    FIXME("restore selection");
    // [gamesController setSelectionIndexes:selectionIndexes];
    [listView setSortDescriptors:listViewSortDescriptors];
    [listView setHeaderState:listViewHeaderState];
    [self OE_switchToView:selectedViewTag];
    [sizeSlider setFloatValue:sliderValue];
    [self changeGridSize:sizeSlider];
    [searchField setStringValue:@""];
	[self search:searchField];

    [_gridView setSelectionIndexes:selectionIndexes byExtendingSelection:NO];
    
    if(selectedViewTag == OEGridViewTag)
    {
        //[_gridView setSelectionIndexes:selectionIndexes];
        [_gridView scrollRectToVisible:gridViewVisibleRect];
    }

    [self updateBlankSlate];
}

#pragma mark -
- (NSArray *)selectedGames
{
    [self doesNotImplementSelector:_cmd];
    return nil;
}

- (NSIndexSet *)selectedIndexes
{
    [self doesNotImplementSelector:_cmd];
    return nil;
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
            FIXME("restore default sort descriptors");
            ; //sortDescriptors = OE_defaultSortDescriptors;
            break;
        default:
            sortDescriptors = [listView sortDescriptors];
            reloadListView = YES;
            break;
    }

    FIXME("restore default sort descriptors");
//    [[self gamesController] setSortDescriptors:sortDescriptors];

    if(reloadListView)
        [listView reloadData];
    else
        [_gridView reloadData];

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

- (void)updateBlankSlate
{
    if(![self shouldShowBlankSlate])
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

        [blankSlateView setRepresentedObject:[self representedObject]];
    }
}

- (BOOL)shouldShowBlankSlate
{
    return NO;
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
    [self updateBlankSlate];
}

#pragma mark -
#pragma mark Toolbar Actions
- (IBAction)search:(id)sender
{
    [self doesNotImplementSelector:_cmd];
}

- (IBAction)changeGridSize:(id)sender
{
    float zoomValue = [sender floatValue];

    [_gridView setCellSize:OEScaleSize(defaultGridSize, zoomValue)];
    [[NSUserDefaults standardUserDefaults] setValue:[NSNumber numberWithFloat:zoomValue] forKey:OELastGridSizeKey];
}

#pragma mark -
#pragma mark Context Menu
- (NSMenu*)menuForItemsAtIndexes:(NSIndexSet *)indexes
{
    return nil;
}

- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
    return [[self libraryController] validateMenuItem:menuItem];
}

#pragma mark - Blank Slate Delegate
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
#pragma mark - Data Sources
- (NSUInteger)numberOfItemsInImageBrowser:(IKImageBrowserView *)aBrowser
{
    return 0;
}

- (NSUInteger)numberOfGroupsInImageBrowser:(IKImageBrowserView *)aBrowser
{
    return 0;
}

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView
{
    return 0;
}

#pragma mark - Delegates
- (void)tableViewWasDoubleClicked:(id)sender
{}

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

- (void)fetchItems
{
    [self doesNotImplementSelector:_cmd];
}

- (void)updateViews
{
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(updateViews) object:nil];
    [self fetchItems];
    [listView noteNumberOfRowsChanged];
    [self setNeedsReloadVisible];
    [self updateBlankSlate];
}

- (void)setNeedsReload
{
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(reloadData) object:nil];
    [self performSelector:@selector(reloadData) withObject:nil afterDelay:reloadDelay];
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

#pragma FIXME("force arraycontroller to rearrange")
//   if(!gamesController) return;
//    [gamesController rearrangeObjects];
    [_gridView performSelectorOnMainThread:@selector(reloadData) withObject:Nil waitUntilDone:NO];
    //[_gridView reloadCellsAtIndexes:indexSet];
    [listView reloadDataForRowIndexes:indexSet
                        columnIndexes:[listView columnIndexesInRect:[listView visibleRect]]];
    [indexSet enumerateIndexesUsingBlock:^(NSUInteger idx, BOOL *stop) {
        [coverFlowView reloadCellDataAtIndex:(int)idx];
    }];
}

- (void)_reloadVisibleData
{
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(_reloadVisibleData) object:nil];
#pragma FIXME("force arraycontroller to rearrange")
    //   if(!gamesController) return;
    //    [gamesController rearrangeObjects];

    [_gridView performSelectorOnMainThread:@selector(reloadData) withObject:Nil waitUntilDone:NO];
    //[_gridView reloadCellsAtIndexes:[_gridView indexesForVisibleCells]];
    [listView reloadDataForRowIndexes:[NSIndexSet indexSetWithIndexesInRange:[listView rowsInRect:[listView visibleRect]]]
                        columnIndexes:[listView columnIndexesInRect:[listView visibleRect]]];
    [coverFlowView performSelectorOnMainThread:@selector(reloadData) withObject:nil waitUntilDone:NO];
}

- (void)reloadData
{
    OECoreDataMainThreadAssertion();
    
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(reloadData) object:nil];

    [self fetchItems];

#pragma FIXME("check if fetching is enough")
    /*
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
     */
    
    [_gridView reloadData];
    [listView reloadData];
    [coverFlowView performSelectorOnMainThread:@selector(reloadData) withObject:nil waitUntilDone:NO];
    
    [self updateBlankSlate];
}

@end
