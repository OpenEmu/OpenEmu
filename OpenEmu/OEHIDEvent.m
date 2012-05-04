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

#import "OEHIDEvent.h"
#import "OEHIDDeviceHandler.h"
#import <IOKit/hid/IOHIDUsageTables.h>
#import <Carbon/Carbon.h>


#import "OEHIDUsageToVK.h"

@interface OEHIDEvent ()
- (id)initWithDeviceHandler:(OEHIDDeviceHandler *)aDeviceHandler value:(IOHIDValueRef)aValue;
- (id)initWithPadNumber:(NSUInteger)padNumber timestamp:(NSTimeInterval)timestamp;

- (BOOL)OE_setupEventWithDeviceHandler:(OEHIDDeviceHandler *)aDeviceHandler value:(IOHIDValueRef)aValue;
@end

@implementation OEHIDEvent
@synthesize padNumber = _padNumber, type = _type, cookie = _cookie, hasPreviousState = _hasPreviousState, timestamp = _timestamp, previousTimestamp = _previousTimestamp;

+ (NSUInteger)keyCodeForVK:(CGCharCode)charCode
{
	for(int i = 0; i < sizeof(hidvk_codes) / sizeof(*hidvk_codes); ++i)
		if(hidvk_codes[i].vkCode == charCode)
			return hidvk_codes[i].hidCode;
    
	return 0;
}

+ (NSString *)stringForHIDKeyCode:(NSUInteger)hidCode
{
	CGKeyCode keyCode = 0xFFFF;
	for(int i = 0; i < sizeof(hidvk_codes) / sizeof(*hidvk_codes); ++i)
		if(hidvk_codes[i].hidCode == hidCode)
		{
			keyCode = hidvk_codes[i].vkCode;
			break;
		}
	
	for(int i = 0; i < sizeof(hidlabels) / sizeof(*hidlabels); ++i)
		if(hidlabels[i].hidCode == hidCode)
			 return hidlabels[i].string;
		
	TISInputSourceRef currentKeyboard = TISCopyCurrentKeyboardInputSource();
	CFDataRef uchr = (CFDataRef)TISGetInputSourceProperty(currentKeyboard, kTISPropertyUnicodeKeyLayoutData);
	CFRelease(currentKeyboard);
	
    // For non-unicode layouts such as Chinese, Japanese, and Korean, get the ASCII capable layout
    if(!uchr)
    {
        currentKeyboard = TISCopyCurrentASCIICapableKeyboardLayoutInputSource();
        uchr = (CFDataRef)TISGetInputSourceProperty(currentKeyboard, kTISPropertyUnicodeKeyLayoutData);
        CFRelease(currentKeyboard);
    }
    
    const UCKeyboardLayout *keyboardLayout = (const UCKeyboardLayout*)CFDataGetBytePtr(uchr);
    
	if(keyboardLayout)
    {
		UInt32 deadKeyState = 0;
		UniCharCount maxStringLength = 255;
		UniCharCount actualStringLength = 0;
		UniChar unicodeString[maxStringLength];
		
		OSStatus status = UCKeyTranslate(keyboardLayout,
										 keyCode, kUCKeyActionDisplay, 0,
										 LMGetKbdType(), 0,
										 &deadKeyState,
										 maxStringLength,
										 &actualStringLength, unicodeString);
		
        
		if(actualStringLength > 0 && status == noErr)
        {
            NSString *lowercaseString = [NSString stringWithCharacters:unicodeString length:(NSUInteger)actualStringLength];
            if (![lowercaseString isEqualToString:[[lowercaseString uppercaseString] lowercaseString]])
                return lowercaseString;
            return [lowercaseString uppercaseString];
        }
	}
    
	return nil;
}

