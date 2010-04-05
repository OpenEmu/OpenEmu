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

- (void) setPlayPause:(BOOL)paused
{
	[delegate setPlayPause:paused];
}

@end
