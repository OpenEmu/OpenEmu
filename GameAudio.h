//
//  GameAudio.h
//  OpenEmu
//
//  Created by Josh Weinberg on 9/7/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <AudioToolbox/AudioToolbox.h>
#import "GameCore.h"
#import <Foundation/Foundation.h>

#define AUDIOBUFFERS 4
#define SAMPLEFRAME (48000 * 4389/262144 + 2)
#define SIZESOUNDBUFFER SAMPLEFRAME*4

@interface AQCallbackWrapper : NSObject
{
	AudioQueueRef					queue;
	UInt32							frameCount;
	UInt32 bufInPos, bufOutPos, bufUsed;
	AudioQueueBufferRef				mBuffers[AUDIOBUFFERS];
	AudioStreamBasicDescription		mDataFormat;

	UInt32 samplesFrame;
	UInt32 sizeSoundBuffer;
	UInt32 channels;
	UInt32 sampleRate;
	
	NSLock* soundLock;
	UInt16* sndBuf;
}

@property(readwrite) UInt32 bufUsed;
@property(readwrite) UInt32 bufOutPos;
@property(readwrite) UInt32 bufInPos;
@property(readwrite) UInt32 samplesFrame;
@property(readwrite) UInt32 sizeSoundBuffer;
@property(readwrite) UInt32 channels;
@property(readwrite) UInt32 sampleRate;
@property(readwrite) UInt32 frameCount;
@property(readonly) 	UInt16* sndBuf;
@property(readonly) AudioQueueRef queue;

- (id) initWithCore:(id <GameCore>) core;
- (void) lock;
- (void) unlock;
@end


@interface GameAudio : NSObject {
	id <GameCore> gameCore;
	AQCallbackWrapper* wrapper;
}

- (void) advanceBuffer;
- (id) initWithCore: (id <GameCore>) core;
- (void) startAudio;
- (void) stopAudio;
- (void) pauseAudio;
- (void) setVolume: (float) volume;
@end
