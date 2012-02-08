//
//  OEMenuTestController.h
//  OpenEmu
//
//  Created by Carl Leimbrock on 07.02.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "OEMenu.h"
#import "OEPopupButton.h"

@interface OEMenuTestController : NSWindowController
@property (retain) IBOutlet NSButton* centerButton;
@property (retain) IBOutlet NSButton* topButton;
@property (retain) IBOutlet NSButton* leftButton;
@property (retain) IBOutlet NSButton* rightButton;
@property (retain) IBOutlet NSButton* bottomButton;

@property (retain) IBOutlet NSMatrix* styleSelection;
@property (retain) IBOutlet NSMenu* testMenu;

@property (retain) IBOutlet OEPopupButton* filterSelection;

- (IBAction)toggleWindowBackground:(id)sender;
- (IBAction)buttonAction:(id)sender;
- (IBAction)styleAction:(id)sender;
@end