- (NSString *)displayDescription
{
    NSString *ret = nil;
    switch(_type)
    {
        case OEHIDAxis :
            // Example: ret = @"P1 -X" for Pad One X axis Negative
            switch(_data.axis.axis)
            {
                case OEHIDAxisX  : ret = @"X";  break;
                case OEHIDAxisY  : ret = @"Y";  break;
                case OEHIDAxisZ  : ret = @"Z";  break;
                case OEHIDAxisRx : ret = @"Rx"; break;
                case OEHIDAxisRy : ret = @"Ry"; break;
                case OEHIDAxisRz : ret = @"Rz"; break;
            }
            char sign = (_data.axis.direction == OEHIDDirectionNull ? '=' :
                         (_data.axis.direction == OEHIDDirectionPositive ? '-' : '+'));
            
            ret = [NSString stringWithFormat:@" %@%c", ret, sign];
            break;
        case OEHIDButton :
            // Example: ret = @"P1 B12" for Pad One Button 12
            ret = [NSString stringWithFormat:@" B%ld", _data.button.buttonNumber];
            break;
        case OEHIDHatSwitch :
            // Example: ret = @"P1 H5/8" for Pad One Hat Switch Position 5 of 8
            ret = [NSString stringWithFormat:@" H%ld/%ld", _data.hatSwitch.position, _data.hatSwitch.count];
            break;
		case OEHIDKeypress :
			return [NSString stringWithFormat:@"%@", [OEHIDEvent stringForHIDKeyCode:_data.key.keycode]];
    }
    
    if(ret != nil) ret = [NSString stringWithFormat:@"P%ld%@", _padNumber, ret];
    else ret = @"";
    
    return ret;
}

- (id)init
{
    return nil;
}

- (id)initWithPadNumber:(NSUInteger)padNumber timestamp:(NSTimeInterval)timestamp
{
    self = [super init];
    if(self != nil)
    {
        _padNumber = padNumber;
        _timestamp = timestamp;
    }
    return self;
}

+ (id)axisEventWithPadNumber:(NSUInteger)padNumber timestamp:(NSTimeInterval)timestamp axis:(OEHIDEventAxis)axis scaledValue:(CGFloat)value
{
    OEHIDEvent *ret = [[self alloc] initWithPadNumber:padNumber timestamp:timestamp];
    ret->_type = OEHIDAxis;
    ret->_data.axis.axis = axis;
    
    if(value < 0.0)      ret->_data.axis.direction = OEHIDDirectionNegative;
    else if(value > 0.0) ret->_data.axis.direction = OEHIDDirectionPositive;
    else                 ret->_data.axis.direction = OEHIDDirectionNull;
    
    ret->_data.axis.minimum = -INT_MAX;
    ret->_data.axis.value   = (NSInteger)(value * INT_MAX);
    ret->_data.axis.maximum = INT_MAX;
    
    return ret;
}

+ (id)buttonEventWithPadNumber:(NSUInteger)padNumber timestamp:(NSTimeInterval)timestamp buttonNumber:(NSUInteger)number state:(NSUInteger)state cookie:(NSUInteger)cookie
{
    OEHIDEvent *ret = [[self alloc] initWithPadNumber:padNumber timestamp:timestamp];
    
    ret->_type = OEHIDButton;
    ret->_cookie = cookie;
    ret->_data.button.buttonNumber = number;
    ret->_data.button.state = state;
    
    return ret;
}

+ (id)hatSwitchEventWithPadNumber:(NSUInteger)padNumber timestamp:(NSTimeInterval)timestamp position:(NSUInteger)position positionCount:(NSUInteger)count cookie:(NSUInteger)cookie
{
    OEHIDEvent *ret = [[self alloc] initWithPadNumber:padNumber timestamp:timestamp];
    
    ret->_type = OEHIDHatSwitch;
    ret->_cookie = cookie;
    ret->_data.hatSwitch.position = position;
    ret->_data.hatSwitch.count    = count;
    
    return ret;
}

+ (id)keyEventWithTimestamp:(NSTimeInterval)timestamp keyCode:(NSUInteger)keyCode state:(NSUInteger)state cookie:(NSUInteger)cookie
{
    OEHIDEvent *ret = [[self alloc] initWithPadNumber:0 timestamp:timestamp];
    
    ret->_type = OEHIDKeypress;
    ret->_cookie = cookie;
    ret->_data.key.keycode = keyCode;
    ret->_data.key.state = state;
    
    return ret;
}

+ (id)eventWithDeviceHandler:(OEHIDDeviceHandler *)aDeviceHandler value:(IOHIDValueRef)aValue
{
    return [[self alloc] initWithDeviceHandler:aDeviceHandler value:aValue];
}

