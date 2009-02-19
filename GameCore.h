/*
 *  GameCore.h
 *  OpenEmu
 *
 *  Created by Josh Weinberg on 9/1/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

@protocol GameCore <NSObject>

#pragma mark Execution
- (void) pause: (bool) b;
- (void) stop;
- (void) start;
- (void) reset;
- (void) executeFrame;

- (void) setup;
- (bool) load: (NSString*) path withParent: (NSDocument*) newParent;


#pragma mark Video
- (int) width;
- (int) height;
- (unsigned char*) buffer;
- (GLenum) pixelFormat;
- (GLenum) pixelType;
- (GLenum) internalPixelFormat;

#pragma mark Audio
- (UInt16*) sndBuf;
- (int) channels;
- (int) samplesFrame;
- (int) sizeSoundBuffer;
- (int) sampleRate;

- (void) setRandomByte;
#pragma mark Input
- (void) buttonPressed: (int) gameButton forPlayer: (int) player;
- (void) buttonRelease: (int) gameButton forPlayer: (int) player;

#pragma mark Optional
@optional
- (void) saveState: (NSString *) fileName;
- (void) loadState: (NSString *) fileName;
- (NSSize) outputSize;
- (void) setRandomByte;
@end
