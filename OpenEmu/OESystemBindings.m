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

#import "OESystemBindings.h"
#import "OEBindingsController_Internal.h"
#import "OEHIDDeviceHandler.h"
#import "OESystemController.h"
#import "OEHIDEvent.h"

static NSString *const _OEBindingsPrefixAxis      = @"Axis.";
static NSString *const _OEBindingsPrefixHatSwitch = @"HatSwitch.";

@interface OESystemBindings ()
{
    NSMutableSet        *bindingsObservers;
    
    NSDictionary        *allKeyBindingsDescriptions;
    NSDictionary        *systemKeyBindingsDescriptions;
    NSDictionary        *keyBindingsDescriptions;
    NSArray             *keyGroupBindingsDescriptions;
    
    NSMutableArray      *keyboardPlayerBindings;
    
    // Map devices identifiers to an array of saved bindings
    // Each object in the array represent settings in the order they were added to the app
    // When a new device with the same manufacturer is plugged in, it inherits the settings
    // of the player they were first setup or the first player if the device doesn't exists
    NSMutableDictionary *manufacturerBindings;
    NSMutableDictionary *deviceHandlersToBindings;
    
    NSMutableArray      *devicePlayerBindings;
}

- (NSString *)OE_descriptionForEvent:(id)anEvent;

- (NSDictionary *)OE_dictionaryRepresentationForManufacturerBindings;
- (NSDictionary *)OE_dictionaryRepresentationForRawBindings:(NSDictionary *)bindingsToConvert;
- (NSDictionary *)OE_dictionaryRepresentationForBindings:(OEPlayerBindings *)bindingsController;
- (NSArray *)OE_dictionaryRepresentationsForBindingsInArray:(NSArray *)bindingsControllers;

- (void)OE_setupKeyBindingDescriptionsWithSystemController:(OESystemController *)aController;
- (NSDictionary *)OE_stringValuesForBindings:(NSDictionary *)bindingsToConvert possibleKeys:(NSDictionary *)nameToKeyMap;
- (NSDictionary *)OE_keyBindingsForKeyNames:(NSArray *)keyNames isSystemWide:(BOOL)systemWide;

- (void)OE_registerDefaultControls:(NSDictionary *)defaultControls;

- (void)OE_setupBindingsWithDictionaryRepresentation:(NSDictionary *)representation;
- (OEPlayerBindings *)OE_bindingsWithDictionaryRepresentation:(NSDictionary *)bindingsToConvert deviceBindings:(BOOL)isDevice playerNumber:(NSUInteger)playerNumber;
- (NSDictionary *)OE_rawBindingsForDictionaryRepresentation:(NSDictionary *)rawBindings withKeyDescriptions:(NSDictionary *)descriptions;

- (NSString *)OE_manufacturerKeyForDeviceHandler:(OEHIDDeviceHandler *)handler;
- (OEDevicePlayerBindings *)OE_deviceBindingsForDeviceHandler:(OEHIDDeviceHandler *)aHandler;
- (NSUInteger)OE_addDeviceBindings:(OEDevicePlayerBindings *)bindingsController;
- (void)OE_notifyObserversForAddedDeviceBindings:(OEDevicePlayerBindings *)aHandler;
- (void)OE_notifyObserversForRemovedDeviceBindings:(OEDevicePlayerBindings *)aHandler;

- (void)OE_notifyExistingBindings:(OEPlayerBindings *)bindings toObserver:(id<OESystemBindingsObserver>)observer;
- (void)OE_notifyExistingBindingsInArray:(NSArray *)bindingsArray toObserver:(id<OESystemBindingsObserver>)observer;
- (void)OE_notifyObserversDidSetEvent:(OEHIDEvent *)anEvent forBindingKey:(id)bindingKey playerNumber:(NSUInteger)playerNumber  __attribute__((nonnull));
- (void)OE_notifyObserversDidUnsetEvent:(OEHIDEvent *)anEvent forBindingKey:(id)bindingKey playerNumber:(NSUInteger)playerNumber  __attribute__((nonnull));

@end

@implementation OESystemBindings
@synthesize bindingsController, systemController;

- (id)OE_initWithBindingsController:(OEBindingsController *)parentController systemController:(OESystemController *)aController dictionaryRepresentation:(NSDictionary *)aDictionary
{
    if(aController == nil) return nil;
    
    if((self = [super init]))
    {
        manufacturerBindings     = [NSMutableDictionary dictionary];
        deviceHandlersToBindings = [NSMutableDictionary dictionary];
        
        keyboardPlayerBindings   = [NSMutableArray      array];
        devicePlayerBindings     = [NSMutableArray      array];
        
        bindingsObservers        = [NSMutableSet        set];
        
        bindingsController       = parentController;
        systemController         = aController;
        
        [self OE_setupKeyBindingDescriptionsWithSystemController:aController];
        
        if(aDictionary != nil) [self OE_setupBindingsWithDictionaryRepresentation:aDictionary];
        else                   [self OE_registerDefaultControls:[systemController defaultControls]];
    }
    
    return self;
}

- (NSDictionary *)OE_dictionaryRepresentation;
{
    NSMutableDictionary *dictionary = [NSMutableDictionary dictionaryWithCapacity:2];
    
    void (^addToDictionary)(NSString *key, id value) =
    ^(NSString *key, id value)
    {
        if(value == nil) return;
        
        [dictionary setObject:value forKey:key];
    };
    
    addToDictionary(@"manufacturerBindings"  , [self OE_dictionaryRepresentationForManufacturerBindings]);
    addToDictionary(@"keyboardPlayerBindings", [self OE_dictionaryRepresentationsForBindingsInArray:keyboardPlayerBindings]);
    
    return [dictionary copy];
}

