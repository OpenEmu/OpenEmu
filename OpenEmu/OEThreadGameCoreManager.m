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
#import "OECorePlugin.h"
#import "OEGameCoreManager_Internal.h"
#import "OESystemPlugin.h"
#import "OpenEmuHelperApp.h"

@implementation OEThreadGameCoreManager
{
    NSThread         *_helperThread;
    NSTimer          *_dummyTimer;

    OEThreadProxy    *_helperProxy;
    OpenEmuHelperApp *_helper;

    OEThreadProxy    *_gameCoreOwnerProxy;

    void(^_completionHandler)(void);
    void(^_errorHandler)(NSError *error);
    void(^_stopHandler)(void);
}

- (void)loadROMWithCompletionHandler:(void(^)(void))completionHandler errorHandler:(void(^)(NSError *))errorHandler;
{
    _completionHandler = [completionHandler copy];
    _errorHandler = [errorHandler copy];

    _helperThread = [[NSThread alloc] initWithTarget:self selector:@selector(_executionThread:) object:nil];

    _helper = [[OpenEmuHelperApp alloc] init];
    _helperProxy = [OEThreadProxy threadProxyWithTarget:_helper thread:_helperThread];

    _gameCoreOwnerProxy = [OEThreadProxy threadProxyWithTarget:[self gameCoreOwner] thread:[NSThread mainThread]];

    [_helperThread start];
}

- (void)dummyTimer:(NSTimer *)dummyTimer
{
    
}

- (void)_executionThread:(id)object
{
    @autoreleasepool
    {
        [[NSThread currentThread] setName:@"org.openemu.core-manager-thread"];
        [[NSThread currentThread] setQualityOfService:NSQualityOfServiceUserInitiated];

        [self setGameCoreHelper:(id<OEGameCoreHelper>)_helperProxy];
        [_helper setGameCoreOwner:(id<OEGameCoreOwner>)_gameCoreOwnerProxy];

        NSError *error;
        if(![_helper loadROMAtPath:[self ROMPath] romCRC32:[self ROMCRC32] romMD5:[self ROMMD5] romHeader:[self ROMHeader] romSerial:[self ROMSerial] systemRegion:[self systemRegion] withCorePluginAtPath:[[self plugin] path] systemPluginPath:[[self systemPlugin] path] error:&error])
        {
            FIXME("Return a proper error object here.");
            if(_errorHandler != nil)
            {
                _errorHandler(error);
                _errorHandler = nil;
            }
            return;
        }

        if (_completionHandler) {
            dispatch_async(dispatch_get_main_queue(), _completionHandler);
            _completionHandler = nil;
        }

        _dummyTimer = [NSTimer scheduledTimerWithTimeInterval:1e9 target:self selector:@selector(dummyTimer:) userInfo:nil repeats:YES];

        CFRunLoopRun();

        if(_stopHandler)
        {
            dispatch_async(dispatch_get_main_queue(), _stopHandler);
            _stopHandler = nil;
        }
    }
}

- (void)_stopHelperThread:(id)object
{
    [_dummyTimer invalidate];
    _dummyTimer = nil;

    CFRunLoopStop(CFRunLoopGetCurrent());

    [self setGameCoreHelper:nil];

    _helperThread       = nil;
    _helperProxy        = nil;
    _helper             = nil;
    _gameCoreOwnerProxy = nil;
    _completionHandler  = nil;
    _errorHandler       = nil;
}

- (void)stop
{
    if([NSThread currentThread] == _helperThread)
        [self _stopHelperThread:nil];
    else
        [self performSelector:@selector(_stopHelperThread:) onThread:_helperThread withObject:nil waitUntilDone:NO];
}

- (void)stopEmulationWithCompletionHandler:(void (^)(void))handler
{
    _stopHandler = [handler copy];
    [[self gameCoreHelper] stopEmulationWithCompletionHandler:
     ^{
         [self stop];
     }];
}

@end
