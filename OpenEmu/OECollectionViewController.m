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
#import "OEGameControlsBar.h"
#import "OEMainWindowController.h"
#import "OEGameViewController.h"
#import "OEGameDocument.h"

#import "OELibraryController.h"
#import "OEROMImporter.h"

#import "OEButton.h"
#import "OESearchField.h"
#import "OETableHeaderCell.h"
#import "OEListViewDataSourceItem.h"

#import "OECoverGridDataSourceItem.h"
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

#import "OESidebarController.h"
#import "OETableView.h"

#import "OEArrayController.h"

#import "OEBackgroundNoisePattern.h"

#import "OEGridGameCell.h"

#import "IKImageFlowView.h"

#import "OEPrefLibraryController.h"

#import "OpenEmu-Swift.h"

#pragma mark - Public variables

NSString * const OELastGridSizeKey       = @"lastGridSize";
NSString * const OELastCollectionViewKey = @"lastCollectionView";

static void *OEUserDefaultsDisplayGameTitleKVOContext = &OEUserDefaultsDisplayGameTitleKVOContext;

@interface OECollectionViewController ()
{
    IBOutlet NSView *gridViewContainer;// gridview
    IBOutlet OEBlankSlateView *blankSlateView;
}

@property(nonatomic, readwrite) OECollectionViewControllerViewTag selectedViewTag;

@end

@implementation OECollectionViewController
@synthesize libraryController, listView=listView;

+ (void)initialize
{
    // Make sure not to reinitialize for subclassed objects
    if(self != [OECollectionViewController class]) return;

    [[NSUserDefaults standardUserDefaults] registerDefaults:@{ OELastGridSizeKey : @1.0f }];
}

- (instancetype)init
{
    self = [super init];
    if (self) {
        _selectedViewTag = -2;
    }
    return self;
}

- (void)dealloc
{
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    [defaults removeObserver:self
                  forKeyPath:OEDisplayGameTitle
                     context:OEUserDefaultsDisplayGameTitleKVOContext];
}

#pragma mark - View Lifecycle

- (void)viewDidLoad
{
    [super viewDidLoad];

    // Setup View
    [[self view] setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];

    // Set up GridView
    [_gridView setCellClass:[OEGridGameCell class]];
    [_gridView setDelegate:self];
    [_gridView setDataSource:self];
    [_gridView setDraggingDestinationDelegate:self];
    [_gridView setCellSize:defaultGridSize];

    //set initial zoom value
    NSSlider *sizeSlider = [[[self libraryController] toolbar] gridSizeSlider];
    [sizeSlider setContinuous:YES];
    CGFloat defaultZoomValue = [[NSUserDefaults standardUserDefaults] floatForKey:OELastGridSizeKey];
    [sizeSlider setFloatValue:defaultZoomValue];
    [self zoomGridViewWithValue:defaultZoomValue];

    // Set up list view
    [listView setTarget:self];
    [listView setDelegate:self];
    [listView setDataSource:self];
    [listView setDoubleAction:@selector(tableViewWasDoubleClicked:)];
    [listView setRowSizeStyle:NSTableViewRowSizeStyleCustom];
    [listView setRowHeight:20.0];
    [listView setSortDescriptors:[self defaultSortDescriptors]];
    [listView setAllowsMultipleSelection:YES];

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
    [blankSlateView setFrame:[[self view] bounds]];

    // If the view has been loaded after a collection has been set via -setRepresentedObject:, set the appropriate
    // fetch predicate to display the items in that collection via -OE_reloadData. Otherwise, the view shows an
    // empty collection until -setRepresentedObject: is received again
    if ([self representedObject])
        [self reloadData];
    
    NSUserDefaults *standardUserDefaults = [NSUserDefaults standardUserDefaults];
    [standardUserDefaults addObserver:self
                           forKeyPath:OEDisplayGameTitle
                              options:0
                              context:OEUserDefaultsDisplayGameTitleKVOContext];
    
    NSNotificationCenter *notificationCenter = [NSNotificationCenter defaultCenter];
    
    // Watch the main thread's managed object context for changes
    NSManagedObjectContext *context = [[OELibraryDatabase defaultDatabase] mainThreadContext];
    [notificationCenter addObserver:self selector:@selector(OE_managedObjectContextDidUpdate:) name:NSManagedObjectContextDidSaveNotification object:context];
    
    [notificationCenter addObserver:self selector:@selector(libraryLocationDidChange:) name:OELibraryLocationDidChangeNotificationName object:nil];
}

- (void)viewWillAppear
{
    [super viewWillAppear];
    
    // Update grid view with current size slider zoom value.
    NSSlider *sizeSlider = [[[self libraryController] toolbar] gridSizeSlider];
    [self zoomGridViewWithValue:[sizeSlider floatValue]];
}