- (void)OE_setupKeyBindingDescriptionsWithSystemController:(OESystemController *)aController;
{
    // Convert control names inot key bindings descriptions
    // -genericControlNames array contains all keys including system ones
    // System keys are set into a separate dictionary, but key indexes are shared for simplicity
    NSArray *systemControlNames  = [aController systemControlNames];
    NSArray *genericControlNames = [aController genericControlNames];
    
    if([systemControlNames count] == 0) systemControlNames = nil;
    
    NSMutableDictionary *systemKeyDescs  = systemControlNames != nil ? [NSMutableDictionary dictionaryWithCapacity:[systemControlNames count]] : nil;
    NSMutableDictionary *genericKeyDescs = [NSMutableDictionary dictionaryWithCapacity:[genericControlNames count]];
    NSMutableDictionary *allKeyDescs     = [NSMutableDictionary dictionaryWithCapacity:[genericControlNames count]];
    
    [genericControlNames enumerateObjectsUsingBlock:
     ^(NSString *obj, NSUInteger idx, BOOL *stop)
     {
         BOOL systemWide = systemControlNames != nil && [systemControlNames containsObject:obj];
         
         OEKeyBindingDescription *keyDesc = [[OEKeyBindingDescription alloc] OE_initWithName:obj index:idx isSystemWide:systemWide];
         
         [systemWide ? systemKeyDescs : genericKeyDescs setObject:keyDesc forKey:obj];
         [allKeyDescs setObject:keyDesc forKey:obj];
     }];
    
    systemKeyBindingsDescriptions = [systemKeyDescs  copy];
    keyBindingsDescriptions       = [genericKeyDescs copy];
    allKeyBindingsDescriptions    = [allKeyDescs     copy];
    
    // Build key-groups to let keys know about their counter-parts
    NSArray *hatSwitchControls = [aController hatSwitchControls];
    NSArray *axisControls      = [aController axisControls];
    
    keyGroupBindingsDescriptions = [[self OE_keyGroupsForControls:hatSwitchControls type:OEKeyGroupTypeHatSwitch availableKeys:keyBindingsDescriptions] arrayByAddingObjectsFromArray:[self OE_keyGroupsForControls:axisControls type:OEKeyGroupTypeAxis availableKeys:keyBindingsDescriptions]];
    
    for(OEKeyBindingGroupDescription *group in keyGroupBindingsDescriptions)
    {
        NSAssert([group isMemberOfClass:[OEKeyBindingGroupDescription class]], @"SOMETHING'S FISHY");
    }
}

- (void)OE_setupBindingsWithDictionaryRepresentation:(NSDictionary *)representation;
{
    if(representation == nil) return;
    
    NSDictionary *manufacturers = [representation objectForKey:@"manufacturerBindings"];
    manufacturerBindings        = [NSMutableDictionary dictionaryWithCapacity:[manufacturers count]];
    
    // Convert manufacturer bindings
    [manufacturers enumerateKeysAndObjectsWithOptions:NSEnumerationConcurrent usingBlock:
     ^(NSString *key, NSArray *obj, BOOL *stop)
     {
         NSMutableArray *conv = [NSMutableArray arrayWithCapacity:[obj count]];
         
         for(NSDictionary *bindings in obj)
             [conv addObject:[self OE_bindingsWithDictionaryRepresentation:bindings deviceBindings:YES playerNumber:0]];
         
         [manufacturerBindings setObject:conv forKey:key];
     }];
    
    // Convert keyboard bindings
    NSArray *encodedBindings = [representation objectForKey:@"keyboardPlayerBindings"];
    keyboardPlayerBindings = [[NSMutableArray alloc] initWithCapacity:[encodedBindings count]];
    
    [encodedBindings enumerateObjectsUsingBlock:
     ^(NSDictionary *encoded, NSUInteger idx, BOOL *stop)
     {
         [keyboardPlayerBindings addObject:[self OE_bindingsWithDictionaryRepresentation:encoded deviceBindings:NO playerNumber:idx + 1]];
     }];
}

- (void)OE_registerDefaultControls:(NSDictionary *)defaultControls
{
    if([defaultControls count] == 0) return;
    
    // We always assume player 1, if we want to have multiple players, we will have to use an NSArray instead
    OEPlayerBindings *bindings = [self keyboardPlayerBindingsForPlayer:1];
    
    [defaultControls enumerateKeysAndObjectsUsingBlock:
     ^(NSString *key, NSNumber *obj, BOOL *stop)
     {
         OEHIDEvent *theEvent = [OEHIDEvent keyEventWithTimestamp:0
                                                          keyCode:[obj unsignedIntValue]
                                                            state:NSOnState
                                                           cookie:NSNotFound];
         
         [bindings assignEvent:theEvent toKeyWithName:key];
     }];
}

- (NSUInteger)numberOfPlayers
{
    return [systemController numberOfPlayers];
}

- (NSArray *)keyboardPlayerBindings;
{
    return keyboardPlayerBindings;
}

- (NSArray *)devicePlayerBindings;
{
    return devicePlayerBindings;
}

#pragma mark -
#pragma mark Encoding Runtime Representation for Archiving

- (NSDictionary *)OE_dictionaryRepresentationForManufacturerBindings;
{
    NSMutableDictionary *ret = [NSMutableDictionary dictionaryWithCapacity:[manufacturerBindings count]];
    
    [manufacturerBindings enumerateKeysAndObjectsUsingBlock:
     ^(NSString *key, NSArray *obj, BOOL *stop)
     {
         NSArray *saveValue = [self OE_dictionaryRepresentationsForBindingsInArray:obj];
         
         if([saveValue count] > 0) [ret setObject:saveValue forKey:key];
     }];
    
    return ret;
}

