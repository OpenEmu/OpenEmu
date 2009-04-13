//
//  OEHIDEvent.m
//  OpenEmu
//
//  Created by Remy Demarest on 25/02/2009.
//  Copyright 2009 Psycho Inc.. All rights reserved.
//

#import "OEHIDEvent.h"

@interface NSObject (MakeTheCompilerHappy)
- (IOHIDDeviceRef)deviceWithManufacturer:(NSString *)aManufacturer productID:(NSNumber *)aProductID locationID:(NSNumber *)aLocationID;
@end


@implementation OEHIDEvent

@synthesize device, buttonNumber, axis, value, padNumber;

- (NSString *)axisName
{
    NSString *ret = @"OEHIDEventAxisNone";
    switch (axis) {
        case OEHIDEventAxisX :
            ret = @"OEHIDEventAxisX";
            break;
        case OEHIDEventAxisY :
            ret = @"OEHIDEventAxisY";
            break;
        case OEHIDEventAxisZ :
            ret = @"OEHIDEventAxisZ";
            break;
        case OEHIDEventAxisRx :
            ret = @"OEHIDEventAxisRx";
            break;
        case OEHIDEventAxisRy :
            ret = @"OEHIDEventAxisRy";
            break;
        case OEHIDEventAxisRz :
            ret = @"OEHIDEventAxisRz";
            break;
        case OEHIDEventAxisNone :
        default:
            break;
    }
    return ret;
}

- (NSString *)displayDescription
{
    NSString *ret = nil;
    switch (axis)
    {
        case OEHIDEventAxisNone :
            ret = [NSString stringWithFormat:NSLocalizedString(@"Btn %d", @"HID Button Number"), buttonNumber];
            break;
        case OEHIDEventAxisX :
            ret = NSLocalizedString(@"X Axis", @"Joystick X Axis");
            break;
        case OEHIDEventAxisY :
            ret = NSLocalizedString(@"Y Axis", @"Joystick Y Axis");
            break;
        case OEHIDEventAxisZ :
            ret = NSLocalizedString(@"Z Axis", @"Joystick Z Axis");
            break;
        case OEHIDEventAxisRx :
            ret = NSLocalizedString(@"Rx Axis", @"Joystick Rx Axis");
            break;
        case OEHIDEventAxisRy :
            ret = NSLocalizedString(@"Ry Axis", @"Joystick Ry Axis");
            break;
        case OEHIDEventAxisRz :
            ret = NSLocalizedString(@"Rz Axis", @"Joystick Rz Axis");
            break;
        default:
            break;
    }
    
    if(ret != nil)
        ret = [NSString stringWithFormat:NSLocalizedString(@"Pad %d %@", @"HID Event pad"), padNumber, ret];
    
    return ret;
}

- (id)init
{
    [self release];
    return nil;
}

+ (id)eventWithDevice:(IOHIDDeviceRef)aDevice deviceNumber:(NSUInteger)aNumber page:(uint32_t)aPage usage:(uint32_t)aUsage value:(CGFloat)aValue
{
    return [[[self alloc] initWithDevice:aDevice deviceNumber:aNumber page:aPage usage:aUsage value:aValue] autorelease];
}

- (id)initWithDevice:(IOHIDDeviceRef)aDevice deviceNumber:(NSUInteger)aNumber page:(uint32_t)aPage usage:(uint32_t)aUsage value:(CGFloat)aValue
{
    self = [super init];
    if(self != nil)
    {
        device = aDevice;
        padNumber = aNumber;
        value = aValue;
        
        if(aPage == 0x01)      axis = aUsage;
        else if(aPage == 0x09) buttonNumber = aUsage;
    }
    return self;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"%@ { device: %p, axis: %@, buttonNumber: %u, value: %f }",
            [super description], device, [self axisName], buttonNumber, value];
}

- (NSString *)manufacturer
{
    return (NSString *)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDManufacturerKey));
}
- (NSString *)product
{
    return (NSString *)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductKey));
}
- (NSNumber *)productID
{
    return (NSNumber *)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductIDKey));
}
- (NSNumber *)locationID
{
    return (NSNumber *)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDLocationIDKey));
}

