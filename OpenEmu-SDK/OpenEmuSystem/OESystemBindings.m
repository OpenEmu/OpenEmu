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
#import "OEDeviceHandler.h"
#import "OESystemController.h"
#import "OEHIDEvent.h"
#import "OEHIDDeviceHandler.h"
#import "OEControllerDescription.h"
#import "OEControlDescription.h"
#import "OEDeviceDescription.h"

static NSString *const _OEKeyboardPlayerBindingRepresentationsKey = @"keyboardPlayerBindings";
static NSString *const _OEControllerBindingRepresentationsKey = @"controllerBindings";

@interface OEHIDEvent ()
- (OEHIDEvent *)OE_eventWithDeviceHandler:(OEDeviceHandler *)aDeviceHandler;
@end

@interface OESystemBindings ()
{
    NSMutableSet           *_bindingsObservers;

    NSDictionary           *_allKeyBindingsDescriptions;
    NSDictionary           *_systemKeyBindingsDescriptions;
    NSDictionary           *_keyBindingsDescriptions;
    NSArray                *_keyGroupBindingsDescriptions;

    NSMutableDictionary    *_keyboardPlayerBindings;

    // Map devices identifiers to an array of saved bindings
    // Each object in the array represent settings in the order they were added to the app
    // When a new device with the same manufacturer is plugged in, it inherits the settings
    // of the player they were first setup or the first player if the device doesn't exists
    NSMutableDictionary    *_parsedManufacturerBindings;
    NSMutableDictionary    *_unparsedManufactuerBindings;
    NSMutableDictionary    *_deviceHandlersToBindings;

    NSMutableDictionary    *_devicePlayerBindings;

    NSMutableDictionary    *_defaultDeviceBindings;

    OEDevicePlayerBindings *_emptyConfiguration;
}

- (NSString *)OE_descriptionForEvent:(id)anEvent;

- (void)OE_setUpKeyBindingDescriptionsWithSystemController:(OESystemController *)aController;
- (NSDictionary *)OE_stringValuesForBindings:(NSDictionary *)bindingsToConvert possibleKeys:(NSDictionary *)nameToKeyMap;

- (void)OE_registerDefaultControls:(NSDictionary *)defaultControls;

- (OEDevicePlayerBindings *)OE_deviceBindingsForDeviceHandler:(OEDeviceHandler *)aHandler;
- (NSUInteger)OE_addDeviceBindings:(OEDevicePlayerBindings *)bindingsController;
- (void)OE_notifyObserversForAddedDeviceBindings:(OEDevicePlayerBindings *)aHandler;
- (void)OE_notifyObserversForRemovedDeviceBindings:(OEDevicePlayerBindings *)aHandler;

- (void)OE_notifyExistingBindings:(OEPlayerBindings *)bindings toObserver:(id<OESystemBindingsObserver>)observer;
- (void)OE_notifyExistingBindingsInArray:(NSArray *)bindingsArray toObserver:(id<OESystemBindingsObserver>)observer;
- (void)OE_notifyObserversDidSetEvent:(OEHIDEvent *)anEvent forBindingKey:(id)bindingKey playerNumber:(NSUInteger)playerNumber  __attribute__((nonnull));
- (void)OE_notifyObserversDidUnsetEvent:(OEHIDEvent *)anEvent forBindingKey:(id)bindingKey playerNumber:(NSUInteger)playerNumber  __attribute__((nonnull));
- (id)OE_playerBindings:(OEKeyboardPlayerBindings *)sender didSetKeyboardEvent:(OEHIDEvent *)anEvent forKey:(NSString *)keyName  __attribute__((nonnull));
- (id)OE_playerBindings:(OEDevicePlayerBindings *)sender didSetDeviceEvent:(OEHIDEvent *)anEvent forKey:(NSString *)keyName  __attribute__((nonnull));

@end

@implementation OESystemBindings

- (id)OE_initWithBindingsController:(OEBindingsController *)parentController systemController:(OESystemController *)aController dictionaryRepresentation:(NSDictionary *)aDictionary
{
    if(aController == nil) return nil;

    if((self = [super init]))
    {
        _parsedManufacturerBindings = [NSMutableDictionary dictionary];
        _defaultDeviceBindings      = [NSMutableDictionary dictionary];
        _deviceHandlersToBindings   = [NSMutableDictionary dictionary];

        _keyboardPlayerBindings     = [NSMutableDictionary dictionary];
        _devicePlayerBindings       = [NSMutableDictionary dictionary];

        _bindingsObservers          = [NSMutableSet        set];

        _bindingsController         = parentController;
        _systemController           = aController;

        [self OE_setUpKeyBindingDescriptionsWithSystemController:aController];

        if(aDictionary != nil) [self OE_setUpKeyboardBindingsWithRepresentations:aDictionary[_OEKeyboardPlayerBindingRepresentationsKey]];
        else                   [self OE_registerDefaultControls:[_systemController defaultKeyboardControls]];

        _unparsedManufactuerBindings = [aDictionary[_OEControllerBindingRepresentationsKey] mutableCopy];

        _emptyConfiguration = [[OEDevicePlayerBindings alloc] OE_initWithSystemBindings:self playerNumber:0 deviceHandler:nil];
        [_emptyConfiguration OE_setBindingEvents:@{ }];
        [_emptyConfiguration OE_setBindingDescriptions:[self OE_stringValuesForBindings:nil possibleKeys:_allKeyBindingsDescriptions]];
    }

    return self;
}

