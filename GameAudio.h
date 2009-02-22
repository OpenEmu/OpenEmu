//
//  GameAudio.h
//  OpenEmu
//
//  Created by Josh Weinberg on 9/7/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#include <AudioToolbox/AudioToolbox.h>
#include <AudioUnit/AudioUnit.h>

@class GameCore;

@interface RenderCallbackData : NSObject
{
	GameCore* _core;
	UInt32 bufInPos, bufOutPos, bufUsed;
	
	UInt32 samplesFrame;
	UInt32 sizeSoundBuffer;
	UInt32 channels;
	UInt32 sampleRate;
	bool paused;
	NSLock* soundLock;
	UInt16* sndBuf;
	BOOL useRingBuffer;
}

@property(readwrite) UInt32 bufUsed;
@property(readwrite) UInt32 bufOutPos;
@property(readwrite) UInt32 bufInPos;
@property(readwrite) UInt32 samplesFrame;
@property(readwrite) UInt32 sizeSoundBuffer;
@property(readwrite) UInt32 channels;
@property(readwrite) UInt32 sampleRate;
@property(readonly) UInt16* sndBuf;
@property(readwrite) bool paused;
@property(readonly) BOOL useRingBuffer;
@property(readonly) GameCore* core;

- (id) initWithCore:(GameCore*) core;
- (void) lock;
- (void) unlock;
@end


@interface GameAudio : NSObject {
	GameCore* gameCore;
	AUGraph							mGraph;
	AUNode							mConverterNode, mMixerNode, mOutputNode;
	AudioUnit						mConverterUnit, mMixerUnit, mOutputUnit;
	
	RenderCallbackData* wrapper;
}

- (void) createGraph;
- (void) advanceBuffer;
- (id) initWithCore: (GameCore*) core;
- (void) startAudio;
- (void) stopAudio;
- (void) pauseAudio;
- (void) setVolume: (float) volume;
@end
