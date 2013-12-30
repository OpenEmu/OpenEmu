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

#import "OESidebarOutlineView.h"
#import "OEDBGame.h"
#import "OEDBAllGamesCollection.h"
#import "OEDBSystem.h"
#import "OEDBSmartCollection.h"
#import "OECollectionViewItemProtocol.h"

#import "OEStorageDeviceManager.h"

#import "OEHUDAlert.h"

#import "NSImage+OEDrawingAdditions.h"
#import "NSArray+OEAdditions.h"

extern NSString *const OELastCollectionSelectedKey;
NSString *const OESuppressRemoveCollectionConfirmationKey = @"removeCollectionWithoutConfirmation";
extern NSString * const OEDBSystemsDidChangeNotification;

NSString * const OESidebarSelectionDidChangeNotificationName = @"OESidebarSelectionDidChange";

NSString * const OESidebarGroupConsolesAutosaveName    = @"sidebarConsolesItem";
NSString * const OESidebarGroupMediaAutosaveName       = @"sidebarMediaItem";
NSString * const OESidebarGroupDevicesAutosaveName     = @"sidebarDevicesItem";
NSString * const OESidebarGroupCollectionsAutosaveName = @"sidebarCollectionsItem";

NSString * const OESidebarMinWidth = @"sidebarMinWidth";
NSString * const OESidebarMaxWidth = @"sidebarMaxWidth";
NSString * const OEMainViewMinWidth = @"mainViewMinWidth";

@interface OESidebarController ()
{
    NSArray *groups;
    NSArray *systems;
    NSArray *collections;
    NSArray *media;
    id editingItem;
}

@end

@implementation OESidebarController
@synthesize groups, database=_database, editingItem;
@synthesize systems, collections, media;
@dynamic view;

+ (void)initialize
{
    // Make sure not to reinitialize for subclassed objects
    if (self != [OESidebarController class])
        return;

    // Collection Icons for sidebar
    NSImage *image = [NSImage imageNamed:@"collections"];

    [image setName:@"collections_simple" forSubimageInRect:NSMakeRect(0, 0, 16, 16)];
    [image setName:@"collections_smart" forSubimageInRect:NSMakeRect(16, 0, 16, 16)];
    
    // Media Icons for sidebar
    NSImage *imageMedia = [NSImage imageNamed:@"media"];
    
    [imageMedia setName:@"media_saved_games" forSubimageInRect:NSMakeRect(0, 0, 16, 16)];
    [imageMedia setName:@"media_screenshots" forSubimageInRect:NSMakeRect(16, 0, 16, 16)];
    [imageMedia setName:@"media_video" forSubimageInRect:NSMakeRect(32, 0, 16, 16)];

    [[NSUserDefaults standardUserDefaults] registerDefaults:(@{
                                                             OESidebarGroupConsolesAutosaveName    : @YES,
                                                             OESidebarGroupCollectionsAutosaveName : @YES,
                                                             })];
}