#pragma mark - System Bindings general use methods

- (NSUInteger)numberOfPlayers
{
    return [_systemController numberOfPlayers];
}

- (void)OE_setUpKeyBindingDescriptionsWithSystemController:(OESystemController *)aController;
{
    // Convert control names into key bindings descriptions
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

    _systemKeyBindingsDescriptions = [systemKeyDescs  copy];
    _keyBindingsDescriptions       = [genericKeyDescs copy];
    _allKeyBindingsDescriptions    = [allKeyDescs     copy];

    // Build key-groups to let keys know about their counter-parts
    NSArray *hatSwitchControls = [aController hatSwitchControls];
    NSArray *axisControls      = [aController axisControls];

    _keyGroupBindingsDescriptions = [[self OE_keyGroupsForControls:hatSwitchControls type:OEKeyGroupTypeHatSwitch availableKeys:_keyBindingsDescriptions] arrayByAddingObjectsFromArray:[self OE_keyGroupsForControls:axisControls type:OEKeyGroupTypeAxis availableKeys:_keyBindingsDescriptions]];

    for(OEKeyBindingGroupDescription *group in _keyGroupBindingsDescriptions)
    {
        NSAssert([group isMemberOfClass:[OEKeyBindingGroupDescription class]], @"SOMETHING'S FISHY");
    }
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

#pragma mark - Parse the receiver's representation dictionaries

- (void)OE_registerDefaultControls:(NSDictionary *)defaultControls
{
    if([defaultControls count] == 0) return;

    // We always assume player 1, if we want to have multiple players, we will have to use an NSArray instead
    OEKeyboardPlayerBindings *bindings = [self keyboardPlayerBindingsForPlayer:1];

    [defaultControls enumerateKeysAndObjectsUsingBlock:
     ^(NSString *key, NSNumber *obj, BOOL *stop)
     {
         OEHIDEvent *theEvent = [OEHIDEvent keyEventWithTimestamp:0
                                                          keyCode:[obj unsignedIntValue]
                                                            state:NSOnState
                                                           cookie:OEUndefinedCookie];

         [bindings assignEvent:theEvent toKeyWithName:key];
     }];
}

- (void)OE_setUpKeyboardBindingsWithRepresentations:(NSArray *)representations;
{
    if(representations == nil) return;

    // Convert keyboard bindings
    _keyboardPlayerBindings = [[NSMutableDictionary alloc] initWithCapacity:[representations count]];

    [representations enumerateObjectsUsingBlock:
     ^(NSDictionary *encoded, NSUInteger idx, BOOL *stop)
     {
         NSMutableDictionary *decodedBindings = [NSMutableDictionary dictionaryWithCapacity:[encoded count]];

         [encoded enumerateKeysAndObjectsUsingBlock:
          ^(NSString *keyName, id value, BOOL *stop)
          {
              OEKeyBindingDescription *desc = _allKeyBindingsDescriptions[keyName];
              if(desc == nil) return;
              decodedBindings[desc] = value;
          }];

         OEKeyboardPlayerBindings *controller = [[OEKeyboardPlayerBindings alloc] OE_initWithSystemBindings:self playerNumber:idx + 1];

         [controller OE_setBindingEvents:decodedBindings];
         [controller OE_setBindingDescriptions:[self OE_stringValuesForBindings:decodedBindings possibleKeys:_keyBindingsDescriptions]];

         _keyboardPlayerBindings[@([controller playerNumber])] = controller;
     }];
}

- (void)OE_parseDefaultControlValuesForControllerDescription:(OEControllerDescription *)controllerDescription
{
    if(_defaultDeviceBindings[controllerDescription] != nil) return;

    NSDictionary *representation = [_systemController defaultDeviceControls][[controllerDescription identifier]];
    if(representation == nil) return;

    _defaultDeviceBindings[controllerDescription] = [self OE_parsedDevicePlayerBindingsForRepresentation:representation withControllerDescription:controllerDescription useValueIdentifier:NO];
}

- (void)OE_parseManufacturerControlValuesForDeviceDescription:(OEDeviceDescription *)deviceDescription
{
    if(_parsedManufacturerBindings[deviceDescription] != nil) return;

    OEControllerDescription *controllerDescription = [deviceDescription controllerDescription];
    NSString *genericDeviceIdentifier = [deviceDescription genericDeviceIdentifier];
    NSArray *genericDeviceBindingsToParse = _unparsedManufactuerBindings[genericDeviceIdentifier];
    [_unparsedManufactuerBindings removeObjectForKey:genericDeviceIdentifier];

    if(genericDeviceBindingsToParse == nil && _parsedManufacturerBindings[controllerDescription] != nil) return;

    NSMutableArray *parsedBindings = _parsedManufacturerBindings[controllerDescription] ? : [NSMutableArray array];

    for(NSDictionary *representation in genericDeviceBindingsToParse)
        [parsedBindings addObject:[self OE_parsedDevicePlayerBindingsForRepresentation:representation withControllerDescription:controllerDescription useValueIdentifier:YES]];

    if(![controllerDescription isGeneric])
    {
        [self OE_parseDefaultControlValuesForControllerDescription:controllerDescription];

        for(NSDictionary *representation in _unparsedManufactuerBindings[[controllerDescription identifier]])
            [parsedBindings addObject:[self OE_parsedDevicePlayerBindingsForRepresentation:representation withControllerDescription:controllerDescription useValueIdentifier:NO]];
        [_unparsedManufactuerBindings removeObjectForKey:[controllerDescription identifier]];
        _parsedManufacturerBindings[controllerDescription] = parsedBindings;
    }

    _parsedManufacturerBindings[deviceDescription] = parsedBindings;
}

- (OEDevicePlayerBindings *)OE_parsedDevicePlayerBindingsForRepresentation:(NSDictionary *)representation withControllerDescription:(OEControllerDescription *)controllerDescription useValueIdentifier:(BOOL)useValueIdentifier
{
    NSMutableDictionary *rawBindings = [NSMutableDictionary dictionaryWithCapacity:[_allKeyBindingsDescriptions count]];
    [_allKeyBindingsDescriptions enumerateKeysAndObjectsUsingBlock:
     ^(NSString *keyName, OEKeyBindingDescription *keyDesc, BOOL *stop)
     {
         id controlIdentifier = representation[keyName];
         if(controlIdentifier == nil)
             return;

         NSAssert(![controlIdentifier isKindOfClass:[NSDictionary class]], @"Default for key %@ in System %@ was not converted to the new system.", keyName, [_systemController systemName]);
         OEControlValueDescription *controlValue = (useValueIdentifier
                                                    ? [controllerDescription controlValueDescriptionForValueIdentifier:controlIdentifier]
                                                    : [controllerDescription controlValueDescriptionForIdentifier:controlIdentifier]);
         OEHIDEvent *event = [controlValue event];

         NSAssert(controlValue != nil, @"Unknown control value for identifier: '%@' associated with key name: '%@'", controlIdentifier, keyName);

         rawBindings[[self OE_keyIdentifierForKeyDescription:keyDesc event:event]] = controlValue;
     }];

    OEDevicePlayerBindings *controller = [[OEDevicePlayerBindings alloc] OE_initWithSystemBindings:self playerNumber:0 deviceHandler:nil];
    [controller OE_setBindingEvents:rawBindings];
    [controller OE_setBindingDescriptions:[self OE_stringValuesForBindings:rawBindings possibleKeys:_allKeyBindingsDescriptions]];

    return controller;
}

- (id)OE_keyIdentifierForKeyDescription:(OEKeyBindingDescription *)keyDescription event:(OEHIDEvent *)event;
{
    id insertedKey = keyDescription;
    OEHIDEventType eventType = [event type];
    if(eventType == OEHIDEventTypeAxis || eventType == OEHIDEventTypeHatSwitch)
    {
        OEKeyBindingGroupDescription *keyGroup = (eventType == OEHIDEventTypeAxis ? [keyDescription OE_axisGroup] : [keyDescription OE_hatSwitchGroup]);
        if(keyGroup != nil) insertedKey = [keyGroup orientedKeyGroupWithBaseKey:keyDescription];
    }

    return insertedKey;
}
#pragma mark - Construct the receiver's representation dictionaries

- (NSDictionary *)OE_dictionaryRepresentation;
{
    NSMutableDictionary *dictionary = [NSMutableDictionary dictionaryWithCapacity:2];

    void (^addToDictionary)(NSString *key, id value) =
    ^(NSString *key, id value)
    {
        if(value == nil) return;

        [dictionary setObject:value forKey:key];
    };

    addToDictionary(_OEControllerBindingRepresentationsKey    , [self OE_dictionaryRepresentationForControllerBindings]);
    addToDictionary(_OEKeyboardPlayerBindingRepresentationsKey, [self OE_arrayRepresentationForKeyboardBindings]);

    return [dictionary copy];
}

- (NSDictionary *)OE_dictionaryRepresentationForControllerBindings;
{
    NSMutableDictionary *ret = [_unparsedManufactuerBindings mutableCopy] ? : [NSMutableDictionary dictionaryWithCapacity:[_parsedManufacturerBindings count]];

    [_parsedManufacturerBindings enumerateKeysAndObjectsUsingBlock:
     ^(id description, NSArray *controllers, BOOL *stop)
     {
         BOOL isDeviceDescription = [description isKindOfClass:[OEDeviceDescription class]];
         if(   ( isDeviceDescription && ![[description controllerDescription] isGeneric])
            || (!isDeviceDescription &&   [description isGeneric]))
             return;

         NSMutableArray *controllerRepresentations = [NSMutableArray arrayWithCapacity:[controllers count]];
         for(OEDevicePlayerBindings *controller in controllers)
         {
             NSDictionary *rawBindings = [controller bindingEvents];
             NSMutableDictionary *bindingRepresentations = [NSMutableDictionary dictionaryWithCapacity:[rawBindings count]];
             [rawBindings enumerateKeysAndObjectsUsingBlock:
              ^(id key, OEControlValueDescription *obj, BOOL *stop)
              {
                  NSString *saveKey = nil;
                  if([key isKindOfClass:[OEKeyBindingDescription class]])
                      saveKey = [key name];
                  else if([key isKindOfClass:[OEOrientedKeyGroupBindingDescription class]])
                      saveKey = [[key baseKey] name];

                  bindingRepresentations[saveKey] = isDeviceDescription ? [obj valueIdentifier] : [obj identifier];
              }];

             [controllerRepresentations addObject:bindingRepresentations];
         }

         ret[[description identifier]] = controllerRepresentations;
     }];

    return ret;
}

- (NSMutableArray *)OE_arrayRepresentationForKeyboardBindings
{
    NSMutableArray *ret = [NSMutableArray arrayWithCapacity:[_keyboardPlayerBindings count]];
    NSUInteger numberOfPlayers = [self numberOfPlayers];
    NSUInteger lastValidPlayerNumber = 1;

    for(NSUInteger i = 1; i <= numberOfPlayers; i++)
    {
        NSDictionary *rawBindings = [_keyboardPlayerBindings[@(i)] bindingEvents];
        NSMutableDictionary *bindingRepresentations = [NSMutableDictionary dictionaryWithCapacity:[rawBindings count]];
        [rawBindings enumerateKeysAndObjectsUsingBlock:
         ^(OEKeyBindingDescription *key, OEHIDEvent *event, BOOL *stop)
         {
             bindingRepresentations[[key name]] = event;
         }];

        ret[[ret count]] = bindingRepresentations;

        if(rawBindings != nil) lastValidPlayerNumber = i;
    }

    if(lastValidPlayerNumber < numberOfPlayers)
        [ret removeObjectsAtIndexes:[NSIndexSet indexSetWithIndexesInRange:NSMakeRange(lastValidPlayerNumber, numberOfPlayers - lastValidPlayerNumber)]];

    return ret;
}

#pragma mark -
#pragma mark Player Bindings Controller Representation

- (NSUInteger)playerNumberForEvent:(OEHIDEvent *)anEvent;
{
    if([anEvent type] == OEHIDEventTypeKeyboard || [anEvent deviceHandler] == nil) return 0;

    OEDeviceHandler *handler = [anEvent deviceHandler];

    return [[[_devicePlayerBindings keysOfEntriesPassingTest:
              ^ BOOL (id key, OEDevicePlayerBindings *obj, BOOL *stop)
              {
                  if([obj deviceHandler] == handler)
                  {
                      *stop = YES;
                      return YES;
                  }

                  return NO;
              }] anyObject] integerValue];
}

- (OEDevicePlayerBindings *)devicePlayerBindingsForPlayer:(NSUInteger)playerNumber;
{
    if(playerNumber == 0 || playerNumber > [self numberOfPlayers]) return nil;

    return _devicePlayerBindings[@(playerNumber)];
}

- (OEKeyboardPlayerBindings *)keyboardPlayerBindingsForPlayer:(NSUInteger)playerNumber;
{
    if(playerNumber == 0 || playerNumber > [self numberOfPlayers]) return nil;

    OEKeyboardPlayerBindings *ret = _keyboardPlayerBindings[@(playerNumber)];

    if(ret == nil)
    {
        [self willChangeValueForKey:@"keyboardPlayerBindings"];
        ret = [[OEKeyboardPlayerBindings alloc] OE_initWithSystemBindings:self playerNumber:playerNumber];

        // At this point, if a keyboard bindings doesn't exist, it means none were saved for this player
        [ret OE_setBindingDescriptions:[self OE_stringValuesForBindings:nil possibleKeys:_keyBindingsDescriptions]];
        [ret OE_setBindingEvents:@{ }];

        _keyboardPlayerBindings[@(playerNumber)] = ret;

        [self didChangeValueForKey:@"keyboardPlayerBindings"];
    }

    return ret;
}

- (NSUInteger)playerForDeviceHandler:(OEDeviceHandler *)deviceHandler;
{
    return [[self devicePlayerBindingsForDeviceHandler:deviceHandler] playerNumber];
}

- (OEDeviceHandler *)deviceHandlerForPlayer:(NSUInteger)playerNumber;
{
    if(playerNumber == 0 || playerNumber > [self numberOfPlayers]) return nil;

    OEDevicePlayerBindings *bindings = _devicePlayerBindings[@(playerNumber)];

    NSAssert([bindings playerNumber] == playerNumber, @"Expected player number for bindings: %ld, got: %ld.", playerNumber, [bindings playerNumber]);

    return [bindings deviceHandler];
}

- (OEDevicePlayerBindings *)devicePlayerBindingsForDeviceHandler:(OEDeviceHandler *)deviceHandler;
{
    return _deviceHandlersToBindings[deviceHandler];
}

- (void)setDeviceHandler:(OEDeviceHandler *)deviceHandler forPlayer:(NSUInteger)playerNumber;
{
    // Find the two bindings to switch.
    OEDevicePlayerBindings *newBindings = [self devicePlayerBindingsForDeviceHandler:deviceHandler];
    NSAssert(newBindings != nil, @"A device handler without device player bindings?!");
    if([newBindings playerNumber] == playerNumber) return;

    OEDevicePlayerBindings *oldBindings = [self devicePlayerBindingsForPlayer:playerNumber];

    // Notify observers to remove all bindings to the old player number of each devices.
    if(oldBindings != nil) [self OE_notifyObserversForRemovedDeviceBindings:oldBindings];
    [self OE_notifyObserversForRemovedDeviceBindings:newBindings];

    // Clean up the keys for each player keys so there's no confusion.
    if(oldBindings != nil) [_devicePlayerBindings removeObjectForKey:@([oldBindings playerNumber])];
    [_devicePlayerBindings removeObjectForKey:@([newBindings playerNumber])];

    // Change the player numbers.
    if(oldBindings != nil) [oldBindings OE_setPlayerNumber:[newBindings playerNumber]];
    [newBindings OE_setPlayerNumber:playerNumber];

    // Move the bindings in the array.
    if(oldBindings != nil) _devicePlayerBindings[@([oldBindings playerNumber])] = oldBindings;
    _devicePlayerBindings[@(playerNumber)] = newBindings;

    // Notify observers to add all bindings for the new player layout.
    if(oldBindings != nil) [self OE_notifyObserversForAddedDeviceBindings:oldBindings];
    [self OE_notifyObserversForAddedDeviceBindings:newBindings];
}

#pragma mark -
#pragma mark Preference Panel Representation Helper Methods

- (NSString *)OE_descriptionForEvent:(id)anEvent;
{
    if(anEvent == nil) return nil;

    // Handle device events.
    if([anEvent isKindOfClass:[OEControlValueDescription class]])
        return [anEvent name];

    // Handle keyboard events.
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
         NSAssert(![obj isKindOfClass:[OEControlDescription class]], @"A OEControlDescription object was wrongfully associated to the key '%@'.", key);

         if([key isKindOfClass:[OEKeyBindingDescription class]])
             // General case - the event value is attached to the key-name for bindings
             [ret setObject:[self OE_descriptionForEvent:obj] forKey:[key name]];
         else if([key isKindOfClass:[OEOrientedKeyGroupBindingDescription class]])
         {
             OEOrientedKeyGroupBindingDescription *group = key;
             OEControlValueDescription *controlValue = obj;
             OEHIDEvent *event = [controlValue event];

             // In case of key-group, we need to create multiple key-strings for each cases of the key, usually 2 for axis type and 4 for hat switch type
             NSAssert([controlValue isKindOfClass:[OEControlValueDescription class]], @"Only OEControlValueDescription can be associated with binding groups, got: %@ %@", [event class], event);

             switch([group type])
             {
                 case OEKeyGroupTypeAxis :
                 {
                     NSAssert([event type] == OEHIDEventTypeAxis, @"Excepting OEHIDEventTypeAxis event type for Axis key group, got: %@", NSStringFromOEHIDEventType([event type]));

                     ret[[[group baseKey] name]] = [controlValue name];
                     ret[[[group oppositeKey] name]] = [[controlValue associatedControlValueDescriptionForEvent:[event axisEventWithOppositeDirection]] name];
                 }
                     break;
                 case OEKeyGroupTypeHatSwitch :
                 {
                     NSAssert([event type] == OEHIDEventTypeHatSwitch, @"Excepting OEHIDEventTypeHatSwitch event type for Axis key group, got: %@", NSStringFromOEHIDEventType([event type]));

                     enum { NORTH, EAST, SOUTH, WEST, HAT_COUNT };

                     OEHIDEventHatDirection direction  = [event hatDirection];
                     __block NSUInteger     currentDir = NORTH;

                     if(direction & OEHIDEventHatDirectionNorth) currentDir = NORTH;
                     if(direction & OEHIDEventHatDirectionEast)  currentDir = EAST;
                     if(direction & OEHIDEventHatDirectionSouth) currentDir = SOUTH;
                     if(direction & OEHIDEventHatDirectionWest)  currentDir = WEST;

                     static OEHIDEventHatDirection dirs[HAT_COUNT] = { OEHIDEventHatDirectionNorth, OEHIDEventHatDirectionEast, OEHIDEventHatDirectionSouth, OEHIDEventHatDirectionWest };

                     [group enumerateKeysFromBaseKeyUsingBlock:
                      ^(OEKeyBindingDescription *key, BOOL *stop)
                      {
                          ret[[key name]] = [[controlValue associatedControlValueDescriptionForEvent:[event hatSwitchEventWithDirection:dirs[currentDir % HAT_COUNT]]] name];

                          currentDir++;
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

    OEKeyBindingDescription *keyDesc = [_allKeyBindingsDescriptions objectForKey:keyName];
    NSAssert(keyDesc != nil, @"Could not find Key Binding Description for key with name \"%@\" in system \"%@\"", keyName, [[self systemController] systemIdentifier]);

    // Trying to set the same event to the same key, ignore it
    if([[[sender bindingEvents] objectForKey:keyDesc] isEqual:anEvent]) return keyDesc;

    [_keyboardPlayerBindings enumerateKeysAndObjectsUsingBlock:
     ^(NSNumber *key, OEKeyboardPlayerBindings *playerBindings, BOOL *stop)
     {
         NSArray *keys = [[playerBindings bindingEvents] allKeysForObject:anEvent];
         NSAssert([keys count] <= 1, @"More than one key is attached to the same event: %@ -> %@", anEvent, keys);

         OEKeyBindingDescription *desc = [keys lastObject];
         if(desc != nil)
         {
             [playerBindings OE_setBindingDescription:nil forKey:[desc name]];
             [playerBindings OE_setBindingEvent:nil forKey:desc];

             [self OE_notifyObserversDidUnsetEvent:anEvent forBindingKey:desc playerNumber:[desc isSystemWide] ? 0 : [playerBindings playerNumber]];
         }
     }];

    NSString *eventString = [self OE_descriptionForEvent:anEvent];

    [sender OE_setBindingDescription:eventString forKey:keyName];
    [sender OE_setBindingEvent:anEvent forKey:keyDesc];

    [self OE_notifyObserversDidSetEvent:anEvent forBindingKey:keyDesc playerNumber:[sender playerNumber]];
    [[self bindingsController] OE_setRequiresSynchronization];

    return keyDesc;
}

- (id)OE_playerBindings:(OEDevicePlayerBindings *)sender didSetDeviceEvent:(OEHIDEvent *)anEvent forKey:(NSString *)keyName;
{
    NSAssert([sender isKindOfClass:[OEDevicePlayerBindings class]], @"Invalid sender: OEKeyboardPlayerBindings expected, got: %@ %@", [sender class], sender);

    NSAssert([_allKeyBindingsDescriptions objectForKey:keyName] != nil, @"Could not find Key Binding Description for key with name \"%@\" in system \"%@\"", keyName, [[self systemController] systemIdentifier]);

    OEControlValueDescription *valueDesc = [[[sender deviceHandler] controllerDescription] controlValueDescriptionForEvent:anEvent];
    NSAssert(valueDesc != nil, @"Controller type '%@' does not recognize the event '%@', when attempting to set the key with name: '%@'.", [[[sender deviceHandler] controllerDescription] identifier], anEvent, keyName);

    // sender is based on another device player bindings
    // it needs to be made independent and added to the manufacturer list
    if([sender OE_isDependent])
    {
        [sender OE_makeIndependent];

        [_parsedManufacturerBindings[[[sender deviceHandler] deviceDescription]] addObject:sender];
    }

    // Search for keys bound to the same event
    NSArray *keys = [[sender bindingEvents] allKeysForObject:valueDesc];
    if([keys count] == 0) keys = [[[sender bindingEvents] keysOfEntriesPassingTest:
                                   ^ BOOL (OEOrientedKeyGroupBindingDescription *key, OEControlValueDescription *obj, BOOL *stop)
                                   {
                                       if(![key isKindOfClass:[OEOrientedKeyGroupBindingDescription class]]) return NO;

                                       return [obj controlDescription] == [valueDesc controlDescription];
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

        for(NSString *key in keys) [sender OE_setBindingDescription:nil forKey:key];

        [sender OE_setBindingEvent:nil forKey:keyDesc];
        [self OE_notifyObserversDidUnsetEvent:anEvent forBindingKey:keyDesc playerNumber:[sender playerNumber]];
    }

    // Find the appropriate key for the event
    keyDesc = [_allKeyBindingsDescriptions objectForKey:keyName];
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
    NSDictionary *eventStrings = [self OE_stringValuesForBindings:@{ keyDesc : valueDesc } possibleKeys:_allKeyBindingsDescriptions];

    [eventStrings enumerateKeysAndObjectsUsingBlock:
     ^(NSString *key, NSString *obj, BOOL *stop)
     {
         [sender OE_setBindingDescription:obj forKey:key];
     }];

    [sender OE_setBindingEvent:valueDesc forKey:keyDesc];
    [self OE_notifyObserversDidSetEvent:anEvent forBindingKey:keyDesc playerNumber:[sender playerNumber]];
    [[self bindingsController] OE_setRequiresSynchronization];

    return keyDesc;
}

- (void)OE_removeConcurrentBindings:(OEDevicePlayerBindings *)sender ofKey:(OEKeyBindingDescription *)keyDesc withEvent:(OEHIDEvent *)anEvent;
{
    NSDictionary                 *rawBindings = [[sender bindingEvents] copy];
    OEKeyBindingGroupDescription *axisGroup   = [keyDesc OE_axisGroup];
    OEKeyBindingGroupDescription *hatGroup    = [keyDesc OE_hatSwitchGroup];

    if(axisGroup == nil && hatGroup == nil) return;

    OEDeviceHandler *handler = [sender deviceHandler];

    switch([anEvent type])
    {
        case OEHIDEventTypeButton :
        case OEHIDEventTypeTrigger :
        {
            [rawBindings enumerateKeysAndObjectsUsingBlock:
             ^ void (OEOrientedKeyGroupBindingDescription *keyDesc, OEControlValueDescription *valueDesc, BOOL *stop)
             {
                 if(![keyDesc isKindOfClass:[OEOrientedKeyGroupBindingDescription class]]) return;

                 OEKeyBindingGroupDescription *keyGroup = [keyDesc parentKeyGroup];
                 if(keyGroup != axisGroup && keyGroup != hatGroup) return;

                 [[keyGroup keyNames] enumerateObjectsUsingBlock:
                  ^(NSString *key, NSUInteger idx, BOOL *stop)
                  {
                      [sender OE_setBindingDescription:nil forKey:key];
                  }];

                 [sender OE_setBindingEvent:nil forKey:keyDesc];
                 [self OE_notifyObserversDidUnsetEvent:[[valueDesc event] OE_eventWithDeviceHandler:handler] forBindingKey:keyDesc playerNumber:[sender playerNumber]];
             }];
        }
            break;
        case OEHIDEventTypeAxis :
        {
            for(OEKeyBindingDescription *keyDesc in [axisGroup keys])
            {
                OEControlValueDescription *valueDesc = [rawBindings objectForKey:keyDesc];
                if(valueDesc != nil)
                {
                    [sender OE_setBindingDescription:nil forKey:[keyDesc name]];
                    [sender OE_setBindingEvent:nil forKey:keyDesc];
                    [self OE_notifyObserversDidUnsetEvent:[[valueDesc event] OE_eventWithDeviceHandler:handler] forBindingKey:keyDesc playerNumber:[sender playerNumber]];
                }
            }

            [rawBindings enumerateKeysAndObjectsUsingBlock:
             ^ void (OEOrientedKeyGroupBindingDescription *keyDesc, OEControlValueDescription *valueDesc, BOOL *stop)
             {
                 if(![keyDesc isKindOfClass:[OEOrientedKeyGroupBindingDescription class]]) return;

                 OEKeyBindingGroupDescription *keyGroup = [keyDesc parentKeyGroup];
                 if(keyGroup != axisGroup && keyGroup != hatGroup) return;

                 [[keyDesc keyNames] enumerateObjectsUsingBlock:
                  ^(NSString *key, NSUInteger idx, BOOL *stop)
                  {
                      [sender OE_setBindingDescription:nil forKey:key];
                  }];

                 [sender OE_setBindingEvent:nil forKey:keyDesc];
                 [self OE_notifyObserversDidUnsetEvent:[[valueDesc event] OE_eventWithDeviceHandler:handler] forBindingKey:keyDesc playerNumber:[sender playerNumber]];
             }];
        }
            break;
        case OEHIDEventTypeHatSwitch :
        {
            NSMutableSet *visitedAxisGroups = [NSMutableSet setWithObjects:axisGroup, hatGroup, nil];
            for(OEKeyBindingDescription *keyDesc in [hatGroup keys])
            {
                OEControlValueDescription *valueDesc = [rawBindings objectForKey:keyDesc];
                if(valueDesc != nil)
                {
                    [sender OE_setBindingDescription:nil forKey:[keyDesc name]];
                    [sender OE_setBindingEvent:nil forKey:keyDesc];
                    [self OE_notifyObserversDidUnsetEvent:[[valueDesc event] OE_eventWithDeviceHandler:handler] forBindingKey:keyDesc playerNumber:[sender playerNumber]];
                }

                OEKeyBindingGroupDescription *temp = [keyDesc OE_axisGroup];
                if(temp != nil) [visitedAxisGroups addObject:temp];
            }

            [rawBindings enumerateKeysAndObjectsUsingBlock:
             ^(OEOrientedKeyGroupBindingDescription *keyDesc, OEControlValueDescription *valueDesc, BOOL *stop)
             {
                 if(![visitedAxisGroups containsObject:keyDesc]) return;

                 [[keyDesc keyNames] enumerateObjectsUsingBlock:
                  ^(NSString *key, NSUInteger idx, BOOL *stop)
                  {
                      [sender OE_setBindingDescription:nil forKey:key];
                  }];

                 [sender OE_setBindingEvent:nil forKey:keyDesc];
                 [self OE_notifyObserversDidUnsetEvent:[[valueDesc event] OE_eventWithDeviceHandler:handler] forBindingKey:keyDesc playerNumber:[sender playerNumber]];
             }];
        }
            break;
        default :
            break;
    }
}

#pragma mark -
#pragma mark Device Handlers Management

- (void)OE_didAddDeviceHandler:(OEDeviceHandler *)aHandler;
{
    // Ignore extra keyboards for now
    if([aHandler isKeyboardDevice]) return;

    [self OE_notifyObserversForAddedDeviceBindings:[self OE_deviceBindingsForDeviceHandler:aHandler]];
}

- (void)OE_didRemoveDeviceHandler:(OEDeviceHandler *)aHandler;
{
    // Ignore extra keyboards for now
    if([aHandler isKeyboardDevice]) return;

    OEDevicePlayerBindings *controller = [_deviceHandlersToBindings objectForKey:aHandler];

    if(controller == nil)
    {
        NSLog(@"WARNING: Trying to remove a device that was not registered with %@", self);
        return;
    }

    NSUInteger playerNumber = [controller playerNumber];

    [self willChangeValueForKey:@"devicePlayerBindings"];
    [_devicePlayerBindings removeObjectForKey:@(playerNumber)];
    [self didChangeValueForKey:@"devicePlayerBindings"];

    [self OE_notifyObserversForRemovedDeviceBindings:controller];

    [controller OE_makeIndependent];

    [controller OE_setDeviceHandler:nil];
    [controller OE_setPlayerNumber:0];
}

- (void)OE_notifyObserversForAddedDeviceBindings:(OEDevicePlayerBindings *)aHandler;
{
    NSUInteger playerNumber = [aHandler playerNumber];
    OEDeviceHandler *deviceHandler = [aHandler deviceHandler];

    [[aHandler bindingEvents] enumerateKeysAndObjectsUsingBlock:
     ^(id key, OEControlValueDescription *obj, BOOL *stop)
     {
         for(id<OESystemBindingsObserver> observer in _bindingsObservers)
             [observer systemBindings:self didSetEvent:[[obj event] OE_eventWithDeviceHandler:deviceHandler] forBinding:key playerNumber:playerNumber];
     }];
}

- (void)OE_notifyObserversForRemovedDeviceBindings:(OEDevicePlayerBindings *)aHandler;
{
    NSUInteger playerNumber = [aHandler playerNumber];
    OEDeviceHandler *deviceHandler = [aHandler deviceHandler];

    // Tell the controllers that the bindings are not used anymore
    [[aHandler bindingEvents] enumerateKeysAndObjectsUsingBlock:
     ^(id key, OEControlValueDescription *obj, BOOL *stop)
     {
         for(id<OESystemBindingsObserver> observer in _bindingsObservers)
             [observer systemBindings:self didUnsetEvent:[[obj event] OE_eventWithDeviceHandler:deviceHandler] forBinding:key playerNumber:playerNumber];
     }];
}

- (OEDevicePlayerBindings *)OE_deviceBindingsForDeviceHandler:(OEDeviceHandler *)aHandler;
{
    OEDevicePlayerBindings *controller = [_deviceHandlersToBindings objectForKey:aHandler];

    // The device was already registered with the system controller
    if(controller != nil) return controller;

    OEDeviceDescription *deviceDescription         = [aHandler deviceDescription];
    OEControllerDescription *controllerDescription = [deviceDescription controllerDescription];
    NSMutableArray *manuBindings                   = _parsedManufacturerBindings[deviceDescription];

    // Allocate a new array to countain OEDevicePlayerBindings objects for the given device type
    if(manuBindings == nil)
    {
        [self OE_parseManufacturerControlValuesForDeviceDescription:deviceDescription];
        manuBindings = _parsedManufacturerBindings[deviceDescription];
    }

    for(OEDevicePlayerBindings *ctrl in manuBindings)
        if([ctrl deviceHandler] == nil)
        {
            controller = ctrl;
            [controller OE_setDeviceHandler:aHandler];
            break;
        }

    // No available slots in the known configurations, look for defaults
    if(controller == nil)
    {
        OEDevicePlayerBindings *ctrl = _defaultDeviceBindings[controllerDescription];
        controller = [ctrl OE_playerBindingsWithDeviceHandler:aHandler playerNumber:0];
    }

    // No defaults, duplicate the first manufacturer  device
    if(controller == nil && [manuBindings count] > 0)
        controller = [manuBindings[0] OE_playerBindingsWithDeviceHandler:aHandler playerNumber:0];

    // Still nothing, create a completely empty controller
    if(controller == nil)
    {
        // This handler is the first of its kind for the application
        controller = [_emptyConfiguration OE_playerBindingsWithDeviceHandler:aHandler playerNumber:0];
    }

    // Keep track of device handlers
    _deviceHandlersToBindings[aHandler] = controller;

    // Add it to the player list
    [self OE_addDeviceBindings:controller];

    return controller;
}

- (NSUInteger)OE_addDeviceBindings:(OEDevicePlayerBindings *)controller;
{
    // Find the first free slot.
    NSUInteger playerNumber = 1;
    while(_devicePlayerBindings[@(playerNumber)] != nil)
        playerNumber++;

    [self willChangeValueForKey:@"devicePlayerBindings"];
    _devicePlayerBindings[@(playerNumber)] = controller;
    [self didChangeValueForKey:@"devicePlayerBindings"];

    [controller OE_setPlayerNumber:playerNumber];

    return playerNumber;
}

#pragma mark -
#pragma mark Bindings Observers

- (void)OE_notifyObserversDidSetEvent:(OEHIDEvent *)anEvent forBindingKey:(id)bindingKey playerNumber:(NSUInteger)playerNumber;
{
    for(id<OESystemBindingsObserver> observer in _bindingsObservers)
        [observer systemBindings:self didSetEvent:anEvent forBinding:bindingKey playerNumber:playerNumber];
}

- (void)OE_notifyObserversDidUnsetEvent:(OEHIDEvent *)anEvent forBindingKey:(id)bindingKey playerNumber:(NSUInteger)playerNumber;
{
    for(id<OESystemBindingsObserver> observer in _bindingsObservers)
        [observer systemBindings:self didUnsetEvent:anEvent forBinding:bindingKey playerNumber:playerNumber];
}

- (void)OE_notifyExistingBindings:(OEPlayerBindings *)bindings toObserver:(id<OESystemBindingsObserver>)observer;
{
    if(bindings == nil) return;

    OEDevicePlayerBindings *deviceBindings = nil;
    if([bindings isKindOfClass:[OEDevicePlayerBindings class]])
        deviceBindings = (OEDevicePlayerBindings *)bindings;

    NSUInteger playerNumber = [bindings playerNumber];

    [[bindings bindingEvents] enumerateKeysAndObjectsUsingBlock:
     ^(id key, id event, BOOL *stop)
     {
         NSUInteger player = playerNumber;
         // playerNumber for system-wide keys should always be 0
         if(_systemKeyBindingsDescriptions != nil               &&
            [key isKindOfClass:[OEKeyBindingDescription class]] &&
            [key isSystemWide])
             player = 0;

         if(deviceBindings != nil) event = [[(OEControlValueDescription *)event event] OE_eventWithDeviceHandler:[deviceBindings deviceHandler]];
         [observer systemBindings:self didSetEvent:event forBinding:key playerNumber:player];
     }];
}

- (void)OE_notifyExistingBindingsInArray:(NSArray *)bindingsArray toObserver:(id<OESystemBindingsObserver>)observer
{
    for(OEPlayerBindings *ctrl in bindingsArray)
        [self OE_notifyExistingBindings:ctrl toObserver:observer];
}

- (void)addBindingsObserver:(id<OESystemBindingsObserver>)observer
{
    if([_bindingsObservers containsObject:observer]) return;

    [self OE_notifyExistingBindingsInArray:[_keyboardPlayerBindings allValues] toObserver:observer];
    [self OE_notifyExistingBindingsInArray:[_devicePlayerBindings allValues]   toObserver:observer];

    [_bindingsObservers addObject:observer];
}

- (void)removeBindingsObserver:(id<OESystemBindingsObserver>)observer
{
    // No need to tell it to unset everything
    [_bindingsObservers removeObject:observer];
}

@end
