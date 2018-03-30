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
#import "OESystemPlugin.h"
#import "OEThreadProxy.h"
#import <OpenEmuXPCCommunicator/OpenEmuXPCCommunicator.h>

@interface OEXPCGameCoreManager ()
{
    NSTask          *_backgroundProcessTask;
    NSString        *_processIdentifier;
    NSPipe          *_standardOutputPipe;
    NSPipe          *_standardErrorPipe;

    NSXPCConnection *_helperConnection;
    OEThreadProxy   *_gameCoreOwnerProxy;
    BOOL             _isStoppingBackgroundProcess;
}

@property(nonatomic, strong) id<OEXPCGameCoreHelper> gameCoreHelper;
@end

@implementation OEXPCGameCoreManager
@dynamic gameCoreHelper;

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

- (void)loadROMWithCompletionHandler:(void(^)(void))completionHandler errorHandler:(void(^)(NSError *))errorHandler;
{
    _processIdentifier = [NSString stringWithFormat:@"%@ # %@", [[[self ROMPath] lastPathComponent] stringByReplacingOccurrencesOfString:@" " withString:@"-"], [[NSUUID UUID] UUIDString]];
    [self _startHelperProcess];

    [[OEXPCCAgent defaultAgent] retrieveListenerEndpointForIdentifier:_processIdentifier completionHandler:
     ^(NSXPCListenerEndpoint *endpoint)
     {
         if(endpoint == nil)
         {
             NSLog(@"No listener endpoint for identifier: %@", self->_processIdentifier);
             dispatch_async(dispatch_get_main_queue(), ^{
                 NSError *error = [NSError errorWithDomain:OEGameCoreErrorDomain
                                                      code:OEGameCoreCouldNotLoadROMError
                                                  userInfo:nil];
                 errorHandler(error);
                 [self stop];
             });
             
             // There's no listener endpoint, so don't bother trying to create an NSXPCConnection.
             // Returning now since calling initWithListenerEndpoint: and passing it nil results in a memory leak.
             // Also, there's no point in trying to get the gameCoreHelper if there's no _helperConnection.
             return;
         }

         self->_gameCoreOwnerProxy = [OEThreadProxy threadProxyWithTarget:[self gameCoreOwner] thread:[NSThread mainThread]];
         self->_helperConnection = [[NSXPCConnection alloc] initWithListenerEndpoint:endpoint];
         [self->_helperConnection setExportedInterface:[NSXPCInterface interfaceWithProtocol:@protocol(OEGameCoreOwner)]];
         [self->_helperConnection setExportedObject:self->_gameCoreOwnerProxy];

         [self->_helperConnection setRemoteObjectInterface:[NSXPCInterface interfaceWithProtocol:@protocol(OEXPCGameCoreHelper)]];
         [self->_helperConnection resume];

         __block void *gameCoreHelperPointer;
         id<OEXPCGameCoreHelper> gameCoreHelper =
         [self->_helperConnection remoteObjectProxyWithErrorHandler:
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

         [gameCoreHelper loadROMAtPath:[self ROMPath] romCRC32:[self ROMCRC32] romMD5:[self ROMMD5] romHeader:[self ROMHeader] romSerial:[self ROMSerial] systemRegion:[self systemRegion] usingCorePluginAtPath:[[self plugin] path] systemPluginPath:[[self systemPlugin] path] completionHandler:
          ^(NSError *error)
          {
              if(error != nil)
              {
                  dispatch_async(dispatch_get_main_queue(), ^{
                      errorHandler(error);
                      [self stop];
                  });

                  // There's no listener endpoint, so don't bother trying to create an NSXPCConnection.
                  // Returning now since calling initWithListenerEndpoint: and passing it nil results in a memory leak.
                  return;
              }

              [self setGameCoreHelper:gameCoreHelper];
              dispatch_async(dispatch_get_main_queue(), ^{
                  completionHandler();
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
    
    BOOL logCoreOutput = [[NSUserDefaults standardUserDefaults] boolForKey:@"OELogCoreOutput"];

    if (logCoreOutput) {
        NSLog(@"Will log core output for %@", _processIdentifier);
        
        _standardOutputPipe = [NSPipe pipe];
        int output_fildes = _standardOutputPipe.fileHandleForReading.fileDescriptor;
        
        [_backgroundProcessTask setStandardOutput:_standardOutputPipe];
        dispatch_read(output_fildes, 72, dispatch_get_main_queue(), ^(dispatch_data_t data, int error){
            [self _didReceiveData:(NSData*)data posixError:error fileDescriptor:output_fildes name:@"stdout"];
        });

        _standardErrorPipe = [NSPipe pipe];
        int error_fildes = _standardErrorPipe.fileHandleForReading.fileDescriptor;
        
        [_backgroundProcessTask setStandardError:_standardErrorPipe];
        dispatch_read(error_fildes, 72, dispatch_get_main_queue(), ^(dispatch_data_t data, int error) {
            [self _didReceiveData:(NSData*)data posixError:error fileDescriptor:error_fildes name:@"stderr"];
        });
    } else {
        /* Do not even receive the core output if we're not doing anything with it
         * to save CPU time */
        [_backgroundProcessTask setStandardOutput:[NSFileHandle fileHandleWithNullDevice]];
        [_backgroundProcessTask setStandardError:[NSFileHandle fileHandleWithNullDevice]];
    }

    [_backgroundProcessTask launch];
}

- (void)stop
{
    if(_isStoppingBackgroundProcess) return;

    _isStoppingBackgroundProcess = YES;

    [self selfRetain];

    [self setGameCoreHelper:nil];
    _gameCoreOwnerProxy = nil;

    [_helperConnection invalidate];
    _helperConnection = nil;

    [_backgroundProcessTask terminate];
}

- (void)_taskDidTerminate:(NSNotification *)notification
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];

    _backgroundProcessTask = nil;
    _standardOutputPipe    = nil;
    _standardErrorPipe     = nil;

    DLog(@"Did stop background process.");

    [self selfRelease];
}

- (void)_didReceiveData:(NSData *)data posixError:(int)error fileDescriptor:(int)fildes name:(NSString *)name
{
    // If the length of the data is zero, then the task is basically over - there is nothing
    // more to get from the handle so we may as well shut down.
    if([data length] > 0 || error)
    {
        if (error) {
            DLog(@"POSIX error while reading XPC helper %@: %d", name, error);
            // POSIX file read errors are not always fatal
        }
        
        // Send the data on to the controller; we can't just use +stringWithUTF8String: here
        // because -[data bytes] is not necessarily a properly terminated string.
        // -initWithData:encoding: on the other hand checks -[data length]
        fprintf(stderr, "%s\n", [[NSString stringWithFormat:@"background process %@: %@: %@", name, [_processIdentifier substringToIndex:[_processIdentifier rangeOfString:@" # "].location], [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding]] UTF8String]);

        dispatch_read(fildes, 72, dispatch_get_main_queue(), ^(dispatch_data_t data, int error) {
            [self _didReceiveData:(NSData*)data posixError:error fileDescriptor:fildes name:name];
        });
    }
    else
    {
        // We're finished here
        DLog(@"background process %@: %@ EOF", _processIdentifier, name);
    }
}


@end
