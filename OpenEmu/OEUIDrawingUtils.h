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
	
	OEUIStateRollover,
	
	OEUIStateUnfocusedUnselected,
	OEUIStateUnfocusedSelected,
	OEUIStateFocusedUnselected,
	OEUIStateFocusedSelected,
} OEUIState;

typedef enum {
	OEButtonStateSelectedDisabled,
	OEButtonStateUnselectedDisabled,
	
	OEButtonStateSelectedHover,
	OEButtonStateUnselectedHover,
	
	OEButtonStateSelectedPressed,
	OEButtonStateUnselectedPressed,
	
	OEButtonStateSelectedInactive,
	OEButtonStateUnselectedInactive,
	
	OEButtonStateSelectedNormal,
	OEButtonStateUnselectedNormal
} OEButtonState;
