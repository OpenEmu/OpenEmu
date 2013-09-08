/*
 Copyright (c) 2010, OpenEmu Team

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
#import <OpenEmuBase/OpenEmuBase.h>
#import "OEGameCoreHelper.h"

@class OEGameCoreController;
@protocol OEDOGameCoreHelperDelegate, OEDOGameCoreDisplayHelper;

// our helper app needs to handle these functions
@protocol OEDOGameCoreHelper <NSObject>

- (oneway void)setVolume:(CGFloat)value;
- (oneway void)setPauseEmulation:(BOOL)pauseEmulation;
- (oneway void)setAudioOutputDeviceID:(AudioDeviceID)deviceID;
- (oneway void)setDrawSquarePixels:(BOOL)drawSquarePixels;

- (oneway void)setupEmulationWithDelegate:(byref id<OEDOGameCoreHelperDelegate>)delegate;
- (oneway void)startEmulationWithDelegate:(byref id<OEDOGameCoreHelperDelegate>)delegate;
- (oneway void)resetEmulationWithDelegate:(byref id<OEDOGameCoreHelperDelegate>)delegate;
- (oneway void)stopEmulationWithDelegate:(byref id<OEDOGameCoreHelperDelegate>)delegate;

- (oneway void)saveStateToFileAtPath:(NSString *)fileName withDelegate:(byref id<OEDOGameCoreHelperDelegate>)delegate;
- (oneway void)loadStateFromFileAtPath:(NSString *)fileName withDelegate:(byref id<OEDOGameCoreHelperDelegate>)delegate;

- (oneway void)setCheat:(NSString *)cheatCode withType:(NSString *)type enabled:(BOOL)enabled;

- (oneway void)loadROMAtPath:(bycopy NSString *)romPath usingCorePluginAtPath:(bycopy NSString *)corePluginPath systemPluginAtPath:(bycopy NSString *)systemPluginPath withDelegate:(byref id<OEDOGameCoreHelperDelegate>)delegate displayHelper:(byref id<OEDOGameCoreDisplayHelper>)displayHelper;

@end

@protocol OEDOGameCoreDisplayHelper <NSObject>

- (oneway void)setEnableVSync:(BOOL)enable;
- (oneway void)setScreenSize:(OEIntSize)newScreenSize withIOSurfaceID:(IOSurfaceID)newSurfaceID;
- (oneway void)setAspectSize:(OEIntSize)newAspectSize;
- (oneway void)setFrameInterval:(NSTimeInterval)newFrameInterval;

@end

@protocol OEDOGameCoreHelperDelegate <NSObject>

- (oneway void)setSystemResponderClient:(byref id)responderClient;

- (oneway void)gameCoreHelperDidSetupEmulationWithSurfaceID:(IOSurfaceID)surfaceID screenSize:(OEIntSize)screenSize aspectSize:(OEIntSize)aspectSize;
- (oneway void)gameCoreHelperDidStartEmulation;
- (oneway void)gameCoreHelperDidResetEmulation;
- (oneway void)gameCoreHelperDidStopEmulation;

- (oneway void)gameCoreHelperDidSaveState:(BOOL)success error:(NSError *)error;
- (oneway void)gameCoreHelperDidLoadState:(BOOL)success error:(NSError *)error;

@end
