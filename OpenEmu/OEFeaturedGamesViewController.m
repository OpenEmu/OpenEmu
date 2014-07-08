//
//  OEFeaturedGamesViewController.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 09/07/14.
//
//

#import "OEFeaturedGamesViewController.h"

@interface OEFeaturedGamesViewController ()
@end

@implementation OEFeaturedGamesViewController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {}
    return self;
}

- (void)loadView
{
    self.view = [[NSView alloc] initWithFrame:NSZeroRect];

    [super loadView];
}

#pragma mark - State Handling
- (id)encodeCurrentState
{
    return nil;
}

- (void)restoreState:(id)state
{}

- (void)setLibraryController:(OELibraryController *)libraryController
{
    _libraryController = libraryController;

    [[libraryController toolbarFlowViewButton] setEnabled:NO];
    [[libraryController toolbarGridViewButton] setEnabled:NO];
    [[libraryController toolbarListViewButton] setEnabled:NO];

    [[libraryController toolbarSearchField] setEnabled:NO];

    [[libraryController toolbarSlider] setEnabled:NO];
}
@end
