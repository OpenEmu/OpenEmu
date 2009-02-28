//
//  OEGamePreferenceController_Toolbar.h
//  OpenEmu
//
//  Created by Remy Demarest on 25/02/2009.
//  Copyright 2009 Psycho Inc.. All rights reserved.
//

#import "OEGamePreferenceController.h"

@interface OEGamePreferenceController (Toolbar)
- (void)setupToolbarForWindow:(NSWindow *)theWindow;
- (void)switchView:(id)sender;
- (NSRect)frameForNewContentViewFrame:(NSRect)viewFrame;
- (NSView *)viewForIdentifier:(NSString*)identifier;
@end
