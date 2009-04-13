//
//  OEHIDEvent.h
//  OpenEmu
//
//  Created by Remy Demarest on 25/02/2009.
//  Copyright 2009 Psycho Inc.. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <IOKit/hid/IOHIDLib.h>

#define OEGlobalEventsKey @"OEGlobalEventsKey"

typedef enum OEHIDEventAxis {
    OEHIDEventAxisNone = 0x00,
    OEHIDEventAxisX    = 0x30,
    OEHIDEventAxisY    = 0x31,
    OEHIDEventAxisZ    = 0x32,
    OEHIDEventAxisRx   = 0x33,
    OEHIDEventAxisRy   = 0x34,
    OEHIDEventAxisRz   = 0x35
} OEHIDEventAxis;

// Dummy class to easily dispatch an event as an action through the responder chains.
@interface OEHIDEvent : NSObject <NSCoding>
{
    IOHIDDeviceRef device;
    NSUInteger padNumber;
    NSUInteger buttonNumber;
    OEHIDEventAxis axis;
    CGFloat value;
}

- (NSString *)axisName;
- (NSString *)displayDescription;

+ (id)eventWithDevice:(IOHIDDeviceRef)aDevice deviceNumber:(NSUInteger)aNumber page:(uint32_t)aPage usage:(uint32_t)aUsage value:(CGFloat)aValue;
- (id)initWithDevice:(IOHIDDeviceRef)aDevice deviceNumber:(NSUInteger)aNumber page:(uint32_t)aPage usage:(uint32_t)aUsage value:(CGFloat)aValue;
@property(readonly) IOHIDDeviceRef device;
@property(readonly) NSUInteger padNumber;
@property(readonly) NSUInteger buttonNumber;
@property(readonly) OEHIDEventAxis axis;
@property(readonly) CGFloat value;
@end

@interface NSEvent (OEEventConversion)
+ (NSEvent *)eventWithKeyCode:(unsigned short)keyCode;
+ (NSEvent *)eventWithKeyCode:(unsigned short)keyCode keyIsDown:(BOOL)_keyDown;
+ (NSString *)charactersForKeyCode:(unsigned short)keyCode;
+ (NSString *)printableCharactersForKeyCode:(unsigned short)keyCode;
+ (NSUInteger)modifierFlagsForKeyCode:(unsigned short)keyCode;
- (NSString *)displayDescription;
@end

