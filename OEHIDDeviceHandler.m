//
//  OEHIDManager.m
//  OpenEmu
//
//  Created by Remy Demarest on 25/02/2009.
//  Copyright 2009 Psycho Inc.. All rights reserved.
//

#import "OEHIDDeviceHandler.h"
#import "OEHIDEvent.h"

@implementation OEHIDDeviceHandler

@synthesize device, deadZone;

+ (id)deviceHandlerWithDevice:(IOHIDDeviceRef)aDevice
{
    return [[[self alloc] initWithDevice:aDevice] autorelease];
}

- (id)init
{
    return [self initWithDevice:NULL];
}

- (id)initWithDevice:(IOHIDDeviceRef)aDevice
{
    self = [super init];
    if(self != nil)
    {
        device = aDevice;
        deadZone = 0.2;
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

- (OEHIDEvent *)eventWithPage:(uint32_t)aPage usage:(uint32_t)aUsage value:(CGFloat)aValue
{
    return [OEHIDEvent eventWithDevice:device page:aPage usage:aUsage value:aValue];
}

- (void)dispatchEventWithPage:(uint32_t)aPage usage:(uint32_t)aUsage value:(CGFloat)aValue
{
    [NSApp sendAction:@selector(handleHIDEvent:) to:nil
                 from:[self eventWithPage:aPage usage:aUsage value:aValue]];
}

@end
