//
//  OEXPCGameCoreManager.m
//  OpenEmu
//
//  Created by Remy Demarest on 20/07/2013.
//
//

#import "OEXPCGameCoreManager.h"
#import "OEXPCGameCoreHelper.h"
#import "OECorePlugin.h"
#import <OpenEmuXPCCommunicator/OpenEmuXPCCommunicator.h>

@implementation OEXPCGameCoreManager
{
    NSTask                  *_backgroundProcessTask;
    NSString                *_processIdentifier;
    NSPipe                  *_standardOutputPipe;

    NSXPCConnection         *_helperConnection;
    NSXPCConnection         *_gameCoreConnection;
    id<OEXPCGameCoreHelper>  _gameCoreHelper;
    id                       _systemClient;
}

+ (BOOL)canUseXPCGameCoreManager
{
    return [NSXPCConnection class] != nil;
}

- (void)loadROMWithCompletionHandler:(void(^)(id<OEGameCoreHelper> helper, id systemClient))completionHandler errorHandler:(void(^)(NSError *error))errorHandler;
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

         _gameCoreHelper = [_helperConnection remoteObjectProxyWithErrorHandler:
                            ^(NSError *error)
                            {
                                errorHandler(error);
                                [self stop];
                            }];

         if(_gameCoreHelper == nil) return;

         [_gameCoreHelper loadROMAtPath:[self ROMPath] usingCorePluginAtPath:[[self plugin] path] systemPluginPath:[[[self systemController] bundle] bundlePath] completionHandler:
          ^(NSXPCListenerEndpoint *gameCoreEndpoint, NSError *error)
          {
              if(gameCoreEndpoint == nil)
              {
                  errorHandler(error);
                  [self stop];
                  return;
              }

              _gameCoreConnection = [[NSXPCConnection alloc] initWithListenerEndpoint:gameCoreEndpoint];
              [_gameCoreConnection setRemoteObjectInterface:[NSXPCInterface interfaceWithProtocol:[[[self systemController] responderClass] gameSystemResponderClientProtocol]]];
              [_gameCoreConnection resume];

              _systemClient = [_gameCoreConnection remoteObjectProxyWithErrorHandler:
                               ^(NSError *error)
                               {
                                   errorHandler(error);
                                   [self stop];
                               }];

              NSLog(@"_systemClient: %@", _systemClient);

              if(_systemClient == nil) return;

              completionHandler(_gameCoreHelper, _systemClient);
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
    [_gameCoreHelper stopEmulationWithCompletionHandler:
     ^{
         [_helperConnection invalidate];
         [_gameCoreConnection invalidate];
         _gameCoreConnection = nil;
         _helperConnection = nil;

         [_backgroundProcessTask terminate];
     }];
}

- (void)_taskDidTerminate:(NSNotification *)notification
{
    _backgroundProcessTask = nil;
    _gameCoreHelper        = nil;
    _systemClient          = nil;
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

@end
