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
    OEBindingMap           *keyMap;
    CFMutableDictionaryRef  joystickStates;
}

@synthesize keyMap = keyMap;

- (id)initWithController:(OESystemController *)controller;
{
    if((self = [super initWithController:controller]))
    {
        keyMap = [[OEBindingMap alloc] initWithSystemController:controller];
        
        joystickStates = CFDictionaryCreateMutable(NULL, 0, NULL, NULL);
    }
    
    return self;
}

- (void)dealloc
{
    CFRelease(joystickStates);
}

- (OESystemKey *)emulatorKeyForKey:(NSString *)aKey index:(NSUInteger)index player:(NSUInteger)thePlayer
{
    return [self emulatorKeyForKeyIndex:index player:thePlayer];
}

- (OESystemKey *)emulatorKeyForKeyIndex:(NSUInteger)index player:(NSUInteger)thePlayer
{
    return [OESystemKey systemKeyWithKey:index player:thePlayer];
}

- (void)pressEmulatorKey:(OESystemKey *)aKey
{
    [self doesNotImplementSelector:_cmd];
}

- (void)releaseEmulatorKey:(OESystemKey *)aKey
{
    [self doesNotImplementSelector:_cmd];
}

- (void)mouseDownAtPoint:(OEIntPoint)aPoint
{
    
}

- (void)mouseUpAtPoint
{
    
}

static void *_OEJoystickStateKeyForEvent(OEHIDEvent *anEvent)
{
    NSUInteger ret = [anEvent padNumber];

    switch([anEvent type])
    {
        case OEHIDEventTypeAxis      : ret |= [anEvent axis] << 32; break;
        case OEHIDEventTypeHatSwitch : ret |=         0x39lu << 32; break;
        default : return NULL;
    }

    return (void *)ret;
}

- (void)systemBindings:(OESystemBindings *)sender didSetEvent:(OEHIDEvent *)theEvent forBinding:(id)bindingDescription playerNumber:(NSUInteger)playerNumber
{
    // Ignore off state events.
    if([theEvent hasOffState]) return;

    switch([theEvent type])
    {
        case OEHIDEventTypeAxis :
            // Register the axis for state watch.
            CFDictionarySetValue(joystickStates, _OEJoystickStateKeyForEvent(theEvent), (void *)OEHIDEventAxisDirectionNull);

            if([bindingDescription isKindOfClass:[OEOrientedKeyGroupBindingDescription class]])
            {
                OEKeyBindingDescription *keyDesc = [bindingDescription baseKey];
                OEKeyBindingDescription *oppDesc = [bindingDescription oppositeKey];
                [keyMap setSystemKey:[self emulatorKeyForKey:[keyDesc name] index:[keyDesc index] player:playerNumber] forEvent:theEvent];
                [keyMap setSystemKey:[self emulatorKeyForKey:[oppDesc name] index:[oppDesc index] player:playerNumber] forEvent:[theEvent axisEventWithOppositeDirection]];
                return;
            }
            break;
        case OEHIDEventTypeHatSwitch :
            // Register the hat switch for state watch.
            CFDictionarySetValue(joystickStates, _OEJoystickStateKeyForEvent(theEvent), (void *)OEHIDEventHatDirectionNull);

            if([bindingDescription isKindOfClass:[OEOrientedKeyGroupBindingDescription class]])
            {
                OEHIDEventHatDirection direction = [theEvent hatDirection];
                __block NSUInteger currentDir  = NORTH;

                if(direction & OEHIDEventHatDirectionNorth) currentDir = NORTH;
                if(direction & OEHIDEventHatDirectionEast)  currentDir = EAST;
                if(direction & OEHIDEventHatDirectionSouth) currentDir = SOUTH;
                if(direction & OEHIDEventHatDirectionWest)  currentDir = WEST;

                static OEHIDEventHatDirection dirs[HAT_COUNT] = { OEHIDEventHatDirectionNorth, OEHIDEventHatDirectionEast, OEHIDEventHatDirectionSouth, OEHIDEventHatDirectionWest };

                [bindingDescription enumerateKeysFromBaseKeyUsingBlock:
                 ^(OEKeyBindingDescription *key, BOOL *stop)
                 {
                     [keyMap setSystemKey:[self emulatorKeyForKey:[key name] index:[key index] player:playerNumber]
                                 forEvent:[theEvent hatSwitchEventWithDirection:dirs[currentDir % HAT_COUNT]]];

                     currentDir++;
                 }];

                return;
            }
            break;
        default :
            break;
    }

    // General fallback for keyboard, button, trigger events and axis and hat switch events not attached to a grouped key.
    [keyMap setSystemKey:[self emulatorKeyForKey:[bindingDescription name] index:[bindingDescription index] player:playerNumber] forEvent:theEvent];
}

