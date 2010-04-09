//
//  MednafenGameEmu.h
//  Mednafen
//
//  Created by Joshua Weinberg on 7/3/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <GameCore.h>
#import <OEHIDEvent.h>
#include <vector>
#include "mednafen.h"

@class OERingBuffer;

#define GBA_CONTROL_COUNT 17
OE_EXTERN NSString *MednafenControlNames[];

@interface MednafenGameCore : GameCore {
	unsigned char *videoBuffer;
	unsigned char *tempBuffer;
	NSLock        *soundLock;
	NSLock        *bufLock;
	uint16        *sndBuf;
    int            oldrun;
	int            position;

    //NSUInteger     buttons[SMSButtonCount];
    //NSUInteger     keycodes[SMSButtonCount];
	BOOL           paused;
	MDFNGI	*		gameInfo;
	MDFN_Rect *		lineWidths;
	std::vector<MDFNSetting> * driverSettings;
	int16 *sound;
	int32 ssize;
}

@end
