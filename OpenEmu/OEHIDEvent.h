/*
 Copyright (c) 2009, OpenEmu Team
 
 
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

#import <Cocoa/Cocoa.h>
#import <IOKit/hid/IOHIDLib.h>

#define OEGlobalEventsKey @"OEGlobalEventsKey"

@class OEDeviceHandler, OEHIDDeviceHandler, OEWiimoteHIDDeviceHandler;

typedef enum _OEHIDEventType : NSUInteger {
    OEHIDEventTypeAxis      = 1,
    // Only for analogic triggers
    OEHIDEventTypeTrigger   = 5,
    OEHIDEventTypeButton    = 2,
    OEHIDEventTypeHatSwitch = 3,
	OEHIDEventTypeKeyboard  = 4,
} OEHIDEventType;

typedef enum _OEHIDAxis : NSUInteger {
    OEHIDEventAxisNone = 0x00,
    OEHIDEventAxisX    = 0x30,
    OEHIDEventAxisY    = 0x31,
    OEHIDEventAxisZ    = 0x32,
    OEHIDEventAxisRx   = 0x33,
    OEHIDEventAxisRy   = 0x34,
    OEHIDEventAxisRz   = 0x35
} OEHIDEventAxis;

typedef enum _OEHIDEventAxisDirection : NSInteger {
    OEHIDEventAxisDirectionNegative = -1,
    OEHIDEventAxisDirectionNull     =  0,
    OEHIDEventAxisDirectionPositive =  1
} OEHIDEventAxisDirection;

typedef enum _OEHIDEventHatSwitchType : NSUInteger {
    OEHIDEventHatSwitchTypeUnknown,
    OEHIDEventHatSwitchType4Ways,
    OEHIDEventHatSwitchType8Ways
} OEHIDEventHatSwitchType;

typedef enum _OEHIDEventHatDirection : NSUInteger {
    OEHIDEventHatDirectionNull      = 0,
    OEHIDEventHatDirectionNorth     = 1 << 0,
    OEHIDEventHatDirectionEast      = 1 << 1,
    OEHIDEventHatDirectionSouth     = 1 << 2,
    OEHIDEventHatDirectionWest      = 1 << 3,
    
    OEHIDEventHatDirectionNorthEast = OEHIDEventHatDirectionNorth | OEHIDEventHatDirectionEast,
    OEHIDEventHatDirectionSouthEast = OEHIDEventHatDirectionSouth | OEHIDEventHatDirectionEast,
    OEHIDEventHatDirectionNorthWest = OEHIDEventHatDirectionNorth | OEHIDEventHatDirectionWest,
    OEHIDEventHatDirectionSouthWest = OEHIDEventHatDirectionSouth | OEHIDEventHatDirectionWest
} OEHIDEventHatDirection;

typedef enum _OEHIDEventState : NSInteger {
    OEHIDEventStateOff,
    OEHIDEventStateOn
} OEHIDEventState;

enum : NSUInteger {
    OEUndefinedCookie = 0
};

extern OEHIDEventHatDirection OEHIDEventHatDirectionFromNSString(NSString *string);
extern NSString *NSStringFromOEHIDHatDirection(OEHIDEventHatDirection dir);
extern NSString *NSLocalizedStringFromOEHIDHatDirection(OEHIDEventHatDirection dir);
extern NSString *OEHIDEventAxisDisplayDescription(OEHIDEventAxis axis, OEHIDEventAxisDirection direction);

extern NSString *NSStringFromOEHIDEventType(OEHIDEventType type);
extern OEHIDEventAxis OEHIDEventAxisFromNSString(NSString *string);
extern NSString *NSStringFromOEHIDEventAxis(OEHIDEventAxis axis);
extern NSString *NSStringFromIOHIDElement(IOHIDElementRef elem);

@interface OEHIDEvent : NSObject <NSCopying, NSCoding>

- (NSString *)displayDescription;

+ (NSUInteger)keyCodeForVirtualKey:(CGCharCode)charCode;
+ (instancetype)eventWithDeviceHandler:(OEHIDDeviceHandler *)aDeviceHandler value:(IOHIDValueRef)aValue;
+ (instancetype)axisEventWithDeviceHandler:(OEDeviceHandler *)aDeviceHandler timestamp:(NSTimeInterval)timestamp axis:(OEHIDEventAxis)axis direction:(OEHIDEventAxisDirection)direction cookie:(NSUInteger)cookie;
+ (instancetype)axisEventWithDeviceHandler:(OEDeviceHandler *)aDeviceHandler timestamp:(NSTimeInterval)timestamp axis:(OEHIDEventAxis)axis value:(CGFloat)value cookie:(NSUInteger)cookie;
+ (instancetype)axisEventWithDeviceHandler:(OEDeviceHandler *)aDeviceHandler timestamp:(NSTimeInterval)timestamp axis:(OEHIDEventAxis)axis minimum:(NSInteger)minimum value:(NSInteger)value maximum:(NSInteger)maximum cookie:(NSUInteger)cookie;
+ (instancetype)triggerEventWithDeviceHandler:(OEDeviceHandler *)aDeviceHandler timestamp:(NSTimeInterval)timestamp axis:(OEHIDEventAxis)axis direction:(OEHIDEventAxisDirection)direction cookie:(NSUInteger)cookie;
+ (instancetype)triggerEventWithDeviceHandler:(OEDeviceHandler *)aDeviceHandler timestamp:(NSTimeInterval)timestamp axis:(OEHIDEventAxis)axis value:(NSInteger)value maximum:(NSInteger)maximum cookie:(NSUInteger)cookie;
+ (instancetype)triggerEventWithDeviceHandler:(OEDeviceHandler *)aDeviceHandler timestamp:(NSTimeInterval)timestamp axis:(OEHIDEventAxis)axis value:(CGFloat)value cookie:(NSUInteger)cookie;
+ (instancetype)buttonEventWithDeviceHandler:(OEDeviceHandler *)aDeviceHandler timestamp:(NSTimeInterval)timestamp buttonNumber:(NSUInteger)number state:(OEHIDEventState)state cookie:(NSUInteger)cookie;
+ (instancetype)hatSwitchEventWithDeviceHandler:(OEDeviceHandler *)aDeviceHandler timestamp:(NSTimeInterval)timestamp type:(OEHIDEventHatSwitchType)hatSwitchType direction:(OEHIDEventHatDirection)aDirection cookie:(NSUInteger)cookie;
+ (instancetype)keyEventWithTimestamp:(NSTimeInterval)timestamp keyCode:(NSUInteger)keyCode state:(OEHIDEventState)state cookie:(NSUInteger)cookie;

@property(readonly) OEDeviceHandler        *deviceHandler;
@property(readonly) NSTimeInterval          timestamp;
@property(readonly) OEHIDEventType          type;
@property(readonly) NSUInteger              cookie;
@property(readonly) NSUInteger              usagePage;
@property(readonly) NSUInteger              usage;
@property(readonly) BOOL                    hasOffState;

// Axis event or Trigger event
@property(readonly) OEHIDEventAxis          axis;              // Axis and Trigger
@property(readonly) OEHIDEventAxisDirection direction;         // Axis and Trigger (only Null and Positive for Trigger)
@property(readonly) OEHIDEventAxisDirection oppositeDirection; // Axis only
@property(readonly) CGFloat                 value;             // Axis and Trigger

// Button event
@property(readonly) NSUInteger              buttonNumber;

// Key event
@property(readonly) NSUInteger              keycode;

// Button or Key event state
@property(readonly) OEHIDEventState         state;

// HatSwitch event
@property(readonly) OEHIDEventHatSwitchType hatSwitchType;
@property(readonly) OEHIDEventHatDirection  hatDirection;

- (BOOL)isEqualToEvent:(OEHIDEvent *)anObject;
- (BOOL)isUsageEqualToEvent:(OEHIDEvent *)anObject; // Checks all properties but state

- (NSUInteger)genericIdentifier;

@end

@interface OEHIDEvent (OEHIDEventCopy)

// Axis event copy
- (instancetype)axisEventWithOppositeDirection;
- (instancetype)axisEventWithDirection:(OEHIDEventAxisDirection)aDirection;

// Hatswitch event copy
- (instancetype)hatSwitchEventWithDirection:(OEHIDEventHatDirection)aDirection;

- (instancetype)eventWithDeviceHandler:(OEDeviceHandler *)aDeviceHandler;

@end

@interface OEHIDEvent (OEHIDEventBinding)

- (NSUInteger)bindingHash;
- (BOOL)isBindingEqualToEvent:(OEHIDEvent *)anEvent;

@end

@interface NSEvent (OEEventConversion)
+ (NSEvent *)eventWithKeyCode:(unsigned short)keyCode;
+ (NSEvent *)eventWithKeyCode:(unsigned short)keyCode keyIsDown:(BOOL)keyDown;
+ (NSString *)charactersForKeyCode:(unsigned short)keyCode;
+ (NSString *)printableCharactersForKeyCode:(unsigned short)keyCode;
+ (NSUInteger)modifierFlagsForKeyCode:(unsigned short)keyCode;
+ (NSString *)displayDescriptionForKeyCode:(unsigned short)keyCode;
- (NSString *)displayDescription;
@end

@interface NSNumber (OEEventConversion)
- (NSString *)displayDescription;
@end
