//
//  OEMainWindowToolbarViewController.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 06.12.12.
//
//

#import "OEMainWindowToolbarViewController.h"

@interface OEMainWindowToolbarViewController ()

@end

@implementation OEMainWindowToolbarViewController

- (void)awakeFromNib
{
    [super awakeFromNib];
    [self disableAllItems];
}

- (void)disableAllItems
{
    [[self toolbarSidebarButton] setEnabled:NO];
    
    [[self toolbarGridViewButton] setEnabled:NO];
    [[self toolbarFlowViewButton] setEnabled:NO];
    [[self toolbarListViewButton] setEnabled:NO];
    
    [[self toolbarAddToSidebarButton] setEnabled:NO];
    [[self toolbarSearchField] setEnabled:NO];
    [[self toolbarSlider] setEnabled:NO];
}

@end
