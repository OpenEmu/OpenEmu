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

#import "OESidebarController.h"
#import "OESidebarGroupItem.h"
#import "OESidebarItem.h"
#import "OELibraryDatabase.h"
#import "OESidebarCell.h"

#import "OEPrefLibraryController.h"

#import "OESidebarOutlineView.h"
#import "OEDBGame.h"
#import "OEDBAllGamesCollection.h"
#import "OEDBSystem.h"
#import "OEDBSmartCollection.h"
#import "OEGameCollectionViewItemProtocol.h"

#import "OEHUDAlert.h"

#import "OEROMImporter.h"

#import "NSArray+OEAdditions.h"

#import "OpenEmu-Swift.h"

extern NSString *const OELastSidebarSelectionKey;
NSString *const OESuppressRemoveCollectionConfirmationKey = @"removeCollectionWithoutConfirmation";
extern NSString * const OEDBSystemAvailabilityDidChangeNotification;

NSString * const OESidebarSelectionDidChangeNotificationName = @"OESidebarSelectionDidChange";

NSString * const OESidebarGroupConsolesAutosaveName    = @"sidebarConsolesItem";
NSString * const OESidebarGroupCollectionsAutosaveName = @"sidebarCollectionsItem";

NSString * const OESidebarMinWidth = @"sidebarMinWidth";
NSString * const OESidebarMaxWidth = @"sidebarMaxWidth";
NSString * const OEMainViewMinWidth = @"mainViewMinWidth";

@interface OESidebarController ()
{
    id editingItem;
}

@property (strong, readwrite) NSArray *groups;
@property (strong, readwrite) NSArray *systems;
@property (strong, readwrite) NSArray *collections;

@property (weak) IBOutlet OEGameScannerViewController *gameScannerViewController;

/// If YES, the sidebar scroll view's scroll bars have been flashed to the user to make sure they know there are more systems than what may be currently visible to them. We only want to do it once, though.
@property (nonatomic) BOOL scrollersFlashed;

@end

@implementation OESidebarController
@synthesize groups, database=_database, editingItem;
@dynamic view;

+ (void)initialize
{
    // Make sure not to reinitialize for subclassed objects
    if (self != [OESidebarController class])
        return;

    [[NSUserDefaults standardUserDefaults] registerDefaults:@{
        OESidebarGroupConsolesAutosaveName    : @YES,
        OESidebarGroupCollectionsAutosaveName : @YES,
    }];
}

- (NSString*)nibName {
    return @"OESidebarController";
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    self.groups = @[
                    [OESidebarGroupItem groupItemWithName:NSLocalizedString(@"Consoles", @"") autosaveName:OESidebarGroupConsolesAutosaveName],
                    [OESidebarGroupItem groupItemWithName:NSLocalizedString(@"Collections", @"") autosaveName:OESidebarGroupCollectionsAutosaveName]
                    ];

    OESidebarOutlineView *sidebarView = (OESidebarOutlineView *)self.view;

    [sidebarView setHeaderView:nil];

    OESidebarCell *cell = [[OESidebarCell alloc] init];
    [cell setThemeKey:@"sidebar"];
    for(NSTableColumn *column in sidebarView.tableColumns)
        [column setDataCell:cell];
    cell.editable = YES;
    
    NSNotificationCenter *defaults = [NSNotificationCenter defaultCenter];
    
    [defaults addObserver:self selector:@selector(controlTextDidEndEditing:) name:NSControlTextDidEndEditingNotification object:cell];
    [defaults addObserver:self selector:@selector(controlTextDidBeginEditing:) name:NSControlTextDidBeginEditingNotification object:cell];
    
    [defaults addObserver:self selector:@selector(libraryLocationDidChange:) name:OELibraryLocationDidChangeNotificationName object:nil];

    sidebarView.indentationPerLevel = 7;
    sidebarView.intercellSpacing = NSMakeSize(0, 4);
    sidebarView.autosaveName = @"sidebarView";
    sidebarView.autoresizesOutlineColumn = NO;
    sidebarView.delegate = self;
    sidebarView.dataSource = self;
    sidebarView.allowsEmptySelection = NO;
    [sidebarView registerForDraggedTypes:@[ OEPasteboardTypeGame, NSFilenamesPboardType ]];
    [sidebarView expandItem:nil expandChildren:YES];

    NSScrollView *enclosingScrollView = sidebarView.enclosingScrollView;
    if (enclosingScrollView != nil) {
        enclosingScrollView.drawsBackground = NO;
        sidebarView.backgroundColor = [NSColor clearColor];
    } else {
        sidebarView.backgroundColor = [NSColor colorWithDeviceWhite:0.19 alpha:1.0];
    }

    [defaults addObserver:self selector:@selector(reloadDataAndPreserveSelection) name:OEDBSystemAvailabilityDidChangeNotification object:nil];

    id viewsNextResponder = self.view.nextResponder;
    if (viewsNextResponder != self) {
        self.view.nextResponder = self;
        self.nextResponder = viewsNextResponder;
    }
}