- (NSString *)nibName
{
    return @"OECollectionViewController";
}

#pragma mark - KVO / Notifications
- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary <NSString *, id> *)change context:(void *)context
{
    if(context == OEUserDefaultsDisplayGameTitleKVOContext)
    {
        [self setNeedsReloadVisible];
    }
    else
    {
        [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
    }
}


- (void)libraryLocationDidChange:(NSNotification*)notification
{
    [self reloadData];
}
#pragma mark - OELibrarySubviewControllerProtocol Implementation
- (void)setRepresentedObject:(id<OECollectionViewItemProtocol>)representedObject
{
    if(representedObject == [self representedObject])
    {
        return;
    }
    [super setRepresentedObject:representedObject];
    [self view];
    [self updateBlankSlate];
}

- (id <OECollectionViewItemProtocol>)representedObject
{
    return [super representedObject];
}

#pragma mark - Selection

- (BOOL)isSelected
{
    [self doesNotImplementSelector:_cmd];
    return NO;
}

- (NSArray *)selectedGames
{
    [self doesNotImplementOptionalSelector:_cmd];
    return nil;
}

- (NSIndexSet *)selectedIndexes
{
    [self doesNotImplementOptionalSelector:_cmd];
    return nil;
}

- (void)imageBrowserSelectionDidChange:(IKImageBrowserView *)aBrowser
{
    [self setSelectionIndexes:[aBrowser selectionIndexes]];
}

#pragma mark - Deleting Items
- (void)delete
{
    [self deleteSelectedItems:self];
}

- (void)delete:(id)sender
{
    [self deleteSelectedItems:sender];
}

- (void)deleteBackward:(id)sender
{
    [self deleteSelectedItems:sender];
}

- (void)deleteBackwardByDecomposingPreviousCharacter:(id)sender
{
    [self deleteSelectedItems:sender];
}

- (void)deleteSelectedItems:(id)sender
{
    [self doesNotImplementOptionalSelector:_cmd];
}

#pragma mark -
#pragma mark View Selection
- (IBAction)switchToGridView:(id)sender
{
    [self OE_switchToView:OEGridViewTag];
}

- (IBAction)switchToListView:(id)sender
{
    [self OE_switchToView:OEListViewTag];
}

- (void)OE_switchToView:(OECollectionViewControllerViewTag)tag
{
    // Set sort descriptors and reload data
    NSArray *sortDescriptors = nil;
    BOOL reloadListView = NO;
    switch (tag) {
        case OEGridViewTag:
                sortDescriptors = [self defaultSortDescriptors];
            break;
        default:
            sortDescriptors = [listView sortDescriptors];
            reloadListView = YES;
            break;
    }

    [self setSortDescriptors:sortDescriptors];

    if(reloadListView)
        [listView reloadData];
    else
        [_gridView reloadData];

    if (self.isSelected) {
        [self OE_setupToolbarStatesForViewTag:tag];
    }
    
    if(_selectedViewTag == tag && tag != OEBlankSlateTag)
        return;

    [self OE_showView:tag];

    _selectedViewTag = tag;
}

- (void)OE_showView:(OECollectionViewControllerViewTag)tag
{
    NSView *view;
    switch (tag) {
        case OEBlankSlateTag:
            view = blankSlateView;
            break;
        case OEGridViewTag:
            view = gridViewContainer;
            break;
        case OEListViewTag:
            view = [listView enclosingScrollView];
            break;
        default:
            break;
    }

    if(!view || [view superview] == [self view]) return;

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
    OELibraryToolbar *toolbar = self.libraryController.toolbar;
    switch (tag) {
        case OEGridViewTag:
            toolbar.gridViewButton.state = NSOnState;
            toolbar.listViewButton.state = NSOffState;
            toolbar.gridSizeSlider.enabled = YES;
            break;
        case OEListViewTag:
            toolbar.gridViewButton.state = NSOffState;
            toolbar.listViewButton.state = NSOnState;
            toolbar.gridSizeSlider.enabled = NO;
            break;
        case OEBlankSlateTag:
            toolbar.gridSizeSlider.enabled = NO;
            toolbar.gridViewButton.enabled = NO;
            toolbar.listViewButton.enabled = NO;
            break;
    }
}

- (void)updateBlankSlate
{
    if (!self.shouldShowBlankSlate) {
        
        [self OE_switchToView:self.OE_currentViewTagByToolbarState];

        if (self.isSelected) {
            OELibraryToolbar *toolbar = self.libraryController.toolbar;
            toolbar.gridViewButton.enabled = YES;
            toolbar.listViewButton.enabled = YES;
            toolbar.gridSizeSlider.enabled = self.selectedViewTag == OEGridViewTag;
            toolbar.searchField.enabled = YES;
            toolbar.searchField.menu = nil;
        }
    }
    else
    {
        [self OE_switchToView:OEBlankSlateTag];

        if (self.isSelected) {
            OELibraryToolbar *toolbar = self.libraryController.toolbar;
            toolbar.gridViewButton.enabled = NO;
            toolbar.listViewButton.enabled = NO;
            toolbar.gridSizeSlider.enabled = NO;
            toolbar.searchField.enabled = NO;
            toolbar.searchField.menu = nil;
        }

        blankSlateView.representedObject = self.representedObject;
    }
}

- (BOOL)shouldShowBlankSlate
{
    return NO;
}

- (OECollectionViewControllerViewTag)OE_currentViewTagByToolbarState
{
    if (self.libraryController.toolbar.gridViewButton.state == NSOnState)
        return OEGridViewTag;
    else
        return OEListViewTag;
}

#pragma mark - Toolbar Actions
- (IBAction)search:(id)sender
{
    [self doesNotImplementSelector:_cmd];
}

- (IBAction)changeGridSize:(id)sender
{
    [self zoomGridViewWithValue:[sender floatValue]];
}

#pragma mark - Context Menu
- (NSMenu*)menuForItemsAtIndexes:(NSIndexSet *)indexes
{
    return nil;
}

- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
    return [[self libraryController] validateMenuItem:menuItem];
}

