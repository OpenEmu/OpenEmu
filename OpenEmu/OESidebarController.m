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
#import "OECollectionViewItemProtocol.h"

#import "OEHUDAlert.h"

#import "NSImage+OEDrawingAdditions.h"
#import "NSArray+OEAdditions.h"

extern NSString *const OELastCollectionSelectedKey;
NSString *const OESuppressRemoveCollectionConfirmationKey = @"removeCollectionWithoutConfirmation";
extern NSString * const OEDBSystemsChangedNotificationName;

NSString * const OESidebarSelectionDidChangeNotificationName = @"OESidebarSelectionDidChange";

NSString * const OESidebarGroupConsolesAutosaveName    = @"sidebarConsolesItem";
NSString * const OESidebarGroupCollectionsAutosaveName = @"sidebarCollectionsItem";
@interface OESidebarController ()
{
    NSArray *groups;
    NSArray *systems;
    NSArray *collections;
    id editingItem;
}

- (void)OE_setupDrop;

@end

@implementation OESidebarController
@synthesize groups, database=_database, editingItem;
@synthesize systems, collections;
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
    
    [[NSUserDefaults standardUserDefaults] registerDefaults:(@{
                                                             OESidebarGroupConsolesAutosaveName    : @YES,
                                                             OESidebarGroupCollectionsAutosaveName : @YES,
                                                             })];
}

