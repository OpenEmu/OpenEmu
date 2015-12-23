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

NSString * const OELastCollectionViewKey = @"lastCollectionView";

const CGFloat MinGridViewZoom = 0.5;
const CGFloat MaxGridViewZoom = 2.5;
const CGFloat DefaultGridViewZoome = 1.0;

static void *OEUserDefaultsDisplayGameTitleKVOContext = &OEUserDefaultsDisplayGameTitleKVOContext;

@interface OECollectionViewController ()
@property(assign) IBOutlet NSView *gridViewContainer;
@property(assign) IBOutlet OEBlankSlateView *blankSlateView;
@property(nonatomic, readwrite) OECollectionViewControllerViewTag selectedViewTag;
@end

@implementation OECollectionViewController
@synthesize controlsToolbar;

- (instancetype)init
{
    self = [super init];
    if (self) {
        _selectedViewTag = OEGridViewTag;
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

    // Set up grid view
    [_gridView setCellClass:[OEGridGameCell class]];
    [_gridView setCellSize:DefaultGridSize];

    // Set up list view
    [_listView setTarget:self];
    [_listView setDoubleAction:@selector(tableViewWasDoubleClicked:)];
    [_listView setRowSizeStyle:NSTableViewRowSizeStyleCustom];
    [_listView setRowHeight:20.0];
    [_listView setSortDescriptors:[self defaultSortDescriptors]];

    // There's no natural order for status indicators, so we don't allow that column to be sorted
    OETableHeaderCell *romStatusHeaderCell = [[_listView tableColumnWithIdentifier:@"listViewStatus"] headerCell];
    [romStatusHeaderCell setClickable:NO];

    [_listView registerForDraggedTypes:[NSArray arrayWithObjects:NSFilenamesPboardType, nil]];

    for(NSTableColumn *aColumn in [_listView tableColumns])
    {
        if([[aColumn dataCell] isKindOfClass:[OECenteredTextFieldCell class]])
            [[aColumn dataCell] setWidthInset:9];

        [[aColumn headerCell] setAlignment:[[aColumn dataCell] alignment]];
    }

    [self addObserver:self forKeyPath:@"controlsToolbar" options:0 context:nil];

    // Setup BlankSlate View
    [_blankSlateView registerForDraggedTypes:[NSArray arrayWithObjects:NSFilenamesPboardType, nil]];
    _blankSlateView.frame = self.view.bounds;

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

- (void)updateToolbar
{
    if(!self.controlsToolbar) return;

    OELibraryToolbar *toolbar = self.libraryController.toolbar;
    toolbar.searchField.menu = nil;

    BOOL toolbarItemsEnabled = NO;
    CGFloat gridSizeFactor = toolbar.gridSizeSlider.floatValue;
    NSString *searchTerm = @"";
    if(!self.shouldShowBlankSlate)
    {
        toolbar.gridViewButton.state = _selectedViewTag == OEGridViewTag ? NSOnState : NSOffState;
        toolbar.listViewButton.state = _selectedViewTag == OEListViewTag ? NSOnState : NSOffState;
        toolbarItemsEnabled = YES;

        gridSizeFactor = _gridView.cellSize.width / DefaultGridSize.width;
        searchTerm = self.currentSearchTerm ?: @"";
    }

    toolbar.gridViewButton.enabled = toolbarItemsEnabled;
    toolbar.listViewButton.enabled = toolbarItemsEnabled;
    toolbar.gridSizeSlider.enabled = toolbarItemsEnabled;
    toolbar.searchField.enabled = toolbarItemsEnabled;

    toolbar.searchField.stringValue = searchTerm;
    toolbar.gridSizeSlider.floatValue = gridSizeFactor;
}

- (NSString *)nibName
{
    return @"OECollectionViewController";
}

#pragma mark - State Management
NSString * const OECollectionViewStateGridZoomFactorKey = @"gridZoom";
NSString * const OECollectionViewStateSearchTermKey = @"searchTerm";
NSString * const OECollectionViewStateSearchDomainKey = @"searchDomain";
NSString * const OECollectionViewStateGridVisibleRectKey = @"gridVisibleRect";
NSString * const OECollectionViewStateSelectionIndexesKey = @"selectionIndexes";
NSString * const OECollectionViewStateSortDescriptorsKey = @"sortDescriptors";
NSString * const OECollectionViewStateListVisibleRectKey = @"listVisibleRect";

- (void)storeStateWithKey:(NSString*)key
{
    NSMutableData *data = [NSMutableData data];
    NSKeyedArchiver *archiver = [[NSKeyedArchiver alloc] initForWritingWithMutableData:data];

    CGFloat gridZoomFactor = _gridView.cellSize.width / DefaultGridSize.width;
    NSRect gridViewVisibleRect = _gridView.enclosingScrollView.documentVisibleRect;
    NSIndexSet *selectionIndexes = self.selectionIndexes;
    NSString *searchTerm = self.currentSearchTerm;
    NSString *searchDomain = @""; // TODO: use self.currentSearchDomain
    NSArray *listViewSortDescriptors = _listView.sortDescriptors;

    [archiver encodeRect:gridViewVisibleRect forKey:OECollectionViewStateGridVisibleRectKey];
    [archiver encodeFloat:gridZoomFactor forKey:OECollectionViewStateGridZoomFactorKey];
    [archiver encodeObject:selectionIndexes forKey:OECollectionViewStateSelectionIndexesKey];
    [archiver encodeObject:searchTerm forKey:OECollectionViewStateSearchTermKey];
    [archiver encodeObject:searchDomain forKey:OECollectionViewStateSearchDomainKey];
    [archiver encodeObject:listViewSortDescriptors forKey:OECollectionViewStateSortDescriptorsKey];

    [archiver finishEncoding];
    [[NSUserDefaults standardUserDefaults] setObject:data forKey:key];
}

#define DecodeWithMethod(_var_, _key_, _METHOD_) if([unarchiver containsValueForKey:_key_]) _var_ = [unarchiver _METHOD_ _key_];
#define DecodeFloatIfSet(_var_, _key_) DecodeWithMethod(_var_, _key_, decodeFloatForKey:)
#define DecodeObjectIfSet(_var_, _key_) DecodeWithMethod(_var_, _key_, decodeObjectForKey:)
#define DecodeRectIfSet(_var_, _key_) DecodeWithMethod(_var_, _key_, decodeRectForKey:)

- (void)restoreStateWithKey:(NSString*)key
{
    NSData *state = [[NSUserDefaults standardUserDefaults] objectForKey:key];
    NSKeyedUnarchiver *unarchiver = nil;;

    if(state && [state isKindOfClass:[NSData class]]) {
        unarchiver = [[NSKeyedUnarchiver alloc] initForReadingWithData:state];
    }

    CGFloat gridZoomFactor = DefaultGridViewZoome;
    NSRect gridViewVisibleRect = NSZeroRect;
    NSIndexSet *selectionIndexes = [NSIndexSet indexSet];
    NSString *searchTerm = @"";
    NSString *searchDomain = @"";
    NSArray *sortDescriptors = [self defaultSortDescriptors];

    DecodeFloatIfSet(gridZoomFactor, OECollectionViewStateGridZoomFactorKey);
    DecodeObjectIfSet(searchTerm, OECollectionViewStateSearchTermKey);
    DecodeObjectIfSet(searchDomain, OECollectionViewStateSearchDomainKey);
    DecodeObjectIfSet(sortDescriptors, OECollectionViewStateSortDescriptorsKey);
    DecodeRectIfSet(gridViewVisibleRect, OECollectionViewStateGridVisibleRectKey);
    DecodeObjectIfSet(selectionIndexes, OECollectionViewStateSelectionIndexesKey);

    [unarchiver finishDecoding];

    self.currentSearchTerm = searchTerm;
    // TODO: add currentSearchDomain property and set it here
    // TODO: might need to reload data here so zooming, selecting, etc. is done with the proper filter
    [self OE_searchFor:searchTerm];
    self.selectionIndexes = selectionIndexes;
    [self zoomGridViewWithValue:gridZoomFactor];
    [[self gridView] scrollRectToVisible:gridViewVisibleRect];
}
#undef DecodeRectIfSet
#undef DecodeObjectIfSet
#undef DecodeFloatIfSet
#undef DecodeWithMethod

#pragma mark - KVO / Notifications
- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary <NSString *, id> *)change context:(void *)context
{
    if(context == OEUserDefaultsDisplayGameTitleKVOContext)
    {
        [self setNeedsReloadVisible];
    }
    else if([keyPath isEqualToString:@"controlsToolbar"])
    {
        [self updateToolbar];
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
            sortDescriptors = [_listView sortDescriptors];
            reloadListView = YES;
            break;
    }

    [self setSortDescriptors:sortDescriptors];

    if(reloadListView)
        [_listView reloadData];
    else
        [_gridView reloadData];

    [self OE_setupToolbarStatesForViewTag:tag];

    if(_selectedViewTag == tag && tag != OEBlankSlateTag)
        return;

    [self OE_showView:tag];

    _selectedViewTag = tag;
    [self updateToolbar];
}

- (void)OE_showView:(OECollectionViewControllerViewTag)tag
{
    NSView *view;
    switch (tag) {
        case OEBlankSlateTag:
            view = _blankSlateView;
            break;
        case OEGridViewTag:
            view = _gridViewContainer;
            break;
        case OEListViewTag:
            view = [_listView enclosingScrollView];
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
    [self updateToolbar];
}

- (void)updateBlankSlate
{
    if (!self.shouldShowBlankSlate) {
        [self OE_switchToView:_selectedViewTag != OEBlankSlateTag ? _selectedViewTag : OEGridViewTag];
    }
    else
    {
        [self OE_switchToView:OEBlankSlateTag];
        _blankSlateView.representedObject = self.representedObject;
    }

    [self updateToolbar];
}

- (BOOL)shouldShowBlankSlate
{
    return NO;
}

#pragma mark - Toolbar Actions
- (IBAction)search:(id)sender
{
    [self OE_searchFor:[sender stringValue]];
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
- (void)OE_searchFor:(NSString*)string
{
    [self doesNotImplementSelector:_cmd];
}

- (void)updateViews
{
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(updateViews) object:nil];
    [self fetchItems];
    [_listView noteNumberOfRowsChanged];
    [self setNeedsReloadVisible];
    
    /* Call -updateBlankSlate if:
        - This collection view controller is selected.
        - The blank slate view is the current view tag. This allows switching to a different view tag if an item has been added.
     */
    if (self.selectedViewTag == OEBlankSlateTag) {
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
    [_listView reloadDataForRowIndexes:indexSet
                        columnIndexes:[_listView columnIndexesInRect:[_listView visibleRect]]];
}

- (void)_reloadVisibleData
{
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(_reloadVisibleData) object:nil];
#pragma FIXME("force arraycontroller to rearrange")
    //   if(!gamesController) return;
    //    [gamesController rearrangeObjects];

    [_gridView performSelectorOnMainThread:@selector(reloadData) withObject:Nil waitUntilDone:NO];
    //[_gridView reloadCellsAtIndexes:[_gridView indexesForVisibleCells]];
    [_listView reloadDataForRowIndexes:[NSIndexSet indexSetWithIndexesInRange:[_listView rowsInRect:[_listView visibleRect]]]
                        columnIndexes:[_listView columnIndexesInRect:[_listView visibleRect]]];
}

- (void)reloadData
{
    OECoreDataMainThreadAssertion();
    
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(reloadData) object:nil];

    [self fetchItems];

    [_gridView reloadData];
    [_listView reloadData];

    [self updateBlankSlate];
}


- (void)zoomGridViewWithValue:(CGFloat)zoomValue
{
    NSLog(@"-zoomGridViewWithValue: %f", zoomValue);
    zoomValue = MAX(MIN(zoomValue, MaxGridViewZoom), MinGridViewZoom);
    _gridView.cellSize = OEScaleSize(DefaultGridSize, zoomValue);
}

@end
