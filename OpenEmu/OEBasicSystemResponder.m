/*
 Copyright (c) 2011, OpenEmu Team
 
 
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

#import "OEBasicSystemResponder.h"
#import "OEAbstractAdditions.h"
#import "OEHIDEvent.h"
#import "OESystemController.h"
#import "OEEvent.h"
#import "OEKeyBindingDescription.h"
#import "OEKeyBindingGroupDescription.h"

enum { NORTH, EAST, SOUTH, WEST, HAT_COUNT };

@implementation OEBasicSystemResponder
{
    OEMapRef               keyMap;
    CFMutableDictionaryRef joystickStates;
}

- (id)initWithController:(OESystemController *)controller;
{
    if((self = [super initWithController:controller]))
    {
        keyMap = OEMapCreate([[controller genericControlNames] count]);
        
        joystickStates = CFDictionaryCreateMutable(NULL, 0, NULL, NULL);
    }
    
    return self;
}

- (void)dealloc
{
    OEMapRelease(keyMap);
    CFRelease(joystickStates);
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

- (void)mouseDownAtPoint:(OEIntPoint)aPoint
{
    
}

- (void)mouseUpAtPoint
{
    
}

#define OEHatSwitchMask        (0x39 << 16)
#define PAD_NUMBER(anEvent)    ([anEvent padNumber] << 24)
#define KEYBOARD_MASK          0x40000000u
#define HID_MASK               0x20000000u
#define BUTTON_TYPE_MASK       0x00100000u
#define BUTTON_NUMBER(anEvent) ([anEvent buttonNumber])
#define AXIS_MASK(anEvent)     ([anEvent axis] << 16)
#define DIRECTION_MASK(dir)    (1 << ((dir) == OEHIDEventAxisDirectionPositive))

- (void)setEventValue:(NSInteger)appKey forEmulatorKey:(OEEmulatorKey)emulKey
{
    OEMapSetValue(keyMap, appKey, emulKey);
}

- (void)unsetEventWithMask:(NSUInteger)keyMask forEmulatorKey:(OEEmulatorKey)emulKey;
{
    OEMapRemoveMaskedKeysForValue(keyMap, keyMask, emulKey);
}

- (void)systemBindings:(OESystemBindings *)sender didSetEvent:(OEHIDEvent *)theEvent forBinding:(id)bindingDescription playerNumber:(NSUInteger)playerNumber
{
    // Ignore off state events.
    if([theEvent hasOffState]) return;
    
    if([theEvent type] == OEHIDEventTypeKeyboard)
    {
        OEEmulatorKey emulKey = [self emulatorKeyForKey:[bindingDescription name] index:[bindingDescription index] player:playerNumber];
        NSInteger appKey = KEYBOARD_MASK | [theEvent keycode];
        [self setEventValue:appKey forEmulatorKey:emulKey];
    }
    else
    {
        NSInteger appKey = HID_MASK | PAD_NUMBER(theEvent);
        
        switch([theEvent type])
        {
            case OEHIDEventTypeButton :
                [self setEventValue:appKey | BUTTON_TYPE_MASK | BUTTON_NUMBER(theEvent) forEmulatorKey:[self emulatorKeyForKey:[bindingDescription name] index:[bindingDescription index] player:playerNumber]];
                break;
            case OEHIDEventTypeAxis :
            {
                OEHIDEventAxisDirection dir  = [theEvent direction];
                OEHIDEventAxis          axis = [theEvent axis];
                
                if(axis == OEHIDEventAxisNone) return;
                
                if([bindingDescription isKindOfClass:[OEKeyBindingDescription class]])
                {
                    OEEmulatorKey emulKey = [self emulatorKeyForKey:[bindingDescription name] index:[bindingDescription index] player:playerNumber];
                    [self setEventValue:appKey | AXIS_MASK(theEvent) | DIRECTION_MASK(dir) forEmulatorKey:emulKey];
                }
                else if([bindingDescription isKindOfClass:[OEOrientedKeyGroupBindingDescription class]])
                {
                    OEKeyBindingDescription *keyDesc = [bindingDescription baseKey];
                    OEKeyBindingDescription *oppDesc = [bindingDescription oppositeKey];
                    
                    OEEmulatorKey emulKey    = [self emulatorKeyForKey:[keyDesc name] index:[keyDesc index] player:playerNumber];
                    OEEmulatorKey oppEmulKey = [self emulatorKeyForKey:[oppDesc name] index:[oppDesc index] player:playerNumber];
                    
                    [self setEventValue:appKey | AXIS_MASK(theEvent) | DIRECTION_MASK([theEvent oppositeDirection]) forEmulatorKey:oppEmulKey];
                    [self setEventValue:appKey | AXIS_MASK(theEvent) | DIRECTION_MASK(dir) forEmulatorKey:emulKey];
                    
                    CFDictionarySetValue(joystickStates, (void *)appKey, (void *)OEHIDEventAxisDirectionNull);
                }
            }
                break;
            case OEHIDEventTypeTrigger :
            {
                // Trigger events are axis events with only one possible direction, they won't clash with axis event
                OEEmulatorKey emulKey = [self emulatorKeyForKey:[bindingDescription name] index:[bindingDescription index] player:playerNumber];
                [self setEventValue:appKey | AXIS_MASK(theEvent) | DIRECTION_MASK(OEHIDEventAxisDirectionPositive) forEmulatorKey:emulKey];
            }
                break;
            case OEHIDEventTypeHatSwitch :
            {
                OEHIDEventHatDirection direction = [theEvent hatDirection];
                
                if([bindingDescription isKindOfClass:[OEKeyBindingDescription class]])
                {
                    OEEmulatorKey emulKey = [self emulatorKeyForKey:[bindingDescription name] index:[bindingDescription index] player:playerNumber];
                    [self setEventValue:appKey | OEHatSwitchMask | [theEvent hatDirection] forEmulatorKey:emulKey];
                }
                else if([bindingDescription isKindOfClass:[OEOrientedKeyGroupBindingDescription class]])
                {
                    __block NSUInteger currentDir  = NORTH;
                    
                    if(direction & OEHIDEventHatDirectionNorth) currentDir = NORTH;
                    if(direction & OEHIDEventHatDirectionEast)  currentDir = EAST;
                    if(direction & OEHIDEventHatDirectionSouth) currentDir = SOUTH;
                    if(direction & OEHIDEventHatDirectionWest)  currentDir = WEST;
                    
                    static OEHIDEventHatDirection dirs[HAT_COUNT] = { OEHIDEventHatDirectionNorth, OEHIDEventHatDirectionEast, OEHIDEventHatDirectionSouth, OEHIDEventHatDirectionWest };
                    
                    [bindingDescription enumerateKeysFromBaseKeyUsingBlock:
                     ^(OEKeyBindingDescription *key, BOOL *stop)
                     {
                         OEEmulatorKey emulKey = [self emulatorKeyForKey:[key name] index:[key index] player:playerNumber];
                         [self setEventValue:appKey | OEHatSwitchMask | dirs[currentDir] forEmulatorKey:emulKey];
                         
                         currentDir++;
                         currentDir %= HAT_COUNT;
                     }];
                    
                    CFDictionarySetValue(joystickStates, (void *)(appKey | OEHatSwitchMask), (void *)OEHIDEventHatDirectionNull);
                }
            }
                break;
            default :
                break;
        }
    }
}

- (void)systemBindings:(OESystemBindings *)sender didUnsetEvent:(OEHIDEvent *)theEvent forBinding:(id)bindingDescription playerNumber:(NSUInteger)playerNumber
{
    if([theEvent type] == OEHIDEventTypeKeyboard)
    {
        OEEmulatorKey emulKey = [self emulatorKeyForKey:[bindingDescription name] index:[bindingDescription index] player:playerNumber];
        [self unsetEventWithMask:KEYBOARD_MASK forEmulatorKey:emulKey];
    }
    else
    {
        if([bindingDescription isKindOfClass:[OEKeyBindingDescription class]])
        {
            OEEmulatorKey emulKey = [self emulatorKeyForKey:[bindingDescription name] index:[bindingDescription index] player:playerNumber];
            [self unsetEventWithMask:HID_MASK forEmulatorKey:emulKey];
        }
        else if([bindingDescription isKindOfClass:[OEOrientedKeyGroupBindingDescription class]])
        {
            [bindingDescription enumerateKeysFromBaseKeyUsingBlock:
             ^(OEKeyBindingDescription *key, BOOL *stop)
             {
                 OEEmulatorKey emulKey = [self emulatorKeyForKey:[key name] index:[key index] player:playerNumber];
                 [self unsetEventWithMask:HID_MASK forEmulatorKey:emulKey];
             }];
            
            switch([theEvent type])
            {
                case OEHIDEventTypeAxis :
                    CFDictionaryRemoveValue(joystickStates, (void *)(HID_MASK | PAD_NUMBER(theEvent) | AXIS_MASK(theEvent)));
                    break;
                case OEHIDEventTypeHatSwitch :
                    CFDictionaryRemoveValue(joystickStates, (void *)(HID_MASK | PAD_NUMBER(theEvent) | OEHatSwitchMask));
                    break;
                default :
                    break;
            }
        }
        else return;
    }
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
    NSUInteger value = HID_MASK | PAD_NUMBER(anEvent);
    NSInteger  dir   = [anEvent direction];
    NSInteger  axis  = value | AXIS_MASK(anEvent);
    OEEmulatorKey key;
    
    OEHIDEventAxisDirection previousDirection = OEHIDEventAxisDirectionNull;
    
    if(CFDictionaryGetValueIfPresent(joystickStates, (void *)axis, (void *)&previousDirection))
    {
        if(previousDirection == dir) return;
        
        switch(previousDirection)
        {
            case OEHIDEventAxisDirectionNegative :
                if(OEMapGetValue(keyMap, axis | DIRECTION_MASK(OEHIDEventAxisDirectionNegative), &key))
                    [self releaseEmulatorKey:key];
                break;
            case OEHIDEventAxisDirectionPositive :
                if(OEMapGetValue(keyMap, axis | DIRECTION_MASK(OEHIDEventAxisDirectionPositive), &key))
                    [self releaseEmulatorKey:key];
            default :
                break;
        }
        
        if(dir != OEHIDEventAxisDirectionNull && OEMapGetValue(keyMap, axis | DIRECTION_MASK(dir), &key))
            [self pressEmulatorKey:key];
        
        CFDictionarySetValue(joystickStates, (void *)axis, (void *)dir);
    }
    else
    {
        if(dir == OEHIDEventAxisDirectionNull)
        {
            if(OEMapGetValue(keyMap, axis | DIRECTION_MASK(OEHIDEventAxisDirectionNegative), &key))
                [self releaseEmulatorKey:key];
            if(OEMapGetValue(keyMap, axis | DIRECTION_MASK(OEHIDEventAxisDirectionPositive), &key))
                [self releaseEmulatorKey:key];
            return;
        }
        else if(dir == OEHIDEventAxisDirectionNegative)
        {
            if(OEMapGetValue(keyMap, axis | DIRECTION_MASK(OEHIDEventAxisDirectionPositive), &key))
                [self releaseEmulatorKey:key];
        }
        else if(dir == OEHIDEventAxisDirectionPositive)
        {
            if(OEMapGetValue(keyMap, axis | DIRECTION_MASK(OEHIDEventAxisDirectionNegative), &key))
                [self releaseEmulatorKey:key];
        }
        
        if(OEMapGetValue(keyMap, axis | DIRECTION_MASK(dir), &key))
            [self pressEmulatorKey:key];
    }
}

- (void)triggerPull:(OEHIDEvent *)anEvent;
{
    OEEmulatorKey key;
    if(OEMapGetValue(keyMap, HID_MASK | PAD_NUMBER(anEvent) | AXIS_MASK(anEvent) | DIRECTION_MASK(OEHIDEventAxisDirectionPositive), &key))
        [self pressEmulatorKey:key];
}

- (void)triggerRelease:(OEHIDEvent *)anEvent;
{
    OEEmulatorKey key;
    if(OEMapGetValue(keyMap, HID_MASK | PAD_NUMBER(anEvent) | AXIS_MASK(anEvent) | DIRECTION_MASK(OEHIDEventAxisDirectionPositive), &key))
        [self releaseEmulatorKey:key];
}

- (void)buttonDown:(OEHIDEvent *)anEvent
{
    OEEmulatorKey key;
    if(OEMapGetValue(keyMap, HID_MASK | PAD_NUMBER(anEvent) | BUTTON_TYPE_MASK | BUTTON_NUMBER(anEvent), &key))
        [self pressEmulatorKey:key];
}

- (void)buttonUp:(OEHIDEvent *)anEvent
{
    OEEmulatorKey key;
    if(OEMapGetValue(keyMap, HID_MASK | PAD_NUMBER(anEvent) | BUTTON_TYPE_MASK | BUTTON_NUMBER(anEvent), &key))
        [self releaseEmulatorKey:key];
}

- (void)hatSwitchChanged:(OEHIDEvent *)anEvent;
{
    OEHIDEventHatDirection previousDirection = OEHIDEventHatDirectionNull;
    
    NSUInteger hatSwitch = (HID_MASK | PAD_NUMBER(anEvent) | OEHatSwitchMask);
    
    if(CFDictionaryGetValueIfPresent(joystickStates, (void *)hatSwitch, (void *)&previousDirection))
    {
        OEHIDEventHatDirection direction = [anEvent hatDirection];
        OEHIDEventHatDirection diff      = previousDirection ^ direction;
        
#define DIFF_DIRECTION(dir) do { \
    if(diff & dir) \
    { \
        OEEmulatorKey key; \
        if(direction & dir) \
        { \
            if(OEMapGetValue(keyMap, HID_MASK | PAD_NUMBER(anEvent) | OEHatSwitchMask | dir, &key)) \
                [self pressEmulatorKey:key]; \
        } \
        else \
        { \
            if(OEMapGetValue(keyMap, HID_MASK | PAD_NUMBER(anEvent) | OEHatSwitchMask | dir, &key)) \
                [self releaseEmulatorKey:key]; \
        } \
    } \
} while(NO)
        
        DIFF_DIRECTION(OEHIDEventHatDirectionNorth);
        DIFF_DIRECTION(OEHIDEventHatDirectionEast);
        DIFF_DIRECTION(OEHIDEventHatDirectionSouth);
        DIFF_DIRECTION(OEHIDEventHatDirectionWest);
        
        CFDictionarySetValue(joystickStates, (void *)hatSwitch, (void *)direction);
    }
    else
    {
        OEEmulatorKey key;
        if([anEvent hasPreviousState] && [anEvent previousHatDirection] != 0 && OEMapGetValue(keyMap, hatSwitch | [anEvent previousHatDirection], &key))
            [self releaseEmulatorKey:key];
        
        if([anEvent hatDirection] != 0 && OEMapGetValue(keyMap, hatSwitch | [anEvent hatDirection], &key))
            [self pressEmulatorKey:key];
    }
}

- (void)mouseDown:(NSEvent *)theEvent
{
    OEIntPoint point = [theEvent locationInGameView];
    [self mouseDownAtPoint:point];
}

- (void)mouseDragged:(NSEvent *)theEvent
{
    OEIntPoint point = [theEvent locationInGameView];
    [self mouseDownAtPoint:point];
}
    
- (void)mouseUp:(NSEvent *)theEvent
{
    [self mouseUpAtPoint];
}

@end
