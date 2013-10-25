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
#import "OEThreadProxy.h"
#import <OpenEmuXPCCommunicator/OpenEmuXPCCommunicator.h>

@interface OEXPCGameCoreManager ()
{
    NSTask          *_backgroundProcessTask;
    NSString        *_processIdentifier;
    NSPipe          *_standardOutputPipe;
    NSPipe          *_standardErrorPipe;

    NSXPCConnection *_helperConnection;
    NSXPCConnection *_gameCoreConnection;
    OEThreadProxy   *_displayHelperProxy;
    id               _systemClient;
    BOOL             _isStoppingBackgroundProcess;
}

@property(nonatomic, strong) id<OEXPCGameCoreHelper> gameCoreHelper;
@end

@implementation OEXPCGameCoreManager
@dynamic gameCoreHelper;

+ (BOOL)canUseXPCGameCoreManager
{
    return [NSHashTable respondsToSelector:@selector(weakObjectsHashTable)];
}

- (void)getTerminatingProcesses:(void(^)(NSMutableSet *processes))block
{
    static dispatch_queue_t queue;
    static NSMutableSet *processes;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        queue = dispatch_queue_create("org.openemu.TerminatingXPCProcesses", DISPATCH_QUEUE_SERIAL);
        processes = [NSMutableSet set];
    });

    dispatch_async(queue, ^{
        block(processes);
    });
}

- (void)selfRetain
{
    [self getTerminatingProcesses:
     ^(NSMutableSet *processes)
     {
         [processes addObject:self];
     }];
}

- (void)selfRelease
{
    [self getTerminatingProcesses:
     ^(NSMutableSet *processes)
     {
         [processes removeObject:self];
     }];
}

