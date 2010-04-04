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