- (NSArray *)OE_dictionaryRepresentationsForBindingsInArray:(NSArray *)bindingsControllers;
{
    NSMutableArray *ret = [NSMutableArray arrayWithCapacity:[bindingsControllers count]];
    
    for(OEPlayerBindings *bindings in bindingsControllers)
        [ret addObject:[self OE_dictionaryRepresentationForBindings:bindings]];
    
    return ret;
}

- (NSDictionary *)OE_dictionaryRepresentationForBindings:(OEPlayerBindings *)controller;
{
    return [self OE_dictionaryRepresentationForRawBindings:[controller OE_rawBindings]];
}

- (NSDictionary *)OE_dictionaryRepresentationForRawBindings:(NSDictionary *)bindingsToConvert;
{
    NSMutableDictionary *ret = [NSMutableDictionary dictionaryWithCapacity:[bindingsToConvert count]];
    
    [bindingsToConvert enumerateKeysAndObjectsUsingBlock:
     ^(id key, id obj, BOOL *stop)
     {
         NSString *saveKey = nil;
         if([key isKindOfClass:[OEKeyBindingDescription class]])
             saveKey = [key name];
         else if([key isKindOfClass:[OEOrientedKeyGroupBindingDescription class]])
             saveKey = [[(OEOrientedKeyGroupBindingDescription *)key type] == OEKeyGroupTypeAxis ? _OEBindingsPrefixAxis : _OEBindingsPrefixHatSwitch stringByAppendingString:[[key baseKey] name]];
         
         if(saveKey != nil) [ret setObject:obj forKey:saveKey];
     }];
    
    return ret;
}

- (OEKeyGroupType)OE_typeForEncodedKey:(NSString *)aKey actualKeyName:(NSString **)realKey;
{
    OEKeyGroupType ret = OEKeyGroupTypeUnknown;
    
    if([aKey hasPrefix:_OEBindingsPrefixAxis])
    {
        ret = OEKeyGroupTypeAxis;
        if(realKey != NULL) *realKey = [aKey substringFromIndex:[_OEBindingsPrefixAxis length]];
    }
    else if([aKey hasPrefix:_OEBindingsPrefixHatSwitch])
    {
        ret = OEKeyGroupTypeHatSwitch;
        if(realKey != NULL) *realKey = [aKey substringFromIndex:[_OEBindingsPrefixHatSwitch length]];
    }
    else if(realKey != NULL) *realKey = aKey;
    
    return ret;
}

- (NSDictionary *)OE_keyBindingsForKeyNames:(NSArray *)keyNames isSystemWide:(BOOL)systemWide;
{
    if([keyNames count] == 0) return nil;
    
    NSMutableDictionary *ret = [NSMutableDictionary dictionaryWithCapacity:[keyNames count]];
    
    [keyNames enumerateObjectsUsingBlock:
     ^(NSString *obj, NSUInteger idx, BOOL *stop)
     {
         OEKeyBindingDescription *keyDesc = [[OEKeyBindingDescription alloc] OE_initWithName:obj index:idx isSystemWide:systemWide];
         
         [ret setObject:keyDesc forKey:obj];
     }];
    
    return [ret copy];
}

- (NSArray *)OE_keyGroupsForControls:(NSArray *)controls type:(OEKeyGroupType)aType availableKeys:(NSDictionary *)availableKeys;
{
    NSMutableArray *ret = [NSMutableArray arrayWithCapacity:[controls count]];
    
    for(NSArray *keyNames in controls)
    {
        NSMutableArray *keys = [NSMutableArray arrayWithCapacity:[keyNames count]];
        
        for(NSString *keyName in keyNames)
            [keys addObject:[availableKeys objectForKey:keyName]];
        
        [ret addObject:[[OEKeyBindingGroupDescription alloc] initWithGroupType:aType keys:keys]];
    }
    
    return [ret copy];
}

#pragma mark -
#pragma mark Decoding Archived Bindings for Runtime Representation

- (OEPlayerBindings *)OE_bindingsWithDictionaryRepresentation:(NSDictionary *)bindingsToConvert deviceBindings:(BOOL)isDevice playerNumber:(NSUInteger)playerNumber;
{
    NSDictionary *decodedBindings = [self OE_rawBindingsForDictionaryRepresentation:bindingsToConvert withKeyDescriptions:allKeyBindingsDescriptions];
    
    OEPlayerBindings *controller =
    (isDevice
     ? [[OEDevicePlayerBindings   alloc] OE_initWithSystemBindings:self playerNumber:playerNumber deviceHandler:nil]
     : [[OEKeyboardPlayerBindings alloc] OE_initWithSystemBindings:self playerNumber:playerNumber]);
    
    [controller OE_setRawBindings:decodedBindings];
    [controller OE_setBindings:[self OE_stringValuesForBindings:decodedBindings possibleKeys:isDevice ? allKeyBindingsDescriptions : keyBindingsDescriptions]];
    
    return controller;
}

- (NSDictionary *)OE_rawBindingsForDictionaryRepresentation:(NSDictionary *)rawBindings withKeyDescriptions:(NSDictionary *)descriptions;
{
    NSMutableDictionary *ret = [NSMutableDictionary dictionaryWithCapacity:[rawBindings count]];
    
    [rawBindings enumerateKeysAndObjectsUsingBlock:
     ^(NSString *keyName, id value, BOOL *stop)
     {
         NSString *realKey = nil;
         OEKeyGroupType type = [self OE_typeForEncodedKey:keyName actualKeyName:&realKey];
         
         OEKeyBindingDescription *desc = [descriptions objectForKey:realKey];
         
         id savedKey = nil;
         
         switch(type)
         {
             case OEKeyGroupTypeUnknown   : savedKey = desc; break;
             case OEKeyGroupTypeAxis      :
                 savedKey = [[desc OE_axisGroup] orientedKeyGroupWithBaseKey:desc];
                 break;
             case OEKeyGroupTypeHatSwitch :
                 savedKey = [[desc OE_hatSwitchGroup] orientedKeyGroupWithBaseKey:desc];
                 break;
             default :
                 break;
         }
         
         if(savedKey == nil) return;
         
         [ret setObject:value forKey:savedKey];
     }];
    
    return ret;
}