- (NSMenu*)gridView:(OEGridView *)gridView menuForItemsAtIndexes:(NSIndexSet *)indexes
{
    return [self menuForItemsAtIndexes:indexes];
}

- (NSMenu *)tableView:(OETableView*)tableView menuForItemsAtIndexes:(NSIndexSet*)indexes
{
    return [self menuForItemsAtIndexes:indexes];
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
    OEDBCollection *collection = [[self representedObject] isKindOfClass:[OEDBCollection class]] ? (OEDBCollection *)[self representedObject] : nil;
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

- (NSUInteger)numberOfItemsInImageFlow:(IKImageFlowView *)aBrowser
{
    return 0;
}

- (id)imageFlow:(id)aFlowLayer itemAtIndex:(int)index
{
    return nil;
}

#pragma mark - Delegates
- (void)tableViewWasDoubleClicked:(id)sender
{}

- (NSString*)tableView:(NSTableView *)tableView typeSelectStringForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    if([[tableColumn identifier] isEqualToString:@"listViewTitle"])
    {
        return [self tableView:tableView objectValueForTableColumn:tableColumn row:row];
    }
    return @"";
}

- (void)imageBrowser:(IKImageBrowserView *)aBrowser removeItemsAtIndexes:(NSIndexSet *)indexes
{
    [self deleteSelectedItems:aBrowser];
}

- (void)gridView:(OEGridView *)gridView setTitle:(NSString *)title forItemAtIndex:(NSInteger)index
{}
#pragma mark - Core Data
- (NSArray*)defaultSortDescriptors
{
    return @[];
}

- (void)setSortDescriptors:(NSArray*)descriptors
{}

- (void)OE_managedObjectContextDidUpdate:(NSNotification *)notification
{
    [self performSelector:@selector(noteNumbersChanged) onThread:[NSThread mainThread] withObject:nil waitUntilDone:NO];
}

#define reloadDelay 0.5
- (void)noteNumbersChanged
{
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(updateViews) object:nil];
    [self performSelector:@selector(updateViews) withObject:nil afterDelay:reloadDelay];
}

- (void)fetchItems
{
    [self doesNotImplementSelector:_cmd];
}


#pragma mark - Private

- (void)updateViews
{
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(updateViews) object:nil];
    [self fetchItems];
    [listView noteNumberOfRowsChanged];
    [self setNeedsReloadVisible];
    
    /* Call -updateBlankSlate if:
        - This collection view controller is selected.
        - The blank slate view is the current view tag. This allows switching to a different view tag if an item has been added.
     */
    if (self.selectedViewTag == OEBlankSlateTag || self.isSelected) {
        [self updateBlankSlate];
    }
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
    [listView reloadDataForRowIndexes:indexSet
                        columnIndexes:[listView columnIndexesInRect:[listView visibleRect]]];
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
}

- (void)reloadData
{
    OECoreDataMainThreadAssertion();
    
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(reloadData) object:nil];

    [self fetchItems];

    [_gridView reloadData];
    [listView reloadData];

    [self updateBlankSlate];
}

- (void)zoomGridViewWithValue:(CGFloat)zoomValue
{
    _gridView.cellSize = OEScaleSize(defaultGridSize, zoomValue);
    [[NSUserDefaults standardUserDefaults] setFloat:zoomValue forKey:OELastGridSizeKey];
}

@end
