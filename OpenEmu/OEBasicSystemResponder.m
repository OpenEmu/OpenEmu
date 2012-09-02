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

#define OEHatSwitchMask     (0x39 << 16)
#define PAD_NUMBER(anEvent) ([anEvent padNumber] << 24)
#define KEYBOARD_MASK 0x40000000u
#define HID_MASK      0x20000000u
#define AXIS_MASK(anEvent) ([anEvent axis] << 16)
#define DIRECTION_MASK(dir) (1 << ((dir) == OEHIDAxisDirectionPositive))

#define GET_EMUL_KEY(keyName) ({                                      \
    NSUInteger index, player;                                         \
    NSString *generic = [[self controller] genericKeyForKey:keyName   \
                                                getKeyIndex:&index    \
                                               playerNumber:&player]; \
    if(generic == nil) return;                                        \
    [self emulatorKeyForKey:keyName index:index player:player];       \
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

- (void)keyboardEvent:(id)theEvent wasSetForKey:(NSString *)keyName
{
    OEEmulatorKey emulKey = GET_EMUL_KEY(keyName);
    NSInteger appKey = KEYBOARD_MASK | [theEvent keycode];
    
    [self setEventValue:appKey forEmulatorKey:emulKey];
}

- (void)keyboardEvent:(id)theEvent wasUnsetForKey:(NSString *)keyName;
{
    [self unsetEventForKey:keyName withValueMask:KEYBOARD_MASK];
}

- (void)HIDEvent:(OEHIDEvent *)theEvent wasSetForKey:(NSString *)keyName
{
    NSInteger appKey = HID_MASK | PAD_NUMBER(theEvent);
    
    switch([theEvent type])
    {
        case OEHIDAxis :
        {
            OEHIDAxisDirection dir = [theEvent direction];
            if(dir == OEHIDAxisDirectionNull) return;
            
            appKey |= ([theEvent axis] << 16);
            
            NSUInteger     index, player;
            NSString      *generic     = [[self controller] genericKeyForKey:keyName getKeyIndex:&index playerNumber:&player];
            
            NSUInteger     oppositeIdx = 0;
            NSString      *oppositeKey = [[self controller] oppositeKeyForAxisKey:generic getKeyIndex:&oppositeIdx];
            
            OEEmulatorKey  emulKey = [self emulatorKeyForKey:generic index:index player:player];
            
            // Register the opposite key for to opposite direction if we have an axis-based key
            if(oppositeKey != nil)
            {
                OEEmulatorKey oppEmulKey = [self emulatorKeyForKey:oppositeKey index:oppositeIdx player:player];
                [self setEventValue:appKey | DIRECTION_MASK([theEvent oppositeDirection]) forEmulatorKey:oppEmulKey];
                
                NSLog(@"Opposite key: %lx (%x), key: %lx (%x)", appKey | DIRECTION_MASK([theEvent oppositeDirection]), DIRECTION_MASK([theEvent oppositeDirection]), appKey | DIRECTION_MASK(dir), DIRECTION_MASK(dir));
                
                CFDictionarySetValue(joystickStates, (void *)appKey, (void *)OEHIDAxisDirectionNull);
            }
            
            [self setEventValue:appKey | DIRECTION_MASK(dir) forEmulatorKey:emulKey];
        }
            break;
        case OEHIDHatSwitch :
        {
            OEHIDHatDirection direction = [theEvent hatDirection];
            if(direction == OEHIDHatDirectionNull) return;
            
            __block NSUInteger currentDir  = NORTH;
            __block BOOL       isHatSwitch = NO;
            
            if(direction & OEHIDHatDirectionNorth) currentDir = NORTH;
            if(direction & OEHIDHatDirectionEast)  currentDir = EAST;
            if(direction & OEHIDHatDirectionSouth) currentDir = SOUTH;
            if(direction & OEHIDHatDirectionWest)  currentDir = WEST;
            
            NSUInteger  index, player;
            NSString   *generic = [[self controller] genericKeyForKey:keyName getKeyIndex:&index playerNumber:&player];
            
            static OEHIDHatDirection dirs[HAT_COUNT] = { OEHIDHatDirectionNorth, OEHIDHatDirectionEast, OEHIDHatDirectionSouth, OEHIDHatDirectionWest };
            
            [[self controller] enumerateKeysLinkedToHatSwitchKey:generic usingBlock:
             ^(NSString *key, NSUInteger keyIdx, BOOL *stop)
             {
                 isHatSwitch = YES;
                 
                 OEEmulatorKey emulKey = [self emulatorKeyForKey:key index:keyIdx player:player];
                 
                 NSLog(@"Key: %@, direction: %@", key, OEHIDEventHatSwitchDisplayDescription(0, dirs[currentDir]));
                 [self setEventValue:appKey | OEHatSwitchMask | dirs[currentDir] forEmulatorKey:emulKey];
                 
                 currentDir++;
                 currentDir %= HAT_COUNT;
             }];
            
            if(isHatSwitch)
                CFDictionarySetValue(joystickStates, (void *)(appKey | OEHatSwitchMask), (void *)OEHIDHatDirectionNull);
            else
                [self setEventValue:appKey | [theEvent hatDirection] | OEHatSwitchMask forEmulatorKey:GET_EMUL_KEY(keyName)];
        }
            break;
        case OEHIDButton :
            if([theEvent state] == NSOffState) return;
            [self setEventValue:appKey | [theEvent cookie] forEmulatorKey:GET_EMUL_KEY(keyName)];
            break;
        default : return;
    }
}

