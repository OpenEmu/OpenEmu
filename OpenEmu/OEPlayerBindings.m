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

#import "OEPlayerBindings.h"
#import "OEBindingsController_Internal.h"
#import "OEHIDDeviceHandler.h"
#import "OEHIDEvent.h"

@interface OEHIDEvent ()
- (OEHIDEvent *)OE_eventWithDeviceHandler:(OEHIDDeviceHandler *)aDeviceHandler;
@end

@implementation OEPlayerBindings
{
    NSMutableDictionary *_bindings;
    NSMutableDictionary *_rawBindings;
}

@synthesize systemBindingsController, playerNumber;

- (id)OE_initWithSystemBindings:(OESystemBindings *)aController playerNumber:(NSUInteger)aPlayerNumber;
{
    if((self = [super init]))
    {
        systemBindingsController = aController;
        playerNumber             = aPlayerNumber;
    }
    
    return self;
}

- (NSDictionary *)OE_bindings
{
    return _bindings;
}

- (void)OE_setBindings:(NSDictionary *)value
{
    if(_bindings != value)
    {
        _bindings = [value mutableCopy];
    }
}

- (NSDictionary *)OE_rawBindings
{
    return _rawBindings;
}

- (void)OE_setRawBindings:(NSDictionary *)value
{
    if(_rawBindings != value)
    {
        _rawBindings = [value mutableCopy];
    }
}

- (id)valueForKey:(NSString *)key
{
    if([key hasPrefix:@"@"]) return [super valueForKey:[key substringFromIndex:1]];
    
    return [[self OE_bindings] objectForKey:key];
}

- (id)assignEvent:(OEHIDEvent *)anEvent toKeyWithName:(NSString *)aKeyName;
{
    return anEvent != nil ? [[self systemBindingsController] OE_playerBindings:self didAssignEvent:anEvent toKeyWithName:aKeyName] : nil;
}

- (void)setValue:(id)value forKey:(NSString *)key
{
    if([key hasPrefix:@"@"]) return [super setValue:value forKey:[key substringFromIndex:1]];
}

- (id)OE_bindingsValueForKey:(NSString *)aKey;
{
    return [[self OE_bindings] objectForKey:aKey];
}

- (void)OE_setBindingsValue:(id)value forKey:(NSString *)aKey;
{
    [self willChangeValueForKey:aKey];
    
    if(value == nil) [_bindings removeObjectForKey:aKey];
    else             [_bindings setObject:value forKey:aKey];
    
    [self didChangeValueForKey:aKey];
}

- (id)OE_rawBindingsValueForKey:(id)aKey;
{
    return [[self OE_rawBindings] objectForKey:aKey];
}

- (void)OE_setRawBindingsValue:(id)value forKey:(id)aKey;
{
    if(value == nil) [_rawBindings removeObjectForKey:aKey];
    else             [_rawBindings setObject:value forKey:aKey];
}

@end

@implementation OEKeyboardPlayerBindings
@end

@interface OEDevicePlayerBindings ()
{
    // Represents the configuration on which self is based
    // If originalBindingsController then self is changed
    // If self is changed then originalBindingsController will become nil
    // and self will manage its own configuration
    OEDevicePlayerBindings *originalBindingsController;
}

@end

@implementation OEDevicePlayerBindings
@synthesize deviceHandler;

- (id)OE_initWithSystemBindings:(OESystemBindings *)aController playerNumber:(NSUInteger)playerNumber
{
    return [self OE_initWithSystemBindings:aController playerNumber:playerNumber deviceHandler:nil];
}

- (id)OE_initWithSystemBindings:(OESystemBindings *)aController playerNumber:(NSUInteger)playerNumber deviceHandler:(OEHIDDeviceHandler *)handler;
{
    if((self = [super OE_initWithSystemBindings:aController playerNumber:playerNumber]))
    {
        deviceHandler = handler;
    }
    
    return self;
}

- (id)OE_playerBindingsWithDeviceHandler:(OEHIDDeviceHandler *)aHandler playerNumber:(NSUInteger)aPlayerNumber;
{
    OEDevicePlayerBindings *ret = [[OEDevicePlayerBindings alloc] OE_initWithSystemBindings:[self systemBindingsController] playerNumber:aPlayerNumber deviceHandler:aHandler];
    
    ret->originalBindingsController = self;
    
    return ret;
}

- (BOOL)OE_isDependent
{
    return originalBindingsController != nil;
}

- (void)OE_makeIndependent;
{
    if(originalBindingsController == nil) return;
    
    OEDevicePlayerBindings *oldParent = originalBindingsController;
    originalBindingsController = nil;
    
    [self OE_setBindings:[oldParent OE_bindings]];
    [self OE_setRawBindings:[oldParent OE_rawBindings]];
}

- (void)OE_setBindingsValue:(id)value forKey:(NSString *)aKey;
{
    NSAssert(originalBindingsController == nil, @"Cannot set bindings when %@ is dependent on %@", self, originalBindingsController);
    
    [super OE_setBindingsValue:value forKey:aKey];
}

- (void)OE_setRawBindingsValue:(id)value forKey:(NSString *)aKey;
{
    NSAssert(originalBindingsController == nil, @"Cannot set raw bindings when %@ is dependent on %@", self, originalBindingsController);
    
    [super OE_setRawBindingsValue:value forKey:aKey];
}

- (NSDictionary *)OE_bindings
{
    return originalBindingsController != nil ? [originalBindingsController OE_bindings] : [super OE_bindings];
}

- (void)OE_setBindings:(NSDictionary *)value
{
    NSAssert(originalBindingsController == nil, @"Cannot set bindings when %@ is dependent on %@", self, originalBindingsController);
    
    [super OE_setBindings:value];
}

- (NSDictionary *)OE_rawBindings
{
    return originalBindingsController != nil ? [originalBindingsController OE_rawBindings] : [super OE_rawBindings];
}

- (OEHIDDeviceHandler *)deviceHandler
{
    return deviceHandler;
}

- (void)OE_setDeviceHandler:(OEHIDDeviceHandler *)value
{
    if(deviceHandler != value)
    {
        if(deviceHandler != nil && value != nil)
            NSLog(@"ERROR: Something fishy is happening here, %@ received handler %@, when handler %@ was already set.", self, deviceHandler, value);
        
        deviceHandler = value;
        
        // Forces bindings to be set to a specific device
        [self OE_setRawBindings:[self OE_rawBindings]];
    }
}

- (void)OE_setRawBindings:(NSDictionary *)value
{
    NSAssert(originalBindingsController == nil, @"Cannot set raw bindings when %@ is dependent on %@", self, originalBindingsController);
    
    NSMutableDictionary *converted = [NSMutableDictionary dictionaryWithCapacity:[value count]];
    
    [value enumerateKeysAndObjectsUsingBlock:
     ^(NSString *key, OEHIDEvent *obj, BOOL *stop)
     {
         [converted setObject:[obj OE_eventWithDeviceHandler:[self deviceHandler]] forKey:key];
     }];
    
    [super OE_setRawBindings:converted];
}

- (void)setValue:(OEHIDEvent *)value forKey:(NSString *)key
{
    // Ignore the event if it's not attached to the device of the receiver
    if([value isKindOfClass:[OEHIDEvent class]] &&
       [value padNumber] != [[self deviceHandler] deviceNumber])
        return;
    
    [super setValue:value forKey:key];
}

@end
