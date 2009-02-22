//
//  GameQTRecorder.h
//  OpenEmu
//
//  Created by Joshua Weinberg on 2/15/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <QTKit/QTKit.h>
#import "GameCore.h"

@interface GameQTRecorder : NSObject {
	QTMovie * movie;
	id <GameCore> gameCore;
	bool recording;
	NSTimer* timer;
}

@property(readonly) bool recording;

- (id) initWithGameCore: (id <GameCore>) core;
- (void) startRecording;
- (void) timerCallInstallLoop;
- (void) addFrame;
- (void) finishRecording;
- (NSArray *)availableComponents;
- (NSData *)getExportSettings;
- (BOOL)writeMovieToFile:(NSString *)file withComponent:(NSDictionary *)component withExportSettings:(NSData *)exportSettings;

@end
