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

#import "OEDBSaveState.h"
#import "OEDBGame.h"
#import "OEDBRom.h"
#import "OEDBSystem.h"
#import "OEDBScreenshot.h"

#import "OEGridView.h"

#import "OEHUDAlert+DefaultAlertsAdditions.h"

@interface OESavedGamesDataWrapper : NSObject
+ (id)wrapperWithItem:(id)item;
+ (id)wrapperWithState:(OEDBSaveState*)state;
+ (id)wrapperWithScreenshot:(OEDBScreenshot*)screenshot;
@property (strong) OEDBSaveState  *state;
@property (strong) OEDBScreenshot *screenshot;
@end

@interface OEMediaViewController ()
@property (strong) NSArray *groupRanges;
@property (strong) NSArray *items;

@property BOOL saveStateMode;

@property BOOL shouldShowBlankSlate;
@property (strong) NSPredicate *searchPredicate;
@end

@implementation OEMediaViewController
- (instancetype)init
{
    self = [super init];
    if (self)
    {
        _searchPredicate = [NSPredicate predicateWithValue:YES];
    }
    return self;
}
- (void)loadView
{
    [super loadView];

    [[self gridView] setAutomaticallyMinimizeRowMargin:YES];
    [[self gridView] setCellClass:[OEGridMediaItemCell class]];
}

- (void)viewDidAppear
{
    [super viewDidAppear];
}

- (void)setRepresentedObject:(id)representedObject
{
    [super setRepresentedObject:representedObject];

    [self setSaveStateMode:[representedObject isKindOfClass:[OEDBSavedGamesMedia class]]];
    [self reloadData];
}
#pragma mark - OELibrarySubviewController Implementation
- (id)encodeCurrentState
{
    return nil;
}

- (void)restoreState:(id)state
{
}

- (NSArray*)selectedGames
{
    return @[];
}

- (NSArray*)selectedSaveStates
{
    NSIndexSet *indices = [self selectionIndexes];
    return [[self items] objectsAtIndexes:indices];
}

- (NSIndexSet*)selectionIndexes
{
    return [[self gridView] selectionIndexes];
}

- (void)setLibraryController:(OELibraryController *)controller
{
    [super setLibraryController:controller];
    
    [[controller toolbarGridViewButton] setEnabled:FALSE];
    [[controller toolbarFlowViewButton] setEnabled:FALSE];
    [[controller toolbarListViewButton] setEnabled:FALSE];
    
    [[controller toolbarSearchField] setEnabled:YES];
    [[controller toolbarSlider] setEnabled:YES];
}

#pragma mark -
- (void)search:(id)sender
{
    NSString *searchTerm = [[[self libraryController] toolbarSearchField] stringValue];
    NSMutableArray *predarray = [NSMutableArray array];
    NSArray *tokens = [searchTerm componentsSeparatedByCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
    NSArray *keys = @[@"rom.game.gameTitle", @"rom.game.name", @"rom.game.system.lastLocalizedName", @"name", @"userDescription"];
    for(NSString *token in tokens)
        if(token.length > 0)
            for(NSString *key in keys)
                [predarray addObject:[NSPredicate predicateWithFormat:@"%K contains[cd] %@", key, token]];

    if([predarray count])
        _searchPredicate = [NSCompoundPredicate orPredicateWithSubpredicates:predarray];
    else
        _searchPredicate = [NSPredicate predicateWithValue:YES];

    [self reloadData];
}

- (void)fetchItems
{
#pragma TODO(Improve group detection)
    NSManagedObjectContext *context = [[OELibraryDatabase defaultDatabase] mainThreadContext];

    NSMutableArray *ranges = [NSMutableArray array];
    NSArray *result = nil;

    NSFetchRequest *req = [[NSFetchRequest alloc] init];
    [req setEntity:[NSEntityDescription entityForName:[self OE_entityName] inManagedObjectContext:context]];

    _shouldShowBlankSlate = [context countForFetchRequest:req error:nil] == 0;
    if(_shouldShowBlankSlate)
    {
        _items       = @[];
        _groupRanges = @[];

        [self updateBlankSlate];
        return;
    }

    [req setSortDescriptors:@[[NSSortDescriptor sortDescriptorWithKey:@"rom.game.gameTitle" ascending:YES],
                              [NSSortDescriptor sortDescriptorWithKey:@"timestamp" ascending:YES]]];
    [req setPredicate:_searchPredicate];

    NSError *error  = nil;
    if(!(result=[context executeFetchRequest:req error:&error]))
    {
        DLog(@"Error fetching save states");
        DLog(@"%@", error);
    }

    NSInteger i;
    if([result count] == 0)
    {
        _groupRanges = @[];
        _items = @[];

        [self updateBlankSlate];
        return;
    }

    NSManagedObjectID *gameID = [[[[result objectAtIndex:0] rom] game] objectID];
    NSUInteger groupStart = 0;
    for(i=0; i < [result count]; i++)
    {
        id state = [result objectAtIndex:i];
        if(![[[[state rom] game] objectID] isEqualTo:gameID])
        {
            [ranges addObject:[NSValue valueWithRange:NSMakeRange(groupStart, i-groupStart)]];
            groupStart = i;
            gameID = [[[state rom] game] objectID];
        }
    }

    if(groupStart != i)
    {
        [ranges addObject:[NSValue valueWithRange:NSMakeRange(groupStart, i-groupStart)]];
    }
    _groupRanges = ranges;
    _items = result;
}

- (NSString*)OE_entityName
{
    return [self saveStateMode] ? [OEDBSaveState entityName] : [OEDBScreenshot entityName];
}
#pragma mark - Context Menu
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
        [menu addItemWithTitle:@"Play Save State" action:@selector(startSaveState:) keyEquivalent:@""];
        [menu addItemWithTitle:@"Rename" action:@selector(beginEditingWithSelectedItem:) keyEquivalent:@""];
        [menu addItemWithTitle:@"Show in Finder" action:@selector(showInFinder:) keyEquivalent:@""];
        [menu addItemWithTitle:@"Delete Save State" action:@selector(deleteSelectedItems:) keyEquivalent:@""];
    }
    else
    {
        [menu addItemWithTitle:@"Show in Finder" action:@selector(showInFinder:) keyEquivalent:@""];
        [menu addItemWithTitle:@"Delete Save States" action:@selector(deleteSelectedItems:) keyEquivalent:@""];
    }

    return [menu numberOfItems] != 0 ? menu : nil;
}


