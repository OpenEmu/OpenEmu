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
#import "OEHIDEvent.h"
#import "OEHIDDeviceHandler.h"
#import "OEWiimoteDeviceHandler.h"

#if __has_feature(objc_bool)
#undef YES
#undef NO
#define YES __objc_yes
#define NO __objc_no
#endif

@interface _OEHIDDeviceIdentifier : NSObject <NSCopying>

- (id)initWithVendorID:(NSUInteger)vendorID deviceID:(NSUInteger)deviceID;
- (id)initWithDescription:(NSString *)description;

@property(readonly) NSUInteger vendorID;
@property(readonly) NSUInteger deviceID;

@end

@interface OEHIDEvent ()
- (BOOL)OE_setupEventWithDeviceHandler:(OEHIDDeviceHandler *)aDeviceHandler value:(IOHIDValueRef)aValue;
@end

@interface OEDeviceHandler ()
@property(readwrite) NSUInteger deviceNumber;
- (void)OE_setupDeviceIdentifier;
@end

@implementation OEDeviceHandler

static NSDictionary *deviceToTypes = nil;

+ (void)initialize
{
    if(self == [OEDeviceHandler class])
    {
        NSString *identifierPath = [[NSBundle mainBundle] pathForResource:@"Controller-Database" ofType:@"plist"];
        NSArray *controllers = [NSPropertyListSerialization propertyListWithData:[NSData dataWithContentsOfFile:identifierPath options:NSDataReadingMappedIfSafe error:NULL] options:0 format:NULL error:NULL];

        NSMutableDictionary *result = [NSMutableDictionary dictionaryWithCapacity:[controllers count]];

        [controllers enumerateObjectsUsingBlock:
         ^(NSDictionary *obj, NSUInteger idx, BOOL *stop)
         {
             NSString *genericName = [obj objectForKey:@"OEGenericControllerName"];

             for(NSDictionary *device in [obj objectForKey:@"OEControllerDevices"])
             {
                 NSUInteger vendorID  = [[device objectForKey:@"OEControllerVendorID"] unsignedIntegerValue];
                 NSUInteger productID = [[device objectForKey:@"OEControllerProductID"] unsignedIntegerValue];
                 _OEHIDDeviceIdentifier *ident = [[_OEHIDDeviceIdentifier alloc] initWithVendorID:vendorID deviceID:productID];

                 [result setObject:genericName forKey:ident];
             }
         }];

        deviceToTypes = [result copy];
    }
}

+ (NSString *)standardDeviceIdentifierForDeviceIdentifier:(NSString *)aString
{
    if([aString hasPrefix:@"#"])
    {
        _OEHIDDeviceIdentifier *ident = [[_OEHIDDeviceIdentifier alloc] initWithDescription:aString];

        return [deviceToTypes objectForKey:ident] ? : ident != nil ? aString : nil;
    }

    return [[deviceToTypes allKeysForObject:aString] count] > 0 ? aString : nil;
}

+ (instancetype)deviceHandlerWithIOHIDDevice:(IOHIDDeviceRef)aDevice;
{
    return [OEHIDDeviceHandler deviceHandlerWithIOHIDDevice:aDevice];
}

+ (instancetype)deviceHandlerWithIOBluetoothDevice:(IOBluetoothDevice *)aDevice;
{
    return [OEWiimoteDeviceHandler deviceHandlerWithIOBluetoothDevice:aDevice];
}

- (NSString *)OE_deviceIdentifier;
{
    _OEHIDDeviceIdentifier *ident = [[_OEHIDDeviceIdentifier alloc] initWithVendorID:[[self vendorID] unsignedIntegerValue] deviceID:[[self productID] unsignedIntegerValue]];

    return [deviceToTypes objectForKey:ident] ? : [ident description];
}

- (void)OE_setupDeviceIdentifier;
{
    _deviceIdentifier = [self OE_deviceIdentifier];
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
    return [NSString stringWithFormat:@"<%@ %p deviceIdentifier: '%@' manufacturer: %@ product: %@ serialNumber: %@ deviceNumber: %lu isKeyboard: %@>", [self class], self, [self deviceIdentifier], [self manufacturer], [self product], [self serialNumber], [self deviceNumber], [self isKeyboardDevice] ? @"YES" : @"NO"];
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

- (NSNumber *)vendorID;
{
    return nil;
}

- (NSNumber *)productID;
{
    return nil;
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

@implementation _OEHIDDeviceIdentifier

static NSMutableSet *allDeviceIdentifiers = nil;

+ (void)initialize
{
    if(self == [_OEHIDDeviceIdentifier class]) return;

    allDeviceIdentifiers = [[NSMutableSet alloc] init];
}

- (id)init { return [self initWithVendorID:0 deviceID:0]; }

- (id)initWithVendorID:(NSUInteger)vendorID deviceID:(NSUInteger)deviceID
{
    if((self = [super init]))
    {
        _vendorID = vendorID;
        _deviceID = deviceID;

        _OEHIDDeviceIdentifier *ret = [allDeviceIdentifiers member:self];

        if(ret == nil) [allDeviceIdentifiers addObject:self];
        else self = ret;
    }

    return self;
}

- (id)initWithDescription:(NSString *)description;
{
    NSScanner *scanner = [NSScanner scannerWithString:description];

    NSUInteger vendorID = 0, deviceID = 0;

    return (([scanner scanString:@"#OEHIDDeviceIdentifier:" intoString:NULL] &&
             [scanner scanHexLongLong:(unsigned long long *)&vendorID]       &&
             [scanner scanHexLongLong:(unsigned long long *)&deviceID])
            ? [self initWithVendorID:vendorID deviceID:deviceID] : nil);
}

- (id)copyWithZone:(NSZone *)zone
{
    return self;
}

- (NSUInteger)hash
{
    return _vendorID ^ _deviceID;
}

- (BOOL)isEqual:(_OEHIDDeviceIdentifier *)object
{
    if(self == object)
        return YES;
    else if([object isKindOfClass:[_OEHIDDeviceIdentifier class]])
        return _vendorID == [object vendorID] && _deviceID == [object deviceID];

    return NO;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"#OEHIDDeviceIdentifier: %#lX %#lX", _vendorID, _deviceID];
}

@end