- (void)viewWillAppear
{
    [super viewWillAppear];
    
    // Scroll to the selection.
    OESidebarOutlineView *sidebarView = (OESidebarOutlineView *)self.view;
    NSInteger selectedRow = sidebarView.selectedRow;
    if (selectedRow != -1) {
        [sidebarView scrollRowToVisible:selectedRow];
    }
}

- (void)viewDidAppear
{
    [super viewDidAppear];
    
    // Make sure the user knows there are more systems to scroll to--but only do it once.
    if (!self.scrollersFlashed) {
        [self.view.enclosingScrollView flashScrollers];
        self.scrollersFlashed = YES;
    }
}

- (void)deviceDidAppear:(id)notification
{
    [self reloadDataAndPreserveSelection];
}

- (void)deviceDidDisappear:(id)notification
{
    [self reloadDataAndPreserveSelection];
}

- (void)setDatabase:(OELibraryDatabase *)database
{
    // get last selected collection item
    NSUserDefaults *standardUserDefaults = [NSUserDefaults standardUserDefaults];
    id itemID = [standardUserDefaults valueForKey:OELastSidebarSelectionKey];

    // set database
    _database = database;
    [self reloadData];

    // Look for the collection item
    id collectionItem = nil;
    if (itemID != nil && [itemID isKindOfClass:[NSString class]]) {
        
        collectionItem = [self.systems firstObjectMatchingBlock:
                          ^ BOOL (id obj) {
                              return [[obj sidebarID] isEqualTo:itemID];
                          }];
        if (!collectionItem)
            collectionItem = [self.collections firstObjectMatchingBlock:
                              ^ BOOL (id obj) {
                                  return [[obj sidebarID] isEqualTo:itemID];
                              }];    }

    // Select the found collection item, or select the first item by default
    if (collectionItem != nil)
        [self selectItem:collectionItem];
}

#pragma mark - Public

- (void)setEnabled:(BOOL)enabled
{
    OESidebarOutlineView *sidebarView = (OESidebarOutlineView *)self.view;
    sidebarView.enabled = enabled;
}

- (IBAction)addCollectionAction:(id)sender
{
    [self addCollection:NO];
}

- (id)addCollection:(BOOL)isSmart
{
    id item = isSmart ? [self.database addNewSmartCollection:nil] : [self.database addNewCollection:nil];

    [self reloadData];
    [self expandCollections:self];
    [self selectItem:item];
    [self startEditingItem:item];

    return item;
}

- (id)duplicateCollection:(id)originalCollection
{
    OEDBCollection *duplicateCollection = [self.database addNewCollection:[NSString stringWithFormat:NSLocalizedString(@"%@ copy", @"Duplicated collection name"), [originalCollection valueForKey:@"name"]]];

    duplicateCollection.games = [originalCollection games];
    [duplicateCollection save];

    [self reloadData];
    [self expandCollections:self];

    return duplicateCollection;
}

- (void)reloadData
{
    if (!self.database)
        return;

    OELibraryDatabase *database = self.database;
    NSManagedObjectContext *context = database.mainThreadContext;

    NSArray *systems = [OEDBSystem enabledSystemsinContext:context] ?: @[];
    self.systems = systems;

    NSArray *collections = database.collections;
    self.collections = collections;

    OESidebarOutlineView *sidebarView = (OESidebarOutlineView *)self.view;
    [sidebarView reloadData];
}

