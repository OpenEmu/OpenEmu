//
//  OEGamePreferenceController_Toolbar.h
//  OpenEmu
//
//  Created by Remy Demarest on 25/02/2009.
//  Copyright 2009 Psycho Inc.. All rights reserved.
//

#import "OEGamePreferenceController.h"

@interface OEGamePreferenceController (Toolbar)
- (void)setupToolbar;
- (void)switchView:(id)sender;
- (NSRect)frameForNewContentViewFrame:(NSRect)viewFrame;
- (NSViewController *)newViewControllerForIdentifier:(NSString*)identifier;
@end
