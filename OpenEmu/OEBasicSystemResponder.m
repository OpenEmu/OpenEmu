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

@interface OEBasicSystemResponder ()

@property (strong) NSMutableArray * neverUpKeys;

@end


@implementation OEBasicSystemResponder
@synthesize neverUpKeys;

- (id)initWithController:(OESystemController *)controller;
{
    if((self = [super initWithController:controller]))
    {
        keyMap = OEMapCreate([[controller genericControlNames] count]);
        neverUpKeys = [NSMutableArray new];
    }
    
    return self;
}

- (void)dealloc
{
    OEMapRelease(keyMap);
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

// TODO: KeyDown and HIDKeyUp are both executed if using keyboard-- Suggesting that Keyboard be removed as a IOHID devcie.

- (void)keyDown:(NSEvent *)theEvent
{
    OEEmulatorKey key;
    if(OEMapGetValue(keyMap, KEYBOARD_MASK | [OEHIDEvent keyCodeForVK:theEvent.keyCode], &key))
    {
        if (![neverUpKeys containsObject:[NSNumber numberWithUnsignedLong:key.key]])
        {
            [neverUpKeys addObject:[NSNumber numberWithUnsignedLong:key.key]];
            [self pressEmulatorKey:key];
        }
    }
}

- (void)keyUp:(NSEvent *)theEvent
{
    OEEmulatorKey key;
    if(OEMapGetValue(keyMap, KEYBOARD_MASK | [OEHIDEvent keyCodeForVK:theEvent.keyCode], &key))
    {
        [neverUpKeys removeObject:[NSNumber numberWithUnsignedLong:key.key]];
        [self releaseEmulatorKey:key];
    }
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