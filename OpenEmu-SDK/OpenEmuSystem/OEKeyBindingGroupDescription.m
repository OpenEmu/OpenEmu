/*
 Copyright (c) 2012, OpenEmu Team
 
 
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

#import "OEKeyBindingGroupDescription.h"
#import "OEBindingsController_Internal.h"

NSString *NSStringFromOEKeyGroupType(OEKeyGroupType type)
{
    NSString *ret = @"<invalid>";
    
    switch(type)
    {
        case OEKeyGroupTypeAxis      : ret = @"OEKeyGroupTypeAxis";      break;
        case OEKeyGroupTypeHatSwitch : ret = @"OEKeyGroupTypeHatSwitch"; break;
        default : break;
    }
    
    return ret;
}

@interface OEOrientedKeyGroupBindingDescription ()
- (id)OE_initWithParentKeyGroup:(OEKeyBindingGroupDescription *)parent baseKey:(OEKeyBindingDescription *)base __attribute__((objc_method_family(init)));
@end

@implementation OEKeyBindingGroupDescription
{
    NSMutableDictionary     *orientedGroups;
    OEKeyBindingDescription *axisKeys[2];
}

@synthesize type, keys;

- (id)init
{
    return nil;
}

- (id)initWithGroupType:(OEKeyGroupType)aType keys:(NSArray *)groupedKeys;
{
    if(aType != OEKeyGroupTypeAxis && aType != OEKeyGroupTypeHatSwitch) return nil;
    
    if((self = [super init]))
    {
        orientedGroups = [NSMutableDictionary dictionaryWithCapacity:[groupedKeys count]];
        type = aType;
        keys = [groupedKeys copy];
        
        if([self class] == [OEKeyBindingGroupDescription class])
            [keys makeObjectsPerformSelector:(aType == OEKeyGroupTypeAxis
                                              ? @selector(OE_setAxisGroup:)
                                              : @selector(OE_setHatSwitchGroup:))
                                  withObject:self];
        
        if(type == OEKeyGroupTypeAxis)
        {
            axisKeys[0] = [groupedKeys objectAtIndex:0];
            axisKeys[1] = [groupedKeys objectAtIndex:1];
        }
    }
    
    return self;
}

- (id)copyWithZone:(NSZone *)zone
{
    return self;
}

- (NSArray *)keyNames
{
    return [keys valueForKey:@"name"];
}

- (BOOL)isEqual:(id)anObject;
{
    if(self == anObject) return YES;
    
    if(![anObject isKindOfClass:[OEKeyBindingGroupDescription class]])
        return NO;
    
    id comp = self;
    if([self isKindOfClass:[OEOrientedKeyGroupBindingDescription class]])
        comp = [comp parentKeyGroup];
    if([anObject isKindOfClass:[OEOrientedKeyGroupBindingDescription class]])
        anObject = [anObject parentKeyGroup];
    
    return comp == anObject;
}

- (OEKeyBindingDescription *)oppositeKeyOfKey:(OEKeyBindingDescription *)aKey;
{
    NSAssert(type == OEKeyGroupTypeAxis, @"Key Group type must be OEKeyGroupTypeAxis.");
    
    OEKeyBindingDescription *ret = nil;
    
    if(NO);
    else if(axisKeys[0] == aKey) ret = axisKeys[1];
    else if(axisKeys[1] == aKey) ret = axisKeys[0];
    
    NSAssert2(ret != nil, @"Key %@ is not part of the group %@", aKey, self);
    
    return ret;
}

- (OEOrientedKeyGroupBindingDescription *)orientedKeyGroupWithBaseKey:(OEKeyBindingDescription *)aKey
{
    NSAssert([keys containsObject:aKey], @"The base key must belong to the key group.");
    
    OEOrientedKeyGroupBindingDescription *ret = [orientedGroups objectForKey:aKey];
    
    if(ret == nil)
    {
        ret = [[OEOrientedKeyGroupBindingDescription alloc] OE_initWithParentKeyGroup:self baseKey:aKey];
        [orientedGroups setObject:ret forKey:aKey];
    }
    
    return ret;
}

- (NSUInteger)indexOfKey:(OEKeyBindingDescription *)aKey;
{
    return [keys indexOfObject:aKey];
}

- (void)enumerateKeysFromKey:(OEKeyBindingDescription *)baseKey usingBlock:(void(^)(OEKeyBindingDescription *key, BOOL *stop))block;
{
    NSUInteger count   = [keys count];
    NSUInteger baseIdx = [keys indexOfObject:baseKey];
    
    // It shouldn't happen but let's avoid weird stuff anyway
    if(count == 0 || baseIdx == NSNotFound) return;
    
    BOOL stop = NO;
    
    for(NSUInteger i = 0; i < count; i++)
    {
        block([keys objectAtIndex:(i + baseIdx) % count], &stop);
        
        if(stop) return;
    }
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@ %p type: %@ keys: { %@ }>", [self class], self, NSStringFromOEKeyGroupType([self type]), [[self keyNames] componentsJoinedByString:@", "]];
}

@end

@implementation OEOrientedKeyGroupBindingDescription
@synthesize parentKeyGroup, baseKey;

- (id)init
{
    return nil;
}

- (id)OE_initWithParentKeyGroup:(OEKeyBindingGroupDescription *)parent baseKey:(OEKeyBindingDescription *)base;
{
    NSAssert([[parent keys] containsObject:base], @"The base key must belong to the key group.");
    
    if((self = [super initWithGroupType:[parent type] keys:[parent keys]]))
    {
        parentKeyGroup = parent;
        baseKey        = base;
    }
    
    return self;
}

- (NSUInteger)hash
{
    return [[self parentKeyGroup] hash];
}

- (BOOL)isEqual:(id)object
{
    if([object isKindOfClass:[OEOrientedKeyGroupBindingDescription class]])
        return [self parentKeyGroup] == [object parentKeyGroup];
    
    return [super isEqual:object];
}

- (OEKeyBindingDescription *)oppositeKey;
{
    return [self oppositeKeyOfKey:baseKey];
}

- (NSUInteger)indexOfBaseKey;
{
    return [self indexOfKey:baseKey];
}

- (void)enumerateKeysFromBaseKeyUsingBlock:(void(^)(OEKeyBindingDescription *key, BOOL *stop))block;
{
    [self enumerateKeysFromKey:baseKey usingBlock:block];
}

- (OEOrientedKeyGroupBindingDescription *)orientedKeyGroupWithBaseKey:(OEKeyBindingDescription *)aKey
{
    return [[self parentKeyGroup] orientedKeyGroupWithBaseKey:aKey];
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@ %p baseKey: %@ parentKeyGroup: %@>", [self class], self, [[self baseKey] name], [self parentKeyGroup]];
}

@end
