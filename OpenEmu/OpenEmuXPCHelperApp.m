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
#import <FeedbackReporter/FRFeedbackReporter.h>
#import <OpenEmuXPCCommunicator/OpenEmuXPCCommunicator.h>

@interface OpenEmuXPCHelperApp () <NSXPCListenerDelegate, OEXPCGameCoreHelper>
{
    NSXPCListener *_mainListener;
    NSXPCConnection *_gameCoreConnection;

    NSXPCListener *_systemListener;
    NSXPCConnection *_systemConnection;

    Protocol *_systemResponderClientProtocol;
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
        if(_gameCoreConnection != nil) return NO;

        _gameCoreConnection = newConnection;
        [_gameCoreConnection setExportedInterface:[NSXPCInterface interfaceWithProtocol:@protocol(OEXPCGameCoreHelper)]];
        [_gameCoreConnection setExportedObject:self];
        [_gameCoreConnection setRemoteObjectInterface:[NSXPCInterface interfaceWithProtocol:@protocol(OEGameCoreDisplayHelper)]];
        [_gameCoreConnection resume];

        [self setDisplayHelper:
         [_gameCoreConnection remoteObjectProxyWithErrorHandler:
          ^(NSError *error)
          {
              [self stopEmulationWithCompletionHandler:^{}];
          }]];
        return YES;
    }
    else if(listener == _systemListener)
    {
        if(_systemConnection != nil) return NO;

        _systemConnection = newConnection;
        [_systemConnection setExportedInterface:[NSXPCInterface interfaceWithProtocol:_systemResponderClientProtocol]];
        [_systemConnection setExportedObject:[self gameCoreProxy]];
        [_systemConnection resume];
        return YES;
    }

    return NO;
}

- (void)loadROMAtPath:(NSString *)romPath usingCorePluginAtPath:(NSString *)pluginPath systemPluginPath:(NSString *)systemPluginPath completionHandler:(void (^)(NSXPCListenerEndpoint *, NSError *))completionHandler
{
    OESystemPlugin *plugin = [OESystemPlugin systemPluginWithBundleAtPath:systemPluginPath];

    if(![self loadROMAtPath:romPath withCorePluginAtPath:pluginPath systemIdentifier:[plugin systemIdentifier]])
    {
        completionHandler(nil, nil);
        return;
    }

    _systemListener = [NSXPCListener anonymousListener];
    [_systemListener setDelegate:self];
    [_systemListener resume];

    _systemResponderClientProtocol = [[[plugin controller] responderClass] gameSystemResponderClientProtocol];

    NSXPCListenerEndpoint *endpoint = [_systemListener endpoint];
    completionHandler(endpoint, nil);
}

- (void)stopEmulationWithCompletionHandler:(void(^)(void))handler;
{
    [super stopEmulationWithCompletionHandler:
     ^{
         handler();

         exit(0);
     }];
}

@end