- (void)awakeFromNib
{
    self.groups = [NSArray arrayWithObjects:
                   [OESidebarGroupItem groupItemWithName:NSLocalizedString(@"CONSOLES", @"") andAutosaveName:OESidebarGroupConsolesAutosaveName],
                   [OESidebarGroupItem groupItemWithName:NSLocalizedString(@"COLLECTIONS", @"") andAutosaveName:OESidebarGroupCollectionsAutosaveName],
                   nil];
    
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
    [sidebarView registerForDraggedTypes:[NSArray arrayWithObjects:@"org.openEmu.rom", NSFilenamesPboardType, nil]];
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
    
    [self OE_setupDrop];

    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(systemsChanged) name:OEDBSystemsChangedNotificationName object:nil];
}
 

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void)OE_finishAwakingUp
{
    
    NSUserDefaults *standardUserDefaults = [NSUserDefaults standardUserDefaults];
    
    // Restore last selected collection item
    id itemID = [standardUserDefaults valueForKey:OELastCollectionSelectedKey];
    DLog(@"%@", itemID);
    
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
    [self addCollection:([NSEvent modifierFlags] & NSAlternateKeyMask) != 0];
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

- (void)reloadData
{
    if(![self database]) return;
    
    self.systems     = [OEDBSystem enabledSystemsInDatabase:[self database]] ? : [NSArray array];
    self.collections = [[self database] collections]    ? : [NSArray array];
        
    OESidebarOutlineView *sidebarView = (OESidebarOutlineView*)[self view];
    [sidebarView reloadData];
}

- (void)selectItem:(id)item
{
    OESidebarOutlineView *sidebarView = (OESidebarOutlineView*)[self view];
    
    if(![item isSelectableInSdebar]) return;

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
    if(![item isEditableInSdebar]) return;
    
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
- (void)systemsChanged
{
    [self reloadData];
    [self outlineViewSelectionDidChange:nil];
}

- (void)importingChanged
{
    [self reloadData];
    [self outlineViewSelectionDidChange:nil];
}

#pragma mark -
#pragma mark Drag and Drop
- (void)OE_setupDrop
{
    NSArray *acceptedTypes = [NSArray arrayWithObjects:NSFilenamesPboardType, OEPasteboardTypeGame, nil];
    [[self view] registerForDraggedTypes:acceptedTypes];
    [(OESidebarOutlineView*)[self view] setDragDelegate:self];
}

- (NSDragOperation)draggingEntered:(id<NSDraggingInfo>)sender
{
    NSPasteboard *pboard = [sender draggingPasteboard];
    return [[pboard types] containsObject:OEPasteboardTypeGame] || NSFilenamesPboardType?NSDragOperationCopy:NSDragOperationNone;
}

- (NSDragOperation)draggingUpdated:(id<NSDraggingInfo>)sender
{
    return [self draggingEntered:sender];
}

- (void)draggingEnded:(id<NSDraggingInfo>)sender
{
}

- (void)draggingExited:(id<NSDraggingInfo>)sender
{
}

- (BOOL)prepareForDragOperation:(id<NSDraggingInfo>)sender
{
    return YES;
}

- (BOOL)performDragOperation:(id<NSDraggingInfo>)sender
{
    return NO;
}

- (void)concludeDragOperation:(id<NSDraggingInfo>)sender
{
}

#pragma mark -
#pragma mark NSOutlineView Delegate
- (void)outlineViewSelectionDidChange:(NSNotification *)notification
{
    [[NSNotificationCenter defaultCenter] postNotificationName:OESidebarSelectionDidChangeNotificationName object:self userInfo:[NSDictionary dictionary]];

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
}

- (void)outlineViewSelectionIsChanging:(NSNotification *)notification
{
}

- (BOOL)outlineView:(NSOutlineView *)outlineView shouldSelectItem:(id)item
{
    return [self database]!=nil && ![item isGroupHeaderInSdebar] && [item isSelectableInSdebar];
}

- (BOOL)outlineView:(NSOutlineView *)outlineView shouldCollapseItem:(id)item
{
    for(id aGroup in self.groups)
        if(aGroup != item && [outlineView isItemExpanded:aGroup]) return YES;

    return NO;
}

#pragma mark -
#pragma mark NSOutlineView DataSource
- (id)outlineView:(NSOutlineView *)outlineView child:(NSInteger)index ofItem:(id)item
{
    if(item == nil)
        return [self.groups objectAtIndex:index];
    
    if(item == [self.groups objectAtIndex:0] )
        return [self.systems objectAtIndex:index];
    
    if(item == [self.groups objectAtIndex:1])
        return [self.collections objectAtIndex:index];
    
    return nil;
}

- (BOOL)outlineView:(NSOutlineView *)outlineView isItemExpandable:(id)item
{
    return [(id<OESidebarItem>)item isGroupHeaderInSdebar];
}

- (NSInteger)outlineView:(NSOutlineView *)outlineView numberOfChildrenOfItem:(id)item
{
    if(item == nil) return [self.groups count];
    
    if(![self database])
    {
        return 0;
    }
    
    if(item == [self.groups objectAtIndex:0])
    {
        int count = [self.systems count];
        return count;
    }
    
    if(item == [self.groups objectAtIndex:1])
        return [self.collections count];
    
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
    BOOL result = [item isEditableInSdebar];
    if(result)
        self.editingItem = item;
    else
        self.editingItem = nil;
    
    return result;
}

- (CGFloat)outlineView:(NSOutlineView *)outlineView heightOfRowByItem:(id)item
{
    if([item isGroupHeaderInSdebar])
        return 26.0;

    return 20.0;
}

- (void)outlineView:(NSOutlineView *)olv willDisplayCell:(NSCell*)cell forTableColumn:(NSTableColumn *)tableColumn item:(id)item 
{
    if([cell isKindOfClass:[OESidebarCell class]])
    {
        [(OESidebarCell*)cell setImage:[item sidebarIcon]];
        [(OESidebarCell*)cell setIsGroup:[item isGroupHeaderInSdebar]];
        
        if(self.editingItem == nil)
            [(OESidebarCell*)cell setIsEditing:NO];
    }
}

- (void)setEditingItem:(id)newEdItem
{    
    editingItem = newEdItem;
}

- (void)removeSelectedItemsOfOutlineView:(NSOutlineView*)outlineView
{
    NSIndexSet *indexes = [outlineView selectedRowIndexes];
    NSUInteger index = [indexes firstIndex];
    
    id item = [outlineView itemAtRow:index];
    BOOL removeItem = NO;
    
    if([item isEditableInSdebar])
    {   
        NSString *msg = NSLocalizedString(@"Do you really want to remove this collection", @"");
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
        if(index == [outlineView numberOfRows]-1)
            index --;
        
        NSIndexSet *selIn = [[NSIndexSet alloc] initWithIndex:index];
        [outlineView selectRowIndexes:selIn byExtendingSelection:NO];
        [self reloadData];
    }
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