- (void)systemBindings:(OESystemBindings *)sender didUnsetEvent:(OEHIDEvent *)theEvent forBinding:(id)bindingDescription playerNumber:(NSUInteger)playerNumber
{
    switch([theEvent type])
    {
        case OEHIDEventTypeAxis :
            CFDictionaryRemoveValue(joystickStates, _OEJoystickStateKeyForEvent(theEvent));

            if([bindingDescription isKindOfClass:[OEOrientedKeyGroupBindingDescription class]])
            {
                [keyMap removeSystemKeyForEvent:theEvent];
                [keyMap removeSystemKeyForEvent:[theEvent axisEventWithOppositeDirection]];
                return;
            }
            break;
        case OEHIDEventTypeHatSwitch :
            CFDictionaryRemoveValue(joystickStates, _OEJoystickStateKeyForEvent(theEvent));

            if([bindingDescription isKindOfClass:[OEOrientedKeyGroupBindingDescription class]])
            {
                [keyMap removeSystemKeyForEvent:[theEvent hatSwitchEventWithDirection:OEHIDEventHatDirectionNorth]];
                [keyMap removeSystemKeyForEvent:[theEvent hatSwitchEventWithDirection:OEHIDEventHatDirectionEast] ];
                [keyMap removeSystemKeyForEvent:[theEvent hatSwitchEventWithDirection:OEHIDEventHatDirectionSouth]];
                [keyMap removeSystemKeyForEvent:[theEvent hatSwitchEventWithDirection:OEHIDEventHatDirectionWest] ];
                return;
            }
            break;
        default :
            break;
    }

    [keyMap removeSystemKeyForEvent:theEvent];
}

- (void)HIDKeyDown:(OEHIDEvent *)anEvent
{
    OESystemKey *key = [keyMap systemKeyForEvent:anEvent];
    if(key != nil) [self pressEmulatorKey:key];
}

- (void)HIDKeyUp:(OEHIDEvent *)anEvent
{
    OESystemKey *key = [keyMap systemKeyForEvent:anEvent];
    if(key != nil) [self releaseEmulatorKey:key];
}

- (void)keyDown:(NSEvent *)theEvent
{
    [super keyDown:theEvent];
}

- (void)keyUp:(NSEvent *)theEvent
{
}

- (void)axisMoved:(OEHIDEvent *)anEvent
{
    OESystemKey             *key               = nil;
    void                    *joystickKey       = _OEJoystickStateKeyForEvent(anEvent);
    OEHIDEventAxisDirection  direction         = [anEvent direction];
    OEHIDEventAxisDirection  previousDirection = OEHIDEventAxisDirectionNull;

    previousDirection = (OEHIDEventAxisDirection)CFDictionaryGetValue(joystickStates, joystickKey);

    if(previousDirection == direction) return;

    switch(previousDirection)
    {
        case OEHIDEventAxisDirectionNegative :
            if((key = [keyMap systemKeyForEvent:[anEvent axisEventWithDirection:OEHIDEventAxisDirectionNegative]]))
                [self releaseEmulatorKey:key];
            break;
        case OEHIDEventAxisDirectionPositive :
            if((key = [keyMap systemKeyForEvent:[anEvent axisEventWithDirection:OEHIDEventAxisDirectionPositive]]))
                [self releaseEmulatorKey:key];
        default :
            break;
    }

    if(direction != OEHIDEventAxisDirectionNull && (key = [keyMap systemKeyForEvent:anEvent]))
        [self pressEmulatorKey:key];

    CFDictionarySetValue(joystickStates, joystickKey, (void *)direction);
}

- (void)triggerPull:(OEHIDEvent *)anEvent;
{
    OESystemKey *key = [keyMap systemKeyForEvent:anEvent];
    if(key != nil) [self pressEmulatorKey:key];
}

- (void)triggerRelease:(OEHIDEvent *)anEvent;
{
    OESystemKey *key = [keyMap systemKeyForEvent:anEvent];
    if(key != nil) [self releaseEmulatorKey:key];
}

- (void)buttonDown:(OEHIDEvent *)anEvent
{
    OESystemKey *key = [keyMap systemKeyForEvent:anEvent];
    if(key != nil) [self pressEmulatorKey:key];
}

- (void)buttonUp:(OEHIDEvent *)anEvent
{
    OESystemKey *key = [keyMap systemKeyForEvent:anEvent];
    if(key != nil) [self releaseEmulatorKey:key];
}

- (void)hatSwitchChanged:(OEHIDEvent *)anEvent;
{
    void                   *joystickKey       = _OEJoystickStateKeyForEvent(anEvent);
    
    OEHIDEventHatDirection  previousDirection = (OEHIDEventHatDirection)CFDictionaryGetValue(joystickStates, joystickKey);

    OEHIDEventHatDirection  direction = [anEvent hatDirection];
    OEHIDEventHatDirection  diff      = previousDirection ^ direction;

#define DIFF_DIRECTION(dir) do { \
    if(diff & dir) \
    { \
        OESystemKey *key = [keyMap systemKeyForEvent:[anEvent hatSwitchEventWithDirection:dir]]; \
        if(direction & dir) [self pressEmulatorKey:key]; \
        else                [self releaseEmulatorKey:key]; \
    } \
} while(NO)

    DIFF_DIRECTION(OEHIDEventHatDirectionNorth);
    DIFF_DIRECTION(OEHIDEventHatDirectionEast);
    DIFF_DIRECTION(OEHIDEventHatDirectionSouth);
    DIFF_DIRECTION(OEHIDEventHatDirectionWest);

    CFDictionarySetValue(joystickStates, joystickKey, (void *)direction);
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
