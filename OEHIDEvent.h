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

@class OEHIDDeviceHandler;

enum _OEHIDEventType {
    OEHIDAxis      = 1,
    OEHIDButton    = 2,
    OEHIDHatSwitch = 3
};
typedef NSUInteger OEHIDEventType;

enum _OEHIDAxis {
    OEHIDAxisNone = 0x00,
    OEHIDAxisX    = 0x30,
    OEHIDAxisY    = 0x31,
    OEHIDAxisZ    = 0x32,
    OEHIDAxisRx   = 0x33,
    OEHIDAxisRy   = 0x34,
    OEHIDAxisRz   = 0x35
};
typedef NSUInteger OEHIDEventAxis;

enum _OEHIDDirection {
    OEHIDDirectionNegative = -1,
    OEHIDDirectionNull     =  0,
    OEHIDDirectionPositive =  1
};
typedef NSInteger OEHIDDirection;

@interface OEHIDEvent : NSObject <NSCoding>
{
@private
    OEHIDEventType         _type;
    NSUInteger             _padNumber;
    NSTimeInterval         _timestamp;
    uint64_t               _absoluteTime;
    union {
        struct {
            OEHIDEventAxis axis;
            OEHIDDirection direction;
            NSInteger      minimum;
            NSInteger      value;
            NSInteger      maximum;
        } axis;
        struct {
            NSUInteger     buttonNumber;
            NSInteger      state;
        } button;
        struct {
            NSUInteger     position;
            NSUInteger     count;
        } hatSwitch;
    }                      _data;
    BOOL                   _isPushed;
}

- (NSString *)displayDescription;

+ (id)eventWithDeviceHandler:(OEHIDDeviceHandler *)aDeviceHandler value:(IOHIDValueRef)aValue;
- (id)initWithDeviceHandler:(OEHIDDeviceHandler *)aDeviceHandler  value:(IOHIDValueRef)aValue;

@property(readonly) NSUInteger     padNumber;
@property(readonly) NSTimeInterval timestamp;
@property(readonly) uint64_t       absoluteTime;
@property(readonly) OEHIDEventType type;
@property(readonly) BOOL           isPushed;

// Axis event
@property(readonly) OEHIDEventAxis axis;
@property(readonly) OEHIDDirection direction;
@property(readonly) NSInteger      minimum;
@property(readonly) NSInteger      value;
@property(readonly) NSInteger      maximum;

// Button event
@property(readonly) NSUInteger     buttonNumber;
@property(readonly) NSInteger      state;

// HatSwitch event
@property(readonly) NSUInteger     position;
@property(readonly) NSUInteger     count;
@end

@interface NSEvent (OEEventConversion)
+ (NSEvent *)eventWithKeyCode:(unsigned short)keyCode;
+ (NSEvent *)eventWithKeyCode:(unsigned short)keyCode keyIsDown:(BOOL)_keyDown;
+ (NSString *)charactersForKeyCode:(unsigned short)keyCode;
+ (NSString *)printableCharactersForKeyCode:(unsigned short)keyCode;
+ (NSUInteger)modifierFlagsForKeyCode:(unsigned short)keyCode;
+ (NSString *)displayDescriptionForKeyCode:(unsigned short)keyCode;
- (NSString *)displayDescription;
@end

