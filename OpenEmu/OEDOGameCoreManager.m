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

#import "OEDOGameCoreManager.h"
#import "OEDOGameCoreHelper.h"
#import "OEGameCoreHelper.h"
#import "OEGameCoreManager_Internal.h"
#import "OEGameDocument.h"
#import "OETaskWrapper.h"
#import "OECorePlugin.h"
#import <OpenEmuBase/OpenEmuBase.h>

@interface OEDOGameCoreHelperDelegateHelper : NSObject <OEDOGameCoreHelperDelegate>
- (instancetype)initWithGameCoreManager:(OEDOGameCoreManager *)manager;
@end

@interface OEDOGameCoreManager () <OEGameCoreHelper, OETaskWrapperController>
{
    OETaskWrapper                    *_taskWrapper;
    NSString                         *_gameCoreHelperIdentifier;
    NSConnection                     *_helperConnection;
    NSMutableDictionary              *_pendingBlocks;
    OEDOGameCoreHelperDelegateHelper *_delegateHelper;
    id<OEDOGameCoreHelper>            _rootProxy;
}

@end

@implementation OEDOGameCoreManager

- (id)initWithROMPath:(NSString *)romPath corePlugin:(OECorePlugin *)plugin systemController:(OESystemController *)systemController displayHelper:(id<OEGameCoreDisplayHelper>)displayHelper
{
    if((self = [super initWithROMPath:romPath corePlugin:plugin systemController:systemController displayHelper:displayHelper]))
    {
        _pendingBlocks = [NSMutableDictionary dictionary];
        _delegateHelper = [[OEDOGameCoreHelperDelegateHelper alloc] initWithGameCoreManager:self];
    }
    return self;
}

- (void)loadROMWithCompletionHandler:(void(^)(id systemClient))completionHandler errorHandler:(void(^)(NSError *))errorHandler;
{
    NSError *error;
    if(![self _startHelperProcessWithError:&error])
    {
        errorHandler(error);
        return;
    }

    [self setGameCoreHelper:(id<OEGameCoreHelper>)[NSNull null]];

    [_rootProxy loadROMAtPath:[self ROMPath]
        usingCorePluginAtPath:[[self plugin] path]
           systemPluginAtPath:[[[self systemController] bundle] bundlePath]
                 withDelegate:_delegateHelper
                displayHelper:(id<OEDOGameCoreDisplayHelper>)[self displayHelper]
            messageIdentifier:
     [self messageIdentifierForResponderClientHandler:
      ^(id responderClient, NSError *error)
      {
          if(responderClient == nil)
          {
              errorHandler(error);
              return;
          }

          [(NSDistantObject *)responderClient setProtocolForProxy:[[[self systemController] responderClass] gameSystemResponderClientProtocol]];
          
          completionHandler(responderClient);
      }]];
}

