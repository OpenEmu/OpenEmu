//
//  OEDOGameCoreManager.m
//  OpenEmu
//
//  Created by Remy Demarest on 20/07/2013.
//
//

#import "OEDOGameCoreManager.h"
#import "OEDOGameCoreHelper.h"
#import "OEGameCoreHelper.h"
#import "OEGameDocument.h"
#import "OETaskWrapper.h"
#import "OECorePlugin.h"
#import <OpenEmuBase/OpenEmuBase.h>

@interface OEDOGameCoreHelperDelegateHelper : NSObject <OEDOGameCoreHelperDelegate>
+ (instancetype)delegateHelperWithCompletionHandler:(void(^)(void))completionHandler;
+ (instancetype)delegateHelperWithSuccessHandler:(void(^)(BOOL, NSError *))successHandler;
+ (instancetype)delegateHelperWithResponderClientHandler:(void(^)(id))responderClient;
@end

@interface OEDOGameCoreManager () <OEGameCoreHelper, OETaskWrapperController>
{
    OETaskWrapper          *_taskWrapper;
    NSString               *_gameCoreHelperIdentifier;
    NSConnection           *_helperConnection;

    id<OEDOGameCoreHelper>  _rootProxy;
}

@end

@implementation OEDOGameCoreManager

- (void)loadROMWithCompletionHandler:(void(^)(id<OEGameCoreHelper> helper, id systemClient))completionHandler errorHandler:(void(^)(NSError *error))errorHandler;
{
    NSError *error;
    if(![self _startHelperProcessWithError:&error])
    {
        errorHandler(error);
        return;
    }

    [_rootProxy loadROMAtPath:[self ROMPath] usingCorePluginAtPath:[[self plugin] path] systemPluginAtPath:[[[self systemController] bundle] bundlePath] withDelegate:
     [OEDOGameCoreHelperDelegateHelper delegateHelperWithResponderClientHandler:
      ^(id responderClient)
      {
          [(NSDistantObject *)responderClient setProtocolForProxy:[[[self systemController] responderClass] gameSystemResponderClientProtocol]];
          
          completionHandler(self, responderClient);
      }] displayHelper:(id<OEDOGameCoreDisplayHelper>)[self displayHelper]];
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
                                        userInfo:[NSDictionary dictionaryWithObject:NSLocalizedString(@"The background process couldn't be launched", @"Not running background process error") forKey:NSLocalizedFailureReasonErrorKey]];
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
                                            userInfo:[NSDictionary dictionaryWithObject:NSLocalizedString(@"Couldn't connect to the background process.", @"Timed out error reason.") forKey:NSLocalizedFailureReasonErrorKey]];
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
                                        userInfo:[NSDictionary dictionaryWithObject:NSLocalizedString(@"The background process connection couldn't be established", @"Invalid helper connection error reason.") forKey:NSLocalizedFailureReasonErrorKey]];

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
                                        userInfo:[NSDictionary dictionaryWithObject:NSLocalizedString(@"The root proxy object is nil.", @"Nil root proxy object error reason.") forKey:NSLocalizedFailureReasonErrorKey]];
        }
        return NO;
    }

    [(NSDistantObject *)_rootProxy setProtocolForProxy:@protocol(OEDOGameCoreHelper)];

    return YES;
}