- (id)initWithDeviceHandler:(OEHIDDeviceHandler *)aDeviceHandler value:(IOHIDValueRef)aValue
{
    self = [self initWithPadNumber:[aDeviceHandler deviceNumber] timestamp:IOHIDValueGetTimeStamp(aValue) / 1e9];
    if(self != nil)
    {
        if(![self OE_setupEventWithDeviceHandler:aDeviceHandler value:aValue])
        {
            return nil;
        }
    }
    
    return self;
}

- (BOOL)OE_setupEventWithDeviceHandler:(OEHIDDeviceHandler *)aDeviceHandler value:(IOHIDValueRef)aValue;
{
    IOHIDElementRef elem   = IOHIDValueGetElement(aValue);
    const uint32_t  page   = IOHIDElementGetUsagePage(elem);
    const uint32_t  usage  = IOHIDElementGetUsage(elem);
    NSUInteger      cookie = (uint32_t)IOHIDElementGetCookie(elem);
    
    _hasPreviousState = _type != 0;
    
    if(!_hasPreviousState) _cookie = cookie;
    else if(_cookie != cookie) return NO;
    
    if(IOHIDValueGetLength(aValue) == 0 ||
       IOHIDValueGetLength(aValue) >  8)
        return NO;
    
    _previousTimestamp   = _timestamp;
    _timestamp           = IOHIDValueGetTimeStamp(aValue) / 1e9;
    
    CFIndex value        = IOHIDValueGetIntegerValue(aValue);
    
    switch(page)
    {
        case kHIDPage_GenericDesktop :
        {
            switch(usage)
            {
                case kHIDUsage_GD_X  :
                case kHIDUsage_GD_Y  :
                case kHIDUsage_GD_Z  :
                case kHIDUsage_GD_Rx :
                case kHIDUsage_GD_Ry :
                case kHIDUsage_GD_Rz :
                    _type = OEHIDAxis;
                    _data.axis.axis = usage;
                    
                    if(_hasPreviousState)
                    {
                        _data.axis.previousValue     = _data.axis.value;
                        _data.axis.previousDirection = _data.axis.direction;
                    }
                    
                    _data.axis.minimum = IOHIDElementGetLogicalMin(elem);
                    _data.axis.value   = value;
                    _data.axis.maximum = IOHIDElementGetLogicalMax(elem);
                    
                    NSInteger zero = (_data.axis.maximum + _data.axis.minimum) / 2 + 1;
                    
                    if(_data.axis.minimum >= 0)
                        _data.axis.minimum -= zero,
                        _data.axis.value   -= zero,
                        _data.axis.maximum -= zero;
                    
                    NSInteger deadZone = (NSInteger)ceil(_data.axis.maximum * [aDeviceHandler deadZone]);
                    
                    if(-deadZone <= _data.axis.value && _data.axis.value <= deadZone)
                        _data.axis.value = 0;
                    
                    if(_data.axis.value > 0)      _data.axis.direction = OEHIDDirectionPositive;
                    else if(_data.axis.value < 0) _data.axis.direction = OEHIDDirectionNegative;
                    else                          _data.axis.direction = OEHIDDirectionNull;
                    break;
                case kHIDUsage_GD_Hatswitch :
                    _type = OEHIDHatSwitch;
                    
                    if(_hasPreviousState) _data.hatSwitch.previousPosition = _data.hatSwitch.position;
                    
                    _data.hatSwitch.position = value;
                    _data.hatSwitch.count    = IOHIDElementGetLogicalMax(elem);
                    break;
            }
            break;
        }
        case kHIDPage_Button :
            _type = OEHIDButton;
            
            if(_hasPreviousState) _data.button.previousState = _data.button.state;
            
            _data.button.buttonNumber = usage;
            _data.button.state = value;
            break;
        case kHIDPage_KeyboardOrKeypad :
            if(!(((usage >= 0x04) && (usage <= 0xA4)) ||
                 ((usage >= 0xE0) && (usage <= 0xE7))))
                return NO;
            
            _type = OEHIDKeypress;
            
            if(_hasPreviousState) _data.key.previousState = _data.key.state;
            
            _data.key.keycode = usage;
            _data.key.state = value;
            break;
    }
    
    return YES;
}

- (NSTimeInterval)elapsedTime;
{
    return [self timestamp] - [self previousTimestamp];
}

