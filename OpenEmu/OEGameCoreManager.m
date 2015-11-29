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

#import "OEGameCoreManager.h"
#import "OECorePlugin.h"
#import "OEGameCoreManager_Internal.h"
#import "OESystemPlugin.h"

#import <OpenEmuBase/OpenEmuBase.h>

NSString * const OEGameCoreErrorDomain = @"OEGameCoreErrorDomain";

@implementation OEGameCoreManager

- (instancetype)initWithROMPath:(NSString *)romPath romCRC32:(NSString *)romCRC32 romMD5:(NSString *)romMD5 romHeader:(NSString *)romHeader romSerial:(NSString *)romSerial systemRegion:(NSString *)systemRegion corePlugin:(OECorePlugin *)plugin systemPlugin:(OESystemPlugin *)systemPlugin gameCoreOwner:(id<OEGameCoreOwner>)gameCoreOwner
{
    if((self = [super init]))
    {
        _ROMPath          = romPath;
        _ROMCRC32         = romCRC32;
        _ROMMD5           = romMD5;
        _ROMHeader        = romHeader;
        _ROMSerial        = romSerial;
        _systemRegion     = systemRegion;
        _plugin           = plugin;
        _systemPlugin     = systemPlugin;
        _gameCoreOwner    = gameCoreOwner;
    }

    return self;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@ %p, ROM: %@, System: %@, Core: %@, Display Helper: %@>", [self class], self, _ROMPath, [_plugin bundleIdentifier], [_systemPlugin systemIdentifier], _gameCoreOwner];
}

- (void)stop
{
    [self doesNotImplementSelector:_cmd];
}

- (void)loadROMWithCompletionHandler:(void(^)(void))completionHandler errorHandler:(void(^)(NSError *))errorHandler;
{
    [self doesNotImplementSelector:_cmd];
}

- (void)setVolume:(CGFloat)value;
{
    [[self gameCoreHelper] setVolume:value];
}

- (void)setPauseEmulation:(BOOL)pauseEmulation;
{
    [[self gameCoreHelper] setPauseEmulation:pauseEmulation];
}

- (void)setAudioOutputDeviceID:(AudioDeviceID)deviceID;
{
    [[self gameCoreHelper] setAudioOutputDeviceID:deviceID];
}

- (void)setCheat:(NSString *)cheatCode withType:(NSString *)type enabled:(BOOL)enabled
{
    [[self gameCoreHelper] setCheat:cheatCode withType:type enabled:enabled];
}

- (void)setDisc:(NSUInteger)discNumber
{
    [[self gameCoreHelper] setDisc:discNumber];
}

- (void)setupEmulationWithCompletionHandler:(void(^)(IOSurfaceID surfaceID, OEIntSize screenSize, OEIntSize aspectSize))handler;
{
    [[self gameCoreHelper] setupEmulationWithCompletionHandler:
     ^(IOSurfaceID surfaceID, OEIntSize screenSize, OEIntSize aspectSize)
     {
         dispatch_async(dispatch_get_main_queue(), ^{
             handler(surfaceID, screenSize, aspectSize);
         });
     }];
}

- (void)startEmulationWithCompletionHandler:(void(^)(void))handler;
{
    [[self gameCoreHelper] startEmulationWithCompletionHandler:
     ^{
         dispatch_async(dispatch_get_main_queue(), ^{
             handler();
         });
     }];
}

- (void)resetEmulationWithCompletionHandler:(void(^)(void))handler;
{
    [[self gameCoreHelper] resetEmulationWithCompletionHandler:
     ^{
         dispatch_async(dispatch_get_main_queue(), ^{
             handler();
         });
     }];
}

- (void)stopEmulationWithCompletionHandler:(void(^)(void))handler;
{
    [[self gameCoreHelper] stopEmulationWithCompletionHandler:
     ^{
         dispatch_async(dispatch_get_main_queue(), ^{
             handler();
             [self stop];
         });
     }];
}

- (void)saveStateToFileAtPath:(NSString *)fileName completionHandler:(void (^)(BOOL success, NSError *error))block;
{
    [[self gameCoreHelper] saveStateToFileAtPath:fileName completionHandler:
     ^(BOOL success, NSError *error)
     {
         dispatch_async(dispatch_get_main_queue(), ^{
             block(success, error);
         });
     }];
}

- (void)loadStateFromFileAtPath:(NSString *)fileName completionHandler:(void (^)(BOOL success, NSError *error))block;
{
    [[self gameCoreHelper] loadStateFromFileAtPath:fileName completionHandler:
     ^(BOOL success, NSError *error)
     {
         dispatch_async(dispatch_get_main_queue(), ^{
             block(success, error);
         });
     }];
}

- (void)handleMouseEvent:(OEEvent *)event
{
    [self.gameCoreHelper handleMouseEvent:event];
}

- (void)setHandleEvents:(BOOL)handleEvents
{
    [self.gameCoreHelper setHandleEvents:handleEvents];
}

- (void)setHandleKeyboardEvents:(BOOL)handleKeyboardEvents
{
    [self.gameCoreHelper setHandleKeyboardEvents:handleKeyboardEvents];
}

- (void)systemBindingsDidSetEvent:(OEHIDEvent *)event forBinding:(__kindof OEBindingDescription *)bindingDescription playerNumber:(NSUInteger)playerNumber
{
    [self.gameCoreHelper systemBindingsDidSetEvent:event forBinding:bindingDescription playerNumber:playerNumber];
}

- (void)systemBindingsDidUnsetEvent:(OEHIDEvent *)event forBinding:(__kindof OEBindingDescription *)bindingDescription playerNumber:(NSUInteger)playerNumber
{
    [self.gameCoreHelper systemBindingsDidUnsetEvent:event forBinding:bindingDescription playerNumber:playerNumber];
}

@end
