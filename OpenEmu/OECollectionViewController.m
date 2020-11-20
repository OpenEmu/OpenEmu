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

#import "OEROMImporter.h"

#import "OEListViewDataSourceItem.h"

#import "OECoverGridDataSourceItem.h"
#import "OEBlankSlateView.h"

#import "OEDBSystem.h"
@import OpenEmuKit;
#import "OEDBGame.h"
#import "OEDBRom.h"
#import "OEDBCollection.h"
#import "OEDBSmartCollection.h"
#import "OEDBSaveState.h"

#import "OEDBAllGamesCollection.h"

#import "OELibraryDatabase.h"

#import "OESidebarController.h"

#import "OEArrayController.h"

#import "OEGridGameCell.h"

#import "OEPrefLibraryController.h"

#import "OpenEmu-Swift.h"

#pragma mark - Public variables

NSString * const OELastGridSizeKey       = @"lastGridSize";
NSString * const OELastCollectionViewKey = @"lastCollectionView";

static void *OEUserDefaultsDisplayGameTitleKVOContext = &OEUserDefaultsDisplayGameTitleKVOContext;

@implementation NSDate (OESortAdditions)

/// Implementation of the sort selector used by the list view's "Last Played" column in OECollectionViewController.xib.
- (NSComparisonResult)OE_compareDMYTranslatingNilToDistantPast:(NSDate *)anotherDate
{
    if (!anotherDate) {
        return [self compare:NSDate.distantPast];
    }
    
    NSCalendar *gregorian = [[NSCalendar alloc] initWithCalendarIdentifier:NSCalendarIdentifierGregorian];
    
    NSDateComponents *selfDMY = [gregorian components:(NSCalendarUnitDay | NSCalendarUnitMonth | NSCalendarUnitYear) fromDate:self];
    NSDateComponents *anotherDMY = [gregorian components:(NSCalendarUnitDay | NSCalendarUnitMonth | NSCalendarUnitYear) fromDate:anotherDate];
    
    if (selfDMY.year != anotherDMY.year) {
        return (selfDMY.year > anotherDMY.year ? NSOrderedDescending : NSOrderedAscending);
    } else if (selfDMY.month != anotherDMY. month) {
        return (selfDMY.month > anotherDMY.month ? NSOrderedDescending : NSOrderedAscending);
    } else if (selfDMY.day != anotherDMY.day) {
        return (selfDMY.day > anotherDMY.day ? NSOrderedDescending : NSOrderedAscending);
    }
    
    return NSOrderedSame;
}

@end

@interface OECollectionViewController ()
{
    IBOutlet NSView *gridViewContainer;// gridview
    IBOutlet OEBlankSlateView *blankSlateView;
}

@property(nonatomic, readwrite) OECollectionViewControllerViewTag selectedViewTag;

@end

@implementation OECollectionViewController
@synthesize listView=listView;

