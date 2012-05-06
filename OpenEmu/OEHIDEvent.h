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

@class OEHIDDeviceHandler;

typedef enum _OEHIDEventType : NSUInteger {
    OEHIDAxis      = 1,
    OEHIDButton    = 2,
    OEHIDHatSwitch = 3,
	OEHIDKeypress  = 4,
} OEHIDEventType;

typedef enum _OEHIDAxis : NSUInteger {
    OEHIDAxisNone = 0x00,
    OEHIDAxisX    = 0x30,
    OEHIDAxisY    = 0x31,
    OEHIDAxisZ    = 0x32,
    OEHIDAxisRx   = 0x33,
    OEHIDAxisRy   = 0x34,
    OEHIDAxisRz   = 0x35
} OEHIDEventAxis;

typedef enum _OEHIDAxisDirection : NSInteger {
    OEHIDAxisDirectionNegative = -1,
    OEHIDAxisDirectionNull     =  0,
    OEHIDAxisDirectionPositive =  1
} OEHIDAxisDirection;

typedef enum _OEHIDHatSwitchType : NSUInteger {
    OEHIDHatSwitchTypeUnknown,
    OEHIDHatSwitchType4Ways,
    OEHIDHatSwitchType8Ways
} OEHIDHatSwitchType;

typedef enum _OEHIDHatDirection : NSUInteger {
    OEHIDHatDirectionNull      = 0,
    OEHIDHatDirectionNorth     = 1 << 0,
    OEHIDHatDirectionEast      = 1 << 1,
    OEHIDHatDirectionSouth     = 1 << 2,
    OEHIDHatDirectionWest      = 1 << 3,
    
    OEHIDHatDirectionNorthEast = OEHIDHatDirectionNorth | OEHIDHatDirectionEast,
    OEHIDHatDirectionSouthEast = OEHIDHatDirectionSouth | OEHIDHatDirectionEast,
    OEHIDHatDirectionNorthWest = OEHIDHatDirectionNorth | OEHIDHatDirectionWest,
    OEHIDHatDirectionSouthWest = OEHIDHatDirectionSouth | OEHIDHatDirectionWest
} OEHIDHatDirection;

extern NSString *NSStringFromOEHIDHatDirection(OEHIDHatDirection dir);
extern NSString *OEHIDEventAxisDisplayDescription(NSUInteger padNumber, OEHIDEventAxis axis, OEHIDAxisDirection direction);
extern NSString *OEHIDEventHatSwitchDisplayDescription(NSUInteger padNumber, OEHIDHatDirection direction);

@interface OEHIDEvent : NSObject <NSCoding>

- (NSString *)displayDescription;

+ (NSUInteger)keyCodeForVK:(CGCharCode)charCode;
+ (id)axisEventWithPadNumber:(NSUInteger)padNumber timestamp:(NSTimeInterval)timestamp axis:(OEHIDEventAxis)axis scaledValue:(CGFloat)value;
+ (id)axisEventWithPadNumber:(NSUInteger)padNumber timestamp:(NSTimeInterval)timestamp axis:(OEHIDEventAxis)axis minimum:(NSInteger)minimum value:(NSInteger)value maximum:(NSInteger)maximum;
+ (id)buttonEventWithPadNumber:(NSUInteger)padNumber timestamp:(NSTimeInterval)timestamp buttonNumber:(NSUInteger)number state:(NSUInteger)state cookie:(NSUInteger)cookie;
+ (id)hatSwitchEventWithPadNumber:(NSUInteger)padNumber timestamp:(NSTimeInterval)timestamp type:(OEHIDHatSwitchType)hatSwitchType direction:(OEHIDHatDirection)aDirection cookie:(NSUInteger)cookie;
+ (id)keyEventWithTimestamp:(NSTimeInterval)timestamp keyCode:(NSUInteger)keyCode state:(NSUInteger)state cookie:(NSUInteger)cookie;
+ (id)eventWithDeviceHandler:(OEHIDDeviceHandler *)aDeviceHandler value:(IOHIDValueRef)aValue;

@property(readonly) NSUInteger         padNumber;
@property(readonly) NSTimeInterval     timestamp;
@property(readonly) NSTimeInterval     previousTimestamp;
@property(readonly) NSTimeInterval     elapsedTime;
@property(readonly) OEHIDEventType     type;
@property(readonly) NSUInteger         cookie;
@property(readonly) BOOL               hasPreviousState;
@property(readonly) BOOL               isOffState;

// Axis event
@property(readonly) OEHIDEventAxis     axis;
@property(readonly) OEHIDAxisDirection previousDirection;
@property(readonly) OEHIDAxisDirection direction;
@property(readonly) OEHIDAxisDirection oppositeDirection;
@property(readonly) NSInteger          minimum;
@property(readonly) NSInteger          previousValue;
@property(readonly) NSInteger          value;
@property(readonly) NSInteger          maximum;

// Button event
@property(readonly) NSUInteger         buttonNumber;
@property(readonly) NSInteger          previousState;
@property(readonly) NSInteger          state;

// Key event
@property(readonly) NSUInteger         keycode;
//@property(readonly) NSInteger          state;
//@property(readonly) NSInteger          previousState;

// HatSwitch event
@property(readonly) OEHIDHatSwitchType hatSwitchType;
@property(readonly) OEHIDHatDirection  previousHatDirection;
@property(readonly) OEHIDHatDirection  hatDirection;

- (BOOL)isEqualToEvent:(OEHIDEvent *)anObject;
- (BOOL)isAxisEqualToEvent:(OEHIDEvent *)anObject;      // only checks the padNumber and the axis, ignores the direction
- (BOOL)isButtonEqualToEvent:(OEHIDEvent *)anObject;    // only checks the padNumber and the button, ignores the state
- (BOOL)isHatSwitchEqualToEvent:(OEHIDEvent *)anObject; // only checks the padNumber
- (BOOL)isKeyEqualToEvent:(OEHIDEvent *)anObject;       // only checks the padNumber and the keycode, ignores the state

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


