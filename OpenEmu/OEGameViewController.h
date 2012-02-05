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

#import "OEMainWindowContentController.h"
@class OEDBRom;
@class OEDBGame;

@class OEGameView;
@protocol OEGameCoreHelper;
@class OEGameCoreController;
@class OEGameCoreManager;
@class OESystemController;
@class OESystemResponder;

@class OEHUDControlsBarWindow;
@class OEGameDocument;
@interface OEGameViewController : OEMainWindowContentController
{
    OEHUDControlsBarWindow *controlsWindow;
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
- (void)captureScreenshotUsingBlock:(void(^)(NSImage *img))block;

#pragma mark -
#pragma mark Info
- (NSSize)defaultScreenSize;
#pragma mark -
#pragma mark Menu Items
- (BOOL)validateMenuItem:(NSMenuItem *)menuItem;
- (void)menuItemAction:(id)sender;
- (void)setupMenuItems;

#pragma mark -
@property (retain) OEDBRom *rom;
@property (assign) OEGameDocument *document;
@end
