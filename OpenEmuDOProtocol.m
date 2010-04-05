//
//  OpenEmuDOProtocol.m
//  OpenEmu
//
//  Created by vade on 4/4/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "OpenEmuDOProtocol.h"


@implementation OpenEmuDistributedObject

@synthesize delegate, surfaceID;

- (void) setVolume:(float)volume
{
	[delegate setVolume:volume];
}

- (void) setPauseEmulation:(BOOL)paused
{
	[delegate setPauseEmulation:paused];
}


- (oneway void)player:(NSUInteger)playerNumber didPressButton:(OEButton)button
{
	[delegate player:playerNumber didPressButton:button];
}

- (oneway void)player:(NSUInteger)playerNumber didReleaseButton:(OEButton)button
{
	[delegate player:playerNumber didReleaseButton:button];
	
}

@end