- (void)selectItem:(id)item
{
    OESidebarOutlineView *sidebarView = (OESidebarOutlineView*)[self view];

    if(![item isSelectableInSidebar]) return;

    NSInteger index = [sidebarView rowForItem:item];
    if(index == -1) return;

    if([sidebarView selectedRow] != index)
    {
        [sidebarView selectRowIndexes:[NSIndexSet indexSetWithIndex:index] byExtendingSelection:NO];
        [self outlineViewSelectionDidChange:[NSNotification notificationWithName:NSOutlineViewSelectionDidChangeNotification object:self]];
    }
}

- (void)startEditingItem:(id)item
{
    OESidebarOutlineView *sidebarView = (OESidebarOutlineView*)[self view];
    if(![item isEditableInSidebar]) return;

    NSInteger index = [sidebarView rowForItem:item];
    if(index == -1) return;

    NSEvent *event = [[NSEvent alloc] init];
    [sidebarView editColumn:0 row:index withEvent:event select:YES];
}

- (void)expandCollections:(id)sender
{
    OESidebarOutlineView *sidebarView = (OESidebarOutlineView*)[self view];
    [sidebarView expandItem:[self.groups objectAtIndex:1]];
}

- (id<OESidebarItem>)selectedSidebarItem
{
    id<OESidebarItem> item = [[self view] itemAtRow:[[self view] selectedRow]];
    NSAssert(item==nil || [item conformsToProtocol:@protocol(OESidebarItem)], @"All sidebar items must conform to OESidebarItem");

    return item;
}
#pragma mark -
- (void)changeDefaultCore:(id)sender
{
    NSDictionary *data = [sender representedObject];
    NSString *systemIdentifier = data[@"system"];
    NSString *coreIdentifier   = data[@"core"];

    NSString *defaultCoreKey = [NSString stringWithFormat:@"defaultCore.%@", systemIdentifier];
    [[NSUserDefaults standardUserDefaults] setObject:coreIdentifier forKey:defaultCoreKey];
}

#pragma mark - Notifications

- (void)reloadDataAndPreserveSelection
{
    id previousSelectedItem = self.selectedSidebarItem;
    NSInteger previousSelectedRow = self.view.selectedRow;

    [self reloadData];

    if (!previousSelectedItem)
        return;

    NSInteger rowToSelect = previousSelectedRow;
    NSInteger reloadedRowForPreviousSelectedItem = [self.view rowForItem:previousSelectedItem];

    // The previously selected item may have been disabled/removed, so we should select another item...
    if (reloadedRowForPreviousSelectedItem == -1) {
        
        // Try to select the previously selected row or a row before it...
        rowToSelect = previousSelectedRow;
        while (rowToSelect > 0 && ![self outlineView:self.view shouldSelectItem:[self.view itemAtRow:rowToSelect]]) {
            rowToSelect--;
        }

        // If we can't select the previously selected row or a row before it, try to select a row after it
        if (![self outlineView:self.view shouldSelectItem:[self.view itemAtRow:rowToSelect]]) {
            rowToSelect = previousSelectedRow;
            
            while (rowToSelect < [self.view numberOfRows] && ![self outlineView:self.view shouldSelectItem:[self.view itemAtRow:rowToSelect]]) {
                rowToSelect++;
            }
        }

        NSAssert(rowToSelect > 0 && rowToSelect < self.view.numberOfRows && [self outlineView:self.view shouldSelectItem:[self.view itemAtRow:rowToSelect]],
                 @"Tried to select a sidebar item but couldn't find any");
        
    } else if (reloadedRowForPreviousSelectedItem != previousSelectedRow) {
        // ...or the previously selected item may have changed to another row, so we need to select that row
        rowToSelect = reloadedRowForPreviousSelectedItem;
    }
    
    if (rowToSelect != NSNotFound) {
        [self.view selectRowIndexes:[NSIndexSet indexSetWithIndex:rowToSelect] byExtendingSelection:NO];
        [self outlineViewSelectionDidChange:[NSNotification notificationWithName:NSOutlineViewSelectionDidChangeNotification object:self]];
    }
}