- (BOOL)_startHelperProcessWithError:(NSError **)outError
{
    // run our background task. Get our IOSurface ids from its standard out.
    NSString *cliPath = [[NSBundle bundleForClass:[self class]] pathForResource:@"OpenEmuHelperApp" ofType: @""];

    // generate a UUID string so we can have multiple screen capture background tasks running.
    _gameCoreHelperIdentifier = [NSString stringWithUUID];
    // NSLog(@"helper tool UUID should be %@", _taskUUIDForDOServer);

    NSArray *args = [NSArray arrayWithObjects:cliPath, _gameCoreHelperIdentifier, nil];

    _taskWrapper = [[OETaskWrapper alloc] initWithController:self arguments:args userInfo:nil];
    [_taskWrapper startProcess];

    if(![_taskWrapper isRunning])
    {
        if(outError != NULL)
            *outError = [NSError errorWithDomain:OEGameDocumentErrorDomain
                                            code:OEHelperAppNotRunningError
                                        userInfo:[NSDictionary dictionaryWithObject:OELocalizedString(@"The background process couldn't be launched", @"Not running background process error") forKey:NSLocalizedFailureReasonErrorKey]];
        return NO;
    }

    // now that we launched the helper, start up our NSConnection for DO object vending and configure it
    // this is however a race condition if our helper process is not fully launched yet.
    // we hack it out here. Normally this while loop is not noticable, its very fast

    NSDate *start = [NSDate date];

    _helperConnection = nil;
    while(_helperConnection == nil)
    {
        _helperConnection = [NSConnection connectionWithRegisteredName:[NSString stringWithFormat:@"org.openemu.OpenEmuHelper-%@", _gameCoreHelperIdentifier] host:nil];

        if(-[start timeIntervalSinceNow] > 3.0)
        {
            [self stop];
            if(outError != NULL)
            {
                *outError = [NSError errorWithDomain:OEGameDocumentErrorDomain
                                                code:OEConnectionTimedOutError
                                            userInfo:[NSDictionary dictionaryWithObject:OELocalizedString(@"Couldn't connect to the background process.", @"Timed out error reason.") forKey:NSLocalizedFailureReasonErrorKey]];
            }
            return NO;
        }
    }


    if(![_helperConnection isValid])
    {
        [self stop];
        if(outError != NULL)
            *outError = [NSError errorWithDomain:OEGameDocumentErrorDomain
                                            code:OEInvalidHelperConnectionError
                                        userInfo:[NSDictionary dictionaryWithObject:OELocalizedString(@"The background process connection couldn't be established", @"Invalid helper connection error reason.") forKey:NSLocalizedFailureReasonErrorKey]];

        return NO;
    }

    // now that we have a valid connection...

    _rootProxy = (id<OEDOGameCoreHelper>)[_helperConnection rootProxy];
    if(_rootProxy == nil)
    {
        NSLog(@"nil root proxy object?");
        [self stop];
        if(outError != NULL)
        {
            *outError = [NSError errorWithDomain:OEGameDocumentErrorDomain
                                            code:OENilRootProxyObjectError
                                        userInfo:[NSDictionary dictionaryWithObject:OELocalizedString(@"The root proxy object is nil.", @"Nil root proxy object error reason.") forKey:NSLocalizedFailureReasonErrorKey]];
        }
        return NO;
    }

    [(NSDistantObject *)_rootProxy setProtocolForProxy:@protocol(OEDOGameCoreHelper)];

    return YES;
}

- (void)stop
{
    // kill our background friend
    [_taskWrapper stopProcess];
    [_helperConnection invalidate];
    _taskWrapper = nil;
    _rootProxy = nil;
    _helperConnection = nil;
}

#pragma mark - OEDOGameCoreHelper delegate methods

- (void)setVolume:(CGFloat)value;
{
    [_rootProxy setVolume:value];
}

- (void)setPauseEmulation:(BOOL)pauseEmulation;
{
    [_rootProxy setPauseEmulation:pauseEmulation];
}

- (void)setAudioOutputDeviceID:(AudioDeviceID)deviceID;
{
    [_rootProxy setAudioOutputDeviceID:deviceID];
}

- (void)setDrawSquarePixels:(BOOL)drawSquarePixels;
{
    [_rootProxy setDrawSquarePixels:drawSquarePixels];
}

- (void)setupEmulationWithCompletionHandler:(void(^)(IOSurfaceID surfaceID, OEIntSize screenSize, OEIntSize aspectSize))handler;
{
    void *blk = _Block_copy((__bridge void *)handler);

    [_rootProxy setupEmulationWithDelegate:_delegateHelper messageIdentifier:
     [self messageIdentifierForSetupCompletionHandler:
      ^(IOSurfaceID surfaceID, OEIntSize screenSize, OEIntSize aspectSize) {
          dispatch_async(dispatch_get_main_queue(), ^{
              void(^block)(IOSurfaceID surfaceID, OEIntSize screenSize, OEIntSize aspectSize) = (__bridge id)blk;
              block(surfaceID, screenSize, aspectSize);
              _Block_release(blk);
          });
      }]];
}

- (void)startEmulationWithCompletionHandler:(void (^)(void))handler
{
    [_rootProxy startEmulationWithDelegate:_delegateHelper messageIdentifier:
     [self messageIdentifierForCompletionHandler:
      ^{
          dispatch_async(dispatch_get_main_queue(), ^{
              handler();
          });
      }]];
}

