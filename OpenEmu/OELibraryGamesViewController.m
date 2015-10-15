//
//  OELibraryGamesViewController.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 10/14/15.
//
//

#import "OELibraryGamesViewController.h"

#import "OEGameCollectionViewController.h"
#import "OESidebarController.h"

@interface OELibraryGamesViewController () <OELibrarySplitViewDelegate>
@end

@implementation OELibraryGamesViewController

- (void)viewDidLoad {
    [super viewDidLoad];
}

- (void)awakeFromNib {
    [self _assignLibraryController];

    NSNotificationCenter *noc = [NSNotificationCenter defaultCenter];
    [noc addObserver:self selector:@selector(_updateCollectionContentsFromSidebar:) name:OESidebarSelectionDidChangeNotificationName object:[self sidebarController]];

    NSView *collectionView = [[self collectionController] view];
    NSView *collectionViewContainer = [self collectionViewContainer];

    [collectionView setFrame:[collectionViewContainer bounds]];
    [collectionView setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];
    [collectionViewContainer addSubview:collectionView];

    [self addChildViewController:[self sidebarController]];
    [self addChildViewController:[self collectionController]];

}
#pragma mark - OELibrarySubviewController
- (id)encodeCurrentState {
    return nil;
}

- (void)restoreState:(id)state{
    return;
}

- (NSArray*)selectedGames
{
    return [[self collectionController] selectedGames];
}

- (void)setLibraryController:(OELibraryController *)libraryController
{
    _libraryController = libraryController;
    [self _assignLibraryController];
}

- (void)_assignLibraryController {
    [[self sidebarController] setDatabase:[_libraryController database]];
    [[self collectionController] setLibraryController:_libraryController];
}

#pragma mark - Sidebar handling
- (void)_updateCollectionContentsFromSidebar:(id)sender {
    id sleectedItem = [[self sidebarController] selectedSidebarItem];
    [[self collectionController] setRepresentedObject:sleectedItem];
}

@end