- (void)importingChanged
{
    [self reloadData];
    [self outlineViewSelectionDidChange:[NSNotification notificationWithName:NSOutlineViewSelectionDidChangeNotification object:self]];
}

- (void)libraryLocationDidChange:(NSNotification*)notification
{
    [self reloadData];
}

#pragma mark -
#pragma mark Drag and Drop
- (BOOL)outlineView:(NSOutlineView *)outlineView acceptDrop:(id < NSDraggingInfo >)info item:(id)item childIndex:(NSInteger)index
{
    NSPasteboard *pboard = [info draggingPasteboard];

    OEDBCollection *collection = nil;
    if([item isKindOfClass:[OEDBCollection class]])
    {
        collection = item;
    }
    else if(![item isKindOfClass:[OESidebarGroupItem class]])
    {
        NSString *name = nil;
        if([[pboard types] containsObject:OEPasteboardTypeGame])
        {
            NSArray *games = [pboard readObjectsForClasses:@[[OEDBGame class]] options:nil];
            if([games count] == 1) name = [[games lastObject] gameTitle];
        }
        else
        {
            NSArray *games = [pboard readObjectsForClasses:@[[NSURL class]] options:nil];
            if([games count] == 1){
                name = [[[[[games lastObject] absoluteString] lastPathComponent] stringByDeletingPathExtension] stringByRemovingPercentEncoding];
            }
        }
        collection = [[self database] addNewCollection:name];
        [self reloadData];
        NSInteger index = [outlineView rowForItem:collection];
        if(index != NSNotFound)
        {
            [outlineView selectRowIndexes:[NSIndexSet indexSetWithIndex:index] byExtendingSelection:NO];
            [[NSNotificationCenter defaultCenter] postNotificationName:OESidebarSelectionDidChangeNotificationName object:self userInfo:nil];
        }
    }

    if([[pboard types] containsObject:OEPasteboardTypeGame])
    {
        if(!collection) return YES;
        // just add to collection
        NSArray *games = [pboard readObjectsForClasses:@[[OEDBGame class]] options:nil];
        [[collection mutableGames] addObjectsFromArray:games];
        [collection save];
    }
    else
    {
        // import and add to collection
        NSArray *files = [pboard readObjectsForClasses:@[[NSURL class]] options:nil];
        NSManagedObjectID *collectionID = [collection permanentID];
        OEROMImporter *importer = [[self database] importer];
        [importer importItemsAtURLs:files intoCollectionWithID:collectionID];
    }

    return YES;
}

- (NSDragOperation)outlineView:(NSOutlineView *)outlineView validateDrop:(id < NSDraggingInfo >)info proposedItem:(id)item proposedChildIndex:(NSInteger)index
{
    NSPasteboard *pboard = [info draggingPasteboard];
    if(![[pboard types] containsObject:OEPasteboardTypeGame] && ![[pboard types] containsObject:NSFilenamesPboardType])
        return NSDragOperationNone;

    // Ignore anything that is between two rows
    if(index != NSOutlineViewDropOnItemIndex) return NSDragOperationNone;

    // Allow drop on systems group, ignoring which system exactly is highlighted
    if(item == [[self groups] objectAtIndex:0] || [item isKindOfClass:[OEDBSystem class]])
    {
        [outlineView setDropItem:[[self groups] objectAtIndex:0] dropChildIndex:NSOutlineViewDropOnItemIndex];
        return NSDragOperationCopy;
    }

    // Allow drop on normal collections
    if([item isMemberOfClass:[OEDBCollection class]])
    {
        if(index != NSOutlineViewDropOnItemIndex)
        {
            index = index != 0 ? index-1:index;
            [outlineView setDropItem:[self outlineView:outlineView child:index ofItem:item] dropChildIndex:NSOutlineViewDropOnItemIndex];
        }
        return NSDragOperationCopy;
    }

    // Everything else goes to whole view
    [outlineView setDropItem:nil dropChildIndex:NSOutlineViewDropOnItemIndex];
    return NSDragOperationCopy;
}

