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
@private
	IBOutlet NSImageView	* controllerView;
	
    IBOutlet NSPopUpButton	* consolesPopupButton;
    IBOutlet NSPopUpButton	* playerPopupButton;
	IBOutlet NSPopUpButton	* inputPopupButton;
	
	IBOutlet OEBackgroundGradientView* gradientOverlay;
	IBOutlet NSView* controlsContainer;
	
	OESystemPlugin* selectedPlugin;
}

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
