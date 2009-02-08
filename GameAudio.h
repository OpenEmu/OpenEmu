//
//  GameAudio.h
//  OpenEmu
//
//  Created by Josh Weinberg on 9/7/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "GameCore.h"
#import <Foundation/Foundation.h>
#include <AudioToolbox/AudioToolbox.h>
#include <AudioUnit/AudioUnit.h>

@interface RenderCallbackData : NSObject
{
	UInt32 bufInPos, bufOutPos, bufUsed;
	
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
@property(readonly) UInt16* sndBuf;

- (id) initWithCore:(id <GameCore>) core;
- (void) lock;
- (void) unlock;
@end


@interface GameAudio : NSObject {
	id <GameCore> gameCore;
	AUGraph							mGraph;
	AUNode							mConverterNode, mMixerNode, mOutputNode;
	AudioUnit						mConverterUnit, mMixerUnit, mOutputUnit;
	
	RenderCallbackData* wrapper;
}

- (void) createGraph;
- (void) advanceBuffer;
- (id) initWithCore: (id <GameCore>) core;
- (void) startAudio;
- (void) stopAudio;
- (void) pauseAudio;
- (void) setVolume: (float) volume;
@end
