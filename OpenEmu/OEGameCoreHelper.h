/*
 Copyright (c) 2013, OpenEmu Team

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
#import <CoreAudio/CoreAudio.h>
#import <OpenEmuSystem/OpenEmuSystem.h>

@class OEEvent;

@protocol OEGameCoreHelper <NSObject>

- (void)setVolume:(CGFloat)value;
- (void)setPauseEmulation:(BOOL)pauseEmulation;
- (void)setAudioOutputDeviceID:(AudioDeviceID)deviceID;

- (void)setupEmulationWithCompletionHandler:(void(^)(IOSurfaceID surfaceID, OEIntSize screenSize, OEIntSize aspectSize))handler;
- (void)startEmulationWithCompletionHandler:(void(^)(void))handler;
- (void)resetEmulationWithCompletionHandler:(void(^)(void))handler;
- (void)stopEmulationWithCompletionHandler:(void(^)(void))handler;

- (void)saveStateToFileAtPath:(NSString *)fileName completionHandler:(void (^)(BOOL success, NSError *error))block;
- (void)loadStateFromFileAtPath:(NSString *)fileName completionHandler:(void (^)(BOOL success, NSError *error))block;

- (void)setCheat:(NSString *)cheatCode withType:(NSString *)type enabled:(BOOL)enabled;
- (void)setDisc:(NSUInteger)discNumber;

- (void)handleMouseEvent:(OEEvent *)event;

- (void)setHandleEvents:(BOOL)handleEvents;
- (void)setHandleKeyboardEvents:(BOOL)handleKeyboardEvents;
- (void)systemBindingsDidSetEvent:(OEHIDEvent *)event forBinding:(__kindof OEBindingDescription *)bindingDescription playerNumber:(NSUInteger)playerNumber;
- (void)systemBindingsDidUnsetEvent:(OEHIDEvent *)event forBinding:(__kindof OEBindingDescription *)bindingDescription playerNumber:(NSUInteger)playerNumber;

@end

@protocol OEGameCoreOwner <NSObject>

- (void)saveState;
- (void)loadState;
- (void)quickSave;
- (void)quickLoad;
- (void)toggleFullScreen;
- (void)toggleAudioMute;
- (void)volumeDown;
- (void)volumeUp;
- (void)stopEmulation;
- (void)resetEmulation;
- (void)toggleEmulationPaused;
- (void)takeScreenshot;
- (void)fastForwardGameplay:(BOOL)enable;
- (void)rewindGameplay:(BOOL)enable;
- (void)stepGameplayFrameForward;
- (void)stepGameplayFrameBackward;

- (void)setEnableVSync:(BOOL)enable;
- (void)setAspectSize:(OEIntSize)newAspectSize;
- (void)setScreenSize:(OEIntSize)newScreenSize withIOSurfaceID:(IOSurfaceID)newSurfaceID;
- (void)setScreenSize:(OEIntSize)newScreenSize aspectSize:(OEIntSize)newAspectSize withIOSurfaceID:(IOSurfaceID)newSurfaceID;
- (void)setDiscCount:(NSUInteger)discCount;

@end