#pragma mark -
#pragma mark Player Bindings Controller Representation

- (OEDevicePlayerBindings *)devicePlayerBindingsForPlayer:(NSUInteger)playerNumber;
{
    if(playerNumber == 0 || playerNumber > [self numberOfPlayers] || playerNumber > [devicePlayerBindings count]) return nil;
    
    id ret = [devicePlayerBindings objectAtIndex:playerNumber - 1];
    return ret != [NSNull null] ? ret : nil;
}

- (OEPlayerBindings *)keyboardPlayerBindingsForPlayer:(NSUInteger)playerNumber;
{
    if(playerNumber == 0 || playerNumber > [self numberOfPlayers]) return nil;
    
    NSUInteger count = [keyboardPlayerBindings count];
    
    if(count < playerNumber)
    {
        NSIndexSet *added = [NSIndexSet indexSetWithIndexesInRange:NSMakeRange(count, playerNumber - count)];
        [self willChange:NSKeyValueChangeInsertion valuesAtIndexes:added forKey:@"keyboardPlayerBindings"];
        
        while([keyboardPlayerBindings count] < playerNumber)
        {
            OEPlayerBindings *controller = [[OEKeyboardPlayerBindings alloc] OE_initWithSystemBindings:self playerNumber:[keyboardPlayerBindings count] + 1];
            
            // At this point, if a keyboard bindings doesn't exist, it means none were saved for this player
            [controller OE_setBindings:[self OE_stringValuesForBindings:nil possibleKeys:keyBindingsDescriptions]];
            [controller OE_setRawBindings:@{ }];
            
            [keyboardPlayerBindings addObject:controller];
        }
        
        [self didChange:NSKeyValueChangeInsertion valuesAtIndexes:added forKey:@"keyboardPlayerBindings"];
    }
    
    return [keyboardPlayerBindings objectAtIndex:playerNumber - 1];
}

- (NSUInteger)playerNumberForEvent:(OEHIDEvent *)anEvent;
{
    if([anEvent type] == OEHIDEventTypeKeyboard) return 0;
    
    NSUInteger deviceNumber = [anEvent padNumber];
    
    return [devicePlayerBindings indexOfObjectPassingTest:
            ^ BOOL (OEDevicePlayerBindings *obj, NSUInteger idx, BOOL *stop)
            {
                return obj != (id)[NSNull null] && [[obj deviceHandler] deviceNumber] == deviceNumber;
            }] + 1;
}

#pragma mark -
#pragma mark Preference Panel Representation Helper Methods

- (NSString *)OE_descriptionForEvent:(id)anEvent;
{
    if(anEvent == nil) return nil;
    
    return ([anEvent respondsToSelector:@selector(displayDescription)]
            ? [anEvent displayDescription]
            : [anEvent description]);
}

- (NSDictionary *)OE_stringValuesForBindings:(NSDictionary *)bindingsToConvert possibleKeys:(NSDictionary *)nameToKeyMap;
{
    NSMutableDictionary *ret = [NSMutableDictionary dictionaryWithCapacity:[nameToKeyMap count]];
    
    [bindingsToConvert enumerateKeysAndObjectsUsingBlock:
     ^(id key, id obj, BOOL *stop)
     {
         if([key isKindOfClass:[OEKeyBindingDescription class]])
             // General case - the event value is attached to the key-name for bindings
             [ret setObject:[self OE_descriptionForEvent:obj] forKey:[key name]];
         else if([key isKindOfClass:[OEOrientedKeyGroupBindingDescription class]])
         {
             OEOrientedKeyGroupBindingDescription *group = key;
             OEHIDEvent                 *event = obj;
             
             // In case of key-group, we need to create multiple key-strings for each cases of the key, usually 2 for axis type and 4 for hat switch type
             NSAssert([event isKindOfClass:[OEHIDEvent class]], @"Only OEHIDEvent can be associated to binding groups, got: %@ %@", [event class], event);
             
             switch([group type])
             {
                 case OEKeyGroupTypeAxis :
                 {
                     NSAssert([event type] == OEHIDEventTypeAxis, @"Excepting OEHIDEventTypeAxis event type for Axis key group, got: %@", NSStringFromOEHIDEventType([event type]));
                     
                     [ret setObject:OEHIDEventAxisDisplayDescription([event axis], [event direction]) forKey:[[group baseKey] name]];
                     [ret setObject:OEHIDEventAxisDisplayDescription([event axis], [event oppositeDirection]) forKey:[[group oppositeKey] name]];
                 }
                     break;
                 case OEKeyGroupTypeHatSwitch :
                 {
                     NSAssert([event type] == OEHIDEventTypeHatSwitch, @"Excepting OEHIDEventTypeHatSwitch event type for Axis key group, got: %@", NSStringFromOEHIDEventType([event type]));
                     
                     static OEHIDEventHatDirection dirs[] = {
                         OEHIDEventHatDirectionNorth,
                         OEHIDEventHatDirectionNorthEast,
                         OEHIDEventHatDirectionEast,
                         OEHIDEventHatDirectionSouthEast,
                         OEHIDEventHatDirectionSouth,
                         OEHIDEventHatDirectionSouthWest,
                         OEHIDEventHatDirectionWest,
                         OEHIDEventHatDirectionNorthWest
                     };
                     
                     OEHIDEventHatDirection baseDir = [event hatDirection];
                     
                     // First find what is the index of the base event direction in dirs[]
                     __block NSUInteger dirIdx = NSNotFound;
                     for(NSUInteger i = 0; i < 8; i++)
                     {
                         if(dirs[i] == baseDir)
                         {
                             dirIdx = i;
                             break;
                         }
                     }
                     
                     NSAssert(dirIdx != NSNotFound, @"Invalid hat direction for event %@", event);
                     
                     [group enumerateKeysFromBaseKeyUsingBlock:
                      ^(OEKeyBindingDescription *key, BOOL *stop)
                      {
                          [ret setObject:NSStringFromOEHIDHatDirection(dirs[dirIdx % 8]) forKey:[key name]];
                          
                          dirIdx += 2;
                      }];
                 }
                     break;
                 default :
                     NSAssert(NO, @"Unknown Key Group Type");
                     break;
             }
         }
     }];
    
    return ret;
}