NSString *EOHIDEventDeviceManufacturerKey = @"EOHIDEventDeviceManufacturerKey";
NSString *EOHIDEventDeviceProductIDKey    = @"EOHIDEventDeviceProductIDKey";
NSString *EOHIDEventDeviceLocationIDKey   = @"EOHIDEventDeviceLocationIDKey";
NSString *EOHIDEventAxisKey               = @"EOHIDEventAxisKey";
NSString *EOHIDEventButtonNumberKey       = @"EOHIDEventButtonNumberKey";

- (id)initWithCoder:(NSCoder *)decoder
{
    axis         = [decoder decodeIntegerForKey:EOHIDEventAxisKey];
    buttonNumber = [decoder decodeIntegerForKey:EOHIDEventButtonNumberKey];
    id gdc       = [objc_getClass("GameDocumentController") sharedDocumentController];
    device       = [gdc deviceWithManufacturer:[decoder decodeObjectForKey:EOHIDEventDeviceManufacturerKey]
                                     productID:[decoder decodeObjectForKey:EOHIDEventDeviceProductIDKey]
                                    locationID:[decoder decodeObjectForKey:EOHIDEventDeviceLocationIDKey]];
    
    return self;
}

- (void)encodeWithCoder:(NSCoder *)encoder
{
    [encoder encodeObject:(id)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDManufacturerKey))
                   forKey:EOHIDEventDeviceManufacturerKey];
    [encoder encodeObject:(id)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductIDKey))
                   forKey:EOHIDEventDeviceProductIDKey];
    [encoder encodeObject:(id)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDLocationIDKey))
                   forKey:EOHIDEventDeviceLocationIDKey];
    [encoder encodeInteger:axis         forKey:EOHIDEventAxisKey];
    [encoder encodeInteger:buttonNumber forKey:EOHIDEventButtonNumberKey];
}

- (NSUInteger)hash
{
    return ([[self manufacturer] hash] ^ [[self productID] hash] ^ [self axis] ^ [self buttonNumber]);
}

- (BOOL)isEqual:(id)anObject
{
    if([anObject isKindOfClass:[OEHIDEvent class]])
        return ([[self manufacturer] isEqualToString:[anObject manufacturer]] &&
                [[self productID]    isEqualToNumber:[anObject productID]]    &&
                [self axis] == [anObject axis] && [self buttonNumber] == [anObject buttonNumber]);
    
    return [super isEqual:anObject];
}

@end

@implementation NSEvent (OEEventConversion)
+ (NSEvent *)eventWithKeyCode:(unsigned short)keyCode
{
    return [self eventWithKeyCode:keyCode keyIsDown:YES];
}
+ (NSEvent *)eventWithKeyCode:(unsigned short)keyCode keyIsDown:(BOOL)_keyDown
{
    CGEventRef event = CGEventCreateKeyboardEvent(NULL, (CGKeyCode)keyCode, _keyDown);
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
- (NSString *)displayDescription
{
    NSString *characters = [self characters];
    
    if([characters length] == 0) return characters;
    
    NSString *ret = nil;
    
    unichar tested = [characters characterAtIndex:0];
    
    if(NSF1FunctionKey <= tested && tested <= NSF35FunctionKey)
        ret = [NSString stringWithFormat:@"F%u", tested - NSF1FunctionKey];
    else if([[NSCharacterSet whitespaceCharacterSet] characterIsMember:tested])
        ret = @"<space>";
    else
    {
        unichar curr = 0;
        if(0);
#define LAZY_CASE(c, val) else if(tested == c) curr = val
        LAZY_CASE(NSLeftArrowFunctionKey,  0x2190); // '←'
        LAZY_CASE(NSUpArrowFunctionKey,    0x2191); // '↑'
        LAZY_CASE(NSRightArrowFunctionKey, 0x2192); // '→'
        LAZY_CASE(NSDownArrowFunctionKey,  0x2193); // '↓'
        LAZY_CASE(NSDeleteFunctionKey,     0x232B); // '⌫'
        LAZY_CASE(NSHomeFunctionKey,       0x2196); // '↖'
        LAZY_CASE(NSEndFunctionKey,        0x2198); // '↘'
        LAZY_CASE(NSPageUpFunctionKey,     0x21DE); // '⇞'
        LAZY_CASE(NSPageDownFunctionKey,   0x21DF); // '⇟'
        LAZY_CASE(NSClearLineFunctionKey,  0x2327); // '⌧'
#undef  LAZY_CASE
        
        if(curr != 0) ret = [NSString stringWithCharacters:&curr length:1];
        else          ret = characters;
    }
    
    return ret;
}
@end
