//
//  GameAudio.h
//  OpenEmu
//
//  Created by Josh Weinberg on 9/7/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <AudioToolbox/AudioToolbox.h>
#import <AudioUnit/AudioUnit.h>

@class GameCore;

@interface GameAudio : NSObject {
	GameCore* gameCore;
	AUGraph							mGraph;
	AUNode							mConverterNode, mMixerNode, mOutputNode;
	AudioUnit						mConverterUnit, mMixerUnit, mOutputUnit;
}

- (id) initWithCore: (GameCore*) core;

- (void) createGraph;
- (void) startAudio;
- (void) stopAudio;
- (void) pauseAudio;
- (void) setVolume: (float) volume;

@end
