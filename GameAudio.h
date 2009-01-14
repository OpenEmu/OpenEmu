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


#define AUDIOBUFFERS 4
#define SAMPLEFRAME (48000 * 4389/262144 + 2)
#define SIZESOUNDBUFFER SAMPLEFRAME*4

@interface GameAudio : NSObject {

}

- (void) advanceBuffer;
- (id) initWithCore: (id <GameCore>) core;
- (void) startAudio;
- (void) stopAudio;
- (void) pauseAudio;
- (void) setVolume: (float) volume;
@end
