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

#import "OEGridView.h"

@interface OESavedGamesDataWrapper : NSObject
+ (id)wrapperWithState:(OEDBSaveState*)state;
+ (id)wrapperWithGame:(OEDBGame*)game;
@property (strong) OEDBGame *game;
@property (strong) OEDBSaveState *state;
@end

@interface OEMediaViewController ()
@property (strong) NSArray *groupRanges;
@property (strong) NSArray *items;
@end

@implementation OEMediaViewController
- (instancetype)init
{
    self = [super init];
    if (self)
    {}
    return self;
}
- (void)loadView
{
    [super loadView];

    [[self gridView] setAutomaticallyMinimizeRowMargin:YES];
    [[self gridView] setCellClass:[OEGridMediaItemCell class]];

    [self OE_setupToolbarStatesForViewTag:OEGridViewTag];
    [self OE_showView:OEGridViewTag];
}

- (void)viewDidAppear
{
    [super viewDidAppear];

    [self OE_setupToolbarStatesForViewTag:OEGridViewTag];
    [self OE_showView:OEGridViewTag];
}

- (void)setRepresentedObject:(id)representedObject
{
    //NSAssert([representedObject isKindOfClass:[OEMedia class]], @"Media View Controller can only represent OEMedia objects.");
    [super setRepresentedObject:representedObject];
    [self reloadData];

    [self OE_setupToolbarStatesForViewTag:OEGridViewTag];
    [self OE_showView:OEGridViewTag];
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
    [[controller toolbarGridViewButton] setEnabled:FALSE];
    [[controller toolbarFlowViewButton] setEnabled:FALSE];
    [[controller toolbarListViewButton] setEnabled:FALSE];
    
    [[controller toolbarSearchField] setEnabled:YES];
    [[controller toolbarSlider] setEnabled:YES];
}

#pragma mark -
- (BOOL)shouldShowBlankSlate
{
    return [[self groupRanges] count] == 0;
}

- (void)fetchItems
{
#pragma TODO(Improve group detection)
    if([self representedObject] != [OEDBSavedGamesMedia sharedDBSavedGamesMedia])
    {
        _groupRanges = @[];
        _items = @[];
        return;
    }

    NSManagedObjectContext *context = [[OELibraryDatabase defaultDatabase] mainThreadContext];

    NSMutableArray *ranges = [NSMutableArray array];
    NSArray *result = nil;

    NSFetchRequest *req = [[NSFetchRequest alloc] init];
    [req setEntity:[NSEntityDescription entityForName:@"SaveState" inManagedObjectContext:context]];
    [req setSortDescriptors:@[[NSSortDescriptor sortDescriptorWithKey:@"rom.game.gameTitle" ascending:YES],
                              [NSSortDescriptor sortDescriptorWithKey:@"timestamp" ascending:YES]]];
    [req setPredicate:[NSPredicate predicateWithFormat:@"rom.game != nil"]];

    NSError *error  = nil;
    if(!(result=[context executeFetchRequest:req error:&error]))
    {
        DLog(@"Error fetching save states");
        DLog(@"%@", error);
    }

    NSInteger i;
    if([result count] == 0)
    {
        return;
    }

    NSManagedObjectID *gameID = [[[[result objectAtIndex:0] rom] game] objectID];
    NSUInteger groupStart = 0;
    for(i=0; i < [result count]; i++)
    {
        OEDBSaveState *state = [result objectAtIndex:i];
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
#pragma mark - Context Menu
- (NSMenu*)menuForItemsAtIndexes:(NSIndexSet *)indexes
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

- (IBAction)showInFinder:(id)sender
{
    NSIndexSet *indexes = [self selectionIndexes];
    NSArray *saveStates = [[self items] objectsAtIndexes:indexes];
    NSArray *urls = [saveStates valueForKeyPath:@"URL.absoluteURL"];
    [[NSWorkspace sharedWorkspace] activateFileViewerSelectingURLs:urls];
}

- (void)deleteSelectedItems:(id)sender
{
    NSIndexSet *selection = [self selectionIndexes];
    NSArray *states = [[[self items] objectsAtIndexes:selection] copy];
    [states enumerateObjectsUsingBlock:^(OEDBSaveState *state, NSUInteger idx, BOOL *stop) {
        [state remove];
    }];
    [[[[self libraryController] database] mainThreadContext] save:nil];
}
#pragma mark - GridView DataSource
- (NSUInteger)numberOfGroupsInImageBrowser:(IKImageBrowserView *)aBrowser
{
    return [_groupRanges count];
}

- (id)imageBrowser:(IKImageBrowserView *)aBrowser itemAtIndex:(NSUInteger)index
{
        return [OESavedGamesDataWrapper wrapperWithState:[[self items] objectAtIndex:index]];
}

- (NSDictionary*)imageBrowser:(IKImageBrowserView *)aBrowser groupAtIndex:(NSUInteger)index
{
    NSValue  *groupRange = [[self groupRanges] objectAtIndex:index];
    NSRange range = [groupRange rangeValue];
    OEDBSaveState *firstState = [[self items] objectAtIndex:range.location];
    return @{
             IKImageBrowserGroupTitleKey : [[[firstState rom] game] gameTitle],
             IKImageBrowserGroupRangeKey : groupRange,
             IKImageBrowserGroupStyleKey : @(IKGroupDisclosureStyle),
             OEImageBrowserGroupSubtitleKey : [[[[firstState rom] game] system] lastLocalizedName]
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

    OEDBSaveState *state = [[self items] objectAtIndex:index];
    [state setName:title];
    [state moveToDefaultLocation];
    [state save];
}

- (void)imageBrowser:(IKImageBrowserView *)aBrowser cellWasDoubleClickedAtIndex:(NSUInteger)index
{
    [NSApp sendAction:@selector(startSaveState:) to:nil from:self];
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
+ (id)wrapperWithState:(OEDBSaveState*)state
{
    OESavedGamesDataWrapper *obj = [[self alloc] init];
    [obj setState:state];
    return obj;
}

+ (id)wrapperWithGame:(OEDBGame *)game
{
    OESavedGamesDataWrapper *obj = [[self alloc] init];
    [obj setGame:game];
    [obj setState:[[[game defaultROM] saveStates] anyObject]];
    return obj;
}

- (NSString *)imageUID
{
    return [[self state] location];
}

- (NSString *)imageRepresentationType
{
    return IKImageBrowserNSURLRepresentationType;
}

- (id)imageRepresentation
{
    return [[self state] screenshotURL];
}

- (NSString *)imageTitle
{
    if([self game])
        return [[self game] displayName];
    return [[self state] displayName];
}

- (NSString *)imageSubtitle
{
    if([self game])
    {
        NSUInteger count = [[[self game] defaultROM] saveStateCount];
        return [NSString stringWithFormat:@"%ld Save%s", count, count!=1 ? "s" : ""];
    }

    return [formatter stringFromDate:[[self state] timestamp]];
}
@end