- (OEHIDEventAxis)axis
{
    NSAssert1([self type] == OEHIDAxis, @"Invalid message sent to event \"%@\"", self);
    return _data.axis.axis;
}

- (OEHIDDirection)previousDirection
{
    NSAssert1([self type] == OEHIDAxis, @"Invalid message sent to event \"%@\"", self);
    return _data.axis.previousDirection;
}

- (OEHIDDirection)direction
{
    NSAssert1([self type] == OEHIDAxis, @"Invalid message sent to event \"%@\"", self);
    return _data.axis.direction;
}

- (NSInteger)minimum
{
    NSAssert1([self type] == OEHIDAxis, @"Invalid message sent to event \"%@\"", self);
    return _data.axis.minimum;
}

- (NSInteger)previousValue
{
    NSAssert1([self type] == OEHIDAxis, @"Invalid message sent to event \"%@\"", self);
    return _data.axis.previousValue;
}

- (NSInteger)value
{
    NSAssert1([self type] == OEHIDAxis, @"Invalid message sent to event \"%@\"", self);
    return _data.axis.value;
}

- (NSInteger)maximum
{
    NSAssert1([self type] == OEHIDAxis, @"Invalid message sent to event \"%@\"", self);
    return _data.axis.maximum;
}

// Button event
- (NSUInteger)buttonNumber
{
    NSAssert1([self type] == OEHIDButton, @"Invalid message sent to event \"%@\"", self);
    return _data.button.buttonNumber;
}

- (NSInteger)previousState
{
    NSAssert1([self type] == OEHIDButton || [self type] == OEHIDKeypress, @"Invalid message sent to event \"%@\"", self);
    return ([self type] == OEHIDButton ? _data.button.previousState : _data.key.previousState);
}

- (NSInteger)state
{
    NSAssert1([self type] == OEHIDButton || [self type] == OEHIDKeypress, @"Invalid message sent to event \"%@\"", self);
    return ([self type] == OEHIDButton ? _data.button.state : _data.key.state);
}

// HatSwitch event
- (NSUInteger)previousPosition
{
    NSAssert1([self type] == OEHIDHatSwitch, @"Invalid message sent to event \"%@\"", self);
    return _data.hatSwitch.previousPosition;
}

- (NSUInteger)position
{
    NSAssert1([self type] == OEHIDHatSwitch, @"Invalid message sent to event \"%@\"", self);
    return _data.hatSwitch.position;
}

- (NSUInteger)count
{
    NSAssert1([self type] == OEHIDHatSwitch, @"Invalid message sent to event \"%@\"", self);
    return _data.hatSwitch.count;
}

- (NSUInteger)keycode
{
    NSAssert1([self type] == OEHIDKeypress, @"Invalid message sent to event \"%@\"", self);	
	return _data.key.keycode;
}

- (NSUInteger)cookie
{
    return _cookie;
}

- (NSString *)description
{
    NSString *subs = @"UNKNOWN TYPE";
    
#define STATE_STR(state) (state == NSOnState ? "On" : "Off")
    
    if(_type == OEHIDAxis)
    {
        char *ax;
        switch(_data.axis.axis)
        {
            case OEHIDAxisX  : ax = "X";    break;
            case OEHIDAxisY  : ax = "Y";    break;
            case OEHIDAxisZ  : ax = "Z";    break;
            case OEHIDAxisRx : ax = "Rx";   break;
            case OEHIDAxisRy : ax = "Ry";   break;
            case OEHIDAxisRz : ax = "Rz";   break;
            default          : ax = "none"; break;
        }
        
        const char * (^dirStr)(OEHIDDirection dir) =
        ^ const char * (OEHIDDirection dir)
        {
            return (dir == OEHIDDirectionNegative ? "Neg" : (dir == OEHIDDirectionPositive ? "Pos" : "Nul"));
        };
        
        subs = [NSString stringWithFormat:@"type=Axis axis=%s direction=%s previousDirection=%s min=%lld max=%lld value=%lld previousValue=%lld",
                ax, dirStr(_data.axis.direction), dirStr(_data.axis.previousDirection),
                (int64_t)_data.axis.minimum, (int64_t)_data.axis.maximum,
                (int64_t)_data.axis.value, (int64_t)_data.axis.previousValue];
    }
    else if(_type == OEHIDButton)
        subs = [NSString stringWithFormat:@"type=Button number=%lld state=%s previousState=%s",
                (int64_t)_data.button.buttonNumber, STATE_STR(_data.button.state), STATE_STR(_data.button.previousState)];
    else if(_type == OEHIDHatSwitch)
        subs = [NSString stringWithFormat:@"type=HatSwitch position=%lld/%lld previousPosition=%lld/%lld",
                (int64_t)_data.hatSwitch.position, (int64_t)_data.hatSwitch.count,
                (int64_t)_data.hatSwitch.previousPosition, (int64_t)_data.hatSwitch.count];
    else if(_type == OEHIDKeypress)
        subs = [NSString stringWithFormat:@"type=Key number=%lld state=%s previousState=%s",
                (int64_t)_data.key.keycode, STATE_STR(_data.key.state), STATE_STR(_data.key.previousState)];
    
#undef STATE_STR
    
    return [NSString stringWithFormat:@"HID Event: pad=%lld %@ %@ cookie=%lu", (int64_t)_padNumber, subs, [self displayDescription], _cookie];
}

