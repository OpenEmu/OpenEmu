//
//  OEMainWindowSubviewController.h
//  OpenEmu
//
//  Created by Carl Leimbrock on 21.10.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "OEMainWindowController.h"
@interface OEMainWindowContentController : NSViewController
@property (assign) OEMainWindowController *windowController;
- (id)initWithWindowController:(OEMainWindowController*) aWindowController;

- (void)contentWillShow;
- (void)contentWillHide;
#pragma mark -
#pragma mark Menu Items
- (BOOL)validateMenuItem:(NSMenuItem *)menuItem;
- (void)menuItemAction:(id)sender;
- (void)setupMenuItems;
@end
