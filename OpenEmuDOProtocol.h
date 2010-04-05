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
// FIXME: Needs a protocol for the delegate
@property(assign) id delegate;
- (oneway void)setDelegate:(byref id)anObject; // set who handles what methods for handling input to the gamecore

// control gamecore
// NOTE: only objects and pointers can be "byref", "in", "out" or "inout"
- (oneway void)setVolume:(float)value;
- (oneway void)setPlayPause:(BOOL)flag;

// IOSurface ids
@property IOSurfaceID surfaceID;
- (oneway void)setSurfaceID:(IOSurfaceID)anID;

@end




// Distributed object that both the helpder and OpenEmu (and debug app) use
// FIXME: the object to vend should be the application controller itself, or some other subordinate,
// not an artificial proxy, proxying is done by DO already.
@interface OpenEmuDistributedObject : NSObject <OpenEmuDOProtocol>
{
	id delegate;
	
	IOSurfaceID surfaceID;
}

@end
