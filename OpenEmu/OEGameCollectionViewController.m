/*
 Copyright (c) 2014, OpenEmu Team

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

#import "OEGameCollectionViewController.h"

#import "OEArrayController.h"
#import "OESidebarController.h"
#import "OEHUDAlert.h"
#import "OEHUDAlert+DefaultAlertsAdditions.h"
#import "OERatingCell.h"
#import "OEMainWindowController.h"
#import "OELibraryController.h"
#import "OELibraryGamesViewController.h"

#import "OELibraryDatabase.h"
#import "OEDBGame.h"
#import "OEDBRom.h"
#import "OEDBSystem.h"
#import "OEDBSaveState.h"
#import "OEDBCollection.h"
#import "OEDBSmartCollection.h"
#import "OEDBAllGamesCollection.h"

#import "OEROMImporter.h"

#import "OEGameDocument.h"

#import "OEThemeObject.h"
#import "OEThemeImage.h"

#import "OESearchField.h"
#import "OETableView.h"

#import "IKImageFlowView.h"

#import "OEDBDataSourceAdditions.h"

#import "OpenEmu-Swift.h"

NSString * const OEGameCollectionViewControllerDidSetSelectionIndexesNotification = @"OEGameCollectionViewControllerDidSetSelectionIndexesNotification";

static NSArray *OE_defaultSortDescriptors;

extern NSString * const OEGameControlsBarCanDeleteSaveStatesKey;

/// Archived URI representations of managed object IDs for selected OEDBGames.
static NSString * const OESelectedGamesKey = @"OESelectedGamesKey";

@interface OECollectionViewController ()
@property (readwrite) OECollectionViewControllerViewTag selectedViewTag;
@end

@interface OEGameCollectionViewController ()

- (NSMenu *)OE_saveStateMenuForGame:(OEDBGame *)game;
- (NSMenu *)OE_ratingMenuForGames:(NSArray *)games;
- (NSMenu *)OE_collectionsMenuForGames:(NSArray *)games;

@property (strong) NSDate *listViewSelectionChangeDate;
@property (readonly) OEArrayController *gamesController;

/// The search term of the currently applied filter.
@property (copy, nullable) NSString *currentSearchTerm;

@end

@implementation OEGameCollectionViewController
@synthesize gamesController=gamesController;

- (NSString*)nibName
{
    return @"OECollectionViewController";
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    // Restore grid/list view mode.
    OECollectionViewControllerViewTag tag = [[NSUserDefaults standardUserDefaults] integerForKey:OELastCollectionViewKey];
    [self OE_setupToolbarStatesForViewTag:tag];
    self.selectedViewTag = tag;

    [[self listView] setDraggingSourceOperationMask:NSDragOperationCopy forLocal:NO];
}

- (void)viewWillAppear
{
    [super viewWillAppear];
    
    [self OE_setupToolbarStatesForViewTag:self.selectedViewTag];
    
    OESearchField *searchField = self.libraryController.toolbar.searchField;
    searchField.enabled = YES;
    searchField.stringValue = self.currentSearchTerm ?: @"";
    
    [self scrollToSelection];
}

- (void)viewDidAppear
{
    [super viewDidAppear];
    [[self listView] bind:@"selectionIndexes" toObject:gamesController withKeyPath:@"selectionIndexes" options:@{}];
}

- (void)viewDidDisappear
{
    [super viewDidDisappear];
    
    [self.listView unbind:@"selectionIndexes"];
    
    // We don't want to clear the search filter if the view is disappearing because of gameplay.
    OEGameDocument *gameDocument = (OEGameDocument *)[[NSDocumentController sharedDocumentController] currentDocument];
    const BOOL playingGame = gameDocument != nil;
    
    if(!playingGame)
    {
        // Clear any previously applied search filter.
        gamesController.filterPredicate = nil;
        self.currentSearchTerm = nil;
        [self.listView reloadData];
        [self.gridView reloadData];
    }
}

- (void)setLibraryController:(OELibraryController *)libraryController
{
    [super setLibraryController:libraryController];
    [self _setupGamesController];
}

- (void)_setupGamesController {
    OELibraryDatabase *database = [[self libraryController] database];
    NSManagedObjectContext *context = [database mainThreadContext];

    OE_defaultSortDescriptors = @[[NSSortDescriptor sortDescriptorWithKey:@"cleanDisplayName" ascending:YES selector:@selector(caseInsensitiveCompare:)]];

    gamesController = [[OEArrayController alloc] init];
    [gamesController setAutomaticallyRearrangesObjects:YES];
    [gamesController setAutomaticallyPreparesContent:NO];
    [gamesController setUsesLazyFetching:NO];

    [gamesController setManagedObjectContext:context];
    [gamesController setEntityName:@"Game"];
    [gamesController setSortDescriptors:OE_defaultSortDescriptors];
    [gamesController setFetchPredicate:[NSPredicate predicateWithValue:NO]];
    [gamesController setAvoidsEmptySelection:NO];
}

- (void)fetchItems
{
    OECoreDataMainThreadAssertion();

    id <OEGameCollectionViewItemProtocol>representedObject = (id <OEGameCollectionViewItemProtocol>)[self representedObject];

    NSPredicate *pred = representedObject ? [representedObject fetchPredicate]:[NSPredicate predicateWithValue:NO];
    [gamesController setFetchPredicate:pred];
    [gamesController setLimit:[representedObject fetchLimit]];
    [gamesController setFetchSortDescriptors:[representedObject fetchSortDescriptors]];

    NSError *error = nil;
    if(![gamesController fetchWithRequest:nil merge:NO error:&error])
    {
        NSLog(@"Error while fetching: %@", gamesController);
        NSLog(@"%@", [error localizedDescription]);
        return;
    }
}

- (NSArray*)defaultSortDescriptors
{
    return OE_defaultSortDescriptors;
}

- (void)setSortDescriptors:(NSArray*)descriptors
{
    [[self gamesController] setSortDescriptors:descriptors];
}

- (void)dealloc
{
    gamesController = nil;
}

#pragma mark - Selection

- (BOOL)isSelected
{
    return [self.libraryController.currentSubviewController isKindOfClass:[OELibraryGamesViewController class]];
}

- (NSArray *)selectedGames
{
    return [gamesController selectedObjects];
}

- (NSIndexSet *)selectionIndexes
{
    return [gamesController selectionIndexes];
}

- (void)setSelectionIndexes:(NSIndexSet *)selectionIndexes
{
    [gamesController setSelectionIndexes:selectionIndexes];

    [[self gridView] setSelectionIndexes:selectionIndexes byExtendingSelection:NO];
    
    // Save selected games to user defaults.
    NSMutableArray <NSData *> *archivableRepresentations = [NSMutableArray array];
    for (OEDBGame *selectedGame in self.selectedGames) {
        NSManagedObjectID *objectID = selectedGame.permanentID;
        NSData *representation = [NSKeyedArchiver archivedDataWithRootObject:objectID.URIRepresentation];
        [archivableRepresentations addObject:representation];
    }
    
    [[NSUserDefaults standardUserDefaults] setObject:archivableRepresentations forKey:OESelectedGamesKey];
    
    [NSNotificationCenter.defaultCenter postNotificationName:OEGameCollectionViewControllerDidSetSelectionIndexesNotification object:self];
}

- (void)scrollToSelection
{
    if (self.selectionIndexes.count > 0) {
        
        NSRect itemFrame = [self.gridView itemFrameAtIndex:self.selectionIndexes.firstIndex];
        [self.gridView scrollRectToVisible:itemFrame];
        
        [self.listView scrollRowToVisible:self.selectionIndexes.firstIndex];
    }
}

#pragma mark - View Selection

- (void)OE_switchToView:(OECollectionViewControllerViewTag)tag
{
    [super OE_switchToView:tag];
    
    [[NSUserDefaults standardUserDefaults] setInteger:self.selectedViewTag forKey:OELastCollectionViewKey];
}

#pragma mark -
- (BOOL)shouldShowBlankSlate
{
    // We cannot use [[gamesController arrangedObjects] count] since that takes into account the filter predicate
    NSFetchRequest *fetchRequest = [gamesController defaultFetchRequest];
    [fetchRequest setFetchLimit:1];
    NSUInteger count = [[gamesController managedObjectContext] countForFetchRequest:fetchRequest error:NULL];
    return count == 0;
}

- (void)setRepresentedObject:(id)representedObject
{
    [super setRepresentedObject:representedObject];
    
    self.libraryController.toolbar.searchField.searchMenuTemplate = nil;
    
    // Restore search field text.
    NSString *newSearchFieldStringValue = self.currentSearchTerm ?: @"";
    self.libraryController.toolbar.searchField.stringValue = newSearchFieldStringValue;
    
    NSAssert([representedObject conformsToProtocol:@protocol(OEGameCollectionViewItemProtocol)], @"");

    [self.listView tableColumnWithIdentifier:@"listViewConsoleName"].hidden = ![representedObject shouldShowSystemColumnInListView];
    [self reloadData];
    
    // Restore game selection.
    NSManagedObjectContext *context = self.libraryController.database.mainThreadContext;
    NSPersistentStoreCoordinator *persistentStoreCoordinator = context.persistentStoreCoordinator;
    NSMutableIndexSet *gameIndexesToSelect = [NSMutableIndexSet indexSet];
    for (NSData *data in [[NSUserDefaults standardUserDefaults] objectForKey:OESelectedGamesKey]) {
        
        NSURL *representation = [NSKeyedUnarchiver unarchiveObjectWithData:data];
        
        if (!representation) {
            continue;
        }
        
        NSManagedObjectID *objectID = [persistentStoreCoordinator managedObjectIDForURIRepresentation:representation];
        
        if (!objectID) {
            continue;
        }
        
        OEDBGame *game = [context objectWithID:objectID];
        
        NSUInteger index = [self.gamesController.arrangedObjects indexOfObject:game];
        
        if (index == NSNotFound) {
            continue;
        }
        
        [gameIndexesToSelect addIndex:index];
    }
    
    self.selectionIndexes = gameIndexesToSelect;
    
    [self scrollToSelection];
}

- (id <OEGameCollectionViewItemProtocol>)representedObject
{
    return (id <OEGameCollectionViewItemProtocol>) [super representedObject];
}

#pragma mark - UI Actions

- (BOOL)validateMenuItem:(NSMenuItem *)item
{
    SEL action = [item action];
    if (action == @selector(showInFinder:))
        return [[self selectedGames] count] > 0;
    return [super validateMenuItem:item];
}

- (void)search:(id)sender
{
    self.currentSearchTerm = [sender stringValue];
    
    NSArray *tokens = [self.currentSearchTerm componentsSeparatedByCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];

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

    gamesController.filterPredicate = pred;

    [self.listView reloadData];
    [self.gridView reloadData];
}

- (IBAction)showInFinder:(id)sender
{
    NSArray *selectedGames = [self selectedGames];
    NSArray *urls = [selectedGames valueForKeyPath:@"defaultROM.URL.absoluteURL"];
    urls = [urls filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"self != nil"]];

    [[NSWorkspace sharedWorkspace] activateFileViewerSelectingURLs:urls];
}

- (IBAction)trashDownloadedFiles:(id)sender
{
    NSArray *selectedGames = [self selectedGames];

    for(OEDBGame *game in selectedGames)
    {
        OEDBRom *rom = [game defaultROM];
        if([[rom source] length] != 0)
        {
            [[NSFileManager defaultManager] trashItemAtURL:[rom URL] resultingItemURL:nil error:nil];
        }
    }

    [self reloadData];
}

- (void)deleteSaveState:(id)stateItem
{
    // TODO: localize and rephrase text
    OEDBSaveState *state = [stateItem representedObject];
    NSString *stateName = [state name];
    OEHUDAlert *alert = [OEHUDAlert deleteStateAlertWithStateName:stateName];

    NSUInteger result = [alert runModal];
    if(result == NSAlertFirstButtonReturn)
        [state deleteAndRemoveFiles];
}

- (void)deleteSelectedItems:(id)sender
{
    OECoreDataMainThreadAssertion();

    NSArray *selectedGames = [self selectedGames];
    BOOL multipleGames = ([selectedGames count]>1);

    // deleting from 'All Games', Smart Collections and consoles removes games from the library
    if([[self representedObject] isKindOfClass:[OEDBSmartCollection class]]
       || [self representedObject]==(id<OEGameCollectionViewItemProtocol>)[OEDBAllGamesCollection sharedDBAllGamesCollection]
       || [[self representedObject] isKindOfClass:[OEDBSystem class]])
    {
        // delete games from library if user allows it
        if([[OEHUDAlert removeGamesFromLibraryAlert:multipleGames] runModal] == NSAlertFirstButtonReturn)
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
                //NSUInteger alertReturn = [[OEHUDAlert removeGameFilesFromLibraryAlert:multipleGames] runModal];
                //deleteFiles = (alertReturn == NSAlertFirstButtonReturn);
                deleteFiles = YES;
            }

            DLog(@"deleteFiles: %d", deleteFiles);
            [selectedGames enumerateObjectsUsingBlock:^(OEDBGame *game, NSUInteger idx, BOOL *stopGames) {
                [game deleteByMovingFile:deleteFiles keepSaveStates:NO];
            }];

            NSManagedObjectContext *context = [[selectedGames lastObject] managedObjectContext];
            [context save:nil];

            NSRect visibleRect = [[self gridView] visibleRect];
            [self reloadData];
            [[self gridView] scrollRectToVisible:visibleRect];
        }
    }
    // deletign from normal collections removes games from that collection
    else if([[self representedObject] isMemberOfClass:[OEDBCollection class]])
    {
        // remove games from collection if user allows it
        if([[OEHUDAlert removeGamesFromCollectionAlert] runModal] == NSAlertFirstButtonReturn)
        {
            OEDBCollection* collection = (OEDBCollection*)[self representedObject];
            [[collection mutableGames] minusSet:[NSSet setWithArray:selectedGames]];
            [collection save];
        }
        [self setNeedsReload];
    }
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
    [self reloadDataIndexes:[self selectionIndexes]];
}


- (void)cancelCoverArtDownload:(id)sender
{
    [[self selectedGames] makeObjectsPerformSelector:@selector(cancelCoverDownload)];
    [(OEDBGame*)[[self selectedGames] lastObject] save];
    [self reloadDataIndexes:[self selectionIndexes]];
}

- (void)addCoverArtFromFile:(id)sender
{
    NSOpenPanel *openPanel = [NSOpenPanel openPanel];

    [openPanel setAllowsMultipleSelection:NO];
    [openPanel setCanChooseDirectories:NO];
    [openPanel setCanChooseFiles:YES];
    NSArray *imageTypes = [NSImage imageTypes];
    [openPanel setAllowedFileTypes:imageTypes];

    [openPanel beginWithCompletionHandler:^(NSInteger result) {
        if(result != NSFileHandlingPanelOKButton)
            return;

        NSArray *selectedGames = [self selectedGames];
        [selectedGames makeObjectsPerformSelector:@selector(setBoxImageByURL:) withObject:[openPanel URL]];
        NSManagedObjectContext *context = [[selectedGames lastObject] managedObjectContext];
        [context save:nil];

        [self reloadDataIndexes:[self selectionIndexes]];
    }];
}

- (void)addSaveStateFromFile:(id)sender
{
    [self doesNotImplementSelector:_cmd];
}

- (void)consolidateFiles:(id)sender
{
    dispatch_async(dispatch_get_main_queue(), ^{
        NSArray *games = [self selectedGames];
        if([games count] == 0) return;

        OEHUDAlert  *alert = [[OEHUDAlert alloc] init];
        [alert setHeadlineText:@""];
        [alert setMessageText:NSLocalizedString(@"Consolidating will copy all of the selected games into the OpenEmu Library folder.\n\nThis cannot be undone.", @"")];
        [alert setDefaultButtonTitle:NSLocalizedString(@"Consolidate", @"")];
        [alert setAlternateButtonTitle:NSLocalizedString(@"Cancel", @"")];
        if([alert runModal] != NSAlertFirstButtonReturn) return;

        alert = [[OEHUDAlert alloc] init];
        [alert setShowsProgressbar:YES];
        [alert setProgress:0.0];
        [alert setHeadlineText:NSLocalizedString(@"Copying Game Files…", @"")];
        [alert setTitle:@""];
        [alert setShowsProgressbar:YES];
        [alert setDefaultButtonTitle:nil];
        [alert setMessageText:nil];

        __block NSInteger alertResult = -1;
        dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1.0 * NSEC_PER_SEC));
        dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
        NSManagedObjectContext *context = [[OELibraryDatabase defaultDatabase] makeWriterChildContext];

        NSArray *gameIDs = [games valueForKey:@"permanentID"];

        dispatch_after(popTime, queue, ^{
            [context performBlockAndWait:^{
                NSError *error = nil;
                for (NSUInteger i=0; i<[gameIDs count]; i++)
                {
                    if(alertResult != -1) break;

                    NSManagedObjectID *gameID = [gameIDs objectAtIndex:i];
                    OEDBGame *game = [OEDBGame objectWithID:gameID inContext:context];
                    NSSet *roms = [game roms];
                    for(OEDBRom *rom in roms)
                    {
                        if(alertResult != -1) break;
                        if([rom consolidateFilesWithError:&error])
                        {
                            break;
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
                    [alert setCallbackHandler:^(OEHUDAlert *alert, NSModalResponse result){
                        NSString *messageText = [error localizedDescription];
                        OEHUDAlert *errorAlert = [OEHUDAlert alertWithMessageText:messageText defaultButton:@"OK" alternateButton:@""];
                        [errorAlert setTitle:@"Consolidating files failed."];
                        [errorAlert runModal];

                        if(originalCompletionHandler) originalCompletionHandler(alert, result);
                    }];
                }

                [context save:nil];
                NSManagedObjectContext *writerContext = [context parentContext];
                [writerContext performBlock:^{
                    [[writerContext userInfo] setObject:@(YES) forKey:OEManagedObjectContextHasDirectChangesKey];
                    [writerContext save:nil];
                }];
                
                [alert closeWithResult:NSAlertFirstButtonReturn];
            }];
        });
        
        [alert setDefaultButtonTitle:@"Stop"];
        alertResult = [alert runModal];
        
    });
}

#pragma mark - Context Menu
- (NSMenu *)menuForItemsAtIndexes:(NSIndexSet*)indexes
{
    NSMenu *menu = [[NSMenu alloc] init];
    NSMenuItem *menuItem;
    NSArray *games = [[gamesController arrangedObjects] objectsAtIndexes:indexes];

    __block BOOL hasLocalFiles = NO;
    __block BOOL hasRemoteFiles = NO;
    [games enumerateObjectsUsingBlock:^(OEDBGame *game, NSUInteger idx, BOOL *stop) {
        if(!hasLocalFiles)
            hasLocalFiles = [[game defaultROM] filesAvailable];
        if(!hasRemoteFiles)
            hasRemoteFiles = [[[game defaultROM] source] length] != 0;
        *stop = hasLocalFiles && hasRemoteFiles;
    }];

    if([indexes count] == 1)
    {
        NSInteger index = [indexes lastIndex];
        [menu addItemWithTitle:NSLocalizedString(@"Play Game", @"") action:@selector(startSelectedGame:) keyEquivalent:@""];
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

        if(hasLocalFiles)
        {
            [menu addItemWithTitle:NSLocalizedString(@"Show in Finder", @"") action:@selector(showInFinder:) keyEquivalent:@""];
            if(hasRemoteFiles)
                [menu addItemWithTitle:NSLocalizedString(@"Trash downloaded Files", @"") action:@selector(trashDownloadedFiles:) keyEquivalent:@""];
        }
        [menu addItem:[NSMenuItem separatorItem]];

        if([[game status] isEqualTo:@(OEDBGameStatusOK)])
            [menu addItemWithTitle:NSLocalizedString(@"Download Cover Art", @"") action:@selector(downloadCoverArt:) keyEquivalent:@""];
        if([[game status] isEqualTo:@(OEDBGameStatusProcessing)])
            [menu addItemWithTitle:NSLocalizedString(@"Cancel Cover Art Download", @"") action:@selector(cancelCoverArtDownload:) keyEquivalent:@""];

        [menu addItemWithTitle:NSLocalizedString(@"Add Cover Art from File…", @"") action:@selector(addCoverArtFromFile:) keyEquivalent:@""];
        [menu addItemWithTitle:NSLocalizedString(@"Consolidate Files…", @"") action:@selector(consolidateFiles:) keyEquivalent:@""];

        //[menu addItemWithTitle:@"Add Save File To Game…" action:@selector(addSaveStateFromFile:) keyEquivalent:@""];
        [menu addItem:[NSMenuItem separatorItem]];
        // Create Add to collection menu
        NSMenuItem *collectionMenuItem = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Add to Collection", @"") action:NULL keyEquivalent:@""];
        [collectionMenuItem setSubmenu:[self OE_collectionsMenuForGames:games]];
        [menu addItem:collectionMenuItem];
        [menu addItem:[NSMenuItem separatorItem]];
        [menu addItemWithTitle:NSLocalizedString(@"Rename Game", @"") action:@selector(beginEditingWithSelectedItem:) keyEquivalent:@""];
        [menu addItemWithTitle:NSLocalizedString(@"Delete Game", @"") action:@selector(deleteSelectedItems:) keyEquivalent:@""];
    }
    else
    {
        if([[NSUserDefaults standardUserDefaults] boolForKey:OEForcePopoutGameWindowKey])
        {
            [menu addItemWithTitle:NSLocalizedString(@"Play Games (Caution)", @"") action:@selector(startSelectedGame:) keyEquivalent:@""];
        }

        // Create Rating Item
        menuItem = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Rating", @"") action:NULL keyEquivalent:@""];
        [menuItem setSubmenu:[self OE_ratingMenuForGames:games]];
        [menu addItem:menuItem];

        
        if(hasLocalFiles)
        {
            [menu addItemWithTitle:NSLocalizedString(@"Show in Finder", @"") action:@selector(showInFinder:) keyEquivalent:@""];
            if(hasRemoteFiles)
                [menu addItemWithTitle:NSLocalizedString(@"Trash downloaded Files", @"") action:@selector(trashDownloadedFiles:) keyEquivalent:@""];
        }
        [menu addItem:[NSMenuItem separatorItem]];

        [menu addItemWithTitle:NSLocalizedString(@"Download Cover Art", @"") action:@selector(downloadCoverArt:) keyEquivalent:@""];
        [menu addItemWithTitle:NSLocalizedString(@"Add Cover Art from File…", @"") action:@selector(addCoverArtFromFile:) keyEquivalent:@""];
        [menu addItemWithTitle:NSLocalizedString(@"Consolidate Files…", @"") action:@selector(consolidateFiles:) keyEquivalent:@""];

        [menu addItem:[NSMenuItem separatorItem]];
        // Create Add to collection menu
        NSMenuItem *collectionMenuItem = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Add to Collection", @"") action:NULL keyEquivalent:@""];
        [collectionMenuItem setSubmenu:[self OE_collectionsMenuForGames:games]];
        [menu addItem:collectionMenuItem];

        [menu addItem:[NSMenuItem separatorItem]];
        [menu addItemWithTitle:NSLocalizedString(@"Delete Games", @"") action:@selector(deleteSelectedItems:) keyEquivalent:@""];
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
            [ratingItem setTitle:NSLocalizedString(@"None", @"")];
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
    
    [self reloadDataIndexes:[self selectionIndexes]];
}

#pragma mark - GridView DataSource
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
    return [self menuForItemsAtIndexes:indexes];
}
#pragma mark - GridView Delegate
- (void)imageBrowser:(IKImageBrowserView *)aBrowser cellWasDoubleClickedAtIndex:(NSUInteger)index
{
    [[self libraryController] startSelectedGame:self];
}

- (void)gridView:(OEGridView*)gridView requestsDownloadRomForItemAtIndex:(NSUInteger)index
{
    OEDBGame *game = [[[self gamesController] arrangedObjects] objectAtIndex:index];
    [game requestROMDownload];
}
#pragma mark - GridView DraggingDestinationDelegate
- (BOOL)performDragOperation:(id<NSDraggingInfo>)sender
{
    NSPasteboard *draggingPasteboard = [sender draggingPasteboard];
    NSImage      *draggingImage      = [[NSImage alloc] initWithPasteboard:draggingPasteboard];
    NSInteger     draggingOperation  = [[self gridView] draggingOperation];

    if (draggingOperation == IKImageBrowserDropOn && draggingImage)
    {
        NSUInteger droppedIndex = [[self gridView] indexAtLocationOfDroppedItem];
        OEDBGame  *droppedGame  = [[[self gridView] cellForItemAtIndex:droppedIndex] representedItem];

        [droppedGame setBoxImageByImage:draggingImage];
        [[droppedGame managedObjectContext] save:nil];
        [self reloadData];
    }
    else if (draggingOperation == IKImageBrowserDropBefore || draggingOperation == IKImageBrowserDropOn)
    {
        id <OEGameCollectionViewItemProtocol>representedObject = (id <OEGameCollectionViewItemProtocol>)[self representedObject];
        NSArray *files = [draggingPasteboard propertyListForType:NSFilenamesPboardType];
        OEROMImporter *romImporter = [[[self libraryController] database] importer];
        OEDBCollection *collection = [representedObject isKindOfClass:[OEDBCollection class]] ? (OEDBCollection *)representedObject : nil;
        [romImporter importItemsAtPaths:files intoCollectionWithID:[collection permanentID]];
    }
    else if (draggingOperation == IKImageBrowserDropNone)
    {
        [self presentError:[NSError errorWithDomain:@"Error in performing drag operation." code:-1 userInfo:nil]];
    }

    [[self gridView] setDraggingOperation:(IKImageBrowserDropOperation)IKImageBrowserDropNone];
    return YES;
}

- (void)gridView:(OEGridView*)gridView setTitle:(NSString*)title forItemAtIndex:(NSInteger)index
{
    if(index >= 0 && index < [[gamesController arrangedObjects] count] && [title length] != 0)
    {
        OEDBGame * game = [[[self gamesController] arrangedObjects] objectAtIndex:index];
        [game setDisplayName:title];
        [game save];
    }
}
#pragma mark - NSTableView DataSource
- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView
{
    if(aTableView == [self listView])
        return [[gamesController arrangedObjects] count];

    return 0;
}

- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)rowIndex
{
    if(tableView != [self listView]) return nil;
    
    if(rowIndex >= [[gamesController arrangedObjects] count]) return nil;
    
    NSObject<OEListViewDataSourceItem> *item = [[gamesController arrangedObjects] objectAtIndex:rowIndex];
    NSString *columnId                       = [tableColumn identifier];
    id result                                = nil;

    if(columnId == nil)                                               result = item;
    else if([columnId isEqualToString:@"listViewStatus"])
    {
        OEThemeImage *image = [item listViewStatus];
        OEThemeState state  = [[[self listView] selectedRowIndexes] containsIndex:rowIndex] ? OEThemeInputStateFocused : OEThemeStateDefault;
        result = [image imageForState:state];
    }
    else if([item respondsToSelector:NSSelectorFromString(columnId)]) result = [item valueForKey:columnId];

    return result;
}

- (void)tableView:(NSTableView *)aTableView setObjectValue:(id)anObject forTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex
{
    if( aTableView == [self listView])
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
            [self reloadData];
        }
        else return;

        if([obj isKindOfClass:[OEDBItem class]])
        {
            OEDBItem *item = (OEDBItem *)obj;
            [[item managedObjectContext] save:nil];
        }
    }
}

- (void)tableView:(NSTableView *)tableView sortDescriptorsDidChange:(NSArray *)oldDescriptors
{
    if(tableView != [self listView]) return;

    if([[[self listView] sortDescriptors] count] > 0)
    {
        // Make sure we do not accumulate sort descriptors and `listViewTitle` is the secondary
        // sort descriptor provided it's not the main sort descriptor
        NSSortDescriptor *mainSortDescriptor = [[[self listView] sortDescriptors] objectAtIndex:0];

        if(![[mainSortDescriptor key] isEqualToString:@"listViewTitle"])
        {
            [[self listView] setSortDescriptors:(@[
                                            mainSortDescriptor,
                                            [NSSortDescriptor sortDescriptorWithKey:@"listViewTitle" ascending:YES selector:@selector(localizedCaseInsensitiveCompare:)],
                                            ])];
        }
    }

    [gamesController setSortDescriptors:[[self listView] sortDescriptors]];
    [[self listView] reloadData];
}

#pragma mark - TableView Drag and Drop
- (BOOL)tableView:(NSTableView *)aTableView acceptDrop:(id < NSDraggingInfo >)info row:(NSInteger)row dropOperation:(NSTableViewDropOperation)operation
{
    NSPasteboard *pboard = [info draggingPasteboard];
    if (![[pboard types] containsObject:NSFilenamesPboardType])
        return NO;

    if([[pboard types] containsObject:OEPasteboardTypeGame])
        return NO;

    NSArray *files = [pboard propertyListForType:NSFilenamesPboardType];
    OEROMImporter *romImporter = [[[self libraryController] database] importer];
    OEDBCollection *collection = [[self representedObject] isKindOfClass:[OEDBCollection class]] ? (OEDBCollection*)[self representedObject] : nil;
    [romImporter importItemsAtPaths:files intoCollectionWithID:[collection permanentID]];

    return YES;
}

- (NSDragOperation)tableView:(NSTableView *)aTableView validateDrop:(id < NSDraggingInfo >)info proposedRow:(NSInteger)row proposedDropOperation:(NSTableViewDropOperation)operation
{
    NSPasteboard *pboard = [info draggingPasteboard];
    if (![[pboard types] containsObject:NSFilenamesPboardType])
        return NSDragOperationNone;

    if([[pboard types] containsObject:OEPasteboardTypeGame])
        return NSDragOperationNone;

    return NSDragOperationCopy;
}


- (BOOL)tableView:(NSTableView *)aTableView writeRowsWithIndexes:(NSIndexSet *)rowIndexes toPasteboard:(NSPasteboard *)pboard
{
    if(aTableView == [self listView])
    {
        NSArray *objects = [[gamesController arrangedObjects] objectsAtIndexes:rowIndexes];
        [pboard writeObjects:objects];
        
        return YES;
    }
    
    return NO;
}

#pragma mark - NSTableView Delegate
- (void)tableView:(NSTableView *)aTableView willDisplayCell:(id)aCell forTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex
{
    if(aTableView == [self listView])
    {
        if(![aCell isKindOfClass:[OERatingCell class]]) [aCell setHighlighted:NO];
    }
}

- (BOOL)tableView:(NSTableView *)tableView shouldEditTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)rowIndex
{
    return (tableView == [self listView] && [[tableColumn identifier] isEqualToString:@"listViewTitle"]);
}


- (void)tableViewSelectionIsChanging:(NSNotification *)notification
{
    NSTableView *tableView = [notification object];

    // We use _listViewSelectionChangeDate to make sure the rating cell tracks the mouse only
    // if a row selection changed some time ago. Since -tableView:shouldTrackCell:forTableColumn:row:
    // is sent *before* -tableViewSelectionDidChange:, we need to make sure that the rating cell
    // does not track the mouse until the selection has changed and we have been able to assign
    // the proper date to _listViewSelectionChangeDate.
    if(tableView == [self listView]) _listViewSelectionChangeDate = [NSDate distantFuture];
}

- (void)tableViewSelectionDidChange:(NSNotification *)aNotification
{
    if([aNotification object] != [self listView]) return;

    _listViewSelectionChangeDate = [NSDate date];

    [self setSelectionIndexes:[[self listView] selectedRowIndexes]];
}

- (BOOL)tableView:(NSTableView *)tableView shouldTrackCell:(NSCell *)cell forTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    if(tableView == [self listView] && [[tableColumn identifier] isEqualToString:@"listViewRating"])
    {
        // We only track the rating cell in selected rows...
        if(![[[self listView] selectedRowIndexes] containsIndex:row]) return NO;

        // ...if we know when the last selection change happened...
        if(!_listViewSelectionChangeDate) return NO;

        // ...and the selection happened a while ago, where 'a while' is the standard double click interval.
        // This means that the user has to click a row to select it, wait the standard double click internval
        // and then click the rating cell to change it. See issue #294.
        return [_listViewSelectionChangeDate timeIntervalSinceNow] < -[NSEvent doubleClickInterval];

    }
    return NO;
}

#pragma mark - NSTableView Interaction
- (void)tableViewWasDoubleClicked:(id)sender
{
    NSAssert(sender == [self listView], @"Sorry, but we're accepting listView senders only at this time");

    NSInteger row = [[self listView] clickedRow];
    if(row == -1) return;

    id game = [self tableView:sender objectValueForTableColumn:nil row:row];
    if(!game) return;

    [[self libraryController] startGame:game];
}

#pragma mark - ImageFlow Data Source
- (NSUInteger)numberOfItemsInImageFlow:(IKImageFlowView *)aBrowser
{
    return [[gamesController arrangedObjects] count];
}

- (id)imageFlow:(id)aFlowLayer itemAtIndex:(int)index
{
    return [[gamesController arrangedObjects] objectAtIndex:index];
}

#pragma mark - ImageFlow Delegates
- (NSUInteger)imageFlow:(IKImageFlowView *)browser writeItemsAtIndexes:(NSIndexSet *)indexes toPasteboard:(NSPasteboard *)pasteboard{ return 0; }

- (void)imageFlow:(IKImageFlowView *)sender removeItemsAtIndexes:(NSIndexSet *)indexes
{}

- (void)imageFlow:(IKImageFlowView *)sender cellWasDoubleClickedAtIndex:(NSInteger)index
{
    [[self libraryController] startSelectedGame:self];
}

- (void)imageFlow:(IKImageFlowView *)sender didSelectItemAtIndex:(NSInteger)index
{
    [[self listView] selectRowIndexes:[NSIndexSet indexSetWithIndex:[sender selectedIndex]] byExtendingSelection:NO];
    [[self listView] scrollRowToVisible:index];
}

@end
