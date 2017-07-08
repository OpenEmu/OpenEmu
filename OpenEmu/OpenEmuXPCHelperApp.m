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

#import "OpenEmuXPCHelperApp.h"
#import "OESystemPlugin.h"
#import <OpenEmuSystem/OpenEmuSystem.h>
#import <OpenEmuXPCCommunicator/OpenEmuXPCCommunicator.h>

@interface OpenEmuXPCHelperApp () <NSXPCListenerDelegate, OEXPCGameCoreHelper>
{
    NSXPCListener *_mainListener;
    NSXPCConnection *_gameCoreConnection;
}

@end

@implementation OpenEmuXPCHelperApp

- (void)launchApplication
{
    NSApplication *application = [NSApplication sharedApplication];
    [application setDelegate:self];
    [application run];
}

- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
    [super applicationDidFinishLaunching:notification];

    _mainListener = [NSXPCListener anonymousListener];
    [_mainListener setDelegate:self];
    [_mainListener resume];

    NSXPCListenerEndpoint *endpoint = [_mainListener endpoint];
    [[OEXPCCAgent defaultAgent] registerListenerEndpoint:endpoint forIdentifier:[OEXPCCAgent defaultProcessIdentifier] completionHandler:^(BOOL success){}];
}

- (BOOL)listener:(NSXPCListener *)listener shouldAcceptNewConnection:(NSXPCConnection *)newConnection
{
    if(listener == _mainListener)
    {
        if(_gameCoreConnection != nil)
            return NO;

        _gameCoreConnection = newConnection;
        [_gameCoreConnection setExportedInterface:[NSXPCInterface interfaceWithProtocol:@protocol(OEXPCGameCoreHelper)]];
        [_gameCoreConnection setExportedObject:self];
        [_gameCoreConnection setRemoteObjectInterface:[NSXPCInterface interfaceWithProtocol:@protocol(OEGameCoreOwner)]];
        [_gameCoreConnection setInvalidationHandler:^{
            [NSApp terminate:nil];
        }];

        [_gameCoreConnection setInterruptionHandler:^{
            [NSApp terminate:nil];
        }];

        [_gameCoreConnection resume];

        self.gameCoreOwner = [_gameCoreConnection remoteObjectProxyWithErrorHandler:^(NSError *error) {
            [self stopEmulationWithCompletionHandler:^{}];
        }];

        return YES;
    }

    return NO;
}

- (void)loadROMAtPath:(NSString *)romPath romCRC32:(NSString *)romCRC32 romMD5:(NSString *)romMD5 romHeader:(NSString *)romHeader romSerial:(NSString *)romSerial systemRegion:(NSString *)systemRegion usingCorePluginAtPath:(NSString *)pluginPath systemPluginPath:(NSString *)systemPluginPath completionHandler:(void (^)(NSError *))completionHandler
{
    NSError *error;
    [self loadROMAtPath:romPath romCRC32:romCRC32 romMD5:romMD5 romHeader:romHeader romSerial:romSerial systemRegion:systemRegion withCorePluginAtPath:pluginPath systemPluginPath:systemPluginPath error:&error];

    completionHandler(error);
}

- (void)stopEmulationWithCompletionHandler:(void(^)(void))handler
{
    [super stopEmulationWithCompletionHandler:^{
        handler();
    }];
}

@end
