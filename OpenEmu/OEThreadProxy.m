//
//  OEThreadProxy.m
//  OpenEmu
//
//  Created by Remy Demarest on 21/07/2013.
//
//

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
