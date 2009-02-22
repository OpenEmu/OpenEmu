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
- (BOOL) load: (NSString*) path withParent: (NSDocument*) newParent;
- (BOOL) loadFile: (NSString*) path;

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

#pragma mark Input
- (void) buttonPressed: (int) gameButton forPlayer: (int) player;
- (void) buttonRelease: (int) gameButton forPlayer: (int) player;

#pragma mark Optional
@optional
- (void) saveState: (NSString *) fileName;
- (void) loadState: (NSString *) fileName;
- (void) requestAudio: (int) frames inBuffer: (void*)buf;
- (NSSize) outputSize;
- (void) setRandomByte;
@end