+ (void)initialize {
    // Make sure not to reinitialize for subclassed objects
    if(self != OECollectionViewController.class) return;

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

- (OELibraryToolbar * _Nullable)toolbar
{
    NSToolbar *toolbar = self.view.window.toolbar;
    if ([toolbar isKindOfClass:OELibraryToolbar.class])
    {
        return (OELibraryToolbar *)toolbar;
    }
    
    return nil;
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

    // Set up list view
    [listView setTarget:self];
    [listView setDelegate:self];
    [listView setDataSource:self];
    [listView setDoubleAction:@selector(tableViewWasDoubleClicked:)];
    [listView setRowSizeStyle:NSTableViewRowSizeStyleCustom];
    [listView setRowHeight:20.0];
    [listView setSortDescriptors:[self defaultSortDescriptors]];
    [listView setAllowsMultipleSelection:YES];
    [listView registerForDraggedTypes:@[NSPasteboardTypeFileURL]];

    // Setup BlankSlate View
    [blankSlateView setDelegate:self];
    [blankSlateView registerForDraggedTypes:@[NSPasteboardTypeFileURL]];

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
    
    [notificationCenter addObserver:self selector:@selector(libraryLocationDidChange:) name:OELibraryLocationDidChangeNotification object:nil];
}

- (void)viewDidAppear
{
    [super viewDidAppear];
    
    // Update grid view with current size slider zoom value.
    NSSlider *sizeSlider = self.toolbar.gridSizeSlider;
    sizeSlider.floatValue = [NSUserDefaults.standardUserDefaults floatForKey:OELastGridSizeKey];
    [self zoomGridViewWithValue:[sizeSlider floatValue]];
    
    // update frame of the blank slate view (in viewDidLoad we didn't have a
    // window to check the toolbar height of).
    // TODO: remove together with the other todo in -OE_staticContentRect
    [blankSlateView setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];
    [blankSlateView setFrame:[self OE_staticContentRect]];
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
    
    [self _doMojaveGridViewHack];
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

- (NSArray<OEDBGame *> *)selectedGames
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

- (void)showGridView
{
    [self OE_switchToView:OEGridViewTag];
}

- (void)showListView
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
    
    [self OE_showView:tag];

    if (tag != OEBlankSlateTag) {
        _selectedViewTag = tag;
    }
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
    if (tag == OEBlankSlateTag) {
        [view setFrame:[self OE_staticContentRect]];
    } else {
        [view setFrame:[self OE_scrollingContentRect]];
    }

    // restore first responder if necessary
    if(makeFirstResponder)
        [[[self view] window] makeFirstResponder:view];
}

- (NSRect)OE_staticContentRect
{
    NSRect frame = self.view.bounds;
    if (!self.view.window)
        // TODO: refactor to avoid triggering this case
        return frame;
    /* offset down to avoid the title bar */
    NSRect contentWithoutTitle = self.view.window.contentLayoutRect;
    NSRect contentWithTitle = self.view.window.contentView.frame;
    CGFloat titleBarHeight = contentWithTitle.size.height - contentWithoutTitle.size.height;
    frame.size.height -= titleBarHeight;
    return frame;
}

- (NSRect)OE_scrollingContentRect
{
    /* scroll views can stay under the title bar for eye candy */
    return self.view.bounds;
}

- (void)updateBlankSlate
{
    if (!self.shouldShowBlankSlate) {
        [self OE_switchToView:self.selectedViewTag];
    }
    else
    {
        [self OE_switchToView:OEBlankSlateTag];
        blankSlateView.representedObject = self.representedObject;
    }
}

- (BOOL)shouldShowBlankSlate
{
    return NO;
}

#pragma mark - Context Menu
- (NSMenu*)menuForItemsAtIndexes:(NSIndexSet *)indexes
{
    return nil;
}

- (NSMenu*)gridView:(OEGridView *)gridView menuForItemsAtIndexes:(NSIndexSet *)indexes
{
    return [self menuForItemsAtIndexes:indexes];
}

- (NSMenu *)tableView:(OEGameTableView*)tableView menuForItemsAtIndexes:(NSIndexSet*)indexes
{
    return [self menuForItemsAtIndexes:indexes];
}
#pragma mark - Blank Slate Delegate
- (NSDragOperation)blankSlateView:(OEBlankSlateView *)blankSlateView validateDrop:(id<NSDraggingInfo>)draggingInfo
{
    if (![[[draggingInfo draggingPasteboard] types] containsObject:NSPasteboardTypeFileURL])
        return NSDragOperationNone;

    return NSDragOperationCopy;
}

- (BOOL)blankSlateView:(OEBlankSlateView*)blankSlateView acceptDrop:(id<NSDraggingInfo>)draggingInfo
{
    NSPasteboard *pboard = [draggingInfo draggingPasteboard];
    if (![[pboard types] containsObject:NSPasteboardTypeFileURL])
        return NO;

    NSArray<NSURL*> *files = [pboard readObjectsForClasses:@[[NSURL class]] options:@{NSPasteboardURLReadingFileURLsOnlyKey: @YES}];
    OEROMImporter *romImporter = self.database.importer;
    OEDBCollection *collection = [[self representedObject] isKindOfClass:[OEDBCollection class]] ? (OEDBCollection *)[self representedObject] : nil;
    [romImporter importItemsAtURLs:files intoCollectionWithID:[collection permanentID]];

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


#pragma mark - Quick Look


- (BOOL)acceptsPreviewPanelControl:(QLPreviewPanel *)panel
{
    return NO;
}


- (void)beginPreviewPanelControl:(QLPreviewPanel *)panel
{
    // We are now responsible of the preview panel
    panel.delegate = self;
    panel.dataSource = self;
}


- (void)endPreviewPanelControl:(QLPreviewPanel *)panel
{
    // Lost responsibility on the preview panel
}


- (BOOL)toggleQuickLook
{
    QLPreviewPanel *panel;
    
    if (![self acceptsPreviewPanelControl:nil])
        return NO;
    
    panel = [QLPreviewPanel sharedPreviewPanel];
    if ([panel isVisible])
        [panel orderOut:nil];
    else
        [panel makeKeyAndOrderFront:nil];
    return YES;
}


- (NSInteger)numberOfPreviewItemsInPreviewPanel:(QLPreviewPanel *)panel
{
    [self doesNotImplementSelector:_cmd];
    return 0;
}


- (id <QLPreviewItem>)previewPanel:(QLPreviewPanel *)panel previewItemAtIndex:(NSInteger)index
{
    [self doesNotImplementSelector:_cmd];
    return nil;
}


- (NSInteger)imageBrowserViewIndexForPreviewItem:(id <QLPreviewItem>)item
{
    return -1;
}


- (void)refreshPreviewPanelIfNeeded
{
    QLPreviewPanel *panel;
  
    if ([QLPreviewPanel sharedPreviewPanelExists]) {
        panel = [QLPreviewPanel sharedPreviewPanel];
        if ([panel isVisible] && [panel delegate] == self)
            [panel reloadData];
    }
}


- (NSRect)previewPanel:(QLPreviewPanel *)panel sourceFrameOnScreenForPreviewItem:(id<QLPreviewItem>)item
{
    if (_selectedViewTag != OEGridViewTag)
        return NSZeroRect;

    NSInteger i = [self imageBrowserViewIndexForPreviewItem:item];
    if (i < 0)
        return NSZeroRect;
    
    IKImageBrowserCell *itemcell = [self.gridView cellForItemAtIndex:i];
    NSRect thumbframe = [itemcell imageFrame];
    NSScrollView *scrollv = [self.gridView enclosingScrollView];
    thumbframe = [self.gridView convertRect:thumbframe toView:scrollv];
    if (!NSContainsRect([scrollv bounds], thumbframe))
        return NSZeroRect;
    
    NSWindow *w = [self.gridView window];
    thumbframe = [scrollv convertRect:thumbframe toView:w.contentView];
    NSRect screenrect = [w convertRectToScreen:thumbframe];
    return screenrect;
}


- (BOOL)previewPanel:(QLPreviewPanel *)panel handleEvent:(NSEvent *)event
{
  if ([event type] == NSEventTypeKeyDown || [event type] == NSEventTypeKeyUp) {
    [self.gridView.window sendEvent:event];
    return YES;
  }
  return NO;
}


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
    if (self.selectedViewTag == OEBlankSlateTag || self.view.superview != nil) {
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

// MARK: - Mojave Grid View Hack

/// Fix display of the grid view at launch on macOS 10.14+. Without this hack, the grid view appears empty or displays a red or magenta color on launch.
/// @note This is intended to be a temporary solution until the grid view is replaced with an NSCollectionView.
- (void)_doMojaveGridViewHack
{
    _gridView.wantsLayer = YES;
    _gridView.wantsLayer = NO;
}

@end
