/*
 Copyright (c) 2010, OpenEmu Team
 
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

#import "OEGameCoreManager.h"
#import "OECorePlugin.h"
#import "OEGameCoreHelper.h"
#import "OpenEmuHelperApp.h"
#import "OEGameDocument.h"
#import "OETaskWrapper.h"

#import <OpenEmuBase/OpenEmuBase.h>

@implementation OEGameCoreManager
{
@protected
    id<OEGameCoreHelper> _rootProxy;
}

- (id)initWithROMAtPath:(NSString *)theRomPath corePlugin:(OECorePlugin *)thePlugin systemIdentifier:(NSString *)identifier error:(NSError **)outError
{
    self = [super init];
    
    if(self != nil)
    {
        _plugin  = thePlugin;
        _romPath = [theRomPath copy];
        
        if(![self startHelperProcessError:outError])
            return nil;
        
        if(![self loadROMWithSystemIdentifier:identifier error:outError])
        {
            [self endHelperProcess];
            return nil;
        }
    }
    return self;
}

- (void)stop
{
    [self endHelperProcess];
}

- (void)dealloc
{
    [self stop];
}

- (BOOL)startHelperProcessError:(NSError **)outError
{
    if(outError != NULL) *outError = [NSError errorWithDomain:NSOSStatusErrorDomain code:unimpErr userInfo:NULL];
    
    return NO;
}

- (void)endHelperProcess
{
    
}

- (BOOL)loadROMWithSystemIdentifier:(NSString *)identifier error:(NSError **)outError
{
    BOOL ret = NO;
    
    @try
    {
        DLog(@"[self rootProxy]: %@", [self rootProxy]);
        ret = [[self rootProxy] loadRomAtPath:_romPath withCorePluginAtPath:[[_plugin bundle] bundlePath] withSystemIdentifier:identifier];
    }
    @catch(NSException *exception)
    {
        NSLog(@"%@", exception);
    }
    
    if(!ret && outError != NULL) 
        *outError = [NSError errorWithDomain:@"OEHelperProcessErrorDomain"
                                        code:OECouldNotLoadROMError
                                    userInfo:[NSDictionary dictionaryWithObject:NSLocalizedString(@"The ROM couldn't be loaded.", @"OEGameCoreManager loadROMError: error reason.") forKey:NSLocalizedFailureReasonErrorKey]];
    
    return ret;
}

- (OEGameCoreController *)owner
{
    return [[[self rootProxy] gameCore] owner];
}

@end

#pragma mark -
#pragma mark Manager using a background process

@implementation OEGameCoreProcessManager
{
    // IPC from our OEHelper
    NSString             *_taskUUIDForDOServer;
    NSConnection         *_taskConnection;
}

- (BOOL)startHelperProcessError:(NSError **)outError
{
    // run our background task. Get our IOSurface ids from its standard out.
    NSString *cliPath = [[NSBundle bundleForClass:[self class]] pathForResource:@"OpenEmuHelperApp" ofType: @""];
    
    // generate a UUID string so we can have multiple screen capture background tasks running.
    _taskUUIDForDOServer = [NSString stringWithUUID];
    // NSLog(@"helper tool UUID should be %@", _taskUUIDForDOServer);
    
    NSArray *args = [NSArray arrayWithObjects:cliPath, _taskUUIDForDOServer, nil];
    
    _helper = [[OETaskWrapper alloc] initWithController:self arguments:args userInfo:nil];
    [_helper startProcess];
    
    if(![_helper isRunning])
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
    
    _taskConnection = nil;
    while(_taskConnection == nil)
    {
        _taskConnection = [NSConnection connectionWithRegisteredName:[NSString stringWithFormat:@"org.openemu.OpenEmuHelper-%@", _taskUUIDForDOServer] host:nil];
        
        if(-[start timeIntervalSinceNow] > 3.0)
        {
            [self endHelperProcess];
            if(outError != NULL)
            {
                *outError = [NSError errorWithDomain:OEGameDocumentErrorDomain
                                                code:OEConnectionTimedOutError
                                            userInfo:[NSDictionary dictionaryWithObject:NSLocalizedString(@"Couldn't connect to the background process.", @"Timed out error reason.") forKey:NSLocalizedFailureReasonErrorKey]];
            }
            return NO;
        }
    }
    
    
    if(![_taskConnection isValid])
    {
        [self endHelperProcess];
        if(outError != NULL)
            *outError = [NSError errorWithDomain:OEGameDocumentErrorDomain
                                            code:OEInvalidHelperConnectionError
                                        userInfo:[NSDictionary dictionaryWithObject:NSLocalizedString(@"The background process connection couldn't be established", @"Invalid helper connection error reason.") forKey:NSLocalizedFailureReasonErrorKey]];
        
        return NO;
    }
    
    // now that we have a valid connection...
    
    _rootProxy = (id<OEGameCoreHelper>)[_taskConnection rootProxy];
    if(_rootProxy == nil)
    {
        NSLog(@"nil root proxy object?");
        [self endHelperProcess];
        if(outError != NULL)
        {
            *outError = [NSError errorWithDomain:OEGameDocumentErrorDomain
                                            code:OENilRootProxyObjectError
                                        userInfo:[NSDictionary dictionaryWithObject:NSLocalizedString(@"The root proxy object is nil.", @"Nil root proxy object error reason.") forKey:NSLocalizedFailureReasonErrorKey]];
        }
        return NO;
    }
    
    [(NSDistantObject *)_rootProxy setProtocolForProxy:@protocol(OEGameCoreHelper)];
    
    return YES;
}

- (void)endHelperProcess
{
    [_rootProxy stopEmulation];
    
    // kill our background friend
    [_helper stopProcess];
    _helper = nil;
    
    _rootProxy = nil;
    
    _taskConnection = nil;
}

#pragma mark -
#pragma mark TaskWrapper delegate methods
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

#pragma mark -
#pragma mark Manager using a background thread

@implementation OEGameCoreThreadManager
{
    // IPC from our OEHelper
    NSThread             *_helper;
    NSString             *_taskUUIDForDOServer;
    NSConnection         *_taskConnection;
    NSError              *_error;

    OpenEmuHelperApp     *_helperObject;
}

- (void)executionThread:(id)object
{
    @autoreleasepool
    {
        _taskUUIDForDOServer = [NSString stringWithUUID];
        
        [[NSThread currentThread] setName:[OEHelperServerNamePrefix stringByAppendingString:_taskUUIDForDOServer]];
        
        _helperObject = [[OpenEmuHelperApp alloc] init];
        
        NSError *localError;
        
        if([_helperObject launchConnectionWithIdentifierSuffix:_taskUUIDForDOServer error:&localError])
            CFRunLoopRun();
        else
            _error = localError;
    }
}

- (void)dumpUpperLoop
{
    CFRunLoopStop(CFRunLoopGetCurrent());
}

- (void)stopRunLoop
{
    [_helperObject stopEmulation];
    CFRunLoopStop(CFRunLoopGetCurrent());
    
    [self performSelector:@selector(dumpUpperLoop) onThread:[NSThread currentThread] withObject:nil waitUntilDone:NO];
}

- (BOOL)startHelperProcessError:(NSError **)outError
{
    _helper = [[NSThread alloc] initWithTarget:self selector:@selector(executionThread:) object:nil];
    [_helper start];
    
    if(![_helper isExecuting])
    {
        if(outError != NULL)
            *outError = [NSError errorWithDomain:OEGameDocumentErrorDomain
                                            code:OEHelperAppNotRunningError
                                        userInfo:
                         [NSDictionary dictionaryWithObjectsAndKeys:
                          NSLocalizedString(@"The background process couldn't be launched", @"Not running background process error"), NSLocalizedFailureReasonErrorKey,
                          _error, NSUnderlyingErrorKey,
                          nil]];
        return NO;
    }
    
    // now that we launched the helper, start up our NSConnection for DO object vending and configure it
    // this is however a race condition if our helper process is not fully launched yet. 
    // we hack it out here. Normally this while loop is not noticable, its very fast
    
    NSDate *start = [NSDate date];
    
    _taskConnection = nil;
    while(_taskConnection == nil)
    {
        _taskConnection = [NSConnection connectionWithRegisteredName:[NSString stringWithFormat:@"org.openemu.OpenEmuHelper-%@", _taskUUIDForDOServer] host:nil];
        
        if(_error != nil && ![_helper isExecuting])
        {
            if (outError) *outError = _error;
            return NO;
        }
        
        if(-[start timeIntervalSinceNow] > 3.0)
        {
            [self endHelperProcess];
            if(outError != NULL)
            {
                *outError = [NSError errorWithDomain:OEGameDocumentErrorDomain
                                                code:OEConnectionTimedOutError
                                            userInfo:[NSDictionary dictionaryWithObject:NSLocalizedString(@"Couldn't connect to the background process.", @"Timed out error reason.") forKey:NSLocalizedFailureReasonErrorKey]];
            }
            return NO;
        }
    }
    
    
    if(![_taskConnection isValid])
    {
        [self endHelperProcess];
        if(outError != NULL)
        {
            *outError = [NSError errorWithDomain:OEGameDocumentErrorDomain
                                            code:OEInvalidHelperConnectionError
                                        userInfo:[NSDictionary dictionaryWithObject:NSLocalizedString(@"The background process connection couldn't be established", @"Invalid helper connection error reason.") forKey:NSLocalizedFailureReasonErrorKey]];
        }
        return NO;
    }
    
    // now that we have a valid connection...
    _rootProxy = (id <OEGameCoreHelper>)[_taskConnection rootProxy];
    if(_rootProxy == nil)
    {
        NSLog(@"nil root proxy object?");
        [self endHelperProcess];
        if(outError != NULL)
        {
            *outError = [NSError errorWithDomain:OEGameDocumentErrorDomain
                                            code:OENilRootProxyObjectError
                                        userInfo:[NSDictionary dictionaryWithObject:NSLocalizedString(@"The root proxy object is nil.", @"Nil root proxy object error reason.") forKey:NSLocalizedFailureReasonErrorKey]];
        }
        return NO;
    }
    
    [(NSDistantObject *)_rootProxy setProtocolForProxy:@protocol(OEGameCoreHelper)];
    
    return YES;
}

- (void)endHelperProcess
{
    // kill our background friend
    [self performSelector:@selector(stopRunLoop) onThread:_helper withObject:nil waitUntilDone:NO];
    
    // Runs the runloop until the helper is actually done to prevent deadlocks if the game core wants the main thread to do stuff...
    while([_helperObject isRunning]) CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.1, YES);
    
    _helper = nil;
    
    _rootProxy = nil;
    
    _taskConnection = nil;
}

@end
