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

#import "OEXPCInterface.h"
#import <objc/runtime.h>

@implementation OEXPCInterface
{
    Protocol *_baseProtocol;
    NSArray *_interfaces;
    CFMutableDictionaryRef _selectorArguments;
    CFMutableDictionaryRef _cachedSelectors;
    NSMutableDictionary *_cachedStringSelectors;
}

enum {
    _OESelectorArgumentIsString = 0x10000000,
    _OEArgumentIndexMask        = 0x0000FFFF,
};

static Class _NSXPCInterfaceClass;
static id (*_old_NSXPCInterface_interfaceWithProtocol_)(id, SEL, Protocol *);

+ (void)load
{
    int major, minor;
    GetSystemVersion(&major, &minor, NULL);
    if(major != 10 || minor != 8) return;

    // Only for Mountain Lion.
    _NSXPCInterfaceClass = [NSXPCInterface class];
    Method allocMethod = class_getClassMethod(_NSXPCInterfaceClass, @selector(interfaceWithProtocol:));
    _old_NSXPCInterface_interfaceWithProtocol_ = (void *)method_getImplementation(allocMethod);

    IMP implementation = imp_implementationWithBlock(^id (id cls, Protocol *protocol){
        OEXPCInterface *interface = [[OEXPCInterface alloc] init];
        [interface setProtocol:protocol];
        return interface;
    });

    class_replaceMethod(object_getClass(_NSXPCInterfaceClass), @selector(interfaceWithProtocol:), implementation, method_getTypeEncoding(allocMethod));
}

- (id)init
{
    _selectorArguments = CFDictionaryCreateMutable(NULL, 0, NULL, NULL);
    _cachedSelectors = CFDictionaryCreateMutable(NULL, 0, NULL, &kCFTypeDictionaryValueCallBacks);
    _cachedStringSelectors = [NSMutableDictionary dictionary];
    return self;
}

- (void)dealloc
{
    CFRelease(_selectorArguments);
    CFRelease(_cachedSelectors);
}

static NSXPCInterface *_interfaceForProtocol(Protocol *protocol)
{
    return _old_NSXPCInterface_interfaceWithProtocol_(_NSXPCInterfaceClass, @selector(interfaceWithProtocol:), protocol);
}

static void _addInterfacesForProtocol(Protocol *protocol, NSMutableArray *interfaces, NSMutableSet *excludedProtocols)
{
    if([excludedProtocols containsObject:protocol]) return;

    [excludedProtocols addObject:protocol];
    [interfaces addObject:_interfaceForProtocol(protocol)];

    unsigned count;
    __unsafe_unretained Protocol **protocols = protocol_copyProtocolList(protocol, &count);
    for(unsigned i = 0; i < count; i++)
        _addInterfacesForProtocol(protocols[i], interfaces, excludedProtocols);

    free(protocols);
}

static NSArray *_interfacesForProtocol(Protocol *protocol)
{
    NSMutableArray *array = [NSMutableArray array];
    NSMutableSet *excludedProtocols = [NSMutableSet setWithObject:@protocol(NSObject)];

    _addInterfacesForProtocol(protocol, array, excludedProtocols);

    return [array copy];
}

- (BOOL)isKindOfClass:(Class)aClass
{
    return [_interfaces[0] isKindOfClass:aClass];
}

- (Protocol *)protocol
{
    @synchronized(self)
    {
        return _baseProtocol;
    }
}

- (void)setProtocol:(Protocol *)protocol
{
    @synchronized(self)
    {
        if(_baseProtocol == protocol) return;

        _baseProtocol = protocol;

        [_cachedStringSelectors removeAllObjects];
        CFDictionaryRemoveAllValues(_cachedSelectors);

        _interfaces = _interfacesForProtocol(protocol);
    }
}

