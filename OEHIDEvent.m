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

@synthesize device, buttonNumber, axis, value;

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

- (id)init
{
    [self release];
    return nil;
}

+ (id)eventWithDevice:(IOHIDDeviceRef)aDevice page:(uint32_t)aPage usage:(uint32_t)aUsage value:(CGFloat)aValue
{
    return [[[self alloc] initWithDevice:aDevice page:aPage usage:aUsage value:aValue] autorelease];
}

- (id)initWithDevice:(IOHIDDeviceRef)aDevice page:(uint32_t)aPage usage:(uint32_t)aUsage value:(CGFloat)aValue
{
    self = [super init];
    if(self != nil)
    {
        device = aDevice;
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
+ (NSUInteger)modifierFlagsForKeyCode:(unsigned short)keyCode
{
    return [[self eventWithKeyCode:keyCode] modifierFlags];
}
@end