- (void)HIDEvent:(OEHIDEvent *)theEvent wasUnsetForKey:(NSString *)keyName;
{
    switch([theEvent type])
    {
        case OEHIDAxis :
        {
            NSUInteger player;
            NSString *generic     = [[self controller] genericKeyForKey:keyName getKeyIndex:NULL playerNumber:&player];
            NSString *oppositeKey = [[self controller] oppositeKeyForAxisKey:generic getKeyIndex:NULL];
            
            if(oppositeKey != nil)
            {
                oppositeKey = [[self controller] playerKeyForKey:oppositeKey player:player];
                [self unsetEventForKey:oppositeKey withValueMask:HID_MASK];
                
                CFDictionaryRemoveValue(joystickStates, (void *)(HID_MASK | PAD_NUMBER(theEvent) | AXIS_MASK(theEvent)));
            }
            
            [self unsetEventForKey:keyName withValueMask:HID_MASK];
        }
            break;
        case OEHIDHatSwitch :
        {
            __block BOOL isHID = NO;
            
            NSUInteger player;
            NSString *generic = [[self controller] genericKeyForKey:keyName getKeyIndex:NULL playerNumber:&player];
            
            [[self controller] enumerateKeysLinkedToHatSwitchKey:generic usingBlock:
             ^(NSString *key, NSUInteger keyIdx, BOOL *stop)
             {
                 isHID = YES;
                 
                 key = [[self controller] playerKeyForKey:key player:player];
                 [self unsetEventForKey:key withValueMask:HID_MASK];
             }];
            
            if(isHID) CFDictionaryRemoveValue(joystickStates, (void *)(HID_MASK | PAD_NUMBER(theEvent) | OEHatSwitchMask));
            else      [self unsetEventForKey:keyName withValueMask:HID_MASK];
        }
            break;
        case OEHIDButton :
            [self unsetEventForKey:keyName withValueMask:HID_MASK];
            break;
        default : break;
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
    NSInteger dir  = [anEvent direction];
    NSInteger axis = value | AXIS_MASK(anEvent);
    OEEmulatorKey key;
    
    OEHIDAxisDirection previousDirection = OEHIDAxisDirectionNull;
    
    if(CFDictionaryGetValueIfPresent(joystickStates, (void *)axis, (void *)&previousDirection))
    {
        if(previousDirection == dir) return;
        
        switch(previousDirection)
        {
            case OEHIDAxisDirectionNegative :
                if(OEMapGetValue(keyMap, axis | DIRECTION_MASK(OEHIDAxisDirectionNegative), &key))
                    [self releaseEmulatorKey:key];
                break;
            case OEHIDAxisDirectionPositive :
                if(OEMapGetValue(keyMap, axis | DIRECTION_MASK(OEHIDAxisDirectionPositive), &key))
                    [self releaseEmulatorKey:key];
            default :
                break;
        }
        
        if(dir != OEHIDAxisDirectionNull && OEMapGetValue(keyMap, axis | DIRECTION_MASK(dir), &key))
            [self pressEmulatorKey:key];
        
        CFDictionarySetValue(joystickStates, (void *)axis, (void *)dir);
    }
    else
    {
        if(dir == OEHIDAxisDirectionNull)
        {
            if(OEMapGetValue(keyMap, axis | DIRECTION_MASK(OEHIDAxisDirectionNegative), &key))
                [self releaseEmulatorKey:key];
            if(OEMapGetValue(keyMap, axis | DIRECTION_MASK(OEHIDAxisDirectionPositive), &key))
                [self releaseEmulatorKey:key];
            return;
        }
        else if(dir == OEHIDAxisDirectionNegative)
        {
            if(OEMapGetValue(keyMap, axis | DIRECTION_MASK(OEHIDAxisDirectionPositive), &key))
                [self releaseEmulatorKey:key];
        }
        else if(dir == OEHIDAxisDirectionPositive)
        {
            if(OEMapGetValue(keyMap, axis | DIRECTION_MASK(OEHIDAxisDirectionNegative), &key))
                [self releaseEmulatorKey:key];
        }
        
        if(OEMapGetValue(keyMap, axis | DIRECTION_MASK(dir), &key))
            [self pressEmulatorKey:key];
    }
}

- (void)buttonDown:(OEHIDEvent *)anEvent
{
    OEEmulatorKey key;
    if(OEMapGetValue(keyMap, HID_MASK | PAD_NUMBER(anEvent) | [anEvent cookie], &key))
        [self pressEmulatorKey:key];
}

- (void)buttonUp:(OEHIDEvent *)anEvent
{
    OEEmulatorKey key;
    if(OEMapGetValue(keyMap, HID_MASK | PAD_NUMBER(anEvent) | [anEvent cookie], &key))
        [self releaseEmulatorKey:key];
}

- (void)hatSwitchChanged:(OEHIDEvent *)anEvent;
{
    OEHIDHatDirection previousDirection = OEHIDHatDirectionNull;
    
    NSUInteger hatSwitch = (HID_MASK | PAD_NUMBER(anEvent) | OEHatSwitchMask);
    
    if(CFDictionaryGetValueIfPresent(joystickStates, (void *)hatSwitch, (void *)&previousDirection))
    {
        OEHIDHatDirection direction = [anEvent hatDirection];
        OEHIDHatDirection diff      = previousDirection ^ direction;
        
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
        
        DIFF_DIRECTION(OEHIDHatDirectionNorth);
        DIFF_DIRECTION(OEHIDHatDirectionEast);
        DIFF_DIRECTION(OEHIDHatDirectionSouth);
        DIFF_DIRECTION(OEHIDHatDirectionWest);
        
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
