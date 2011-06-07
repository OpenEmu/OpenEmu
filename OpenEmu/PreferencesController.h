//
//  PreferencesController.h
//  OEPreferencesMockup
//
//  Created by Christoph Leimbrock on 17.05.11.
//  Copyright 2011 none. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class OEBackgroundImageView;
@class OEBackgroundGradientView;
@class OEToolbarView;
@interface PreferencesController : NSWindowController {
@private
	OEToolbarView* toolbar;
	IBOutlet NSView *library, *controls, *gameplay, *cores;
	
	IBOutlet NSPopUpButton* consolesPopupButton;
	IBOutlet OEBackgroundGradientView* coreGradientOverlayView;
}

- (IBAction)showLibrary:(id)sender;
- (IBAction)showGameplay:(id)sender;
- (IBAction)showControls:(id)sender;
- (IBAction)showCores:(id)sender;
@end
