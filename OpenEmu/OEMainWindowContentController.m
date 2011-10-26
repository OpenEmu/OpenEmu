//
//  OEMainWindowSubviewController.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 21.10.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OEMainWindowContentController.h"

@implementation OEMainWindowContentController
@synthesize windowController;

- (id)initWithWindowController:(OEMainWindowController*)aWindowController
{
    self = [super initWithNibName:[self nibName] bundle:[NSBundle mainBundle]];
    if (self) 
    {
        [self setWindowController:aWindowController];
    }
    return self;
}

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) 
    {
    }
    return self;
}


- (void)contentWillShow
{
}
- (void)contentWillHide
{}
#pragma mark -
#pragma mark Menu Items
- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
    return NO;
}
- (void)menuItemAction:(id)sender
{}
- (void)setupMenuItems
{}
@end