- (NSMenu*)OE_screenshotMenuForItensAtIndexes:(NSIndexSet *)indexes
{

    NSMenu *menu = [[NSMenu alloc] init];

    if([indexes count] == 1)
    {
        [menu addItemWithTitle:@"Rename" action:@selector(beginEditingWithSelectedItem:) keyEquivalent:@""];
        [menu addItemWithTitle:@"Show in Finder" action:@selector(showInFinder:) keyEquivalent:@""];
        [menu addItemWithTitle:@"Delete Screenshot" action:@selector(deleteSelectedItems:) keyEquivalent:@""];
    }
    else
    {
        [menu addItemWithTitle:@"Show in Finder" action:@selector(showInFinder:) keyEquivalent:@""];
        [menu addItemWithTitle:@"Delete Screenshots" action:@selector(deleteSelectedItems:) keyEquivalent:@""];
    }

    return menu;
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

        if([alert runModal] == NSAlertDefaultReturn)
        {
            [items makeObjectsPerformSelector:@selector(remove)];
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

        if([alert runModal] == NSAlertDefaultReturn)
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
    return @{
             IKImageBrowserGroupTitleKey : [game gameTitle] ?: [game displayName],
             IKImageBrowserGroupRangeKey : groupRange,
             IKImageBrowserGroupStyleKey : @(IKGroupDisclosureStyle),
             OEImageBrowserGroupSubtitleKey : [system lastLocalizedName] ?: [system name]
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
        if(![saveState isSpecialState] || [[OEHUDAlert renameSpecialStateAlert] runModal] == NSAlertDefaultReturn)
        {
            [saveState setName:title];
            [saveState moveToSaveStateFolder];

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
    id item = [[self items] objectAtIndex:index];

    if([item isKindOfClass:[OEDBSaveState class]])
    {
        [NSApp sendAction:@selector(startSaveState:) to:nil from:self];
    }
    else
    {
        NSLog(@"No action for items of %@ type", [item className]);
    }
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

+ (id)wrapperWithItem:(id)item
{
    if([item isKindOfClass:[OEDBSaveState class]])
        return [self wrapperWithState:item];
    else if([item isKindOfClass:[OEDBScreenshot class]])
        return [self wrapperWithScreenshot:item];
    return nil;
}

+ (id)wrapperWithState:(OEDBSaveState*)state
{
    OESavedGamesDataWrapper *obj = [[self alloc] init];
    [obj setState:state];
    return obj;
}

+ (id)wrapperWithScreenshot:(OEDBScreenshot*)screenshot;
{
    OESavedGamesDataWrapper *obj = [[self alloc] init];
    [obj setScreenshot:screenshot];
    return obj;
}

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