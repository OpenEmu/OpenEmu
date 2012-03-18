/*
 Copyright (c) 2011, OpenEmu Team
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the OpenEmu Team nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#import <Cocoa/Cocoa.h>

@class OEDBRom;
@class OEDBGame;
@class OEDBSaveState;

@class OEGameView;
@protocol OEGameCoreHelper;
@class OEGameCoreController;
@class OEGameCoreManager;
@class OESystemController;
@class OESystemResponder;

@class OEHUDControlsBarWindow;
@class OEGameDocument;
@class OEDBSaveState;

@class OECorePlugin;
@protocol OEGameViewControllerDelegate;

@interface OEGameViewController : NSViewController
{
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

- (id)initWithRom:(OEDBRom *)rom;
- (id)initWithRom:(OEDBRom *)rom core:(OECorePlugin*)core;
- (id)initWithRom:(OEDBRom *)rom error:(NSError **)outError;
- (id)initWithRom:(OEDBRom *)rom core:(OECorePlugin*)core error:(NSError **)outError;
- (id)initWithGame:(OEDBGame *)game;
- (id)initWithGame:(OEDBGame *)game core:(OECorePlugin*)core;
- (id)initWithGame:(OEDBGame *)game error:(NSError **)outError;
- (id)initWithGame:(OEDBGame *)game core:(OECorePlugin*)core error:(NSError **)outError;

- (id)initWithSaveState:(OEDBSaveState *)state;
- (id)initWithSaveState:(OEDBSaveState *)state error:(NSError **)outError;

@property (strong) OEHUDControlsBarWindow *controlsWindow;

@property(weak)   id<OEGameViewControllerDelegate> delegate;

@property(strong) OEDBRom        *rom;
@property(weak)   OEGameDocument *document;

#pragma mark - Menu Items
- (IBAction)volumeUp:(id)sender;
- (IBAction)volumeDown:(id)sender;
- (IBAction)saveState:(id)sender;
- (IBAction)loadState:(id)sender;
- (IBAction)pauseEmulation:(id)sender;
- (IBAction)resumeEmulation:(id)sender;

#pragma mark -
#pragma mark Controlling Emulation
- (void)restartUsingCore:(OECorePlugin*)core;
- (void)resetGame;
- (void)terminateEmulation;

- (void)pauseGame;
- (void)playGame;
- (BOOL)isEmulationPaused;
- (void)setPauseEmulation:(BOOL)flag;
- (void)setVolume:(float)volume;

- (void)toggleFullscreen;
#pragma mark -
- (void)loadSaveState:(OEDBSaveState *)state;
- (void)deleteSaveState:(OEDBSaveState *)state;
- (void)saveStateAskingUserForName:(NSString*)proposedName;
- (void)saveStateWithName:(NSString*)stateName;
- (BOOL)loadStateFromFile:(NSString*)fileName error:(NSError**)error;

#pragma mark -
- (void)captureScreenshotUsingBlock:(void(^)(NSImage *img))block;

#pragma mark -
#pragma mark Info
- (NSSize)defaultScreenSize;
- (NSString*)coreIdentifier;
- (NSString*)systemIdentifier;
#pragma mark -
#pragma mark Menu Items
- (BOOL)validateMenuItem:(NSMenuItem *)menuItem;
- (void)menuItemAction:(id)sender;
- (void)setupMenuItems;
@end

@protocol OEGameViewControllerDelegate <NSObject>
@optional

- (void)emulationDidFinishForGameViewController:(OEGameViewController *)sender;
- (BOOL)gameViewControllerShouldToggleFullScreenMode:(OEGameViewController *)sender;

@end
