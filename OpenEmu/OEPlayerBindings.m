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
#import "OEControlDescription.h"
#import "OEDeviceHandler.h"
#import "OEHIDEvent.h"

@implementation OEPlayerBindings
{
    NSMutableDictionary *_bindingDescriptions;
    NSMutableDictionary *_bindingEvents;
}

@synthesize systemBindingsController, playerNumber;

+ (id)allocWithZone:(NSZone *)zone
{
    NSAssert(self != [OEPlayerBindings class], @"Do not allocate instances of OEPlayerBindings");
    return [super allocWithZone:zone];
}

- (id)OE_initWithSystemBindings:(OESystemBindings *)aController playerNumber:(NSUInteger)aPlayerNumber;
{
    if((self = [super init]))
    {
        systemBindingsController = aController;
        playerNumber             = aPlayerNumber;
    }
    
    return self;
}

- (NSDictionary *)bindingDescriptions
{
    return _bindingDescriptions;
}

- (void)OE_setBindingDescriptions:(NSDictionary *)value
{
    if(_bindingDescriptions != value)
    {
        _bindingDescriptions = [value mutableCopy];
    }
}

- (NSDictionary *)bindingEvents
{
    return _bindingEvents;
}

- (void)OE_setBindingEvents:(NSDictionary *)value
{
    if(_bindingEvents != value)
    {
        _bindingEvents = [value mutableCopy];
    }
}

- (id)valueForKey:(NSString *)key
{
    if([key hasPrefix:@"@"]) return [super valueForKey:[key substringFromIndex:1]];
    
    return [[self bindingDescriptions] objectForKey:key];
}

- (id)assignEvent:(OEHIDEvent *)anEvent toKeyWithName:(NSString *)aKeyName;
{
    return anEvent != nil ? [[self systemBindingsController] OE_playerBindings:self didAssignEvent:anEvent toKeyWithName:aKeyName] : nil;
}

- (void)setValue:(id)value forKey:(NSString *)key
{
    if([key hasPrefix:@"@"]) return [super setValue:value forKey:[key substringFromIndex:1]];
}

- (id)OE_bindingDescriptionForKey:(NSString *)aKey;
{
    return [[self bindingDescriptions] objectForKey:aKey];
}

- (void)OE_setBindingDescription:(id)value forKey:(NSString *)aKey;
{
    [self willChangeValueForKey:@"bindingDescriptions"];
    [self willChangeValueForKey:aKey];
    
    if(value == nil) [_bindingDescriptions removeObjectForKey:aKey];
    else             [_bindingDescriptions setObject:value forKey:aKey];
    
    [self didChangeValueForKey:aKey];
    [self didChangeValueForKey:@"bindingDescriptions"];
}

- (id)OE_bindingEventForKey:(id)aKey;
{
    return [[self bindingEvents] objectForKey:aKey];
}

- (void)OE_setBindingEvent:(id)value forKey:(id)aKey;
{
    [self willChangeValueForKey:@"bindingEvents"];
    
    if(value == nil) [_bindingEvents removeObjectForKey:aKey];
    else             [_bindingEvents setObject:value forKey:aKey];
    
    [self didChangeValueForKey:@"bindingEvents"];
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@ %p playerNumber: %lu rawBindings: %@ bindings: %@>", [self class], self, [self playerNumber], [self bindingEvents], [self bindingDescriptions]];
}

@end

@implementation OEKeyboardPlayerBindings
@end

@interface OEDevicePlayerBindings ()
{
    NSMutableSet *dependentBindings;
}

// Represents the configuration on which self is based
// If originalBindingsController then self is changed
// If self is changed then originalBindingsController will become nil
// and self will manage its own configuration
@property(nonatomic, getter=OE_originalBindingsController, setter=OE_setOriginalBindingsController:) OEDevicePlayerBindings *originalBindingsController;

- (void)OE_devicePlayerBindingsDidBecomeIndependent:(OEDevicePlayerBindings *)bindings;

@end

static void *const OEDevicePlayerBindingOriginalBindingsObserver = (void *)&OEDevicePlayerBindingOriginalBindingsObserver;

@implementation OEDevicePlayerBindings
@synthesize deviceHandler;

- (id)OE_initWithSystemBindings:(OESystemBindings *)aController playerNumber:(NSUInteger)playerNumber
{
    return [self OE_initWithSystemBindings:aController playerNumber:playerNumber deviceHandler:nil];
}

