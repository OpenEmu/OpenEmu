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

#import "OEThreadProxy.h"

@implementation OEThreadProxy
{
    CFMutableDictionaryRef _cachedMethodSignatures;
}

+ (id)threadProxyWithTarget:(id)target;
{
    return [self threadProxyWithTarget:target thread:[[NSThread alloc] init]];
}

+ (id)threadProxyWithTarget:(id)target thread:(NSThread *)thread;
{
    return [[self alloc] initWithTarget:target thread:thread];
}

- (id)initWithTarget:(id)target thread:(NSThread *)thread
{
    _cachedMethodSignatures = CFDictionaryCreateMutable(NULL, 0, NULL, &kCFTypeDictionaryValueCallBacks);
    _target = target;
    _thread = thread;

    return self;
}

- (void)dealloc
{
    CFRelease(_cachedMethodSignatures);
}

- (id)forwardingTargetForSelector:(SEL)aSelector
{
    if([NSThread currentThread] == _thread)
    {
        NSLog(@"Will fast invoke: %@ with target: %@", _target, NSStringFromSelector(aSelector));
        return _target;
    }

    return nil;
}

- (NSMethodSignature *)methodSignatureForSelector:(SEL)sel
{
    NSMethodSignature *signature = (__bridge NSMethodSignature *)CFDictionaryGetValue(_cachedMethodSignatures, sel);

    if(signature != nil)
        return signature;

    if([_target respondsToSelector:sel])
        signature = [_target methodSignatureForSelector:sel];

    if(signature == nil)
        signature = [super methodSignatureForSelector:sel];

    if(signature != nil)
        CFDictionaryGetValue(_cachedMethodSignatures, (__bridge void *)signature);

    return signature;
}

- (void)forwardInvocation:(NSInvocation *)invocation
{
    if(![_target respondsToSelector:[invocation selector]])
        return;

    if([NSThread currentThread] == _thread)
        [invocation invokeWithTarget:_target];
    else
    {
        [invocation retainArguments];

        NSMethodSignature *signature = [invocation methodSignature];
        for(NSUInteger argIndex = 2, argCount = [signature numberOfArguments]; argIndex < argCount; argIndex++)
        {
            const char *type = [signature getArgumentTypeAtIndex:argIndex];
            if(type[0] == '@' && type[1] == '?')
            {
                void *argument = nil;
                [invocation getArgument:&argument atIndex:argIndex];
                if(argument == NULL) continue;

                argument = Block_copy(argument);
                [invocation setArgument:&argument atIndex:argIndex];
                Block_release(argument);
            }
        }
        [invocation performSelector:@selector(invokeWithTarget:) onThread:_thread withObject:_target waitUntilDone:NO];
    }
}

@end
