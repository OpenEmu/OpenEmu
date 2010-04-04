//
//  OpenEmuDOProtocol.m
//  OpenEmu
//
//  Created by vade on 4/4/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "OpenEmuDOProtocol.h"


@implementation OpenEmuDistributedObject

- (oneway void) setDelegate:(byref id)d
{
	delegate = d;
}

// control gamecore
- (oneway void) setVolume:(byref float)volume
{
	[delegate setVolume:volume];
}

- (oneway void) setPlayPause:(byref BOOL)paused
{
	[delegate setPlayPause:paused];
}


// IOSurface ids
- (oneway void) setSurfaceID:(in IOSurfaceID) sID
{
	surfaceID = sID;
}

- (IOSurfaceID) surfaceID
{
	return surfaceID;
}

@end