- (id)OE_playerBindings:(OEPlayerBindings *)sender didAssignEvent:(OEHIDEvent *)anEvent toKeyWithName:(NSString *)keyName;
{
    NSAssert([anEvent isKindOfClass:[OEHIDEvent class]], @"Can only set OEHIDEvents for bindings.");
    
    // Make a copy because events are reused to remember the previous state/timestamp
    anEvent = [anEvent copy];
    
    id ret = ([anEvent type] == OEHIDEventTypeKeyboard
              ? [self OE_playerBindings:(OEKeyboardPlayerBindings *)sender didSetKeyboardEvent:anEvent forKey:keyName]
              : [self OE_playerBindings:(OEDevicePlayerBindings *)sender didSetDeviceEvent:anEvent forKey:keyName]);
    
    return ret;
}

- (id)OE_playerBindings:(OEKeyboardPlayerBindings *)sender didSetKeyboardEvent:(OEHIDEvent *)anEvent forKey:(NSString *)keyName;
{
    NSAssert([sender isKindOfClass:[OEKeyboardPlayerBindings class]], @"Invalid sender: OEKeyboardPlayerBindings expected, got: %@ %@", [sender class], sender);
    
    OEKeyBindingDescription *keyDesc = [allKeyBindingsDescriptions objectForKey:keyName];
    // Trying to set the same event to the same key, ignore it
    if([[[sender OE_rawBindings] objectForKey:keyDesc] isEqual:anEvent]) return keyDesc;
    
    for(OEPlayerBindings *playerBindings in keyboardPlayerBindings)
    {
        NSArray *keys = [[playerBindings OE_rawBindings] allKeysForObject:anEvent];
        NSAssert([keys count] <= 1, @"More than one key is attached to the same event: %@ -> %@", anEvent, keys);
        
        OEKeyBindingDescription *desc = [keys lastObject];
        if(desc != nil)
        {
            [playerBindings OE_setBindingsValue:nil forKey:[desc name]];
            [playerBindings OE_setRawBindingsValue:nil forKey:desc];
            
            [self OE_notifyObserversDidUnsetEvent:anEvent forBindingKey:desc playerNumber:[desc isSystemWide] ? 0 : [playerBindings playerNumber]];
        }
    }
    
    NSString *eventString = [self OE_descriptionForEvent:anEvent];
    
    [sender OE_setBindingsValue:eventString forKey:keyName];
    [sender OE_setRawBindingsValue:anEvent forKey:keyDesc];
    
    [self OE_notifyObserversDidSetEvent:anEvent forBindingKey:keyDesc playerNumber:[sender playerNumber]];
    [[self bindingsController] OE_setRequiresSynchronization];
    
    return keyDesc;
}

- (id)OE_playerBindings:(OEDevicePlayerBindings *)sender didSetDeviceEvent:(OEHIDEvent *)anEvent forKey:(NSString *)keyName;
{
    NSAssert([sender isKindOfClass:[OEDevicePlayerBindings class]], @"Invalid sender: OEKeyboardPlayerBindings expected, got: %@ %@", [sender class], sender);
    
    // sender is based on another device player bindings
    // it needs to be made independent and added to the manufacturer list
    if([sender OE_isDependent])
    {
        [sender OE_makeIndependent];
        
        [[manufacturerBindings objectForKey:[self OE_manufacturerKeyForDeviceHandler:[sender deviceHandler]]] addObject:sender];
    }
    
    // Search for keys bound to the same event
    NSArray *keys = [[sender OE_rawBindings] allKeysForObject:anEvent];
    if([keys count] == 0) keys = [[[sender OE_rawBindings] keysOfEntriesPassingTest:
                                   ^ BOOL (OEOrientedKeyGroupBindingDescription *key, OEHIDEvent *obj, BOOL *stop)
                                   {
                                       if(![key isKindOfClass:[OEOrientedKeyGroupBindingDescription class]]) return NO;
                                       
                                       return [obj isUsageEqualToEvent:anEvent];
                                   }] allObjects];
    
    // Remove bindings for these keys
    NSAssert([keys count] <= 1, @"More than one key is attached to the same event: %@ -> %@", anEvent, keys);
    id keyDesc = [keys lastObject];
    
    if(keyDesc != nil)
    {
        NSArray *keys = nil;
        if([keyDesc isKindOfClass:[OEKeyBindingDescription class]])
            keys = @[ [keyDesc name] ];
        else if([keyDesc isKindOfClass:[OEKeyBindingGroupDescription class]])
            keys = [keyDesc keyNames];
        
        for(NSString *key in keys) [sender OE_setBindingsValue:nil forKey:key];
        
        [sender OE_setRawBindingsValue:nil forKey:keyDesc];
        [self OE_notifyObserversDidUnsetEvent:anEvent forBindingKey:keyDesc playerNumber:[sender playerNumber]];
    }
    
    // Find the appropriate key for the event
    keyDesc = [allKeyBindingsDescriptions objectForKey:keyName];
    [self OE_removeConcurrentBindings:sender ofKey:keyDesc withEvent:anEvent];
    
    switch([anEvent type])
    {
        case OEHIDEventTypeAxis :
            if([keyDesc OE_axisGroup] != nil)
                keyDesc = [[keyDesc OE_axisGroup] orientedKeyGroupWithBaseKey:keyDesc];
            break;
        case OEHIDEventTypeHatSwitch :
            if([keyDesc OE_hatSwitchGroup] != nil)
                keyDesc = [[keyDesc OE_hatSwitchGroup] orientedKeyGroupWithBaseKey:keyDesc];
            break;
        default :
            break;
    }
    
    // Update the bindings for the event
    NSDictionary *eventStrings = [self OE_stringValuesForBindings:@{ keyDesc : anEvent } possibleKeys:allKeyBindingsDescriptions];
    
    [eventStrings enumerateKeysAndObjectsUsingBlock:
     ^(NSString *key, NSString *obj, BOOL *stop)
     {
         [sender OE_setBindingsValue:obj forKey:key];
     }];
    
    [sender OE_setRawBindingsValue:anEvent forKey:keyDesc];
    [self OE_notifyObserversDidSetEvent:anEvent forBindingKey:keyDesc playerNumber:[sender playerNumber]];
    [[self bindingsController] OE_setRequiresSynchronization];
    
    return keyDesc;
}

