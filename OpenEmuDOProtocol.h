//
//  OpenEmuDOProtocol.h
//  OpenEmu
//
//  Created by vade on 4/4/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@protocol OpenEmuDOProtocol

// eventually to handle updating inputs / pausing etc.
- (oneway void) setDelegate:(byref id)delegate;	// set who handles what.

//
//- (oneway void) setScreenSize:(NSSize) screenSize;
//- (NSSize) screenSize;


// IOSurface ids
- (oneway void) setSurfaceID:(in IOSurfaceID) surfaceID;
- (IOSurfaceID) surfaceID;

@end


// Distributed object that both the helpder and OpenEmu (and debug app) use
@interface OpenEmuDistributedObject : NSObject <OpenEmuDOProtocol>
{
	id delegate;
	
	IOSurfaceID surfaceID;
}

- (oneway void) setDelegate:(byref id)delegate;	// set who handles what.


// IOSurface ids
- (oneway void) setSurfaceID:(in IOSurfaceID) surfaceID;
- (IOSurfaceID) surfaceID;

@end