- (void)resetEmulationWithCompletionHandler:(void(^)(void))handler;
{
    [_rootProxy resetEmulationWithDelegate:_delegateHelper messageIdentifier:
     [self messageIdentifierForCompletionHandler:
      ^{
          dispatch_async(dispatch_get_main_queue(), ^{
              handler();
          });
      }]];
}

- (void)stopEmulationWithCompletionHandler:(void(^)(void))handler;
{
    [_rootProxy stopEmulationWithDelegate:_delegateHelper messageIdentifier:
     [self messageIdentifierForCompletionHandler:
      ^{
          dispatch_async(dispatch_get_main_queue(), ^{
              handler();
          });
      }]];
}

- (void)saveStateToFileAtPath:(NSString *)fileName completionHandler:(void (^)(BOOL success, NSError *error))handler;
{
    [_rootProxy saveStateToFileAtPath:fileName withDelegate:_delegateHelper messageIdentifier:
     [self messageIdentifierForSuccessHandler:
      ^(BOOL success, NSError *error)
      {
          dispatch_async(dispatch_get_main_queue(), ^{
              handler(success, error);
          });
      }]];
}

- (void)loadStateFromFileAtPath:(NSString *)fileName completionHandler:(void (^)(BOOL success, NSError *error))handler;
{
    [_rootProxy loadStateFromFileAtPath:fileName withDelegate:_delegateHelper messageIdentifier:
     [self messageIdentifierForSuccessHandler:
      ^(BOOL success, NSError *error)
      {
          dispatch_async(dispatch_get_main_queue(), ^{
              handler(success, error);
          });
      }]];
}

- (void)setCheat:(NSString *)cheatCode withType:(NSString *)type enabled:(BOOL)enabled;
{
    [_rootProxy setCheat:cheatCode withType:type enabled:enabled];
}

#pragma mark - Message set up

- (NSString *)messageIdentifierForSetupCompletionHandler:(void(^)(IOSurfaceID surfaceID, OEIntSize screenSize, OEIntSize aspectSize))completionHandler;
{
    NSString *identifier = [NSString stringWithUUID];
    completionHandler = [completionHandler copy];
    _pendingBlocks[identifier] = completionHandler;
    return identifier;
}

- (NSString *)messageIdentifierForCompletionHandler:(void(^)(void))completionHandler;
{
    NSString *identifier = [NSString stringWithUUID];
    completionHandler = [completionHandler copy];
    _pendingBlocks[identifier] = completionHandler;
    return identifier;
}

- (NSString *)messageIdentifierForSuccessHandler:(void(^)(BOOL, NSError *))successHandler;
{
    NSString *identifier = [NSString stringWithUUID];
    successHandler = [successHandler copy];
    _pendingBlocks[identifier] = successHandler;
    return identifier;
}

- (NSString *)messageIdentifierForResponderClientHandler:(void(^)(id, NSError *))responderClient;
{
    NSString *identifier = [NSString stringWithUUID];
    responderClient = [responderClient copy];
    _pendingBlocks[identifier] = responderClient;
    return identifier;
}

#pragma mark - Message replies

- (void)performCompletionBlockWithIdentifier:(NSString *)identifier;
{
    void(^block)(void) = _pendingBlocks[identifier];
    [_pendingBlocks removeObjectForKey:identifier];
    block();
}

- (void)performSetupCompletionBlockWithIdentifier:(NSString *)identifier surfaceID:(IOSurfaceID)surfaceID screenSize:(OEIntSize)screenSize aspectSize:(OEIntSize)aspectSize
{
    void(^block)(IOSurfaceID, OEIntSize, OEIntSize) = _pendingBlocks[identifier];
    [_pendingBlocks removeObjectForKey:identifier];
    block(surfaceID, screenSize, aspectSize);
}

- (void)performSuccessBlockWithIdentifier:(NSString *)identifier success:(BOOL)success error:(NSError *)error
{
    void(^block)(BOOL, NSError *) = _pendingBlocks[identifier];
    [_pendingBlocks removeObjectForKey:identifier];
    block(success, error);
}