- (void)awakeFromNib
{
    self.groups = @[
                    [OESidebarGroupItem groupItemWithName:NSLocalizedString(@"CONSOLES", @"") autosaveName:OESidebarGroupConsolesAutosaveName],
                    //[OESidebarGroupItem groupItemWithName:NSLocalizedString(@"MEDIA", @"") autosaveName:OESidebarGroupMediaAutosaveName],
                    [OESidebarGroupItem groupItemWithName:NSLocalizedString(@"DEVICES", @"") autosaveName:OESidebarGroupDevicesAutosaveName],
                    [OESidebarGroupItem groupItemWithName:NSLocalizedString(@"COLLECTIONS", @"") autosaveName:OESidebarGroupCollectionsAutosaveName]
                    ];

    OESidebarOutlineView *sidebarView = (OESidebarOutlineView*)[self view];

    [sidebarView setHeaderView:nil];

    OESidebarCell *cell = [[OESidebarCell alloc] init];
    [cell setEditable:YES];

    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(controlTextDidEndEditing:) name:NSControlTextDidEndEditingNotification object:cell];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(controlTextDidBeginEditing:) name:NSControlTextDidBeginEditingNotification object:cell];

    [[[sidebarView tableColumns] objectAtIndex:0] setDataCell:cell];
    [sidebarView setIndentationPerLevel:7];
    [sidebarView setAutosaveName:@"sidebarView"];
    [sidebarView setAutoresizesOutlineColumn:NO];
    [sidebarView registerForDraggedTypes:[NSArray arrayWithObjects:OEPasteboardTypeGame, NSFilenamesPboardType, nil]];
    [sidebarView setDelegate:self];
    [sidebarView setDataSource:self];
    for(OESidebarGroupItem *groupItem in [self groups])
    {
        if([[NSUserDefaults standardUserDefaults] boolForKey:[groupItem autosaveName]])
            [sidebarView expandItem:groupItem];
        else
            [sidebarView collapseItem:groupItem];
    }
    [sidebarView setAllowsEmptySelection:NO];

    NSScrollView *enclosingScrollView = [sidebarView enclosingScrollView];
    if(enclosingScrollView != nil)
    {
        [enclosingScrollView setDrawsBackground:NO];
        [sidebarView setBackgroundColor:[NSColor clearColor]];
    }
    else
        [sidebarView setBackgroundColor:[NSColor colorWithDeviceWhite:0.19 alpha:1.0]];

    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(reloadDataAndPreserveSelection) name:OEDBSystemsDidChangeNotification object:nil];

    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(deviceDidAppear:) name:OEStorageDeviceDidAppearNotificationName object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(deviceDidDisappear:) name:OEStorageDeviceDidDisappearNotificationName object:nil];
}

- (void)deviceDidAppear:(id)notification
{
    [self reloadDataAndPreserveSelection];
    id devicesItem = [self.groups firstObjectMatchingBlock:^BOOL(id obj) {
        return [[obj autosaveName] isEqualTo:OESidebarGroupDevicesAutosaveName];
    }];
    [[self view] expandItem:devicesItem];
}

- (void)deviceDidDisappear:(id)notification
{
    [self reloadDataAndPreserveSelection];
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void)setDatabase:(OELibraryDatabase *)database
{
    // get last selected collection item
    NSUserDefaults *standardUserDefaults = [NSUserDefaults standardUserDefaults];
    id itemID = [standardUserDefaults valueForKey:OELastCollectionSelectedKey];

    // set database
    _database = database;
    [self reloadData];

    // Look for the collection item
    id collectionItem = nil;
    if(itemID != nil && [itemID isKindOfClass:[NSString class]])
    {
        collectionItem = [[self systems] firstObjectMatchingBlock:
                          ^ BOOL (id obj) {
                              return [[obj sidebarID] isEqualTo:itemID];
                          }];
        if(!collectionItem)
            collectionItem = [[self collections] firstObjectMatchingBlock:
                              ^ BOOL (id obj) {
                                  return [[obj sidebarID] isEqualTo:itemID];
                              }];
    }

    // Select the found collection item, or select the first item by default
    if(collectionItem != nil) [self selectItem:collectionItem];
}
#pragma mark -
#pragma mark Public
- (void)setEnabled:(BOOL)enabled
{
    OESidebarOutlineView *sidebarView = (OESidebarOutlineView*)[self view];
    [sidebarView setEnabled:enabled];
}

- (IBAction)addCollectionAction:(id)sender
{
    [self addCollection:NO];
}

- (id)addCollection:(BOOL)isSmart
{
    id item = isSmart ? [[self database] addNewSmartCollection:nil] : [[self database] addNewCollection:nil];

    [self reloadData];
    [self expandCollections:self];
    [self selectItem:item];
    [self startEditingItem:item];

    return item;
}

- (id)duplicateCollection:(id)originalCollection
{
    id duplicateCollection = [[self database] addNewCollection:[NSString stringWithFormat:NSLocalizedString(@"%@ copy", @"File name format for copies"), [originalCollection valueForKey:@"name"]]];

    [[duplicateCollection mutableGames] setSet:[originalCollection games]];
    [[duplicateCollection libraryDatabase] save:nil];

    [self reloadData];
    [self expandCollections:self];

    return duplicateCollection;
}

