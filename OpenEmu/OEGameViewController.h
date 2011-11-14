//
//  OEGameViewController.h
//  OpenEmu
//
//  Created by Carl Leimbrock on 25.10.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OEMainWindowContentController.h"
@class OEDBRom;
@class OEDBGame;

@class OEGameView;
@protocol OEGameCoreHelper;
@class OEGameCoreController;
@class OEGameCoreManager;
@class OESystemController;
@class OESystemResponder;

@class OEHUDControlsWindow;
@class OEGameDocument;
@interface OEGameViewController : OEMainWindowContentController
{
    OEHUDControlsWindow *controlsWindow;
    // IPC from our OEHelper
    id<OEGameCoreHelper>  rootProxy;
    OEGameCoreManager    *gameCoreManager;
    
    // Standard game document stuff
    NSTimer              *frameTimer;
    // OEGameQTRecorder     *recorder;
    //NSString             *emulatorName;
    OEGameView           *gameView;
    OESystemController   *gameSystemController;
    
    OESystemResponder    *gameSystemResponder;
    OEGameCoreController *gameController;
    BOOL                  keyedOnce;
    
    BOOL emulationRunning;
}
- (id)initWithWindowController:(OEMainWindowController*)aWindowController andRom:(OEDBRom*)rom;
- (id)initWithWindowController:(OEMainWindowController*)aWindowController andRom:(OEDBRom*)rom error:(NSError**)outError;
- (id)initWithWindowController:(OEMainWindowController*)aWindowController andGame:(OEDBGame*)gam;
- (id)initWithWindowController:(OEMainWindowController*)aWindowController andGame:(OEDBGame*)gam error:(NSError**)outError;

- (void)contentWillShow;
- (void)contentWillHide;
#pragma mark -
#pragma mark Controlling Emulation
#pragma mark -
- (void)resetGame;
- (void)terminateEmulation;

- (void)pauseGame;
- (void)playGame;
- (BOOL)isEmulationPaused;
- (void)setPauseEmulation:(BOOL)flag;
- (void)setVolume:(float)volume;

- (void)toggleFullscreen;
#pragma mark -
- (void)loadState:(id)state;
- (void)deleteState:(id)state;
- (void)saveStateAskingUserForName:(NSString*)proposedName;
- (void)saveStateWithName:(NSString*)stateName;

- (BOOL)saveStateToToFile:(NSString*)fileName error:(NSError**)error;
- (BOOL)loadStateFromFile:(NSString*)fileName error:(NSError**)error;

#pragma mark -
- (void)captureScreenshotUsingBlock:(void(^)(NSImage* img))block;

#pragma mark -
#pragma mark Info
- (NSSize)defaultScreenSize;
#pragma mark -
#pragma mark Menu Items
- (BOOL)validateMenuItem:(NSMenuItem *)menuItem;
- (void)menuItemAction:(id)sender;
- (void)setupMenuItems;

#pragma mark -
@property (retain) OEDBRom* rom;
@property (assign) OEGameDocument* document;
@end
