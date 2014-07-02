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

#import "OERetrodeViewController.h"
#import "OERetrode.h"

#import "OEGridGameCell.h"
#import "OEGameInfoHelper.h"
#import "OEDBDataSourceAdditions.h"
@interface OERetrodeGame : NSObject
+ (instancetype)gameWithDictionary:(NSDictionary*)dict;
@property NSURL *url;
@property NSString *title;
@property NSString *path;
@property NSInteger gridRating;
@end

@interface OERetrodeViewController () <NSFileManagerDelegate>
@property (strong) NSMutableArray *items;
@end

@implementation OERetrodeViewController
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
    [[self gridView] setCellClass:[OEGridGameCell class]];

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
    NSLog(@"%@", [representedObject className]);
    NSAssert([representedObject isKindOfClass:[OERetrode class]], @"Retrode View Controller can only represent OERetrode objects.");
    [super setRepresentedObject:representedObject];
    [representedObject setDelegate:self];

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
    return [[self items] count] == 0;
}

- (void)fetchItems
{
#pragma TODO(Improve group detection)
    OERetrode *retrode = (OERetrode *)[self representedObject];

    NSArray *games = [retrode games];
    NSMutableArray *previousItems = [[self items] copy];
    _items = [NSMutableArray array];
    [games enumerateObjectsUsingBlock:^(NSDictionary *dict, NSUInteger idx, BOOL *stop) {
        NSString *path = [dict valueForKey:@"path"];

        __block OERetrodeGame *item = nil;
        [previousItems enumerateObjectsUsingBlock:^(OERetrodeGame *obj, NSUInteger idx, BOOL *stop) {
            if([[obj path] isEqualToString:path])
            {
                *stop = YES;
                item = obj;
            }
        }];

        if(item == nil)
        {
            item = [OERetrodeGame gameWithDictionary:dict];
            dispatch_async(dispatch_get_main_queue(), ^{
                NSDictionary *dict = @{@"URL" : [NSURL fileURLWithPath:[item path]]};
                NSDictionary *result = [[OEGameInfoHelper sharedHelper] gameInfoWithDictionary:dict];
                NSString *gameTitle = [result objectForKey:@"gameTitle"];
                if(gameTitle != nil)
                    [item setTitle:gameTitle];
                NSString *imageURLString = [result objectForKey:@"boxImageURL"];
                if(imageURLString != nil)
                    [item setUrl:[NSURL URLWithString:imageURLString]];
                [self reloadData];
            });
        }

        [_items addObject:item];
    }];
    DLog(@"%@", _items);
}
#pragma mark - Context Menu
- (NSMenu*)menuForItemsAtIndexes:(NSIndexSet *)indexes
{
    NSMenu *menu = [[NSMenu alloc] init];

    if([indexes count] == 1)
    {
        [menu addItemWithTitle:@"Import Game" action:@selector(deleteSelectedItems:) keyEquivalent:@""];
        [menu addItemWithTitle:@"Show in Finder" action:@selector(showInFinder:) keyEquivalent:@""];
    }
    else
    {
        [menu addItemWithTitle:@"Import Games" action:@selector(deleteSelectedItems:) keyEquivalent:@""];
        [menu addItemWithTitle:@"Show in Finder" action:@selector(showInFinder:) keyEquivalent:@""];
    }

    return [menu numberOfItems] != 0 ? menu : nil;
}

- (IBAction)showInFinder:(id)sender
{
    NSIndexSet *indexes = [self selectionIndexes];
    NSArray *saveStates = [[self items] objectsAtIndexes:indexes];
    NSMutableArray *urls = [NSMutableArray array];

    [saveStates enumerateObjectsUsingBlock:^(OERetrodeGame *g, NSUInteger idx, BOOL *stop) {
        [urls addObject:[NSURL fileURLWithPath:[g path]]];
    }];

    [[NSWorkspace sharedWorkspace] activateFileViewerSelectingURLs:urls];
}

#pragma mark - GridView DataSource
- (id)imageBrowser:(IKImageBrowserView *)aBrowser itemAtIndex:(NSUInteger)index
{
    return [[self items] objectAtIndex:index];
}

- (NSUInteger)numberOfItemsInImageBrowser:(IKImageBrowserView *)aBrowser
{
    return [[self items] count];
}

- (NSUInteger)numberOfGroupsInImageBrowser:(IKImageBrowserView *)aBrowser
{
    return 1;
}

- (NSDictionary*)imageBrowser:(IKImageBrowserView *)aBrowser groupAtIndex:(NSUInteger)index
{
    return @{
             IKImageBrowserGroupTitleKey : OELocalizedString(@"Slot 1", @"Retrode Slot 1 Group Header"),
             IKImageBrowserGroupRangeKey : [NSValue valueWithRange:(NSRange){0, [[self items] count]}],
             IKImageBrowserGroupStyleKey : @(IKGroupDisclosureStyle),
             };
}

#pragma mark - Retrode Delegate
- (void)retrodeDidDisconnect:(OERetrode*)retrode
{
    DLog();
}
- (void)retrodeDidRescanGames:(OERetrode*)retrode
{
    DLog();
    [self reloadData];
}
- (void)retrodeHardwareDidBecomeAvailable:(OERetrode*)retrode
{
    DLog();
}
- (void)retrodeHardwareDidBecomeUnavailable:(OERetrode*)retrode
{
    DLog();
}

- (void)retrodeDidMount:(OERetrode*)retrode
{
    DLog();
}
- (void)retrodeDidUnmount:(OERetrode*)retrode
{
    DLog();
}

- (void)retrodeDidEnterDFUMode:(OERetrode*)retrode
{
    DLog();
}
- (void)retrodeDidLeaveDFUMode:(OERetrode*)retrode
{
    DLog();
}

@end

@implementation OERetrodeGame

+ (instancetype)gameWithDictionary:(NSDictionary*)dict
{
    OERetrodeGame *game = [[OERetrodeGame alloc] init];
    [game setPath:[dict valueForKey:@"path"]];
    [game setTitle:[dict valueForKey:@"gameTitle"]];

    return game;
}

- (NSInteger)gridStatus
{
    return 0;
}

- (NSString *)imageRepresentationType
{
    return [self url] ? IKImageBrowserNSURLRepresentationType : IKImageBrowserNSImageRepresentationType;
}

- (NSString*)imageTitle
{
    return [self title] ?: @"";
}

- (id)imageRepresentation
{
    return [self url] ?: [OEDBGame artworkPlacholderWithAspectRatio:1.365385];
}

- (NSString*)imageUID
{
    return [[self url] absoluteString] ?: @":MissingArtwork(1.365385)";
}

@end