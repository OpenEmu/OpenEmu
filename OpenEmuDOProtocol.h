//
//  OpenEmuDOProtocol.h
//  OpenEmu
//
//  Created by vade on 4/4/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "GameCore.h"


// our helper app needs to handle these functions
@protocol OpenEmuDOProtocol

// control gamecore
- (oneway void)setVolume:(float)value;
- (oneway void)setPauseEmulation:(BOOL)flag;
- (oneway void)player:(NSUInteger)playerNumber didPressButton:(OEButton)button;
- (oneway void)player:(NSUInteger)playerNumber didReleaseButton:(OEButton)button;
- (void)postEvent:(NSEvent *)theEvent;

@property IOSurfaceID surfaceID;
- (oneway void)setSurfaceID:(IOSurfaceID)anID;

@end