static BOOL _OEProtocolContainsSelector(Protocol *protocol, SEL selector)
{
#define FIND_SELECTOR(required, instance) do { \
    BOOL isDone = NO; \
    unsigned count; \
    struct objc_method_description *descs = protocol_copyMethodDescriptionList(protocol, required, instance, &count); \
    for(unsigned i = 0; i < count; i++) { \
        if(descs[i].name == selector) { \
            isDone = YES; \
            break; \
        } \
    } \
    free(descs); \
    if(isDone) return YES; \
} while(NO)
    FIND_SELECTOR(YES, YES);
    FIND_SELECTOR(YES, NO);
    FIND_SELECTOR(NO, YES);
    FIND_SELECTOR(NO, NO);

    return NO;
}

- (NSUInteger)OE_selectorArgumentIndexForInvocation:(NSInvocation *)invocation
{
    SEL selector = [invocation selector];
    const void *index = 0;
    if(CFDictionaryGetValueIfPresent(_selectorArguments, selector, &index)) return (NSUInteger)index;

    __block NSUInteger argumentIndex = 0;
    [[NSStringFromSelector(selector) componentsSeparatedByString:@":"] enumerateObjectsUsingBlock:
     ^(NSString *obj, NSUInteger idx, BOOL *stop)
     {
         if(![obj hasSuffix:@"Selector"]) return;

         argumentIndex = idx + 2;
         *stop = YES;
     }];

    if(argumentIndex != 0)
    {
        switch([[invocation methodSignature] getArgumentTypeAtIndex:argumentIndex][0])
        {
            case ':' : break;
            case '@' :
                argumentIndex |= _OESelectorArgumentIsString;
                break;
            default :
                NSAssert(NO, @"Argument type is wrong!");
                break;
        }
    }

    CFDictionarySetValue(_selectorArguments, selector, (void *)argumentIndex);
    return argumentIndex;
}

- (NSXPCInterface *)OE_searchInterfaceForSelector:(SEL)selector
{
    for(NSXPCInterface *interface in _interfaces)
        if(_OEProtocolContainsSelector([interface protocol], selector))
            return interface;

    return _interfaces[0];
}

- (NSXPCInterface *)OE_interfaceForSelector:(SEL)selector
{
    NSXPCInterface *interface = (__bridge NSXPCInterface *)CFDictionaryGetValue(_cachedSelectors, selector);
    if(interface != nil) return interface;

    interface = [self OE_searchInterfaceForSelector:selector];
    CFDictionarySetValue(_cachedSelectors, selector, (__bridge void *)interface);
    return interface;
}

- (NSXPCInterface *)OE_interfaceForSelectorString:(NSString *)selector
{
    NSXPCInterface *interface = _cachedStringSelectors[selector];
    if(interface != nil) return interface;

    interface = [self OE_searchInterfaceForSelector:NSSelectorFromString(selector)];
    _cachedStringSelectors[selector] = interface;
    return interface;
}

- (NSXPCInterface *)OE_interfaceForInvocation:(NSInvocation *)invocation
{
    NSUInteger argumentIndex = [self OE_selectorArgumentIndexForInvocation:invocation];

    if(argumentIndex == 0) return _interfaces[0];

    if(argumentIndex & _OESelectorArgumentIsString)
    {
        CFStringRef selectorString;
        [invocation getArgument:&selectorString atIndex:argumentIndex & _OEArgumentIndexMask];
        return [self OE_interfaceForSelectorString:(__bridge NSString *)selectorString];
    }

    SEL selector;
    [invocation getArgument:&selector atIndex:argumentIndex];
    return [self OE_interfaceForSelector:selector];
}

- (NSMethodSignature *)methodSignatureForSelector:(SEL)sel
{
    return [NSXPCInterface instanceMethodSignatureForSelector:sel];
}

- (void)forwardInvocation:(NSInvocation *)invocation
{
    [invocation invokeWithTarget:[self OE_interfaceForInvocation:invocation]];
}

@end