- (void)OE_removeConcurrentBindings:(OEDevicePlayerBindings *)sender ofKey:(OEKeyBindingDescription *)keyDesc withEvent:(OEHIDEvent *)anEvent;
{
    NSDictionary                  *rawBindings = [[sender OE_rawBindings] copy];
    OEKeyBindingGroupDescription *axisGroup   = [keyDesc OE_axisGroup];
    OEKeyBindingGroupDescription *hatGroup    = [keyDesc OE_hatSwitchGroup];
    
    if(axisGroup == nil && hatGroup == nil) return;
    
    switch([anEvent type])
    {
        case OEHIDEventTypeButton :
        {
            [rawBindings enumerateKeysAndObjectsUsingBlock:
             ^ void (OEOrientedKeyGroupBindingDescription *keyDesc, OEHIDEvent *anEvent, BOOL *stop)
             {
                 if(![keyDesc isKindOfClass:[OEOrientedKeyGroupBindingDescription class]]) return;
                 
                 OEKeyBindingGroupDescription *keyGroup = [keyDesc parentKeyGroup];
                 if(keyGroup != axisGroup && keyGroup != hatGroup) return;
                 
                 [[keyGroup keyNames] enumerateObjectsUsingBlock:
                  ^(NSString *key, NSUInteger idx, BOOL *stop)
                  {
                      [sender OE_setBindingsValue:nil forKey:key];
                  }];
                 
                 [sender OE_setRawBindingsValue:nil forKey:keyDesc];
                 [self OE_notifyObserversDidUnsetEvent:anEvent forBindingKey:keyDesc playerNumber:[sender playerNumber]];
             }];
        }
            break;
        case OEHIDEventTypeAxis :
        {
            for(OEKeyBindingDescription *keyDesc in [axisGroup keys])
            {
                OEHIDEvent *event = [rawBindings objectForKey:keyDesc];
                if(event != nil)
                {
                    [sender OE_setBindingsValue:nil forKey:[keyDesc name]];
                    [sender OE_setRawBindingsValue:nil forKey:keyDesc];
                    [self OE_notifyObserversDidUnsetEvent:anEvent forBindingKey:keyDesc playerNumber:[sender playerNumber]];
                }
            }
            
            [rawBindings enumerateKeysAndObjectsUsingBlock:
             ^ void (OEOrientedKeyGroupBindingDescription *keyDesc, OEHIDEvent *anEvent, BOOL *stop)
             {
                 if(![keyDesc isKindOfClass:[OEOrientedKeyGroupBindingDescription class]]) return;
                 
                 OEKeyBindingGroupDescription *keyGroup = [keyDesc parentKeyGroup];
                 if(keyGroup != axisGroup && keyGroup != hatGroup) return;
                 
                 [[keyDesc keyNames] enumerateObjectsUsingBlock:
                  ^(NSString *key, NSUInteger idx, BOOL *stop)
                  {
                      [sender OE_setBindingsValue:nil forKey:key];
                  }];
                 
                 [sender OE_setRawBindingsValue:nil forKey:keyDesc];
                 [self OE_notifyObserversDidUnsetEvent:anEvent forBindingKey:keyDesc playerNumber:[sender playerNumber]];
             }];
        }
            break;
        case OEHIDEventTypeHatSwitch :
        {
            NSMutableSet *visitedAxisGroups = [NSMutableSet setWithObjects:axisGroup, hatGroup, nil];
            for(OEKeyBindingDescription *keyDesc in [hatGroup keys])
            {
                OEHIDEvent *event = [rawBindings objectForKey:keyDesc];
                if(event != nil)
                {
                    [sender OE_setBindingsValue:nil forKey:[keyDesc name]];
                    [sender OE_setRawBindingsValue:nil forKey:keyDesc];
                    [self OE_notifyObserversDidUnsetEvent:anEvent forBindingKey:keyDesc playerNumber:[sender playerNumber]];
                }
                
                OEKeyBindingGroupDescription *temp = [keyDesc OE_axisGroup];
                if(temp != nil) [visitedAxisGroups addObject:temp];
            }
            
            [rawBindings enumerateKeysAndObjectsUsingBlock:
             ^(OEOrientedKeyGroupBindingDescription *keyDesc, OEHIDEvent *event, BOOL *stop)
             {
                 if(![visitedAxisGroups containsObject:keyDesc]) return;
                 
                 [[keyDesc keyNames] enumerateObjectsUsingBlock:
                  ^(NSString *key, NSUInteger idx, BOOL *stop)
                  {
                      [sender OE_setBindingsValue:nil forKey:key];
                  }];
                 
                 [sender OE_setRawBindingsValue:nil forKey:keyDesc];
                 [self OE_notifyObserversDidUnsetEvent:anEvent forBindingKey:keyDesc playerNumber:[sender playerNumber]];
             }];
        }
            break;
        default :
            break;
    }
}
    

