//
//  OEXPCGameCoreHelper.h
//  OpenEmu
//
//  Created by Remy Demarest on 20/07/2013.
//
//

#import <Foundation/Foundation.h>
#import "OEGameCoreHelper.h"

@protocol OEXPCGameCoreHelper <OEGameCoreHelper, NSObject>

- (void)setVolume:(CGFloat)value;
- (void)setPauseEmulation:(BOOL)pauseEmulation;
- (void)setAudioOutputDeviceID:(AudioDeviceID)deviceID;
- (void)setDrawSquarePixels:(BOOL)drawSquarePixels;

- (void)setupEmulationWithCompletionHandler:(void(^)(IOSurfaceID surfaceID, OEIntSize screenSize, OEIntSize aspectSize))handler;
- (void)startEmulationWithCompletionHandler:(void(^)(void))handler;
- (void)resetEmulationWithCompletionHandler:(void(^)(void))handler;
- (void)stopEmulationWithCompletionHandler:(void(^)(void))handler;

- (void)saveStateToFileAtPath:(NSString *)fileName completionHandler:(void (^)(BOOL success, NSError *error))block;
- (void)loadStateFromFileAtPath:(NSString *)fileName completionHandler:(void (^)(BOOL success, NSError *error))block;

- (void)setCheat:(NSString *)cheatCode withType:(NSString *)type enabled:(BOOL)enabled;

- (void)loadROMAtPath:(NSString *)romPath usingCorePluginAtPath:(NSString *)pluginPath systemPluginPath:(NSString *)systemPluginPath completionHandler:(void(^)(NSXPCListenerEndpoint *gameCoreEndpoint, NSError *error))completionHandler;

@end
