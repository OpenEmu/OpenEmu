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

@interface OELibraryGamesViewController ()
@end

@implementation OELibraryGamesViewController

- (void)viewDidLoad {
    [super viewDidLoad];
}

- (void)awakeFromNib {
    [self _assignLibraryDatabase];
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
    return @[];
}


- (void)setLibraryController:(OELibraryController *)libraryController
{
    _libraryController = libraryController;
    [self _assignLibraryDatabase];
}

- (void)_assignLibraryDatabase {
    [[self sidebarController] setDatabase:[_libraryController database]];
    [[self gameCollectionController] setLibraryController:_libraryController];
}

@end
