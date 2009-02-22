//
//  GameCoreBase.h
//  OpenEmu
//
//  Created by Joshua Weinberg on 2/21/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//


#define DEBUG_PRINT

#ifdef DEBUG_PRINT
#define DLog(format, ...) NSLog((format), ##__VA_ARGS__)
#else
#define DLog(format, ...) 1
#endif

#import <Cocoa/Cocoa.h>
#import "GameCore.h"
#import "GameButtons.h"

@class GameDocument;

@interface GameCoreBase : NSObject <GameCore> {
	NSThread* emulationThread;
	GameDocument* parent;
}

@end
