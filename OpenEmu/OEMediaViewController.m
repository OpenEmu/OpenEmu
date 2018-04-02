/*
 Copyright (c) 2013, OpenEmu Team

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

#import "OEMediaViewController.h"

#import "OEDBSavedGamesMedia.h"
#import "OEGridMediaItemCell.h"
#import "OEGridMediaGroupItemCell.h"

#import "OELibraryDatabase.h"
#import "OEDBSaveState.h"
#import "OEDBGame.h"
#import "OEDBRom.h"
#import "OEDBSystem+CoreDataProperties.h"
#import "OEDBScreenshot+CoreDataProperties.h"

#import "OEROMImporter.h"

#import "OELibraryController.h"

#import "OEButton.h"
#import "OESearchField.h"

#import "OEGridView.h"

#import "OEHUDAlert+DefaultAlertsAdditions.h"

#import "OpenEmu-Swift.h"

NSString * const OEMediaViewControllerDidSetSelectionIndexesNotification = @"OEMediaViewControllerDidSetSelectionIndexesNotification";

/// Archived URI representations of managed object IDs for selected media.
static NSString * const OESelectedMediaKey = @"_OESelectedMediaKey";

@interface OESavedGamesDataWrapper : NSObject <NSPasteboardWriting>

+ (instancetype)wrapperWithItem:(id)item;
+ (instancetype)wrapperWithState:(OEDBSaveState*)state;
+ (instancetype)wrapperWithScreenshot:(OEDBScreenshot*)screenshot;

@property (strong) OEDBSaveState  *state;
@property (strong) OEDBScreenshot *screenshot;

@end

@interface OEMediaViewController ()

@property (strong) NSArray *groupRanges;
@property (strong) NSArray *items;
@property (strong) NSArray *searchKeys;

@property (readwrite) BOOL saveStateMode;

@property BOOL shouldShowBlankSlate;
@property (strong) NSPredicate *searchPredicate;
@property (copy, nullable) NSString *currentSearchTerm;

@end

@implementation OEMediaViewController

- (instancetype)init
{
    self = [super init];
    if (self)
    {
        _searchPredicate = [NSPredicate predicateWithValue:YES];
        _searchKeys = @[@"rom.game.gameTitle", @"rom.game.name", @"rom.game.system.lastLocalizedName", @"name", @"userDescription"];
    }
    return self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];

    self.gridView.automaticallyMinimizeRowMargin = YES;
    self.gridView.cellClass = [OEGridMediaItemCell class];
}

- (void)viewWillAppear
{
    [super viewWillAppear];
    
    [self _setupToolbar];
    
    OESearchField *searchField = self.libraryController.toolbar.searchField;
    searchField.enabled = YES;
    searchField.stringValue = self.currentSearchTerm ?: @"";
    
    [self restoreSelectionFromDefaults];
}

- (void)viewDidDisappear
{
    [super viewDidDisappear];
    
    // We don't want to clear the search filter if the view is disappearing because of gameplay.
    OEGameDocument *gameDocument = (OEGameDocument *)[[NSDocumentController sharedDocumentController] currentDocument];
    const BOOL playingGame = gameDocument != nil;
    
    if (!playingGame) {
        // Clear any previously applied search filter.
        self.currentSearchTerm = nil;
        _searchPredicate = [NSPredicate predicateWithValue:YES];
        [self reloadData];
    }
}

- (void)restoreSelectionFromDefaults
{
    // Restore media selection.
    NSManagedObjectContext *context = self.libraryController.database.mainThreadContext;
    NSPersistentStoreCoordinator *persistentStoreCoordinator = context.persistentStoreCoordinator;
    NSMutableIndexSet *mediaItemsToSelect = [NSMutableIndexSet indexSet];
    NSString *defaultsKey = [self.OE_entityName stringByAppendingString:OESelectedMediaKey];
    
    for (NSData *data in [[NSUserDefaults standardUserDefaults] objectForKey:defaultsKey]) {
        
        NSURL *representation = [NSKeyedUnarchiver unarchiveObjectWithData:data];
        
        if (!representation) {
            continue;
        }
        
        NSManagedObjectID *objectID = [persistentStoreCoordinator managedObjectIDForURIRepresentation:representation];
        
        if (!objectID) {
            continue;
        }
        
        __kindof OEDBItem *mediaItem = [context objectWithID:objectID];
        
        NSUInteger index = [self.items indexOfObject:mediaItem];
        
        if (index == NSNotFound) {
            continue;
        }
        
        [mediaItemsToSelect addIndex:index];
    }
    
    self.selectionIndexes = mediaItemsToSelect;
    
    // Scroll to the first selected item.
    if (self.selectionIndexes.count > 0) {
        NSRect itemFrame = [self.gridView itemFrameAtIndex:self.selectionIndexes.firstIndex];
        [self.gridView scrollRectToVisible:itemFrame];
    }
}

- (void)_setupToolbar
{
    OELibraryController *libraryController = self.libraryController;
    OELibraryToolbar *toolbar = libraryController.toolbar;
    
    toolbar.gridViewButton.enabled = NO;
    toolbar.listViewButton.enabled = NO;
    toolbar.gridViewButton.state = NSOffState;
    toolbar.listViewButton.state = NSOffState;
    
    toolbar.gridSizeSlider.enabled = !_shouldShowBlankSlate;
    
    NSSearchField *field = toolbar.searchField;
    field.searchMenuTemplate = nil;
    field.enabled = YES;
    field.stringValue = @"";
    field.enabled = !_shouldShowBlankSlate;
    
    [self _setupSearchMenuTemplate];
}

- (void)updateBlankSlate
{
    [super updateBlankSlate];

    if (self.isSelected) {
        
        OELibraryController *libraryController = self.libraryController;
        OELibraryToolbar *toolbar = libraryController.toolbar;
        toolbar.searchField.enabled = !_shouldShowBlankSlate;
        toolbar.gridSizeSlider.enabled = !_shouldShowBlankSlate;
        
        toolbar.gridViewButton.enabled = NO;
        toolbar.listViewButton.enabled = NO;
        toolbar.gridViewButton.state = NSOffState;
        toolbar.listViewButton.state = NSOffState;
    }
}

- (void)_setupSearchMenuTemplate
{
    NSMenuItem *item = nil;
    NSMenu *menu = [[NSMenu alloc] initWithTitle:@""];

    item = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Everything", @"Search field filter selection title")
                                      action:@selector(searchScopeDidChange:) keyEquivalent:@""];
    [item setState:NSOnState];
    [item setRepresentedObject:@[@"rom.game.gameTitle", @"rom.game.name", @"rom.game.system.lastLocalizedName", @"name", @"userDescription"]];
    [menu addItem:item];

    item = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Name", @"Search field filter selection title")
                                      action:@selector(searchScopeDidChange:) keyEquivalent:@""];
    [item setRepresentedObject:@[@"name"]];
    [menu addItem:item];

    item = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Description", @"Search field filter selection title")
                                      action:@selector(searchScopeDidChange:) keyEquivalent:@""];
    [item setRepresentedObject:@[@"userDescription"]];
    [menu addItem:item];

    item = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Game Name", @"")
                                      action:@selector(searchScopeDidChange:) keyEquivalent:@""];
    [item setRepresentedObject:@[@"rom.game.gameTitle", @"rom.game.name"]];
    [menu addItem:item];

    item = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"System", @"")
                                      action:@selector(searchScopeDidChange:) keyEquivalent:@""];
    [item setRepresentedObject:@[@"rom.game.system.lastLocalizedName"]];
    [menu addItem:item];

    for(item in [menu itemArray]){
        [item setTarget:self];
        [item setIndentationLevel:1];
    }

    item = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Filter by:", @"Search field menu, first item, instructional") action:NULL keyEquivalent:@""];
    [menu insertItem:item atIndex:0];

    [[[[self libraryController] toolbar] searchField] setSearchMenuTemplate:menu];
}

- (void)searchScopeDidChange:(NSMenuItem*)sender
{
    NSMenu *menu = [sender menu];
    [[menu itemArray] enumerateObjectsUsingBlock:^(NSMenuItem *item, NSUInteger idx, BOOL *stop) {
        [item setState:NSOffState];
    }];

    [sender setState:NSOnState];
    [self setSearchKeys:[sender representedObject]];
    NSSearchField *field = [[[self libraryController] toolbar] searchField];
    [self search:field];
}

- (void)setRepresentedObject:(id)representedObject
{
    [super setRepresentedObject:representedObject];
    
    if (representedObject) {
        self.saveStateMode = [representedObject isKindOfClass:[OEDBSavedGamesMedia class]];
        [self reloadData];
    }
    
    // Restore search field text.
    NSString *newSearchFieldStringValue = self.currentSearchTerm ?: @"";
    self.libraryController.toolbar.searchField.stringValue = newSearchFieldStringValue;

    [self _setupSearchMenuTemplate];
}

- (OECollectionViewControllerViewTag)OE_currentViewTagByToolbarState
{
    return OEGridViewTag;
}

- (BOOL)isSelected
{
    return self.libraryController.currentSubviewController == self;
}

#pragma mark - OELibrarySubviewController Implementation

- (NSArray*)selectedGames
{
    return @[];
}

- (NSArray <OEDBSaveState *> *)selectedSaveStates
{
    if (!self.saveStateMode) {
        return @[];
    }
    
    NSIndexSet *indices = self.selectionIndexes;
    return [self.items objectsAtIndexes:indices];
}

- (NSArray <OEDBScreenshot *> *)selectedScreenshots {
    
    if (self.saveStateMode) {
        return @[];
    }
    
    NSIndexSet *indices = self.selectionIndexes;
    return [self.items objectsAtIndexes:indices];
}

- (NSIndexSet*)selectionIndexes
{
    return [[self gridView] selectionIndexes];
}

- (void)setSelectionIndexes:(NSIndexSet *)selectionIndexes
{
    [super setSelectionIndexes:selectionIndexes];
    
    [self.gridView setSelectionIndexes:selectionIndexes byExtendingSelection:NO];
    
    [NSNotificationCenter.defaultCenter postNotificationName:OEMediaViewControllerDidSetSelectionIndexesNotification object:self];
}

- (void)imageBrowserSelectionDidChange:(IKImageBrowserView *)aBrowser
{
    [super imageBrowserSelectionDidChange:aBrowser];
    
    // Save selected media to user defaults.
    NSMutableArray <NSData *> *archivableRepresentations = [NSMutableArray array];
    for (__kindof OEDBItem *item in [self.items objectsAtIndexes:self.selectionIndexes]) {
        
        NSManagedObjectID *objectID = item.permanentID;
        NSData *representation = [NSKeyedArchiver archivedDataWithRootObject:objectID.URIRepresentation];
        
        [archivableRepresentations addObject:representation];
    }

    NSString *defaultsKey = [[self OE_entityName] stringByAppendingString:OESelectedMediaKey];
    [[NSUserDefaults standardUserDefaults] setObject:archivableRepresentations forKey:defaultsKey];
    
    [self refreshPreviewPanelIfNeeded];
}

#pragma mark -

- (void)search:(id)sender
{
    self.currentSearchTerm = self.libraryController.toolbar.searchField.stringValue;
    
    NSArray *tokens = [self.currentSearchTerm componentsSeparatedByCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];

    // Create predicates matching all search term tokens (AND) for each search key
    NSMutableArray *searchKeyPredicates = [NSMutableArray array];
    for (NSString *key in self.searchKeys) {
        NSMutableArray *tokenPredicates = [NSMutableArray array];
        for (NSString *token in tokens) {
            if (token.length > 0) {
                [tokenPredicates addObject:[NSPredicate predicateWithFormat:@"%K contains[cd] %@", key, token]];
            }
        }
        
        // Note: an AND predicate with no subpredicates is TRUEPREDICATE
        NSCompoundPredicate *searchKeyPredicate = [NSCompoundPredicate andPredicateWithSubpredicates:tokenPredicates];
        
        [searchKeyPredicates addObject:searchKeyPredicate];
    }

    // Combine search key predicates so any of them will match (OR)
    if (searchKeyPredicates.count > 0)
        _searchPredicate = [NSCompoundPredicate orPredicateWithSubpredicates:searchKeyPredicates];
    else
        _searchPredicate = [NSPredicate predicateWithValue:YES];

    [self reloadData];
}

- (void)fetchItems
{   
#pragma TODO(Improve group detection)
    if([self representedObject] == nil) return;

    NSManagedObjectContext *context = [[OELibraryDatabase defaultDatabase] mainThreadContext];

    NSMutableArray *ranges = [NSMutableArray array];
    NSArray *result = nil;

    NSFetchRequest *req = [[NSFetchRequest alloc] init];
    [req setEntity:[NSEntityDescription entityForName:[self OE_entityName] inManagedObjectContext:context]];

    NSPredicate *baseFilter = [NSPredicate predicateWithValue:YES];
    if([[self representedObject] respondsToSelector:@selector(baseFilterPredicate)]){
        baseFilter = [[self representedObject] baseFilterPredicate];
    }
    [req setPredicate:baseFilter];

    _shouldShowBlankSlate = [context countForFetchRequest:req error:nil] == 0;
    if(_shouldShowBlankSlate)
    {
        _items       = @[];
        _groupRanges = @[];

        [self updateBlankSlate];
        return;
    }

    [req setSortDescriptors:@[[NSSortDescriptor sortDescriptorWithKey:@"rom.game.name" ascending:YES],
                              [NSSortDescriptor sortDescriptorWithKey:@"timestamp" ascending:YES]]];

    [req setPredicate:[NSCompoundPredicate andPredicateWithSubpredicates:@[baseFilter, _searchPredicate]]];

    NSError *error  = nil;
    if(!(result=[context executeFetchRequest:req error:&error]))
    {
        DLog(@"Error fetching save states");
        DLog(@"%@", error);
    }

    if([result count] == 0)
    {
        _groupRanges = @[];
        _items = @[];

        [self updateBlankSlate];
        return;
    }

    NSManagedObjectID *gameID = [[[[result objectAtIndex:0] rom] game] objectID];
    NSUInteger groupStart = 0;
    NSInteger i;
    for(i=0; i < [result count]; i++)
    {
        OEDBSaveState *state = [result objectAtIndex:i];
        NSManagedObjectID *objectID = [[[state rom] game] objectID];
        if(![objectID isEqualTo:gameID])
        {
            [ranges addObject:[NSValue valueWithRange:NSMakeRange(groupStart, i-groupStart)]];
            groupStart = i;
            gameID = objectID;
        }
    }

    if(groupStart != i)
    {
        [ranges addObject:[NSValue valueWithRange:NSMakeRange(groupStart, i-groupStart)]];
    }
    _groupRanges = ranges;
    _items = result;
}

- (void)reloadData
{
    // Preserve the selection.
    NSArray <__kindof OEDBItem *> *selectedItems = [self.items objectsAtIndexes:self.selectionIndexes];
    
    [super reloadData];
    
    NSMutableIndexSet *indexesToSelect = [NSMutableIndexSet indexSet];
    for (__kindof OEDBItem *item in selectedItems) {
        
        NSUInteger index = [self.items indexOfObject:item];
        
        if (index != NSNotFound)
            [indexesToSelect addIndex:index];
    }
    
    [self setSelectionIndexes:indexesToSelect];
}

- (NSString*)OE_entityName
{
    return [self saveStateMode] ? [OEDBSaveState entityName] : [OEDBScreenshot entityName];
}

#pragma mark - Context Menu

- (BOOL)validateMenuItem:(NSMenuItem *)item
{
    SEL action = [item action];
    if (action == @selector(showInFinder:))
        return [[self selectionIndexes] count] > 0;
    return [super validateMenuItem:item];
}

- (NSMenu*)menuForItemsAtIndexes:(NSIndexSet *)indexes
{
    if([self saveStateMode])
        return [self OE_saveStateMenuForItensAtIndexes:indexes];
    else
        return [self OE_screenshotMenuForItensAtIndexes:indexes];
}

- (NSMenu*)OE_saveStateMenuForItensAtIndexes:(NSIndexSet *)indexes
{
    NSMenu *menu = [[NSMenu alloc] init];

    if([indexes count] == 1)
    {

        [menu addItemWithTitle:NSLocalizedString(@"Play Save State", @"SaveState View Context menu")
                        action:@selector(startSaveState:) keyEquivalent:@""];
        [menu addItemWithTitle:NSLocalizedString(@"Rename", @"SaveState View Context menu")
                        action:@selector(beginEditingWithSelectedItem:) keyEquivalent:@""];
        [menu addItemWithTitle:NSLocalizedString(@"Show in Finder", @"SaveState View Context menu")
                        action:@selector(showInFinder:) keyEquivalent:@""];
        [menu addItemWithTitle:NSLocalizedString(@"Delete Save State", @"SaveState View Context menu")
                        action:@selector(deleteSelectedItems:) keyEquivalent:@""];
    }
    else
    {
        [menu addItemWithTitle:NSLocalizedString(@"Show in Finder", @"SaveState View Context menu")
                        action:@selector(showInFinder:) keyEquivalent:@""];
        [menu addItemWithTitle:NSLocalizedString(@"Delete Save States", @"SaveState View Context menu (plural)")
                        action:@selector(deleteSelectedItems:) keyEquivalent:@""];
    }

    return [menu numberOfItems] != 0 ? menu : nil;
}


- (NSMenu*)OE_screenshotMenuForItensAtIndexes:(NSIndexSet *)indexes
{

    NSMenu *menu = [[NSMenu alloc] init];
    NSMenu *shareMenu = [self OE_shareMenuForItemsAtIndexes:indexes];

    if([indexes count] == 1)
    {
        [[menu addItemWithTitle:NSLocalizedString(@"Share", @"SaveState View Context menu")
                         action:nil keyEquivalent:@""] setSubmenu:shareMenu];
        [menu addItemWithTitle:NSLocalizedString(@"Rename", @"SaveState View Context menu")
                        action:@selector(beginEditingWithSelectedItem:) keyEquivalent:@""];
        [menu addItemWithTitle:NSLocalizedString(@"Show in Finder", @"SaveState View Context menu")
                        action:@selector(showInFinder:) keyEquivalent:@""];
        [menu addItemWithTitle:NSLocalizedString(@"Delete Screenshot", @"Screenshot View Context menu")
                        action:@selector(deleteSelectedItems:) keyEquivalent:@""];
    }
    else
    {
        [[menu addItemWithTitle:NSLocalizedString(@"Share", @"SaveState View Context menu")
                         action:nil keyEquivalent:@""] setSubmenu:shareMenu];
        [menu addItemWithTitle:NSLocalizedString(@"Show in Finder", @"SaveState View Context menu")
                        action:@selector(showInFinder:) keyEquivalent:@""];
        [menu addItemWithTitle:NSLocalizedString(@"Delete Screenshots", @"Screenshot View Context menu (plural)")
                        action:@selector(deleteSelectedItems:) keyEquivalent:@""];
    }

    return menu;
}

- (NSMenu*)OE_shareMenuForItemsAtIndexes:(NSIndexSet *)indexes
{
    NSMenu *menu = [[NSMenu alloc] init];
    NSArray *items = [[self items] objectsAtIndexes:indexes];
    NSArray *urls  = [items valueForKeyPath:@"URL.absoluteURL"];
    NSArray *sharingServices = [NSSharingService sharingServicesForItems:urls];
    
    for (NSSharingService *currentService in sharingServices)
    {
        NSMenuItem *item = [[NSMenuItem alloc] initWithTitle:[currentService title]
                                                      action:@selector(shareFromService:) keyEquivalent:@""];
        [item setImage:[currentService image]];
        [item setRepresentedObject:currentService];
        [menu addItem:item];
    }
    
    return menu;
}
             
 - (IBAction)shareFromService:(id)sender
{
     NSIndexSet *indexes = [self selectionIndexes];
     NSArray *items = [[self items] objectsAtIndexes:indexes];
     NSArray *urls  = [items valueForKeyPath:@"URL.absoluteURL"];
     
     [[sender representedObject] performWithItems:urls];
 }

- (IBAction)showInFinder:(id)sender
{
    NSIndexSet *indexes = [self selectionIndexes];
    NSArray *items = [[self items] objectsAtIndexes:indexes];
    NSArray *urls  = [items valueForKeyPath:@"URL.absoluteURL"];

    [[NSWorkspace sharedWorkspace] activateFileViewerSelectingURLs:urls];
}

- (void)deleteSelectedItems:(id)sender
{
    NSIndexSet *selection = [self selectionIndexes];
    NSArray *items = [[[self items] objectsAtIndexes:selection] copy];

    if([[items lastObject] isKindOfClass:[OEDBSaveState class]])
    {
        OEHUDAlert *alert = nil;
        if([items count] < 1)
        {
            DLog(@"delete empty selection");
            return;
        }
        else if([items count] == 1)
        {
            alert = [OEHUDAlert deleteStateAlertWithStateName:[[items lastObject] displayName]];
        }
        else if([items count] > 1)
        {
            alert = [OEHUDAlert deleteStateAlertWithStateCount:[items count]];
        }

        if([alert runModal] == NSAlertFirstButtonReturn)
        {
            [items makeObjectsPerformSelector:@selector(deleteAndRemoveFiles)];
            [[[[self libraryController] database] mainThreadContext] save:nil];
            [self reloadData];
        }
    }
    else if([[items lastObject] isKindOfClass:[OEDBScreenshot class]])
    {
        OEHUDAlert *alert = nil;
        if([items count] < 1)
        {
            DLog(@"delete empty selection");
            return;
        }
        else if([items count] == 1)
        {
            OEDBScreenshot *screenshot = [items lastObject];
            alert = [OEHUDAlert deleteScreenshotAlertWithScreenshotName:[screenshot name]];
        }
        else if([items count] > 1)
        {
            alert = [OEHUDAlert deleteScreenshotAlertWithScreenshotCount:[items count]];
        }

        if([alert runModal] == NSAlertFirstButtonReturn)
        {
            [items makeObjectsPerformSelector:@selector(delete)];
            [[[[self libraryController] database] mainThreadContext] save:nil];
            [self reloadData];
        }
    }
    else
    {
        NSLog(@"No delete action for items of %@ type", [[items lastObject] className]);
    }
}
#pragma mark - GridView DataSource
- (NSUInteger)numberOfGroupsInImageBrowser:(IKImageBrowserView *)aBrowser
{
    return [_groupRanges count];
}

- (id)imageBrowser:(IKImageBrowserView *)aBrowser itemAtIndex:(NSUInteger)index
{
    return [OESavedGamesDataWrapper wrapperWithItem:[[self items] objectAtIndex:index]];
}

- (NSDictionary*)imageBrowser:(IKImageBrowserView *)aBrowser groupAtIndex:(NSUInteger)index
{
    NSValue  *groupRange = [[self groupRanges] objectAtIndex:index];
    NSRange range = [groupRange rangeValue];
    id  firstItem = [[self items] objectAtIndex:range.location];
    OEDBGame   *game   = [[firstItem rom]  game];
    OEDBSystem *system = [[[firstItem rom] game] system];
    TODO("seen a crash here for screenshots without a game");
    return @{
             IKImageBrowserGroupTitleKey : ([game gameTitle] ?: [game displayName]) ?: @"Missing Value",
             IKImageBrowserGroupRangeKey : groupRange,
             IKImageBrowserGroupStyleKey : @(IKGroupDisclosureStyle),
             OEImageBrowserGroupSubtitleKey : ([system lastLocalizedName] ?: [system name]) ?: @"Missing Value",
             };
}

- (NSUInteger)numberOfItemsInImageBrowser:(IKImageBrowserView *)aBrowser
{
    return [[self items] count];
}

- (void)gridView:(OEGridView *)gridView setTitle:(NSString *)title forItemAtIndex:(NSInteger)index
{
    if(index < 0 || index >= [_items count] || [title length] == 0)
        return;

    // ignore users who are trying to manually create auto or quick saves
    if([title rangeOfString:OESaveStateSpecialNamePrefix].location == 0)
        return;

    id item = [[self items] objectAtIndex:index];

    if([item isKindOfClass:[OEDBSaveState class]])
    {
        if([[item displayName] isEqualToString:title]) return;

        OEDBSaveState *saveState = item;
        if(![saveState isSpecialState] || [[OEHUDAlert renameSpecialStateAlert] runModal] == NSAlertFirstButtonReturn)
        {
            [saveState setName:title];
            [saveState moveToDefaultLocation];

            if([saveState writeToDisk] == NO)
            {
                // TODO: delete save state with
                NSLog(@"Writing save state '%@' failed. It should be delted!", title);
            }
        }
    }
    else if([item isKindOfClass:[OEDBScreenshot class]])
    {
        OEDBScreenshot *screenshot = item;
        [screenshot setName:title];
        [screenshot updateFile];
    } else {
        NSLog(@"unkown item type");
    }

    [item save];
}

- (void)imageBrowser:(IKImageBrowserView *)aBrowser cellWasDoubleClickedAtIndex:(NSUInteger)index
{
    id item = self.items[index];

    if ([item isKindOfClass:[OEDBSaveState class]]) {
        
        [NSApp sendAction:@selector(startSaveState:) to:nil from:self];
        
    } else if ([item isKindOfClass:[OEDBScreenshot class]]) {
        
        [self showInFinder:nil];
        
    } else {
        
        NSLog(@"No action for items of %@ type", [item className]);
    }
}

- (void)startSelectedGame:(id)sender
{
    [NSApp sendAction:@selector(startSaveState:) to:nil from:self];
}

#pragma mark - GridView DraggingDestinationDelegate
- (BOOL)performDragOperation:(id<NSDraggingInfo>)sender
{
    NSPasteboard *draggingPasteboard = [sender draggingPasteboard];
    NSInteger     draggingOperation  = [[self gridView] draggingOperation];

    if (draggingOperation == IKImageBrowserDropBefore || draggingOperation == IKImageBrowserDropOn)
    {
        NSArray *files = [draggingPasteboard propertyListForType:NSFilenamesPboardType];
        OEROMImporter *romImporter = [[[self libraryController] database] importer];
        [romImporter importItemsAtPaths:files intoCollectionWithID:nil];
    }
    else if (draggingOperation == IKImageBrowserDropNone)
    {
        [self presentError:[NSError errorWithDomain:@"Error in performing drag operation." code:-1 userInfo:nil]];
    }

    [[self gridView] setDraggingOperation:(IKImageBrowserDropOperation)IKImageBrowserDropNone];
    return YES;
}


#pragma mark - QLPreviewPanelDataSource


- (BOOL)acceptsPreviewPanelControl:(QLPreviewPanel *)panel
{
    id<OECollectionViewItemProtocol> viewItem = [self representedObject];
    if ([viewItem respondsToSelector:@selector(collectionSupportsQuickLook)])
        return [viewItem collectionSupportsQuickLook];
    return NO;
}


- (NSInteger)numberOfPreviewItemsInPreviewPanel:(QLPreviewPanel *)panel
{
    return self.selectionIndexes.count;
}


- (id <QLPreviewItem>)previewPanel:(QLPreviewPanel *)panel previewItemAtIndex:(NSInteger)index
{
    __block NSInteger reali = NSNotFound;
    __block NSInteger i = index;
    
    [self.selectionIndexes enumerateRangesUsingBlock:^(NSRange range, BOOL * stop) {
        if (i < range.length) {
            *stop = YES;
            reali = range.location + i;
        } else {
            i -= range.length;
        }
    }];
    return reali == NSNotFound ? nil : self.items[reali];
}


- (NSInteger)imageBrowserViewIndexForPreviewItem:(id <QLPreviewItem>)item
{
    /* only search thru selected items because otherwise it might take forever */
    NSInteger res =  [self.items indexOfObjectAtIndexes:self.selectionIndexes options:0 passingTest:^BOOL(id  _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
        return item == obj;
    }];
    return res == NSNotFound ? self.selectionIndexes.firstIndex : res;
}