- (void)loadROMWithCompletionHandler:(void(^)(id systemClient))completionHandler errorHandler:(void(^)(NSError *))errorHandler;
{
    _processIdentifier = [NSString stringWithFormat:@"%@ # %@", [[[self ROMPath] lastPathComponent] stringByReplacingOccurrencesOfString:@" " withString:@"-"], [[NSUUID UUID] UUIDString]];
    [self _startHelperProcess];

    [[OEXPCCAgent defaultAgent] retrieveListenerEndpointForIdentifier:_processIdentifier completionHandler:
     ^(NSXPCListenerEndpoint *endpoint)
     {
         _displayHelperProxy = [OEThreadProxy threadProxyWithTarget:[self displayHelper] thread:[NSThread mainThread]];
         _helperConnection = [[NSXPCConnection alloc] initWithListenerEndpoint:endpoint];
         [_helperConnection setExportedInterface:[NSXPCInterface interfaceWithProtocol:@protocol(OEGameCoreDisplayHelper)]];
         [_helperConnection setExportedObject:_displayHelperProxy];

         [_helperConnection setRemoteObjectInterface:[NSXPCInterface interfaceWithProtocol:@protocol(OEXPCGameCoreHelper)]];
         [_helperConnection resume];

         __block void *gameCoreHelperPointer;
         id<OEXPCGameCoreHelper> gameCoreHelper =
         [_helperConnection remoteObjectProxyWithErrorHandler:
          ^(NSError *error)
          {
              NSLog(@"Helper Connection (%p) failed with error: %@", gameCoreHelperPointer, error);
              dispatch_async(dispatch_get_main_queue(), ^{
                  errorHandler(error);
                  [self stop];
              });
          }];

         gameCoreHelperPointer = (__bridge void *)gameCoreHelper;

         if(gameCoreHelper == nil) return;

         [gameCoreHelper loadROMAtPath:[self ROMPath] usingCorePluginAtPath:[[self plugin] path] systemPluginPath:[[[self systemController] bundle] bundlePath] completionHandler:
          ^(NSXPCListenerEndpoint *gameCoreEndpoint, NSError *error)
          {
              if(gameCoreEndpoint == nil)
              {
                  dispatch_async(dispatch_get_main_queue(), ^{
                      errorHandler(error);
                      [self stop];
                  });
              }

              _gameCoreConnection = [[NSXPCConnection alloc] initWithListenerEndpoint:gameCoreEndpoint];
              [_gameCoreConnection setRemoteObjectInterface:[NSXPCInterface interfaceWithProtocol:[[[self systemController] responderClass] gameSystemResponderClientProtocol]]];
              [_gameCoreConnection resume];

              __block void *systemClientError;
              _systemClient =
              [_gameCoreConnection remoteObjectProxyWithErrorHandler:
               ^(NSError *error)
               {
                   NSLog(@"Game Core Connection (%p) failed with error: %@", systemClientError, error);
                   dispatch_async(dispatch_get_main_queue(), ^{
                       errorHandler(error);
                       [self stop];
                   });
               }];

              systemClientError = (__bridge void *)_systemClient;

              DLog(@"_systemClient: %@", _systemClient);

              if(_systemClient == nil) return;

              [self setGameCoreHelper:gameCoreHelper];
              dispatch_async(dispatch_get_main_queue(), ^{
                  completionHandler(_systemClient);
              });
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
    [[_standardOutputPipe fileHandleForReading] readInBackgroundAndNotify];

    _standardErrorPipe = [NSPipe pipe];
    [_backgroundProcessTask setStandardError:_standardErrorPipe];

    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(_didReceiveErrorData:) name:NSFileHandleReadCompletionNotification object:[_standardErrorPipe fileHandleForReading]];
    [[_standardErrorPipe fileHandleForReading] readInBackgroundAndNotify];

    [_backgroundProcessTask launch];
}

- (void)stop
{
    if(_isStoppingBackgroundProcess) return;

    _isStoppingBackgroundProcess = YES;

    [self selfRetain];

    [self setGameCoreHelper:nil];
    _displayHelperProxy = nil;
    _systemClient       = nil;

    [_helperConnection invalidate];
    [_gameCoreConnection invalidate];
    _gameCoreConnection = nil;
    _helperConnection   = nil;

    [_backgroundProcessTask terminate];
}

- (void)_taskDidTerminate:(NSNotification *)notification
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];

    _backgroundProcessTask = nil;
    _processIdentifier     = nil;
    _standardOutputPipe    = nil;
    _standardErrorPipe     = nil;

    DLog(@"Did stop background process.");

    [self selfRelease];
}

- (void)_didReceiveErrorData:(NSNotification *)notification
{
    NSData *data = [[notification userInfo] objectForKey:NSFileHandleNotificationDataItem];

    // If the length of the data is zero, then the task is basically over - there is nothing
    // more to get from the handle so we may as well shut down.
    if([data length] > 0)
    {
        // Send the data on to the controller; we can't just use +stringWithUTF8String: here
        // because -[data bytes] is not necessarily a properly terminated string.
        // -initWithData:encoding: on the other hand checks -[data length]
        fprintf(stderr, "%s\n", [[NSString stringWithFormat:@"background process error: %@: %@", [_processIdentifier substringToIndex:[_processIdentifier rangeOfString:@" # "].location], [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding]] UTF8String]);
        [[notification object] readInBackgroundAndNotify];
    }
    else
    {
        // We're finished here
        [self stop];
    }
}

- (void)_didReceiveData:(NSNotification *)notification
{
    NSData *data = [[notification userInfo] objectForKey:NSFileHandleNotificationDataItem];

    // If the length of the data is zero, then the task is basically over - there is nothing
    // more to get from the handle so we may as well shut down.
    if([data length] > 0)
    {
        // Send the data on to the controller; we can't just use +stringWithUTF8String: here
        // because -[data bytes] is not necessarily a properly terminated string.
        // -initWithData:encoding: on the other hand checks -[data length]
        fprintf(stderr, "%s\n", [[NSString stringWithFormat:@"background process error: %@: %@", [_processIdentifier substringToIndex:[_processIdentifier rangeOfString:@" # "].location], [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding]] UTF8String]);
        [[notification object] readInBackgroundAndNotify];
    }
    else
    {
        // We're finished here
        [self stop];
    }
}

@end
