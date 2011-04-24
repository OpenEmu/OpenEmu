//
//  OEBasicSystemResponder.m
//  OpenEmu
//
//  Created by Remy Demarest on 23/04/2011.
//  Copyright 2011 NuLayer Inc. All rights reserved.
//

#import "OEBasicSystemResponder.h"
#import "OEAbstractAdditions.h"
#import "OEHIDEvent.h"
#import "OESystemController.h"

@implementation OEBasicSystemResponder

- (id)initWithController:(OESystemController *)controller;
{
    if((self = [super initWithController:controller]))
    {
        keyMap = OEMapCreate([[controller genericControlNames] count]);
    }
    
    return self;
}

- (void)dealloc
{
    OEMapRelease(keyMap);
    [super dealloc];
}

- (OEEmulatorKey)emulatorKeyForKey:(NSString *)aKey index:(NSUInteger)index player:(NSUInteger)thePlayer
{
    return [self emulatorKeyForKeyIndex:index player:thePlayer];
}

- (OEEmulatorKey)emulatorKeyForKeyIndex:(NSUInteger)index player:(NSUInteger)thePlayer
{
    [self doesNotImplementSelector:_cmd];
    return (OEEmulatorKey){0, 0};
}

- (void)pressEmulatorKey:(OEEmulatorKey)aKey
{
    [self doesNotImplementSelector:_cmd];
}

- (void)releaseEmulatorKey:(OEEmulatorKey)aKey
{
    [self doesNotImplementSelector:_cmd];
}

#define OEHatSwitchMask     (0x39 << 16)
#define PAD_NUMBER  ([anEvent padNumber] << 24)
#define KEYBOARD_MASK 0x40000000u
#define HID_MASK      0x20000000u
#define DIRECTION_MASK(dir) (1 << ((dir) > OEHIDDirectionNull))

#define GET_EMUL_KEY(keyName) ({                                               \
    NSUInteger index, player;                                                  \
    player = [[self controller] playerNumberInKey:keyName getKeyIndex:&index]; \
    if(player == NSNotFound) return;                                           \
    [self emulatorKeyForKey:keyName index:index player:player];                \
})

- (void)setEventValue:(NSInteger)appKey forEmulatorKey:(OEEmulatorKey)emulKey
{
    OEMapSetValue(keyMap, appKey, emulKey);
}

- (void)unsetEventForKey:(NSString *)keyName withValueMask:(NSUInteger)keyMask
{
    OEEmulatorKey emulKey = GET_EMUL_KEY(keyName);
    OEMapRemoveMaskedKeysForValue(keyMap, keyMask, emulKey);
}

- (void)keyboardEventWasSet:(id)theEvent forKey:(NSString *)keyName
{
    OEEmulatorKey emulKey = GET_EMUL_KEY(keyName);
    NSInteger appKey = KEYBOARD_MASK | [theEvent keycode];
    
    [self setEventValue:appKey forEmulatorKey:emulKey];
}

- (void)keyboardEventWasRemovedForKey:(NSString *)keyName
{
    [self unsetEventForKey:keyName withValueMask:KEYBOARD_MASK];
}

- (void)HIDEventWasSet:(id)theEvent forKey:(NSString *)keyName
{
    OEEmulatorKey emulKey = GET_EMUL_KEY(keyName);
    
    NSInteger   appKey  = 0;
    OEHIDEvent *anEvent = theEvent;
    appKey = HID_MASK | [anEvent padNumber] << 24;
    
    switch ([anEvent type])
    {
        case OEHIDAxis :
        {
            OEHIDDirection dir = [anEvent direction];
            if(dir == OEHIDDirectionNull) return;
            appKey |= ([anEvent axis] << 16);
            appKey |= 1 << (dir > OEHIDDirectionNull);
        }
            break;
        case OEHIDButton :
            if([anEvent state]    == NSOffState) return;
            appKey |= [anEvent cookie];
            break;
        case OEHIDHatSwitch :
            if([anEvent position] == 0)          return;
            appKey |= [anEvent position] | OEHatSwitchMask;
            break;
        default : return;
    }
    
    [self setEventValue:appKey forEmulatorKey:emulKey];
}

- (void)HIDEventWasRemovedForKey:(NSString *)keyName
{
    [self unsetEventForKey:keyName withValueMask:HID_MASK];
}

- (void)HIDKeyDown:(OEHIDEvent *)anEvent
{
    OEEmulatorKey key;
    if(OEMapGetValue(keyMap, KEYBOARD_MASK | [anEvent keycode], &key))
        [self pressEmulatorKey:key];
}

- (void)HIDKeyUp:(OEHIDEvent *)anEvent
{
    OEEmulatorKey key;
    if(OEMapGetValue(keyMap, KEYBOARD_MASK | [anEvent keycode], &key))
        [self releaseEmulatorKey:key];
}

- (void)keyDown:(NSEvent *)theEvent
{
}

- (void)keyUp:(NSEvent *)theEvent
{
}

- (void)axisMoved:(OEHIDEvent *)anEvent
{
    NSUInteger value = HID_MASK | PAD_NUMBER;
    NSInteger dir  = [anEvent direction];
    NSInteger axis = value | [anEvent axis] << 16;
    OEEmulatorKey key;
    
    if(dir == OEHIDDirectionNull)
    {
        if(OEMapGetValue(keyMap, axis | DIRECTION_MASK(OEHIDDirectionNegative), &key))
            [self releaseEmulatorKey:key];
        if(OEMapGetValue(keyMap, axis | DIRECTION_MASK(OEHIDDirectionPositive), &key))
            [self releaseEmulatorKey:key];
        return;
    }
    else if(dir == OEHIDDirectionNegative)
    {
        if(OEMapGetValue(keyMap, axis | DIRECTION_MASK(OEHIDDirectionPositive), &key))
            [self releaseEmulatorKey:key];
    }
    else if(dir == OEHIDDirectionPositive)
    {
        if(OEMapGetValue(keyMap, axis | DIRECTION_MASK(OEHIDDirectionNegative), &key))
            [self releaseEmulatorKey:key];
    }
    
    value = axis  | DIRECTION_MASK(dir);
    if(OEMapGetValue(keyMap, value, &key))
        [self pressEmulatorKey:key];
}

- (void)buttonDown:(OEHIDEvent *)anEvent
{
    OEEmulatorKey key;
    if(OEMapGetValue(keyMap, HID_MASK | PAD_NUMBER | [anEvent cookie], &key))
        [self pressEmulatorKey:key];
}

- (void)buttonUp:(OEHIDEvent *)anEvent
{
    OEEmulatorKey key;
    if(OEMapGetValue(keyMap, HID_MASK | PAD_NUMBER | [anEvent cookie], &key))
        [self releaseEmulatorKey:key];
}

- (void)hatSwitchChanged:(OEHIDEvent *)anEvent;
{
    OEEmulatorKey key;
    if([anEvent hasPreviousState] && [anEvent previousPosition] != 0 && OEMapGetValue(keyMap, HID_MASK | PAD_NUMBER | OEHatSwitchMask | [anEvent previousPosition], &key))
        [self releaseEmulatorKey:key];
    
    if([anEvent position] != 0 && OEMapGetValue(keyMap, HID_MASK | PAD_NUMBER | OEHatSwitchMask | [anEvent position], &key))
        [self pressEmulatorKey:key];
}

@end
