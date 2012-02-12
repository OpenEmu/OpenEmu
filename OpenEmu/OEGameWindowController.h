//
//  OEGameWindowController.h
//  OpenEmu
//
//  Created by Remy Demarest on 12/02/2012.
//  Copyright (c) 2012 NuLayer Inc. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class OEGameViewController;

@interface OEGameWindowController : NSWindowController

- (id)initWithGameViewController:(OEGameViewController *)aController contentRect:(NSRect)aRect;

@property(strong, readonly) OEGameViewController *gameViewController;

- (IBAction)terminateEmulation:(id)sender;

@end
