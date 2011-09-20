//
//  OENewGameDocument.h
//  OpenEmu
//
//  Created by Carl Leimbrock on 05.09.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
@class OEHUDControlsWindow, OEGameView, OEGameCoreManager;
@class OEGameQTRecorder, OESystemController, OESystemResponder, OEGameCoreController;
@protocol OEGameCoreHelper;
@interface OENewGameDocument : NSObject{
	id rom;
	NSURL* url;
	
	OEHUDControlsWindow	*controlsWindow;
	// IPC from our OEHelper
    id<OEGameCoreHelper>  rootProxy;
    
    OEGameCoreManager    *gameCoreManager;
    
    // Standard game document stuff
    NSTimer              *frameTimer;
    OEGameQTRecorder     *recorder;
    NSString             *emulatorName;
    OEGameView           *gameView;
    OESystemController   *gameSystemController;
    OESystemResponder    *gameSystemResponder;
    OEGameCoreController *gameController;
    BOOL                  keyedOnce;
}

+ (id)newDocumentWithROM:(id)rom error:(NSError**)error;
+ (id)newDocumentWithRomAtURL:(NSURL*)url error:(NSError**)error;

- (void)openWindow:(NSRect)contentFrame;
- (OEGameView*)gameView;
#pragma mark -
- (void)resetGame;
- (void)terminateEmulation;

- (void)pauseGame;
- (void)playGame;
- (BOOL)isEmulationPaused;
- (void)setPauseEmulation:(BOOL)flag;
#pragma mark -
- (void)loadState:(id)state;
- (void)saveState:(NSString*)stateName; // alternatively: -(void)saveState:(OESaveState**)outState ?

- (BOOL)saveStateToToFile:(NSString*)fileName error:(NSError**)error;
- (BOOL)loadStateFromFile:(NSString*)fileName error:(NSError**)error;

#pragma mark -
- (void)captureScreenshotUsingBlock:(void(^)(NSImage* img))block;
#pragma mark -
#pragma mark Properties
@property (retain, readonly) id rom;
@property (retain, readonly) NSURL* url;
@end
