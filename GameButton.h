//
//  GameButton.h
//  OpenEmu
//
//  Created by Josh Weinberg on 9/8/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#import "GameButtons.h"
#include <IOKit/hid/IOHIDUsageTables.h>

typedef enum 
{
	eAxis_None,
	eAxis_Positive,
	eAxis_Negative,
} eAxis;


@interface KeyboardButton : NSObject <NSCoding> {
	eButton_Type button;
	
	NSString* character;
	int keyCode;
}

@property(readwrite) eButton_Type button;
@property(readonly) NSString* character;
@property(readonly) int keyCode;


+ (NSString*) charForKey: (int) key;
- (id) initWithEvent:(NSEvent*) event;
- (id) initWithKeycode:(int) keyCode;
@end




@interface GameButton : NSObject <NSCoding> {
	int page;
	int usage;

	int value;
	int player;

	
	eAxis axis;
	
	eButton_Type button;
}

@property(readonly) int page;
@property(readonly) int usage;
@property(readonly) int player;
@property(readwrite) eButton_Type button;
@property(readonly) eAxis axis;


- (id) initWithPage: (int) aPage usage: (int) aUsage value: (int) aValue forButton: (eButton_Type) aButton player:(int) aPlayer;

@end
