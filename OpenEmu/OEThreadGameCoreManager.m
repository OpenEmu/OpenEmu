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

#import "OEThreadGameCoreManager.h"
#import "OEThreadProxy.h"
#import "OpenEmuHelperApp.h"
#import "OEGameCoreManager_Internal.h"
#import "OECorePlugin.h"

@implementation OEThreadGameCoreManager
{
    NSThread         *_helperThread;
    NSTimer          *_dummyTimer;

    OEThreadProxy    *_helperProxy;
    OpenEmuHelperApp *_helper;

    OEThreadProxy    *_displayHelperProxy;
    OEThreadProxy    *_systemClientProxy;
    id                _systemClient;

    void(^_completionHandler)(id systemClient, NSError *error);
}

- (void)loadROMWithCompletionHandler:(void(^)(id systemClient, NSError *error))completionHandler;
{
    _completionHandler = [completionHandler copy];

    _helperThread = [[NSThread alloc] initWithTarget:self selector:@selector(_executionThread:) object:nil];

    _helper = [[OpenEmuHelperApp alloc] init];
    _helperProxy = [OEThreadProxy threadProxyWithTarget:_helper thread:_helperThread];

    _displayHelperProxy = [OEThreadProxy threadProxyWithTarget:[self displayHelper] thread:[NSThread mainThread]];

    [_helperThread start];
}

- (void)dummyTimer:(NSTimer *)dummyTimer
{
    
}

- (void)_executionThread:(id)object
{
    @autoreleasepool
    {
        [self setGameCoreHelper:(id<OEGameCoreHelper>)_helperProxy];
        [_helper setDisplayHelper:(id<OEGameCoreDisplayHelper>)_displayHelperProxy];
        if(![_helper loadROMAtPath:[self ROMPath] withCorePluginAtPath:[[self plugin] path] systemIdentifier:[[self systemController] systemIdentifier]])
        {
            FIXME("Return a proper error object here.");
            _completionHandler(nil, nil);
            return;
        }

        _systemClient = [_helper gameCore];
        _systemClientProxy = [OEThreadProxy threadProxyWithTarget:_systemClient thread:_helperThread];

        dispatch_async(dispatch_get_main_queue(), ^{
            _completionHandler(_systemClientProxy, nil);
        });

        _dummyTimer = [NSTimer scheduledTimerWithTimeInterval:1e9 target:self selector:@selector(dummyTimer:) userInfo:nil repeats:YES];

        CFRunLoopRun();
    }
}

- (void)_stopHelperThread:(id)object
{
    [_helper stopEmulation];
    [_dummyTimer invalidate];
    _dummyTimer = nil;

    CFRunLoopStop(CFRunLoopGetCurrent());

    _helperThread       = nil;
    _helperProxy        = nil;
    _helper             = nil;
    _displayHelperProxy = nil;
    _systemClientProxy  = nil;
    _systemClient       = nil;
}

- (void)stop
{
    if([NSThread currentThread] == _helperThread)
        [self _stopHelperThread:nil];
    else
        [self performSelector:@selector(_stopHelperThread:) onThread:_helperThread withObject:nil waitUntilDone:YES];
}

@end