- (void)reloadData
{
    if(![self database]) return;

    self.systems     = [OEDBSystem enabledSystemsInDatabase:[self database]] ? : [NSArray array];
    self.collections = [[self database] collections]    ? : [NSArray array];
    //self.media = [[self database] media]    ? : [NSArray array];

    OESidebarOutlineView *sidebarView = (OESidebarOutlineView*)[self view];
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
        [self outlineViewSelectionDidChange:nil];
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

    NSAssert([item conformsToProtocol:@protocol(OESidebarItem)], @"All sidebar items must conform to OESidebarItem");

    return item;
}

#pragma mark -
#pragma mark Notifications
- (void)reloadDataAndPreserveSelection
{
    id        previousSelectedItem = [self selectedSidebarItem];
    NSInteger previousSelectedRow  = [[self view] selectedRow];

    [self reloadData];

    if(!previousSelectedItem) return;

    NSInteger rowToSelect = previousSelectedRow;
    NSInteger reloadedRowForPreviousSelectedItem = [[self view] rowForItem:previousSelectedItem];

    // The previously selected item may have been disabled/removed, so we should select another item...
    if(reloadedRowForPreviousSelectedItem == -1)
    {
        // Try to select the previously selected row or a row before it...
        rowToSelect = previousSelectedRow;
        while(rowToSelect > 0 && ![self outlineView:[self view] shouldSelectItem:[[self view] itemAtRow:rowToSelect]])
            rowToSelect--;

        // If we can't select the previously selected row or a row before it, try to select a row after it
        if(![self outlineView:[self view] shouldSelectItem:[[self view] itemAtRow:rowToSelect]])
        {
            rowToSelect = previousSelectedRow;
            while(rowToSelect < [[self view] numberOfRows] && ![self outlineView:[self view] shouldSelectItem:[[self view] itemAtRow:rowToSelect]])
                rowToSelect++;
        }

        NSAssert(rowToSelect > 0 && rowToSelect < [[self view] numberOfRows] && [self outlineView:[self view] shouldSelectItem:[[self view] itemAtRow:rowToSelect]],
                 @"Tried to select a sidebar item but couldn't find any");
    }
    // ...or the previously selected item may have changed to another row, so we need to select that row
    else if(reloadedRowForPreviousSelectedItem != previousSelectedRow)
        rowToSelect = reloadedRowForPreviousSelectedItem;

    if(rowToSelect != previousSelectedRow && rowToSelect != NSNotFound)
    {
        [[self view] selectRowIndexes:[NSIndexSet indexSetWithIndex:rowToSelect] byExtendingSelection:NO];
        [self outlineViewSelectionDidChange:nil];
    }
}

