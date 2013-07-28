//
//  OpenEmuXPCHelperApp.m
//  OpenEmu
//
//  Created by Remy Demarest on 20/07/2013.
//
//

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

         [[NSApplication sharedApplication] terminate:self];
     }];
}

@end