#pragma mark - NSOutlineView Delegate

- (void)outlineViewSelectionDidChange:(NSNotification *)notification
{
    if (!self.database)
        return;

    if (![self outlineView:self.view shouldSelectItem:self.selectedSidebarItem]) {
        
        DLog(@"invalid selection");
        
        NSInteger row = self.view.selectedRow;
        if (row == NSNotFound)
            row = 1;
        else
            row ++;
        
        [self.view selectRowIndexes:[NSIndexSet indexSetWithIndex:row] byExtendingSelection:NO];
    }

    id <OESidebarItem> selectedItem = self.selectedSidebarItem;
    if ([selectedItem conformsToProtocol:@protocol(OESidebarItem)]) {
        [[NSUserDefaults standardUserDefaults] setValue:selectedItem.sidebarID forKey:OELastSidebarSelectionKey];
    }

    [[NSNotificationCenter defaultCenter] postNotificationName:OESidebarSelectionDidChangeNotificationName object:self userInfo:nil];
}

- (void)outlineViewSelectionIsChanging:(NSNotification *)notification
{}

- (BOOL)outlineView:(NSOutlineView *)outlineView shouldSelectItem:(id)item
{
    id <OESidebarItem> sidebarItem = item;
    const BOOL gameScannerIssuesVisible = self.gameScannerViewController.view.superview != nil;
    
    return self.database != nil &&
           !sidebarItem.isGroupHeaderInSidebar &&
           sidebarItem.isSelectableInSidebar &&
           !gameScannerIssuesVisible;
}

- (BOOL)outlineView:(NSOutlineView *)outlineView shouldCollapseItem:(id)item
{
    return NO;
}

- (BOOL)outlineView:(NSOutlineView *)outlineView shouldExpandItem:(id)item
{
    return YES;
}

#pragma mark - NSOutlineView Type Select

- (NSString*)outlineView:(NSOutlineView *)outlineView typeSelectStringForTableColumn:(NSTableColumn *)tableColumn item:(id)item
{
    return [item isSelectableInSidebar] ? [item sidebarName] : @"";
}

#pragma mark - NSOutlineView DataSource

- (id)outlineView:(NSOutlineView *)outlineView child:(NSInteger)index ofItem:(id)item
{
    if (item) {
        NSString *autosaveName = [item isKindOfClass:[OESidebarGroupItem class]] ? ((OESidebarGroupItem *)item).autosaveName : nil;
        if ([autosaveName isEqualToString:OESidebarGroupConsolesAutosaveName])
            return self.systems[index];
        else
            return self.collections[index];
    } else {
        return self.groups[index];
    }
}

- (BOOL)outlineView:(NSOutlineView *)outlineView isItemExpandable:(id)item
{
    return ((id <OESidebarItem>)item).isGroupHeaderInSidebar;
}

- (NSInteger)outlineView:(NSOutlineView *)outlineView numberOfChildrenOfItem:(id)item
{
    if (!item)
        return self.groups.count;

    if (!self.database)
        return 0;

    NSString *autosaveName = [item isKindOfClass:[OESidebarGroupItem class]] ? ((OESidebarGroupItem *)item).autosaveName : nil;
    if ([autosaveName isEqualToString:OESidebarGroupConsolesAutosaveName])
        return self.systems.count;
    else if ([autosaveName isEqualToString:OESidebarGroupCollectionsAutosaveName])
        return self.collections.count;

    return 0;
}

- (id)outlineView:(NSOutlineView *)outlineView objectValueForTableColumn:(NSTableColumn *)tableColumn byItem:(id)item
{
    return ((id <OESidebarItem>)item).sidebarName;
}

- (void)outlineView:(NSOutlineView *)outlineView setObjectValue:(id)object forTableColumn:(NSTableColumn *)tableColumn byItem:(id)item
{
    self.editingItem = nil;

    if ([[object stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]] isNotEqualTo:@""]) {
        
        ((id <OESidebarItem>)item).sidebarName = object;
        [(OEDBCollection *)item save];
        [self reloadData];

        NSInteger row = [outlineView rowForItem:item];
        [outlineView selectRowIndexes:[NSIndexSet indexSetWithIndex:row] byExtendingSelection:NO];
    }
}