NSString *OEHIDEventTypeKey         = @"OEHIDEventTypeKey";
NSString *OEHIDEventPadNumberKey    = @"OEHIDEventPadNumberKey";
NSString *OEHIDEventAxisKey         = @"OEHIDEventAxisKey";
NSString *OEHIDEventDirectionKey    = @"OEHIDEventDirectionKey";
NSString *OEHIDEventButtonNumberKey = @"OEHIDEventButtonNumberKey";
NSString *OEHIDEventStateKey        = @"OEHIDEventStateKey";
NSString *OEHIDEventPositionKey     = @"OEHIDEventPositionKey";
NSString *OEHIDEventCountKey        = @"OEHIDEventCountKey";
NSString *OEHIDEventKeycodeKey      = @"OEHIDEventKeycodeKey";
NSString *OEHIDEventCookieKey       = @"OEHIDEventCookieKey";

- (id)initWithCoder:(NSCoder *)decoder
{
    if((self = [super init]))
    {
        _type      = [decoder decodeIntegerForKey:OEHIDEventTypeKey];
        _padNumber = [decoder decodeIntegerForKey:OEHIDEventPadNumberKey];
        _cookie    = [decoder decodeIntegerForKey:OEHIDEventCookieKey];
        
        switch([self type])
        {
            case OEHIDAxis :
                _data.axis.axis           = [decoder decodeIntegerForKey:OEHIDEventAxisKey];
                _data.axis.direction      = [decoder decodeIntegerForKey:OEHIDEventDirectionKey];
                break;
            case OEHIDButton :
                _data.button.buttonNumber = [decoder decodeIntegerForKey:OEHIDEventButtonNumberKey];
                _data.button.state        = [decoder decodeIntegerForKey:OEHIDEventStateKey];
                break;
            case OEHIDHatSwitch :
                _data.hatSwitch.position  = [decoder decodeIntegerForKey:OEHIDEventPositionKey];
                _data.hatSwitch.count     = [decoder decodeIntegerForKey:OEHIDEventCountKey];
                break;
            case OEHIDKeypress :
                _data.key.keycode         = [decoder decodeIntegerForKey:OEHIDEventKeycodeKey];
                _data.key.state           = [decoder decodeIntegerForKey:OEHIDEventStateKey];
                break;
        }
    }
    
    return self;
}

- (void)encodeWithCoder:(NSCoder *)encoder
{
    [encoder encodeInteger:self.type      forKey:OEHIDEventTypeKey];
    [encoder encodeInteger:self.padNumber forKey:OEHIDEventPadNumberKey];
    [encoder encodeInteger:self.cookie    forKey:OEHIDEventCookieKey];
    
    switch([self type])
    {
        case OEHIDAxis :
            [encoder encodeInteger:self.axis         forKey:OEHIDEventAxisKey];
            [encoder encodeInteger:self.direction    forKey:OEHIDEventDirectionKey];
            break;
        case OEHIDButton :
            [encoder encodeInteger:self.buttonNumber forKey:OEHIDEventButtonNumberKey];
            [encoder encodeInteger:self.state        forKey:OEHIDEventStateKey];
            break;
        case OEHIDHatSwitch :
            [encoder encodeInteger:self.position     forKey:OEHIDEventPositionKey];
            [encoder encodeInteger:self.count        forKey:OEHIDEventCountKey];
            break;
		case OEHIDKeypress :
			[encoder encodeInteger:self.keycode      forKey:OEHIDEventKeycodeKey];
            [encoder encodeInteger:self.state        forKey:OEHIDEventStateKey];
            break;
    }
}

