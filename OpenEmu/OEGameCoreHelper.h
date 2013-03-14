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
#import "OEGameCore.h"

@class OEGameCoreController;

@protocol OEGameCoreHelperDelegate <NSObject>
- (void) gameCoreDidChangeScreenSizeTo:(OEIntSize)screenSize;
- (void) gameCoreDidChangeAspectSizeTo:(OEIntSize)aspectSize;
- (void) toggleVSync:(GLint)swapInt;
- (void) setPauseEmulation:(BOOL)paused;
@end

// our helper app needs to handle these functions
@protocol OEGameCoreHelper <NSObject>

// control gamecore
- (oneway void)setVolume:(float)value;
- (oneway void)volumeUp;
- (oneway void)volumeDown;
- (oneway void)setPauseEmulation:(BOOL)flag;

// gamecore attributes
@property(readonly) OEIntSize   screenSize; 
                                            
@property(readonly) OEIntSize   aspectSize;
@property(readonly) BOOL isEmulationPaused;

@property(readwrite) BOOL drawSquarePixels;
@property(readonly) IOSurfaceID surfaceID;
@property(weak) id <OEGameCoreHelperDelegate> delegate;

- (byref OEGameCore *)gameCore;

- (BOOL)loadRomAtPath:(bycopy NSString *)aPath withCorePluginAtPath:(bycopy NSString *)pluginPath;
- (void)setupEmulation;
- (void)stopEmulation;

- (BOOL)saveStateToFileAtPath:(NSString *)fileName;
- (BOOL)loadStateFromFileAtPath:(NSString *)fileName;

- (void)setCheat:(NSString *)code setType:(NSString *)type setEnabled:(BOOL)enabled;

@end