@end


#pragma mark - OESavedGamesDataWrapper

@implementation OESavedGamesDataWrapper

static NSDateFormatter *formatter = nil;

+ (void)initialize
{
    if (self == [OESavedGamesDataWrapper class]) {
        formatter = [[NSDateFormatter alloc] init];
        [formatter setDoesRelativeDateFormatting:YES];
        [formatter setDateStyle:NSDateFormatterMediumStyle];
        [formatter setTimeStyle:NSDateFormatterMediumStyle];
    }
}

+ (instancetype)wrapperWithItem:(id)item
{
    if([item isKindOfClass:[OEDBSaveState class]])
        return [self wrapperWithState:item];
    else if([item isKindOfClass:[OEDBScreenshot class]])
        return [self wrapperWithScreenshot:item];
    return nil;
}

+ (instancetype)wrapperWithState:(OEDBSaveState*)state
{
    OESavedGamesDataWrapper *obj = [[self alloc] init];
    [obj setState:state];
    return obj;
}

+ (instancetype)wrapperWithScreenshot:(OEDBScreenshot*)screenshot;
{
    OESavedGamesDataWrapper *obj = [[self alloc] init];
    [obj setScreenshot:screenshot];
    return obj;
}

#pragma mark - NSPasteboardWriting

- (NSArray*)writableTypesForPasteboard:(NSPasteboard *)pasteboard
{
    if([self state])
        return [[self state] writableTypesForPasteboard:pasteboard];
    if([self screenshot])
        return [[self screenshot] writableTypesForPasteboard:pasteboard];

    return @[];
}

- (id)pasteboardPropertyListForType:(NSString *)type
{
    if([self state])
        return [[self state] pasteboardPropertyListForType:type];
    if([self screenshot])
        return [[self screenshot] pasteboardPropertyListForType:type];

    return nil;
}

#pragma mark - ImageKit DataSource Item
- (NSString *)imageUID
{
    if([self state])
        return [[self state] location];
    if([self screenshot])
        return [[self screenshot] location];
    return nil;
}

- (NSString *)imageRepresentationType
{
    return IKImageBrowserNSURLRepresentationType;
}

- (id)imageRepresentation
{
    if([self state])
            return [[self state] screenshotURL];
    if([self screenshot])
            return [[self screenshot] URL];
    return nil;
}

- (NSString *)imageTitle
{
    if([self state])
        return [[self state] displayName];
    if([self screenshot])
        return [[self screenshot] name];
    return nil;
}

- (NSString *)imageSubtitle
{
    if([self state])
        return [formatter stringFromDate:[[self state] timestamp]];
    if([self screenshot])
        return [formatter stringFromDate:[[self screenshot] timestamp]];
    return @"";
}

@end