@end

@implementation NSEvent (OEEventConversion)
+ (NSEvent *)eventWithKeyCode:(unsigned short)keyCode
{
    return [self eventWithKeyCode:keyCode keyIsDown:YES];
}

+ (NSEvent *)eventWithKeyCode:(unsigned short)keyCode keyIsDown:(BOOL)keyDown
{
    CGEventRef event = CGEventCreateKeyboardEvent(NULL, (CGKeyCode)keyCode, keyDown);
    NSEvent *ret = [self eventWithCGEvent:event];
    CFRelease(event);
    return ret;
}

+ (NSString *)charactersForKeyCode:(unsigned short)keyCode
{
    return [[self eventWithKeyCode:keyCode] characters];
}

+ (NSString *)printableCharactersForKeyCode:(unsigned short)keyCode
{
    return [[self eventWithKeyCode:keyCode] displayDescription];
}

+ (NSUInteger)modifierFlagsForKeyCode:(unsigned short)keyCode
{
    return [[self eventWithKeyCode:keyCode] modifierFlags];
}

+ (NSString *)displayDescriptionForKeyCode:(unsigned short)keyCode
{
    return [[self eventWithKeyCode:keyCode] displayDescription];
}

- (NSString *)displayDescription
{
    NSString *characters = [self characters];
    
    if([characters length] == 0) return characters;
    
    NSString *ret = nil;
    
    unichar tested = [characters characterAtIndex:0];
    
    if(NSF1FunctionKey <= tested && tested <= NSF35FunctionKey)
        ret = [NSString stringWithFormat:@"F%u", tested - NSF1FunctionKey + 1];
    else if(tested == ' ' || tested == 0xA0)//[[NSCharacterSet whitespaceCharacterSet] characterIsMember:tested])
        ret = @"<space>";
    else
    {
        unichar curr = 0;
        if(NO);
#define LAZY_CASE(c, val) else if(tested == c) curr = val
        LAZY_CASE(NSLeftArrowFunctionKey,  0x21E0); // '⇠'
        LAZY_CASE(NSUpArrowFunctionKey,    0x21E1); // '⇡'
        LAZY_CASE(NSRightArrowFunctionKey, 0x21E2); // '⇢'
        LAZY_CASE(NSDownArrowFunctionKey,  0x21E3); // '⇣'
        LAZY_CASE(NSDeleteFunctionKey,     0x2326); // '⌦'
        LAZY_CASE(NSHomeFunctionKey,       0x2196); // '↖'
        LAZY_CASE(NSEndFunctionKey,        0x2198); // '↘'
        LAZY_CASE(NSPageUpFunctionKey,     0x21DE); // '⇞'
        LAZY_CASE(NSPageDownFunctionKey,   0x21DF); // '⇟'
        LAZY_CASE(NSClearLineFunctionKey,  0x2327); // '⌧'
        LAZY_CASE('\r',                    0x21A9); // '↩'
        LAZY_CASE(0x03,                    0x2305); // '⌅'
        LAZY_CASE('\n',                    0x2305); // '⌅'
        LAZY_CASE('\t',                    0x21E5); // '⇥'
        LAZY_CASE(0x19,                    0x21E4); // '⇤'
        LAZY_CASE(0x7F,                    0x232B); // '⌫'
        LAZY_CASE(0x1B,                    0x238B); // '⎋'
#undef  LAZY_CASE
        
        if(curr != 0) ret = [NSString stringWithCharacters:&curr length:1];
        else          ret = characters;
    }
    
    return ret;
}

@end

@implementation NSNumber (OEEventConversion)
- (NSString *)displayDescription
{
    return [NSEvent displayDescriptionForKeyCode:[self unsignedShortValue]];
}
@end
