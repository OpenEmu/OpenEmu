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
@interface OEPreferencesController : NSWindowController 
{
@private
	OEToolbarView *toolbar;	
	
	IBOutlet OEBackgroundGradientView *coreGradientOverlayView;
	
	NSView *nextView;
}

- (void)switchView:(id)sender;
- (void)switchView:(id)sender animate:(BOOL)animateFlag;

@property (strong, readwrite) NSArray *preferencePanes;
@end
