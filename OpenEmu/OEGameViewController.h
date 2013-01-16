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

extern NSString *const OEGameVolumeKey;
extern NSString *const OEGameVideoFilterKey;
extern NSString *const OEGameCoresInBackgroundKey;
extern NSString *const OEDontShowGameTitleInWindowKey;
extern NSString *const OEAutoSwitchCoreAlertSuppressionKey;
extern NSString *const OEForceCorePicker;
extern NSString *const OEGameViewControllerEmulationWillFinishNotification;
extern NSString *const OEGameViewControllerEmulationDidFinishNotification;
extern NSString *const OEGameViewControllerROMKey;


@class OEDBRom;
@class OEDBGame;
@class OEDBSaveState;

@class OEGameView;
@protocol OEGameCoreHelper;
@class OEGameCoreController;
@class OEGameCoreManager;
@class OESystemController;
@class OESystemResponder;

@class OEGameControlsBar;
@class OEGameDocument;
@class OEDBSaveState;

@class OECorePlugin;
@protocol OEGameViewControllerDelegate;
@protocol OEGameIntegralScalingDelegate;

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
    BOOL                  keyedOnce;
    
    NSTimer* gameViewTransitionTimer;
}

- (id)initWithRom:(OEDBRom *)rom;
- (id)initWithRom:(OEDBRom *)rom core:(OECorePlugin *)core;
- (id)initWithRom:(OEDBRom *)rom error:(NSError **)outError;
- (id)initWithRom:(OEDBRom *)rom core:(OECorePlugin *)core error:(NSError **)outError;
- (id)initWithGame:(OEDBGame *)game;
- (id)initWithGame:(OEDBGame *)game core:(OECorePlugin *)core;
- (id)initWithGame:(OEDBGame *)game error:(NSError **)outError;
- (id)initWithGame:(OEDBGame *)game core:(OECorePlugin *)core error:(NSError **)outError;

- (id)initWithSaveState:(OEDBSaveState *)state;
- (id)initWithSaveState:(OEDBSaveState *)state error:(NSError **)outError;
#pragma mark -
@property(strong) OEGameControlsBar *controlsWindow;
@property(readonly) OEGameView *gameView;

@property(unsafe_unretained) id<OEGameViewControllerDelegate> delegate;
@property(unsafe_unretained) id<OEGameIntegralScalingDelegate> integralScalingDelegate;

@property(strong) OEDBRom        *rom;
@property(weak)   OEGameDocument *document;


#pragma mark - HUD Bar Actions
// switchCore:: expects sender or [sender representedObject] to be an OECorePlugin object and prompts the user for confirmation
- (void)switchCore:(id)sender;
- (void)editControls:(id)sender;
- (void)selectFilter:(id)sender;

#pragma mark - Volume
- (void)setVolume:(float)volume asDefault:(BOOL)defaultFlag;
- (void)changeVolume:(id)sender;
- (IBAction)volumeUp:(id)sender;
- (IBAction)volumeDown:(id)sender;
- (void)mute:(id)sender;
- (void)unmute:(id)sender;

#pragma mark - Controlling Emulation
- (void)resetGame;
- (void)terminateEmulation;

- (IBAction)pauseGame:(id)sender;
- (IBAction)playGame:(id)sender;
- (void)toggleEmulationPaused;
- (void)setPauseEmulation:(BOOL)pauseEmulation;
- (BOOL)isEmulationRunning;

#pragma mark - Saving States
- (IBAction)saveState:(id)sender;
- (IBAction)quickSave:(id)sender;
- (void)saveStateWithName:(NSString *)stateName;

#pragma mark - Loading States
// loadState: expects sender or [sender representedObject] to be an OEDBSaveState object
- (IBAction)loadState:(id)sender;
- (IBAction)quickLoad:(id)sender;

#pragma mark -
// deleteSaveState: expects sender or [sender representedObject] to be an OEDBSaveState object and prompts the user for confirmation
- (void)deleteSaveState:(id)sender;

#pragma mark - Info
- (NSSize)defaultScreenSize;
- (NSString*)coreIdentifier;
- (NSString*)systemIdentifier;

#pragma mark -
#pragma mark Menu Items
- (BOOL)validateMenuItem:(NSMenuItem *)menuItem;

@end


#pragma mark -
#pragma mark Delegate
@protocol OEGameViewControllerDelegate <NSObject>
@optional
- (void)emulationWillFinishForGameViewController:(OEGameViewController *)sender;
- (void)emulationDidFinishForGameViewController:(OEGameViewController *)sender;
- (BOOL)gameViewControllerShouldToggleFullScreenMode:(OEGameViewController *)sender;
@end
