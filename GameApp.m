//
//  GameApp.m
//  OpenEmu
//
//  Created by Josh Weinberg on 9/13/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import "GameApp.h"
#import "GameInput.h"
#import "GameDocumentController.h"

@implementation GameApp

- (void)sendEvent:(NSEvent *)theEvent
{
//NSLog(@"Events!!!");

	
	
	if([[GameDocumentController sharedDocumentController] isGameKey] && [inputController handlesEvent: theEvent])
	{
		switch ([theEvent type]) {
			case NSKeyUp:
				//NSLog(@"Key up");
				[inputController keyUp:theEvent];
				break;
			case NSKeyDown:
				[inputController keyDown:theEvent];
				break;
			default:
				break;
				//NSLog(@"What gives?");
		}
	}
	else
	{
		[super sendEvent:theEvent];
	}
}

@end
