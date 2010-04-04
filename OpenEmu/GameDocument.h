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

@class GameCore;
@class GameAudio;
@class OEGameLayer;
@class GameQTRecorder;

@interface GameDocument : NSDocument
{
    NSTimer        *frameTimer;
    CALayer        *rootLayer;
    OEGameLayer    *gameLayer;
    GameQTRecorder *recorder;
    BOOL            keyedOnce;
    GameCore       *gameCore;
    GameAudio      *audio;
    NSString       *emulatorName;
    NSWindow       *gameWindow;
    NSView         *view;
	IBOutlet NSToolbarItem *playPauseToolbarItem;
}

@property(retain) IBOutlet NSWindow *gameWindow;
@property(retain) IBOutlet NSView   *view;

@property(readonly) GameCore *gameCore;
@property(getter=isEmulationPaused) BOOL pauseEmulation;
@property(readonly) BOOL isFullScreen;
@property(readonly) NSString *emulatorName;

- (void)scrambleBytesInRam:(NSUInteger)bytes;
- (void)refresh;
- (void)saveStateToFile:(NSString *)fileName;
- (void)loadStateFromFile:(NSString *)fileName;

- (IBAction)loadState:(id)sender;
- (IBAction)resetGame:(id)sender;
- (IBAction)saveState:(id)sender;
- (IBAction)scrambleRam:(id)sender;
- (IBAction)toggleFullScreen:(id)sender;
- (IBAction)playPauseGame:(id)sender;

- (BOOL)backgroundPauses;
- (BOOL)defaultsToFullScreenMode;

- (void)savePanelDidEnd:(NSSavePanel *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo;
- (void)openPanelDidEnd:(NSOpenPanel *)panel returnCode:(int)returnCode contextInfo:(void *)contextInfo;

- (NSImage *)screenShot;

@end