- (void)importingChanged
{
    [self reloadData];
    [self outlineViewSelectionDidChange:nil];
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
                name = [[[[[games lastObject] absoluteString] lastPathComponent] stringByDeletingPathExtension] stringByReplacingPercentEscapesUsingEncoding:NSUTF8StringEncoding];
            }
        }
        collection = [[OELibraryDatabase defaultDatabase] addNewCollection:name];
        [[collection libraryDatabase] save:nil];
        [self reloadData];
        NSInteger index = [outlineView rowForItem:collection];
        if(index != -1)
        {
            [outlineView selectRowIndexes:[NSIndexSet indexSetWithIndex:index] byExtendingSelection:NO];
        }
    }

    if([[pboard types] containsObject:OEPasteboardTypeGame])
    {
        if(!collection) return YES;
        // just add to collection
        NSArray *games = [pboard readObjectsForClasses:@[[OEDBGame class]] options:nil];
        [[collection mutableGames] addObjectsFromArray:games];
        [[collection libraryDatabase] save:nil];
    }
    else
    {
        // import and add to collection
        NSArray *files = [pboard readObjectsForClasses:@[[NSURL class]] options:nil];
        NSURL *collectionID = [[collection objectID] URIRepresentation];
        OEROMImporter *importer = [[OELibraryDatabase defaultDatabase] importer];
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
    if([item isKindOfClass:[OEDBCollection class]])
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
#pragma mark -
#pragma mark NSOutlineView Delegate
- (void)outlineViewSelectionDidChange:(NSNotification *)notification
{
    if(![self database]) return;

    if(![self outlineView:[self view] shouldSelectItem:[self selectedSidebarItem]])
    {
        DLog(@"invalid selection");
        NSInteger row = [[self view] selectedRow];
        if(row == NSNotFound) row = 1;
        else row ++;
        [[self view] selectRowIndexes:[NSIndexSet indexSetWithIndex:row] byExtendingSelection:NO];
    }

    id<OESidebarItem> selectedItem = [self selectedSidebarItem];
    if([selectedItem conformsToProtocol:@protocol(OECollectionViewItemProtocol)])
        [[NSUserDefaults standardUserDefaults] setValue:[selectedItem sidebarID] forKey:OELastCollectionSelectedKey];

    [[NSNotificationCenter defaultCenter] postNotificationName:OESidebarSelectionDidChangeNotificationName object:self userInfo:nil];
}

- (void)outlineViewSelectionIsChanging:(NSNotification *)notification
{
}

- (BOOL)outlineView:(NSOutlineView *)outlineView shouldSelectItem:(id)item
{
    return [self database]!=nil && ![item isGroupHeaderInSidebar] && [item isSelectableInSidebar];
}

- (BOOL)outlineView:(NSOutlineView *)outlineView shouldCollapseItem:(id)item
{
    for(id aGroup in self.groups)
        if(aGroup != item && [outlineView isItemExpanded:aGroup])
        {
            if([item isKindOfClass:[OESidebarGroupItem class]])
            {
                [[NSUserDefaults standardUserDefaults] setBool:NO forKey:[item autosaveName]];
            }
            return YES;
        }
    return NO;
}

- (BOOL)outlineView:(NSOutlineView *)outlineView shouldExpandItem:(id)item
{
    if([item isKindOfClass:[OESidebarGroupItem class]])
    {
        [[NSUserDefaults standardUserDefaults] setBool:YES forKey:[item autosaveName]];
    }
    return YES;
}


#pragma mark - NSOutlineView Type Select
- (NSString*)outlineView:(NSOutlineView *)outlineView typeSelectStringForTableColumn:(NSTableColumn *)tableColumn item:(id)item
{
    return [item isSelectableInSidebar] ? [item sidebarName] : @"";
}

#pragma mark -
#pragma mark NSOutlineView DataSource
- (id)outlineView:(NSOutlineView *)outlineView child:(NSInteger)index ofItem:(id)item
{
    if(item == nil)
    {
        if(index!=0 && [[[OEStorageDeviceManager sharedStorageDeviceManager] devices] count] == 0)
            index += 1;

        return [[self groups] objectAtIndex:index];
    }

    NSString *autosaveName = [item isKindOfClass:[OESidebarGroupItem class]]?[item autosaveName]:nil;
    if([autosaveName isEqualToString:OESidebarGroupConsolesAutosaveName])
        return [[self systems] objectAtIndex:index];
    //else if([autosaveName isEqualToString:OESidebarGroupMediaAutosaveName])
        //return [[self media] objectAtIndex:index];
    else if([autosaveName isEqualToString:OESidebarGroupDevicesAutosaveName])
        return [[[OEStorageDeviceManager sharedStorageDeviceManager] devices] objectAtIndex:index];
    else if([autosaveName isEqualToString:OESidebarGroupCollectionsAutosaveName])
        return [[self collections] objectAtIndex:index];

    return nil;
}

- (BOOL)outlineView:(NSOutlineView *)outlineView isItemExpandable:(id)item
{
    return [(id<OESidebarItem>)item isGroupHeaderInSidebar];
}

- (NSInteger)outlineView:(NSOutlineView *)outlineView numberOfChildrenOfItem:(id)item
{
    if(item == nil)
    {
        if([[[OEStorageDeviceManager sharedStorageDeviceManager] devices] count] == 0)
            return [[self groups] count] -1;
        else
            return [[self groups] count];
    }

    if(![self database])
    {
        return 0;
    }

    NSString *autosaveName = [item isKindOfClass:[OESidebarGroupItem class]]?[item autosaveName]:nil;
    if([autosaveName isEqualToString:OESidebarGroupConsolesAutosaveName])
        return [[self systems] count];
    //else if([autosaveName isEqualToString:OESidebarGroupMediaAutosaveName])
        //return [[self media] count];
    else if([autosaveName isEqualToString:OESidebarGroupDevicesAutosaveName])
        return [[[OEStorageDeviceManager sharedStorageDeviceManager] devices] count];
    else if([autosaveName isEqualToString:OESidebarGroupCollectionsAutosaveName])
        return [[self collections] count];

    return 0;
}

- (id)outlineView:(NSOutlineView *)outlineView objectValueForTableColumn:(NSTableColumn *)tableColumn byItem:(id)item
{
    return [item sidebarName];
}

- (void)outlineView:(NSOutlineView *)outlineView setObjectValue:(id)object forTableColumn:(NSTableColumn *)tableColumn byItem:(id)item
{
    self.editingItem = nil;

    if([[object stringByTrimmingCharactersInSet:[NSCharacterSet characterSetWithCharactersInString:@" "]] isNotEqualTo:@""])
    {
        [item setSidebarName:object];
        [self reloadData];

        NSInteger row = [outlineView rowForItem:item];
        [outlineView selectRowIndexes:[NSIndexSet indexSetWithIndex:row] byExtendingSelection:NO];
    }
}

- (BOOL)outlineView:(NSOutlineView *)outlineView shouldEditTableColumn:(NSTableColumn *)tableColumn item:(id)item
{
    BOOL result = [item isEditableInSidebar];
    if(result)
        self.editingItem = item;
    else
        self.editingItem = nil;

    return result;
}

- (CGFloat)outlineView:(NSOutlineView *)outlineView heightOfRowByItem:(id)item
{
    if([item isGroupHeaderInSidebar])
        return 26.0;

    return 20.0;
}

- (void)outlineView:(NSOutlineView *)olv willDisplayCell:(NSCell*)cell forTableColumn:(NSTableColumn *)tableColumn item:(id)item
{
    if([cell isKindOfClass:[OESidebarCell class]])
    {
        [(OESidebarCell*)cell setImage:[item sidebarIcon]];
        [(OESidebarCell*)cell setIsGroup:[item isGroupHeaderInSidebar]];

        if(self.editingItem == nil)
            [(OESidebarCell*)cell setIsEditing:NO];
        if(self.view.isDrawingAboveDropHighlight)
            [cell setHighlighted:NO];
    }
}

- (void)setEditingItem:(id)newEdItem
{
    editingItem = newEdItem;
}

- (void)removeItemAtIndex:(NSUInteger)index
{
    id item = [[self view] itemAtRow:index];
    BOOL removeItem = NO;

    if([item isEditableInSidebar] || [item isKindOfClass:[OEDBSmartCollection class]])
    {
        NSString *msg = NSLocalizedString(@"Are you sure want to remove this collection?", @"");
        NSString *confirm = NSLocalizedString(@"Remove", @"");
        NSString *cancel = NSLocalizedString(@"Cancel", @"");

        OEHUDAlert* alert = [OEHUDAlert alertWithMessageText:msg defaultButton:confirm alternateButton:cancel];
        [alert showSuppressionButtonForUDKey:OESuppressRemoveCollectionConfirmationKey];
        removeItem = [alert runModal];
    }

    if(removeItem)
    {
        [[self database] removeCollection:item];

        // keep selection on last object if the one we removed was last
        if(index == [[self view] numberOfRows]-1)
            index --;

        NSIndexSet *selIn = [[NSIndexSet alloc] initWithIndex:index];
        [[self view] selectRowIndexes:selIn byExtendingSelection:NO];
        [self reloadData];
    }
}

- (void)renameItemAtIndex:(NSUInteger)index
{
    id item = [[self view] itemAtRow:index];
    [self selectItem:item];
    [self startEditingItem:item];
}

- (void)removeSelectedItemsOfOutlineView:(NSOutlineView *)outlineView
{
    NSIndexSet *indexes = [outlineView selectedRowIndexes];
    NSUInteger index = [indexes firstIndex];

    [self removeItemAtIndex:index];
}

- (void)removeItemForMenuItem:(NSMenuItem *)menuItem
{
    [self removeItemAtIndex:[menuItem tag]];
}

- (void)renameItemForMenuItem:(NSMenuItem *)menuItem
{
    [self renameItemAtIndex:[menuItem tag]];
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