- (void)performResponderClientBlockWithIdentifier:(NSString *)identifier systemClient:(id)systemClient error:(NSError *)error;
{
    void(^block)(id, NSError *) = _pendingBlocks[identifier];
    [_pendingBlocks removeObjectForKey:identifier];
    block(systemClient, error);
}

#pragma mark - TaskWrapper delegate methods

- (void)appendOutput:(NSString *)output fromProcess:(OETaskWrapper *)aTask
{
    printf("%s", [output UTF8String]);
}

- (void)processStarted:(OETaskWrapper *)aTask
{
}

- (void)processFinished:(OETaskWrapper *)aTask withStatus:(NSInteger)statusCode
{
}

@end

@implementation OEDOGameCoreHelperDelegateHelper
{
    __weak OEDOGameCoreManager *_gameCoreManager;
}

- (instancetype)initWithGameCoreManager:(OEDOGameCoreManager *)manager
{
    if((self = [super init]))
    {
        _gameCoreManager = manager;
    }
    return self;
}

- (void)callSetupCompletionBlockForIdentifier:(NSString *)identifier withSurfaceID:(IOSurfaceID)surfaceID screenSize:(OEIntSize)screenSize aspectSize:(OEIntSize)aspectSize
{
    [_gameCoreManager performSetupCompletionBlockWithIdentifier:identifier surfaceID:surfaceID screenSize:screenSize aspectSize:aspectSize];
}

- (void)callCompletionBlockForIdentifier:(NSString *)identifier
{
    [_gameCoreManager performCompletionBlockWithIdentifier:identifier];
}

- (void)callSuccessBlockForIdentifier:(NSString *)identifier withSuccess:(BOOL)success error:(NSError *)error
{
    [_gameCoreManager performSuccessBlockWithIdentifier:identifier success:success error:error];
}

- (void)callResponderClientBlockForIdentifier:(NSString *)identifier withClient:(id)client error:(NSError *)error
{
    [_gameCoreManager performResponderClientBlockWithIdentifier:identifier systemClient:client error:error];
}

- (oneway void)gameCoreHelperDidSetSystemResponderClient:(byref id)responderClient withMessageIdentifier:(NSString *)identifier
{
    [self callResponderClientBlockForIdentifier:identifier withClient:responderClient error:nil];
}

- (oneway void)gameCoreHelperFailedToLoadROMWithError:(NSError *)error messageIdentifier:(NSString *)identifier
{
    [self callResponderClientBlockForIdentifier:identifier withClient:nil error:error];
}

- (oneway void)gameCoreHelperDidSetupEmulationWithSurfaceID:(IOSurfaceID)surfaceID screenSize:(OEIntSize)screenSize aspectSize:(OEIntSize)aspectSize messageIdentifier:(NSString *)identifier;
{
    [self callSetupCompletionBlockForIdentifier:identifier withSurfaceID:surfaceID screenSize:screenSize aspectSize:aspectSize];
}

- (oneway void)gameCoreHelperDidStartEmulationWithMessageIdentifier:(NSString *)identifier;
{
    [self callCompletionBlockForIdentifier:identifier];
}

- (oneway void)gameCoreHelperDidResetEmulationWithMessageIdentifier:(NSString *)identifier;
{
    [self callCompletionBlockForIdentifier:identifier];
}

- (oneway void)gameCoreHelperDidStopEmulationWithMessageIdentifier:(NSString *)identifier;
{
    [self callCompletionBlockForIdentifier:identifier];
}

- (oneway void)gameCoreHelperDidSaveState:(BOOL)success error:(NSError *)error withMessageIdentifier:(NSString *)identifier;
{
    [self callSuccessBlockForIdentifier:identifier withSuccess:success error:error];
}

- (oneway void)gameCoreHelperDidLoadState:(BOOL)success error:(NSError *)error withMessageIdentifier:(NSString *)identifier;
{
    [self callSuccessBlockForIdentifier:identifier withSuccess:success error:error];
}

@end
