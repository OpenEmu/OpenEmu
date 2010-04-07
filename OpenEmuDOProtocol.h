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

// gamecore attributes
@property(readonly) NSUInteger screenWidth;
@property(readonly) NSUInteger screenHeight;
@property(readonly) NSUInteger bufferWidth;
@property(readonly) NSUInteger bufferHeight;
@property(readonly) BOOL isEmulationPaused;

@property(readonly) CGRect sourceRect;

@property IOSurfaceID surfaceID;
- (oneway void)setSurfaceID:(IOSurfaceID)anID;

@end
