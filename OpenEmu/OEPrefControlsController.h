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
@property (retain) IBOutlet NSView	 *controllerView;

@property (retain) IBOutlet NSPopUpButton	 *consolesPopupButton;
@property (retain) IBOutlet NSPopUpButton	 *playerPopupButton;
@property (retain) IBOutlet NSPopUpButton	 *inputPopupButton;

@property (retain) IBOutlet OEBackgroundGradientView *gradientOverlay;
@property (retain) IBOutlet NSView *controlsContainer;
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
