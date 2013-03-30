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

#if __has_feature(objc_bool)
#undef YES
#undef NO
#define YES __objc_yes
#define NO __objc_no
#endif

NSString *const OEDeviceHandlerDidReceiveLowBatteryWarningNotification = @"OEDeviceHandlerDidReceiveLowBatteryWarningNotification";

@interface OEHIDEvent ()
- (BOOL)OE_setupEventWithDeviceHandler:(OEHIDDeviceHandler *)aDeviceHandler value:(IOHIDValueRef)aValue;
@end

@interface OEDeviceHandler ()
{
    OEDeviceDescription *_deviceDescription;
}

@property(readwrite) NSUInteger deviceNumber;
@property(readwrite) NSUInteger deviceIdentifier;
@end

@implementation OEDeviceHandler

+ (instancetype)deviceHandlerWithIOHIDDevice:(IOHIDDeviceRef)aDevice;
{
    return [OEHIDDeviceHandler deviceHandlerWithIOHIDDevice:aDevice];
}

- (OEControllerDescription *)controllerDescription
{
    return [[self deviceDescription] controllerDescription];
}

- (OEDeviceDescription *)deviceDescription
{
    if(_deviceDescription == nil)
        _deviceDescription = [OEDeviceDescription deviceDescriptionForDeviceHandler:self];

    return _deviceDescription;
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
    return 0;
}

- (NSUInteger)productID;
{
    return 0;
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

@end