- (void)OE_removeBindingsToEvent:(OEHIDEvent *)anEvent fromPlayerBindings:(NSArray *)controllers;
{
    for(OEPlayerBindings *playerBindings in controllers)
    {
        NSArray *allKeys = [[playerBindings OE_rawBindings] allKeysForObject:anEvent];
        NSAssert([allKeys count] <= 1, @"More than one key is attached to the same event: %@ -> %@", anEvent, allKeys);
        
        id keyDesc = [allKeys lastObject];
        if(keyDesc == nil) continue;
        
        if([keyDesc isKindOfClass:[OEKeyBindingDescription class]])
            [playerBindings OE_setBindingsValue:nil forKey:[keyDesc name]];
        else if([keyDesc isKindOfClass:[OEOrientedKeyGroupBindingDescription class]])
            [keyDesc enumerateKeysFromBaseKeyUsingBlock:
             ^(OEKeyBindingDescription *key, BOOL *stop)
             {
                 [playerBindings OE_setBindingsValue:nil forKey:[key name]];
             }];
        
        [playerBindings OE_setRawBindingsValue:nil forKey:keyDesc];
        [self OE_notifyObserversDidUnsetEvent:anEvent forBindingKey:keyDesc playerNumber:[playerBindings playerNumber]];
        
        // No need to go further the key should be set only once
        break;
    }
}

#pragma mark -
#pragma mark Device Handlers Management

- (NSString *)OE_manufacturerKeyForDeviceHandler:(OEHIDDeviceHandler *)handler;
{
    return [NSString stringWithFormat:@"%@ %@", [handler manufacturer], [handler product]];
}

- (void)OE_didAddDeviceHandler:(OEHIDDeviceHandler *)aHandler;
{
    [self OE_notifyObserversForAddedDeviceBindings:[self OE_deviceBindingsForDeviceHandler:aHandler]];
}

- (void)OE_didRemoveDeviceHandler:(OEHIDDeviceHandler *)aHandler;
{
    OEDevicePlayerBindings *controller = [deviceHandlersToBindings objectForKey:aHandler];
    
    if(controller == nil)
    {
        NSLog(@"WARNING: Trying to remove a device that was not registered with %@", self);
        return;
    }
    
    NSUInteger playerNumber = [controller playerNumber];
    
    // Don't bother replacing with NSNull if it's at the end of the array
    if(playerNumber == [devicePlayerBindings count])
    {
        NSIndexSet *removed = [NSIndexSet indexSetWithIndex:playerNumber - 1];
        [self willChange:NSKeyValueChangeRemoval valuesAtIndexes:removed forKey:@"devicePlayerBindings"];
        [devicePlayerBindings removeLastObject];
        [self didChange:NSKeyValueChangeRemoval valuesAtIndexes:removed forKey:@"devicePlayerBindings"];
    }
    else
    {
        NSIndexSet *removed = [NSIndexSet indexSetWithIndex:playerNumber - 1];
        [self willChange:NSKeyValueChangeReplacement valuesAtIndexes:removed forKey:@"devicePlayerBindings"];
        [devicePlayerBindings replaceObjectAtIndex:playerNumber - 1 withObject:[NSNull null]];
        [self didChange:NSKeyValueChangeReplacement valuesAtIndexes:removed forKey:@"devicePlayerBindings"];
    }
    
    [self OE_notifyObserversForRemovedDeviceBindings:controller];
    
    [controller OE_setDeviceHandler:nil];
    [controller OE_setPlayerNumber:0];
}

- (void)OE_notifyObserversForAddedDeviceBindings:(OEDevicePlayerBindings *)aHandler;
{
    NSUInteger playerNumber = [aHandler playerNumber];
    
    [[aHandler OE_rawBindings] enumerateKeysAndObjectsUsingBlock:
     ^(id key, id obj, BOOL *stop)
     {
         for(id<OESystemBindingsObserver> observer in bindingsObservers)
             [observer systemBindings:self didSetEvent:obj forBinding:key playerNumber:playerNumber];
     }];
}

- (void)OE_notifyObserversForRemovedDeviceBindings:(OEDevicePlayerBindings *)aHandler;
{
    NSUInteger playerNumber = [aHandler playerNumber];
    
    // Tell the controllers that the bindings are not used anymore
    [[aHandler OE_rawBindings] enumerateKeysAndObjectsUsingBlock:
     ^(id key, id event, BOOL *stop)
     {
         for(id<OESystemBindingsObserver> observer in bindingsObservers)
             [observer systemBindings:self didUnsetEvent:event forBinding:key playerNumber:playerNumber];
     }];
}

