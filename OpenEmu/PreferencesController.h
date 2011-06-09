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
	NSArray* preferencePanes;
	OEToolbarView* toolbar;	
	
	IBOutlet OEBackgroundGradientView* coreGradientOverlayView;
}

- (void)switchView:(id)sender;

@property (retain, readwrite) NSArray* preferencePanes;
@end
