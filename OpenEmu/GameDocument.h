/*
 Copyright (c) 2009, OpenEmu Team
 All rights reserved.
 
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
#import <QuartzCore/CoreAnimation.h>
#import "TaskWrapper.h"

@class OEGameCoreController;
@class OEGameView;
@class GameQTRecorder;
@protocol OEGameCoreHelper;

extern NSString *const OEGameDocumentErrorDomain;

enum _OEGameDocumentErrorCodes 
{
    OENoError                      =  0,
    OEFileDoesNotExistError        = -1,
    OEIncorrectFileError           = -2,
    OEHelperAppNotRunningError     = -3,
    OEConnectionTimedOutError      = -4,
    OEInvalidHelperConnectionError = -5,
    OENilRootProxyObjectError      = -6,
};

@interface GameDocument : NSDocument <TaskWrapperController>
{
    // IPC from our OEHelper
    TaskWrapper          *helper;
    NSString             *taskUUIDForDOServer;
    NSConnection         *taskConnection;
    
    id<OEGameCoreHelper>  rootProxy;
    
    // Standard game document stuff
    NSTimer              *frameTimer;
    GameQTRecorder       *recorder;
    NSString             *emulatorName;
    NSWindow             *gameWindow;
    OEGameView           *view;
    OEGameCoreController *gameController;
    NSToolbarItem        *playPauseToolbarItem;
    BOOL                  keyedOnce;
}

@property(retain) IBOutlet NSToolbarItem *playPauseToolbarItem;
@property(retain) IBOutlet NSWindow      *gameWindow;
@property(retain) IBOutlet OEGameView    *view;

@property(getter=isEmulationPaused) BOOL pauseEmulation;
@property(readonly) BOOL isFullScreen;
@property(readonly) NSString *emulatorName;

// new task stuff
- (BOOL)startHelperProcessError:(NSError **)outError;
- (void)endHelperProcess;

- (void)scrambleBytesInRam:(NSUInteger)bytes;
- (void)refresh DEPRECATED_ATTRIBUTE;
- (void)saveStateToFile:(NSString *)fileName;
- (void)loadStateFromFile:(NSString *)fileName;

- (void)captureScreenshotUsingBlock:(void(^)(NSImage *img))block;

- (IBAction)loadState:(id)sender;
- (IBAction)resetGame:(id)sender;
- (IBAction)saveState:(id)sender;
- (IBAction)scrambleRam:(id)sender;
- (IBAction)toggleFullScreen:(id)sender;
- (IBAction)playPauseGame:(id)sender;

- (BOOL)backgroundPauses;
- (BOOL)defaultsToFullScreenMode;

- (NSImage *)screenShot DEPRECATED_ATTRIBUTE;
- (void)applicationWillTerminate:(NSNotification *)aNotification;

@end