- (OEDevicePlayerBindings *)OE_deviceBindingsForDeviceHandler:(OEHIDDeviceHandler *)aHandler;
{
    OEDevicePlayerBindings *controller = [deviceHandlersToBindings objectForKey:aHandler];
    
    // The device was already registered with the system controller
    if(controller != nil) return controller;
    
    NSString       *manufacturerKey = [self OE_manufacturerKeyForDeviceHandler:aHandler];
    NSMutableArray *manuBindings    = [manufacturerBindings objectForKey:manufacturerKey];
    
    // Allocate a new array to countain OEDevicePlayerBindings objects for the given device type
    if(manuBindings == nil)
    {
        manuBindings = [NSMutableArray array];
        [manufacturerBindings setObject:manuBindings forKey:manufacturerKey];
    }
    
    // Search a suitable OEDevicePlayerBindings
    if([manuBindings count] == 0)
    {
        // This handler is the first of its kind for the application
        controller = [[OEDevicePlayerBindings alloc] OE_initWithSystemBindings:self playerNumber:0 deviceHandler:aHandler];
        [controller OE_setRawBindings:@{ }];
        [controller OE_setBindings:[self OE_stringValuesForBindings:nil possibleKeys:allKeyBindingsDescriptions]];
        
        // Add the device to the manufacturer's bindings even if the user never set any bindings
        [manuBindings addObject:controller];
    }
    else
    {
        // Search for an existing OEDevicePlayerBindings that is not yet assigned
        OEDevicePlayerBindings *first = nil;
        for(OEDevicePlayerBindings *ctrl in manuBindings)
        {
            if(first == nil) first = ctrl;
            
            if([ctrl deviceHandler] == nil)
            {
                controller = ctrl;
                break;
            }
        }
        
        if(controller != nil)
            // A free bindings controller was found, assign it the device
            // The device will receive the settings of that slot
            [controller OE_setDeviceHandler:aHandler];
        else
            // No free slot available, "duplicate" the first slot settings that already exist at this point
            // DO NOT add the new controller to the manufacturer, it will be added if the user changes this device specifically
            controller = [first OE_playerBindingsWithDeviceHandler:aHandler playerNumber:0];
    }
    
    // Keep track of device handlers
    [deviceHandlersToBindings setObject:controller forKey:aHandler];
    
    // Add it to the player list
    FIXME("What should we do if all player slots of the system are full?");
    [self OE_addDeviceBindings:controller];
    
    return controller;
}

- (NSUInteger)OE_addDeviceBindings:(OEDevicePlayerBindings *)controller;
{
    __block NSUInteger ret = NSNotFound;
    
    // Devices that are disconnected while a game is running are replaced by NSNull
    [devicePlayerBindings enumerateObjectsUsingBlock:
     ^(id obj, NSUInteger idx, BOOL *stop)
     {
         if(obj == [NSNull null])
         {
             ret = idx;
             *stop = NO;
         }
     }];
    
    if(ret == NSNotFound)
    {
        // Append the new device at the end of the list
        ret = [devicePlayerBindings count];
        
        NSIndexSet *indexSet = [NSIndexSet indexSetWithIndex:ret];
        [self willChange:NSKeyValueChangeInsertion valuesAtIndexes:indexSet forKey:@"devicePlayerBindings"];
        [devicePlayerBindings addObject:controller];
        [self didChange:NSKeyValueChangeInsertion valuesAtIndexes:indexSet forKey:@"devicePlayerBindings"];
    }
    else
    {
        NSIndexSet *indexSet = [NSIndexSet indexSetWithIndex:ret];
        [self willChange:NSKeyValueChangeReplacement valuesAtIndexes:indexSet forKey:@"devicePlayerBindings"];
        [devicePlayerBindings replaceObjectAtIndex:ret withObject:controller];
        [self didChange:NSKeyValueChangeReplacement valuesAtIndexes:indexSet forKey:@"devicePlayerBindings"];
    }
    
    [controller OE_setPlayerNumber:ret + 1];
    
    return ret;
}

#pragma mark -
#pragma mark Bindings Observers

- (void)OE_notifyObserversDidSetEvent:(OEHIDEvent *)anEvent forBindingKey:(id)bindingKey playerNumber:(NSUInteger)playerNumber;
{
    for(id<OESystemBindingsObserver> observer in bindingsObservers)
        [observer systemBindings:self didSetEvent:anEvent forBinding:bindingKey playerNumber:playerNumber];
}

- (void)OE_notifyObserversDidUnsetEvent:(OEHIDEvent *)anEvent forBindingKey:(id)bindingKey playerNumber:(NSUInteger)playerNumber;
{
    for(id<OESystemBindingsObserver> observer in bindingsObservers)
        [observer systemBindings:self didUnsetEvent:anEvent forBinding:bindingKey playerNumber:playerNumber];
}

- (void)OE_notifyExistingBindings:(OEPlayerBindings *)bindings toObserver:(id<OESystemBindingsObserver>)observer;
{
    if(bindings == nil) return;
    
    NSUInteger playerNumber = [bindings playerNumber];
    
    [[bindings OE_rawBindings] enumerateKeysAndObjectsUsingBlock:
     ^(id key, id event, BOOL *stop)
     {
         NSUInteger player = playerNumber;
         // playerNumber for system-wide keys should always be 0
         if(systemKeyBindingsDescriptions != nil                 &&
            [key isKindOfClass:[OEKeyBindingDescription class]] &&
            [key isSystemWide])
             player = 0;
         
         [observer systemBindings:self didSetEvent:event forBinding:key playerNumber:player];
     }];
}

- (void)OE_notifyExistingBindingsInArray:(NSArray *)bindingsArray toObserver:(id<OESystemBindingsObserver>)observer
{
    for(OEPlayerBindings *ctrl in bindingsArray)
        if(ctrl != (id)[NSNull null])
            [self OE_notifyExistingBindings:ctrl toObserver:observer];
}

- (void)addBindingsObserver:(id<OESystemBindingsObserver>)observer
{
    if([bindingsObservers containsObject:observer]) return;
    
    [self OE_notifyExistingBindingsInArray:keyboardPlayerBindings toObserver:observer];
    [self OE_notifyExistingBindingsInArray:devicePlayerBindings   toObserver:observer];
    
    [bindingsObservers addObject:observer];
}

- (void)removeBindingsObserver:(id<OESystemBindingsObserver>)observer
{
    // No need to tell it to unset everything
    [bindingsObservers removeObject:observer];
}

@end
