//
//  OEHIDManager.m
//  OpenEmu
//
//  Created by Remy Demarest on 25/02/2009.
//  Copyright 2009 Psycho Inc.. All rights reserved.
//

#import "OEHIDDeviceHandler.h"
#import "NSApplication+OEHIDAdditions.h"
#import "OEHIDEvent.h"

@implementation OEHIDDeviceHandler

@synthesize device, deviceNumber, deadZone;

+ (id)deviceHandlerWithDevice:(IOHIDDeviceRef)aDevice
{
    return [[[self alloc] initWithDevice:aDevice] autorelease];
}

- (id)init
{
    return [self initWithDevice:NULL];
}

static OEHIDDeviceHandler *nilHandler = nil;
static NSUInteger lastDeviceNumber = 0;
 
- (id)initWithDevice:(IOHIDDeviceRef)aDevice
{
    self = [super init];
    if(self != nil)
    {
        if(aDevice == NULL)
        {
            if(nilHandler == nil)
            {
                device = NULL;
                deviceNumber = 0;
                deadZone = 0.0;
                nilHandler = [self retain];
            }
            else
            {
                [self release];
                self = [nilHandler retain];
            }
        }
        else
        {
            deviceNumber = ++lastDeviceNumber;
            device = aDevice;
            deadZone = 0.2;
        }
    }
    return self;
}

- (BOOL)isEqual:(id)anObject
{
    if(self == anObject)
        return YES;
    if([anObject isKindOfClass:[self class]])
        return [(id)device isEqual:(id)[anObject device]];
    return [super isEqual:anObject];
}

- (NSUInteger)hash
{
    return [(id)device hash];
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

- (OEHIDEvent *)eventWithHIDValue:(IOHIDValueRef)aValue
{
    return [OEHIDEvent eventWithDeviceHandler:self value:aValue];
}

- (void)dispatchEventWithHIDValue:(IOHIDValueRef)aValue
{
    [NSApp postHIDEvent:[self eventWithHIDValue:aValue]];
    //[NSApp sendAction:@selector(handleHIDEvent:) to:nil from:[self eventWithHIDValue:aValue]];
}

@end
