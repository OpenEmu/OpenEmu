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

#import "OEProtocolFlatting.h"
#import <objc/runtime.h>
#import <objc/Protocol.h>

@interface Protocol (OEAdditions) <NSCopying>
@end

@implementation Protocol
- (id)copyWithZone:(NSZone *)zone
{
    return self;
}
@end

@protocol _OE_ParentDummyProtocol <NSObject>
- (void)dummyMethod:(id)dummy;
@end

@protocol _OE_ChildDummyProtocol <_OE_ParentDummyProtocol, NSObject>
@end

inline BOOL _OEXPCRequiresFlattenedProtcol(void)
{
    __block BOOL result = NO;

    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        NSXPCInterface *interface = [NSXPCInterface interfaceWithProtocol:@protocol(_OE_ChildDummyProtocol)];
        @try
        {
            [interface setClasses:[NSSet set] forSelector:@selector(dummyMethod:) argumentIndex:0 ofReply:NO];
        }
        @catch(NSException *exception)
        {
            result = YES;
        }
    });

    return result;
}

static BOOL _OEGetPropertyFlags(Protocol *protocolToRead, objc_property_t propertyToFind, BOOL *isRequired, BOOL *isInstance)
{
    const char *propertyName = property_getName(propertyToFind);
    objc_property_t property;

#define FIND_PROPERTY(required, instance) do { \
    property = protocol_getProperty(protocolToRead, propertyName, required, instance); \
    if(property == propertyToFind) \
    { \
        *isRequired = required; \
        *isInstance = instance; \
        return YES; \
    } \
} while(NO)

    FIND_PROPERTY(YES, YES);
    FIND_PROPERTY(NO, YES);
    FIND_PROPERTY(YES, NO);
    FIND_PROPERTY(NO, NO);

    return NO;
}

static void _OEAddPropertyToProtocol(Protocol *protocolToModify, Protocol *protocolToRead, objc_property_t propertyToAdd)
{
    const char *propertyName = property_getName(propertyToAdd);

    BOOL isRequired = NO;
    BOOL isInstance = NO;
    __unused BOOL succeeded = _OEGetPropertyFlags(protocolToRead, propertyToAdd, &isRequired, &isInstance);
    NSAssert(succeeded, @"Protocol '%@' does not contain the property '%s' it's supposed to contain", protocolToRead, propertyName);

    unsigned attributesCount = 0;
    objc_property_attribute_t *attributes = property_copyAttributeList(propertyToAdd, &attributesCount);

    protocol_addProperty(protocolToModify, propertyName, attributes, attributesCount, isRequired, isInstance);

    free(attributes);
}

static void _OEAddMethodsFromProtocol(Protocol *protocolToModify, Protocol *protocolToAdd, BOOL requiredMethods, BOOL instanceMethods)
{
    unsigned methodCount = 0;
    struct objc_method_description *methods = protocol_copyMethodDescriptionList(protocolToAdd, requiredMethods, instanceMethods, &methodCount);

    for(unsigned i = 0; i < methodCount; i++)
        protocol_addMethodDescription(protocolToModify, methods[i].name, methods[i].types, requiredMethods, instanceMethods);

    free(methods);
}

static void _OEAppendProtocol(Protocol *protocolToModify, Protocol *protocolToAdd, NSMutableSet *ignoredProtocols, BOOL *didFindNSObject)
{
    if([ignoredProtocols containsObject:protocolToAdd])
    {
        if(protocolToAdd == @protocol(NSObject))
            *didFindNSObject = YES;
        return;
    }

    [ignoredProtocols addObject:protocolToAdd];

    // Add parent protocols.
    unsigned conformingProtocolCount = 0;
    __unsafe_unretained Protocol **conformingProtocols = protocol_copyProtocolList(protocolToAdd, &conformingProtocolCount);
    for(unsigned i = 0; i < conformingProtocolCount; i++)
        _OEAppendProtocol(protocolToModify, conformingProtocols[i], ignoredProtocols, didFindNSObject);
    free(conformingProtocols);

    // Add properties.
    unsigned propertyCount = 0;
    objc_property_t *properties = protocol_copyPropertyList(protocolToAdd, &propertyCount);
    for(unsigned i = 0; i < propertyCount; i++)
        _OEAddPropertyToProtocol(protocolToModify, protocolToAdd, properties[i]);
    free(properties);

    // Add methods.
    _OEAddMethodsFromProtocol(protocolToModify, protocolToAdd, YES, YES);
    _OEAddMethodsFromProtocol(protocolToModify, protocolToAdd, NO, YES);
    _OEAddMethodsFromProtocol(protocolToModify, protocolToAdd, YES, NO);
    _OEAddMethodsFromProtocol(protocolToModify, protocolToAdd, NO, NO);
}

Protocol *OEFlattenedXPCProtocolForProtocol(Protocol *baseProtocol)
{
    if(!_OEXPCRequiresFlattenedProtcol())
        return baseProtocol;

    static dispatch_queue_t queue;
    static NSMutableDictionary *existingProtocols;

    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        queue = dispatch_queue_create("org.openemu.XPCProtocolCreation", DISPATCH_QUEUE_CONCURRENT);
        existingProtocols = [NSMutableDictionary dictionary];
    });

    __block Protocol *result = nil;
    dispatch_sync(queue, ^{
        result = existingProtocols[baseProtocol];
    });

    if(result != nil) return result;

    dispatch_barrier_sync(queue, ^{
        result = existingProtocols[baseProtocol];
        if(result != nil) return;

        NSString *protocolName = [NSString stringWithFormat:@"OEFlattenedXPCProtocol_%@", NSStringFromProtocol(baseProtocol)];
        Protocol *result = objc_allocateProtocol([protocolName UTF8String]);

        BOOL didFindNSObject = NO;
        NSMutableSet *ignoredProtocols = [NSMutableSet setWithObject:@protocol(NSObject)];
        _OEAppendProtocol(result, baseProtocol, ignoredProtocols, &didFindNSObject);

        if(didFindNSObject) protocol_addProtocol(result, @protocol(NSObject));

        objc_registerProtocol(result);
        existingProtocols[baseProtocol] = result;
    });

    return result;
}