- (id)OE_initWithSystemBindings:(OESystemBindings *)aController playerNumber:(NSUInteger)playerNumber deviceHandler:(OEDeviceHandler *)handler;
{
    if((self = [super OE_initWithSystemBindings:aController playerNumber:playerNumber]))
    {
        deviceHandler = handler;
    }
    
    return self;
}

- (id)OE_playerBindingsWithDeviceHandler:(OEDeviceHandler *)aHandler playerNumber:(NSUInteger)aPlayerNumber;
{
    if(_originalBindingsController != nil)
        return [_originalBindingsController OE_playerBindingsWithDeviceHandler:aHandler playerNumber:aPlayerNumber];
    
    OEDevicePlayerBindings *ret = [[OEDevicePlayerBindings alloc] OE_initWithSystemBindings:[self systemBindingsController] playerNumber:aPlayerNumber deviceHandler:aHandler];
    
    [ret OE_setOriginalBindingsController:self];
    
    if(dependentBindings == nil)
        dependentBindings = [[NSMutableSet alloc] initWithCapacity:1];
    
    [dependentBindings addObject:ret];
    
    return ret;
}

- (BOOL)OE_isDependent
{
    return _originalBindingsController != nil;
}

- (void)OE_makeIndependent;
{
    if(_originalBindingsController == nil) return;
    
    OEDevicePlayerBindings *oldParent = _originalBindingsController;
    [self OE_setOriginalBindingsController:nil];
    
    [self OE_setBindingDescriptions:[oldParent bindingDescriptions]];
    [self OE_setBindingEvents:[oldParent bindingEvents]];
    
    [oldParent OE_devicePlayerBindingsDidBecomeIndependent:self];
}

- (void)OE_devicePlayerBindingsDidBecomeIndependent:(OEDevicePlayerBindings *)bindings;
{
    [dependentBindings removeObject:bindings];
    
    if([dependentBindings count] == 0) dependentBindings = nil;
}

- (void)OE_setBindingDescription:(id)value forKey:(NSString *)aKey;
{
    NSAssert(_originalBindingsController == nil, @"Cannot set bindings when %@ is dependent on %@", self, _originalBindingsController);
    
    [super OE_setBindingDescription:value forKey:aKey];
}

- (void)OE_setBindingEvent:(id)value forKey:(NSString *)aKey;
{
    NSAssert(_originalBindingsController == nil, @"Cannot set raw bindings when %@ is dependent on %@", self, _originalBindingsController);
    
    [super OE_setBindingEvent:value forKey:aKey];
}

- (NSDictionary *)bindingDescriptions
{
    return _originalBindingsController != nil ? [_originalBindingsController bindingDescriptions] : [super bindingDescriptions];
}

- (void)OE_setBindingDescriptions:(NSDictionary *)value
{
    NSAssert(_originalBindingsController == nil, @"Cannot set bindings when %@ is dependent on %@", self, _originalBindingsController);
    
    [super OE_setBindingDescriptions:value];
}

- (OEDeviceHandler *)deviceHandler
{
    return deviceHandler;
}

- (void)OE_setDeviceHandler:(OEDeviceHandler *)value
{
    if(deviceHandler != value)
    {
        if(deviceHandler != nil && value != nil)
            NSLog(@"ERROR: Something fishy is happening here, %@ received handler %@, when handler %@ was already set.", self, deviceHandler, value);
        
        deviceHandler = value;
        
        // Forces bindings to be set to a specific device
        [self OE_setBindingEvents:[self bindingEvents]];
    }
}

- (void)OE_setBindingEvents:(NSDictionary *)value
{
    NSAssert(_originalBindingsController == nil, @"Cannot set raw bindings when %@ is dependent on %@", self, _originalBindingsController);
    [super OE_setBindingEvents:value];
}

- (void)willChangeValueForKey:(NSString *)key
{
    [super willChangeValueForKey:key];
    
    [dependentBindings makeObjectsPerformSelector:@selector(willChangeValueForKey:) withObject:key];
}

- (void)didChangeValueForKey:(NSString *)key
{
    [super didChangeValueForKey:key];
    
    [dependentBindings makeObjectsPerformSelector:@selector(didChangeValueForKey:) withObject:key];
}

@end
