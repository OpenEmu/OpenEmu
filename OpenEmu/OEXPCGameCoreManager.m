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

#import "OEXPCGameCoreManager.h"
#import "OEXPCGameCoreHelper.h"
#import "OECorePlugin.h"
#import "OEGameCoreManager_Internal.h"
#import <OpenEmuXPCCommunicator/OpenEmuXPCCommunicator.h>

@interface OEXPCGameCoreManager ()
{
    NSTask          *_backgroundProcessTask;
    NSString        *_processIdentifier;
    NSPipe          *_standardOutputPipe;

    NSXPCConnection *_helperConnection;
    NSXPCConnection *_gameCoreConnection;
    id               _systemClient;
}

@property(nonatomic, strong) id<OEXPCGameCoreHelper> gameCoreHelper;
@end

@implementation OEXPCGameCoreManager
@dynamic gameCoreHelper;

+ (BOOL)canUseXPCGameCoreManager
{
    return [NSXPCConnection class] != nil;
}

- (void)loadROMWithCompletionHandler:(void(^)(id systemClient, NSError *error))completionHandler;
{
    _processIdentifier = [NSString stringWithFormat:@"%@-%@", [[[self ROMPath] lastPathComponent] stringByReplacingOccurrencesOfString:@" " withString:@"-"], [[NSUUID UUID] UUIDString]];
    [self _startHelperProcess];

    [[OEXPCCAgent defaultAgent] retrieveListenerEndpointForIdentifier:_processIdentifier completionHandler:
     ^(NSXPCListenerEndpoint *endpoint)
     {
         _helperConnection = [[NSXPCConnection alloc] initWithListenerEndpoint:endpoint];
         [_helperConnection setExportedInterface:[NSXPCInterface interfaceWithProtocol:@protocol(OEGameCoreDisplayHelper)]];
         [_helperConnection setExportedObject:[self displayHelper]];

         [_helperConnection setRemoteObjectInterface:[NSXPCInterface interfaceWithProtocol:@protocol(OEXPCGameCoreHelper)]];
         [_helperConnection resume];

         id<OEXPCGameCoreHelper> gameCoreHelper =
         [_helperConnection remoteObjectProxyWithErrorHandler:
          ^(NSError *error)
          {
              dispatch_async(dispatch_get_main_queue(), ^{
                  completionHandler(nil, error);
                  [self stop];
              });
          }];

         if(gameCoreHelper == nil) return;

         [gameCoreHelper loadROMAtPath:[self ROMPath] usingCorePluginAtPath:[[self plugin] path] systemPluginPath:[[[self systemController] bundle] bundlePath] completionHandler:
          ^(NSXPCListenerEndpoint *gameCoreEndpoint, NSError *error)
          {
              if(gameCoreEndpoint == nil)
              {
                  completionHandler(nil, error);
                  [self stop];
                  return;
              }

              _gameCoreConnection = [[NSXPCConnection alloc] initWithListenerEndpoint:gameCoreEndpoint];
              [_gameCoreConnection setRemoteObjectInterface:[NSXPCInterface interfaceWithProtocol:[[[self systemController] responderClass] gameSystemResponderClientProtocol]]];
              [_gameCoreConnection resume];

              _systemClient = [_gameCoreConnection remoteObjectProxyWithErrorHandler:
                               ^(NSError *error)
                               {
                                   dispatch_async(dispatch_get_main_queue(), ^{
                                       completionHandler(nil, error);
                                       [self stop];
                                   });
                               }];

              NSLog(@"_systemClient: %@", _systemClient);

              if(_systemClient == nil) return;

              [self setGameCoreHelper:gameCoreHelper];
              completionHandler(_systemClient, nil);
          }];
     }];
}

- (void)_startHelperProcess
{
    OEXPCCAgentConfiguration *configuration = [OEXPCCAgentConfiguration defaultConfiguration];

    NSString *helperPath = [[NSBundle mainBundle] pathForResource:@"OpenEmuHelperApp" ofType:nil];
    _backgroundProcessTask = [[NSTask alloc] init];

    [_backgroundProcessTask setLaunchPath:helperPath];
    [_backgroundProcessTask setArguments:@[ [configuration agentServiceNameProcessArgument], [configuration processIdentifierArgumentForIdentifier:_processIdentifier] ]];

    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(_taskDidTerminate:) name:NSTaskDidTerminateNotification object:_backgroundProcessTask];

    _standardOutputPipe = [NSPipe pipe];
    [_backgroundProcessTask setStandardOutput:_standardOutputPipe];

    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(_didReceiveData:) name:NSFileHandleReadCompletionNotification object:[_standardOutputPipe fileHandleForReading]];

    [_backgroundProcessTask launch];
}

- (void)stop
{
    [[self gameCoreHelper] stopEmulationWithCompletionHandler:
     ^{
         [self setGameCoreHelper:nil];
         _systemClient       = nil;

         [_helperConnection invalidate];
         [_gameCoreConnection invalidate];
         _gameCoreConnection = nil;
         _helperConnection   = nil;

         [[NSNotificationCenter defaultCenter] removeObserver:self name:NSTaskDidTerminateNotification object:_backgroundProcessTask];

         [_backgroundProcessTask terminate];
     }];
}

- (void)_taskDidTerminate:(NSNotification *)notification
{
    _backgroundProcessTask = nil;
    _processIdentifier     = nil;
    _standardOutputPipe    = nil;
}

- (void)_didReceiveData:(NSNotification *)notification
{
    NSData *data = [[notification userInfo] objectForKey: NSFileHandleNotificationDataItem];

    // If the length of the data is zero, then the task is basically over - there is nothing
    // more to get from the handle so we may as well shut down.
    if([data length] > 0)
    {
        // Send the data on to the controller; we can't just use +stringWithUTF8String: here
        // because -[data bytes] is not necessarily a properly terminated string.
        // -initWithData:encoding: on the other hand checks -[data length]
        NSLog(@"%@: %@", _processIdentifier, [[NSString alloc] initWithData: data encoding: NSUTF8StringEncoding]);
        [[notification object] readInBackgroundAndNotify];
    }
    else
    {
        // We're finished here
        [self stop];
    }
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

@end
