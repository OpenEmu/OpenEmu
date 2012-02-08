//
//  OEPrefControlsController.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 07.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "OEPreferencePane.h"

@class OEBackgroundGradientView;
@class OESystemPlugin;
@interface OEPrefControlsController : NSViewController <OEPreferencePane> {
	OESystemPlugin *selectedPlugin;
}

#pragma mark -
@property (strong) IBOutlet NSView	 *controllerView;

@property (strong) IBOutlet NSPopUpButton	 *consolesPopupButton;
@property (strong) IBOutlet NSPopUpButton	 *playerPopupButton;
@property (strong) IBOutlet NSPopUpButton	 *inputPopupButton;

@property (strong) IBOutlet OEBackgroundGradientView *gradientOverlay;
@property (strong) IBOutlet NSView *controlsContainer;
#pragma mark -
#pragma UI Methods
- (IBAction)changeSystem:(id)sender;
- (IBAction)changePlayer:(id)sender;
- (IBAction)changeInputDevice:(id)sender;

#pragma mark -
- (NSImage*)icon;
- (NSString*)title;

- (NSSize)viewSize;
@end