- (void)stop
{
    [_rootProxy stopEmulationWithDelegate:nil];

    // kill our background friend
    [_taskWrapper stopProcess];
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

- (void)setupEmulationWithCompletionHandler:(void(^)(void))handler;
{
    [_rootProxy setupEmulationWithDelegate:[OEDOGameCoreHelperDelegateHelper delegateHelperWithCompletionHandler:handler]];
}

- (void)resetEmulationWithCompletionHandler:(void(^)(void))handler;
{
    [_rootProxy resetEmulationWithDelegate:[OEDOGameCoreHelperDelegateHelper delegateHelperWithCompletionHandler:handler]];
}

- (void)stopEmulationWithCompletionHandler:(void(^)(void))handler;
{
    [_rootProxy stopEmulationWithDelegate:[OEDOGameCoreHelperDelegateHelper delegateHelperWithCompletionHandler:handler]];
}

- (void)saveStateToFileAtPath:(NSString *)fileName completionHandler:(void (^)(BOOL success, NSError *error))block;
{
    [_rootProxy saveStateToFileAtPath:fileName withDelegate:block];
}

- (void)loadStateFromFileAtPath:(NSString *)fileName completionHandler:(void (^)(BOOL success, NSError *error))block;
{
    [_rootProxy loadStateFromFileAtPath:fileName withDelegate:block];
}

- (void)setCheat:(NSString *)cheatCode withType:(NSString *)type enabled:(BOOL)enabled;
{
    [_rootProxy setCheat:cheatCode withType:type enabled:enabled];
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


typedef enum : NSUInteger
{
    OEDOGameCoreHelperDelegateHelperTypeCompletionHandler,
    OEDOGameCoreHelperDelegateHelperTypeSuccessHandler,
    OEDOGameCoreHelperDelegateHelperTypeResponderClientHandler
} OEDOGameCoreHelperDelegateHelperType;

@implementation OEDOGameCoreHelperDelegateHelper
{
    id _handler;
    OEDOGameCoreHelperDelegateHelperType _handlerType;
}

- (void)_getDelegateHelperSet:(void(^)(NSMutableSet *helpers))block
{
    static dispatch_queue_t helpersQueue;
    static NSMutableSet *helpersSet;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        helpersQueue = dispatch_queue_create("org.openemu.OEDOGameCoreHelperDelegateHelper.Queue", DISPATCH_QUEUE_SERIAL);
        helpersSet = [NSMutableSet set];
    });

    dispatch_async(helpersQueue, ^{
        block(helpersSet);
    });
}

- (void)_addHelper
{
    [self _getDelegateHelperSet:
     ^(NSMutableSet *helpers)
     {
         [helpers addObject:self];
     }];
}

- (void)_removeHelper
{
    [self _getDelegateHelperSet:
     ^(NSMutableSet *helpers)
     {
         [helpers removeObject:self];
     }];
}

+ (instancetype)delegateHelperWithCompletionHandler:(void(^)(void))completionHandler;
{
    return [[self alloc] initWithHandler:completionHandler type:OEDOGameCoreHelperDelegateHelperTypeCompletionHandler];
}

+ (instancetype)delegateHelperWithSuccessHandler:(void(^)(BOOL, NSError *))successHandler;
{
    return [[self alloc] initWithHandler:successHandler type:OEDOGameCoreHelperDelegateHelperTypeSuccessHandler];
}

+ (instancetype)delegateHelperWithResponderClientHandler:(void(^)(id))responderClient;
{
    return [[self alloc] initWithHandler:responderClient type:OEDOGameCoreHelperDelegateHelperTypeResponderClientHandler];
}

- (id)initWithHandler:(id)handler type:(OEDOGameCoreHelperDelegateHelperType)type;
{
    if((self = [super init]))
    {
        _handler = [handler copy];
        _handlerType = type;
        [self _addHelper];
    }
    return self;
}

- (void)callCompletionHandler
{
    NSAssert(_handlerType == OEDOGameCoreHelperDelegateHelperTypeCompletionHandler, @"Requesting wrong type of handler");
    if(_handler != nil) ((void(^)(void))_handler)();
    [self _removeHelper];
}

- (void)callSuccessHandlerWithSuccess:(BOOL)success error:(NSError *)error
{
    NSAssert(_handlerType == OEDOGameCoreHelperDelegateHelperTypeSuccessHandler, @"Requesting wrong type of handler");
    if(_handler != nil) ((void(^)(BOOL, NSError *))_handler)(success, error);
    [self _removeHelper];
}

- (void)callResponderClientHandlerWithClient:(id)client
{
    NSAssert(_handlerType == OEDOGameCoreHelperDelegateHelperTypeResponderClientHandler, @"Requesting wrong type of handler");
    if(_handler != nil) ((void(^)(id))_handler)(client);
    [self _removeHelper];
}

- (oneway void)setSystemResponderClient:(byref id)responderClient;
{
    [self callResponderClientHandlerWithClient:responderClient];
}

- (oneway void)gameCoreHelperDidSetupEmulation;
{
    [self callCompletionHandler];
}

- (oneway void)gameCoreHelperDidResetEmulation;
{
    [self callCompletionHandler];
}

- (oneway void)gameCoreHelperDidStopEmulation;
{
    [self callCompletionHandler];
}

- (oneway void)gameCoreHelperDidSaveState:(BOOL)success error:(NSError *)error;
{
    [self callSuccessHandlerWithSuccess:success error:error];
}

- (oneway void)gameCoreHelperDidLoadState:(BOOL)success error:(NSError *)error;
{
    [self callSuccessHandlerWithSuccess:success error:error];
}

@end
