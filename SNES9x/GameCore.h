/*
 *  GameCore.h
 *  OpenEmu
 *
 *  Created by Josh Weinberg on 9/1/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */


typedef enum{
	eButton_A,
	eButton_B,
	eButton_X,
	eButton_Y,
	eButton_START,
	eButton_SELECT,
	eButton_UP,
	eButton_DOWN,
	eButton_RIGHT,
	eButton_LEFT,
	eButton_L,
	eButton_R,
}eButton_Type;

@protocol GameCore

#pragma mark Exectuion
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

#pragma mark Input
- (void) buttonPressed: (int) gameButton forPlayer: (int) player;
- (void) buttonRelease: (int) gameButton forPlayer: (int) player;

#pragma mark Optional
@optional
- (void) saveState: (NSString *) fileName;
- (void) loadState: (NSString *) fileName;
- (void) requestAudio: (int) frames inBuffer: (void*)buf;
@end