- (BOOL)outlineView:(NSOutlineView *)outlineView shouldEditTableColumn:(NSTableColumn *)tableColumn item:(id)item
{
    id <OESidebarItem> sidebarItem = item;
    
    BOOL result = sidebarItem.isEditableInSidebar;
    if (result)
        self.editingItem = sidebarItem;
    else
        self.editingItem = nil;

    return result;
}

- (CGFloat)outlineView:(NSOutlineView *)outlineView heightOfRowByItem:(id)item
{
    id <OESidebarItem> sidebarItem = item;
    return sidebarItem.isGroupHeaderInSidebar ? 26.0 : 20.0;
}

- (void)outlineView:(NSOutlineView *)olv willDisplayCell:(NSCell*)cell forTableColumn:(NSTableColumn *)tableColumn item:(id)item
{
    id <OESidebarItem> sidebarItem = item;

    if ([cell isKindOfClass:[OESidebarCell class]]) {
        
        OESidebarCell *sidebarCell = (OESidebarCell *)cell;
        sidebarCell.image = sidebarItem.sidebarIcon;
        NSString *badge = [sidebarItem respondsToSelector:@selector(badge)] ? sidebarItem.badge : nil;
        sidebarCell.badge = badge;
        sidebarCell.isGroup = sidebarItem.isGroupHeaderInSidebar;

        if (self.editingItem == nil)
            sidebarCell.isEditing = NO;
        if (self.view.isDrawingAboveDropHighlight)
            cell.highlighted = NO;
    }
}

- (void)setEditingItem:(id)newEdItem
{
    editingItem = newEdItem;
}

- (void)removeItemAtIndex:(NSUInteger)index
{
    id item = [self.view itemAtRow:index];

    if ([item isEditableInSidebar]) {
        
        NSString *msg = NSLocalizedString(@"Are you sure you want to remove this collection?", @"");
        NSString *confirm = NSLocalizedString(@"Remove", @"");
        NSString *cancel = NSLocalizedString(@"Cancel", @"");

        OEHUDAlert *alert = [OEHUDAlert alertWithMessageText:msg defaultButton:confirm alternateButton:cancel];
        [alert showSuppressionButtonForUDKey:OESuppressRemoveCollectionConfirmationKey];

        if ([alert runModal] == NSAlertFirstButtonReturn) {
            
            [(OEDBCollection *)item delete];
            [(OEDBCollection *)item save];

            // keep selection on last object if the one we removed was last
            if (index == self.view.numberOfRows - 1)
                index --;

            NSIndexSet *selIn = [[NSIndexSet alloc] initWithIndex:index];
            [self.view selectRowIndexes:selIn byExtendingSelection:NO];
            [self reloadData];

            OESidebarOutlineView *sidebarView = (OESidebarOutlineView *)self.view;
            [[NSNotificationCenter defaultCenter] postNotificationName:NSOutlineViewSelectionDidChangeNotification object:sidebarView];
        }
    }
}

- (void)renameItemAtIndex:(NSUInteger)index
{
    id item = [self.view itemAtRow:index];
    [self selectItem:item];
    [self startEditingItem:item];
}

- (void)removeSelectedItemsOfOutlineView:(NSOutlineView *)outlineView
{
    NSIndexSet *indexes = outlineView.selectedRowIndexes;
    NSUInteger index = indexes.firstIndex;

    [self removeItemAtIndex:index];
}

- (void)removeItemForMenuItem:(NSMenuItem *)menuItem
{
    [self removeItemAtIndex:menuItem.tag];
}

- (void)renameItemForMenuItem:(NSMenuItem *)menuItem
{
    [self renameItemAtIndex:menuItem.tag];
}

#pragma mark -

- (void)controlTextDidBeginEditing:(NSNotification *)aNotification
{
}

- (void)controlTextDidEndEditing:(NSNotification *)aNotification
{
    self.editingItem = nil;
}

@end
