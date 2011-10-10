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
@class OEDBRom;
@protocol OEGameCoreHelper;
@class OENewGameDocument;
@protocol OENewGameDocumentDelegateProtocol <NSObject>
@optional
- (void)gameDocumentDidTerminateEmulation:(OENewGameDocument*)doc;
@end
@interface OENewGameDocument : NSObject <OENewGameDocumentDelegateProtocol>{
	OEDBRom* rom;
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
    
    id<OENewGameDocumentDelegateProtocol>   _delegate;
    
    BOOL emulationRunning;
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
- (void)setVolume:(float)volume;
#pragma mark -
- (void)loadState:(id)state;
- (void)saveStateAskingUser:(NSString*)proposedName;
- (void)saveState:(NSString*)stateName; // alternatively: -(void)saveState:(OESaveState**)outState ?

- (BOOL)saveStateToToFile:(NSString*)fileName error:(NSError**)error;
- (BOOL)loadStateFromFile:(NSString*)fileName error:(NSError**)error;

#pragma mark -
- (void)captureScreenshotUsingBlock:(void(^)(NSImage* img))block;
#pragma mark -
#pragma mark Properties
@property (retain, readonly) OEDBRom* rom;
@property (retain, readonly) NSURL* url;

@property (assign) id<OENewGameDocumentDelegateProtocol> delegate;
@end

