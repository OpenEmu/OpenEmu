//
//  OEMainWindowController.h
//  OpenEmu
//
//  Created by Carl Leimbrock on 21.10.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class OEMainWindowContentController;
@interface OEMainWindowController : NSWindowController
{
    OEMainWindowContentController* currentContentController;
}
#pragma mark -
@property (retain, nonatomic) OEMainWindowContentController* currentContentController;
@property (retain) OEMainWindowContentController* defaultContentController;

#pragma mark -
#pragma mark Menu Items
- (void)setupMenuItems;
- (BOOL)validateMenuItem:(NSMenuItem *)menuItem;
- (void)menuItemAction:(id)sender;

#pragma mark -
#pragma mark Toolbar Elements
@property (retain) IBOutlet NSButton* toolbarSidebarButton;
@property (retain) IBOutlet NSButton* toolbarGridViewButton;
@property (retain) IBOutlet NSButton* toolbarFlowViewButton;
@property (retain) IBOutlet NSButton* toolbarListViewButton;

@property (retain) IBOutlet NSButton* toolbarAddToSidebarButton;
@property (retain) IBOutlet NSSearchField* toolbarSearchField;
@property (retain) IBOutlet NSSlider* toolbarSlider;
@end