//
//  OEUIDrawingUtils.h
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 19.04.11.
//  Copyright 2011 none. All rights reserved.
//

#import <Foundation/Foundation.h>

typedef enum {
	OEUIStateInactive,
	OEUIStateEnabled,
	OEUIStateActive,
	OEUIStatePressed,
	
	OEUIStateUnfocusedUnselected,
	OEUIStateUnfocusedSelected,
	OEUIStateFocusedUnselected,
	OEUIStateFocusedSelected,
} OEUIState;

// rounds a rect to avoid sub pixels
CGRect RoundCGRect(CGRect imageFrame);
NSRect RoundNSRect(NSRect imageFrame);