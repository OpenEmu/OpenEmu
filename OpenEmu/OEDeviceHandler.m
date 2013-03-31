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

#import "OEDeviceHandler.h"
#import "NSApplication+OEHIDAdditions.h"
#import "OEDeviceDescription.h"
#import "OEHIDEvent.h"
#import "OEHIDDeviceHandler.h"
#import "OEControlDescription.h"

#if __has_feature(objc_bool)
#undef YES
#undef NO
#define YES __objc_yes
#define NO __objc_no
#endif

NSString *const OEDeviceHandlerDidReceiveLowBatteryWarningNotification = @"OEDeviceHandlerDidReceiveLowBatteryWarningNotification";

@interface OEDeviceHandler ()
{
    OEDeviceDescription *_deviceDescription;
    NSMutableDictionary *_deadZones;
}

@property(readwrite) NSUInteger deviceNumber;
@property(readwrite) NSUInteger deviceIdentifier;
@end

@implementation OEDeviceHandler

- (id)init
{
    return nil;
}

- (id)initWithDeviceDescription:(OEDeviceDescription *)deviceDescription
{
    if((self = [super init]))
    {
        _deviceDescription = deviceDescription;
        FIXME("Save default dead zones in user defaults based on device description.");
        _defaultDeadZone = 0.2;
        _deadZones = [[NSMutableDictionary alloc] init];
    }

    return self;
}

- (OEControllerDescription *)controllerDescription
{
    return [[self deviceDescription] controllerDescription];
}

- (void)setUpControllerDescription:(OEControllerDescription *)description usingRepresentation:(NSDictionary *)controlRepresentations
{
    NSAssert(NO, @"Need to implement the method in a subclass.");
}

- (id)copyWithZone:(NSZone *)zone
{
    return self;
}

- (BOOL)isKeyboardDevice;
{
    return NO;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@ %p deviceDescription: '%@' manufacturer: %@ product: %@ serialNumber: %@ deviceIdentifier: %lu deviceNumber: %lu isKeyboard: %@>", [self class], self, [self deviceDescription], [self manufacturer], [self product], [self serialNumber], [self deviceIdentifier], [self deviceNumber], [self isKeyboardDevice] ? @"YES" : @"NO"];
}

- (NSString *)serialNumber;
{
    return nil;
}

- (NSString *)manufacturer;
{
    return nil;
}

- (NSString *)product;
{
    return nil;
}

- (NSUInteger)vendorID;
{
    return [_deviceDescription vendorID];
}

- (NSUInteger)productID;
{
    return [_deviceDescription productID];
}

- (NSNumber *)locationID;
{
    return nil;
}

- (BOOL)connect;
{
    return YES;
}

- (void)disconnect;
{

}

- (CGFloat)deadZoneForControlCookie:(NSUInteger)controlCookie;
{
    NSNumber *deadZone = _deadZones[@(controlCookie)];

    return deadZone != nil ? [deadZone doubleValue] : _defaultDeadZone;
}

- (CGFloat)deadZoneForControlDescription:(OEControlDescription *)controlDesc;
{
    return [self deadZoneForControlCookie:[[controlDesc genericEvent] cookie]];
}

- (void)setDeadZone:(CGFloat)deadZone forControlDescription:(OEControlDescription *)controlDesc;
{
    FIXME("Save dead zones in user defaults based on the serial number.");
    NSAssert(controlDesc != nil, @"Cannot set the dead zone of nil!");
    NSAssert([controlDesc type] == OEHIDEventTypeAxis || [controlDesc type] == OEHIDEventTypeTrigger, @"Only analogic controls have dead zones.");
    _deadZones[@([[controlDesc genericEvent] cookie])] = @(deadZone);
}

@end
