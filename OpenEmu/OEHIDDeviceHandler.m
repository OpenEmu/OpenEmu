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

#import "OEHIDDeviceHandler.h"
#import "NSApplication+OEHIDAdditions.h"
#import "OEHIDEvent.h"

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

@interface OEHIDDeviceHandler ()
{
    NSMapTable *mapTable;
	
	//force feedback support
	FFDeviceObjectReference  ffDevice;
}

@end

@implementation OEHIDDeviceHandler
{
    NSMapTable *mapTable;
	
	//force feedback support
	FFDeviceObjectReference  ffDevice;
}

static NSDictionary *deviceToTypes = nil;

+ (void)initialize
{
    if(self == [OEHIDDeviceHandler class])
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

- (NSString *)OE_deviceIdentifier;
{
    _OEHIDDeviceIdentifier *ident = [[_OEHIDDeviceIdentifier alloc] initWithVendorID:[[self vendorID] unsignedIntegerValue] deviceID:[[self productID] unsignedIntegerValue]];
    
    return [deviceToTypes objectForKey:ident] ? : [ident description];
}

@synthesize device, deviceNumber, deadZone;

+ (id)deviceHandlerWithDevice:(IOHIDDeviceRef)aDevice
{
    return [[self alloc] initWithDevice:aDevice];
}

- (id)init
{
    return [self initWithDevice:NULL];
}

static NSUInteger lastDeviceNumber = 0;
 
- (id)initWithDevice:(IOHIDDeviceRef)aDevice
{
    if((self = [super init]))
    {
        mapTable = [[NSMapTable alloc] initWithKeyOptions:NSPointerFunctionsOpaqueMemory | NSPointerFunctionsIntegerPersonality valueOptions:NSPointerFunctionsObjectPersonality capacity:10];
        
        deviceNumber = ++lastDeviceNumber;
        if(aDevice == NULL)
        {
            deadZone     = 0.0;
            device       = NULL;
        }
        else
        {
            CFRetain(aDevice);

            device = aDevice;
            deadZone = 0.2;
            
            if(![self isKeyboardDevice])
            {
                _deviceIdentifier = [self OE_deviceIdentifier];
                
                NSArray *elements = (__bridge_transfer NSArray *)IOHIDDeviceCopyMatchingElements(device, (__bridge CFDictionaryRef)@{ @kIOHIDElementUsagePageKey : @(kHIDPage_GenericDesktop) }, 0);
                
                NSLog(@"Device: %@", self);
                
                NSMutableArray *posNegElements = [NSMutableArray array];
                NSMutableArray *posElements    = [NSMutableArray array];
                
                for(id element in elements)
                {
                    IOHIDElementRef elem  = (__bridge IOHIDElementRef)element;
                    uint32_t        usage = IOHIDElementGetUsage(elem);
                    
                    if(usage < kHIDUsage_GD_X || usage > kHIDUsage_GD_Rz) continue;
                    
                    CFIndex minimum = IOHIDElementGetLogicalMin(elem);
                    CFIndex maximum = IOHIDElementGetLogicalMax(elem);
                    
                    if(minimum == 0) [posElements addObject:element];
                    else if(minimum < 0 && maximum > 0) [posNegElements addObject:element];
                }
                
                if([posNegElements count] != 0 && [posElements count] != 0)
                {
                    for(id element in posElements)
                    {
                        IOHIDElementRef elem  = (__bridge IOHIDElementRef)element;
                        IOHIDElementSetProperty(elem, CFSTR(kOEHIDElementIsTriggerKey), (__bridge CFTypeRef)@YES);
                    }
                }
            }
        }
    }
    
    return self;
}

- (void)dealloc
{
	if(ffDevice != NULL) FFReleaseDevice(ffDevice);
    if(device   != NULL) CFRelease(device);
}

- (id)copyWithZone:(NSZone *)zone
{
    return self;
}

- (BOOL)isEqual:(id)anObject
{
    if(self == anObject)
        return YES;
    if([anObject isKindOfClass:[self class]])
        return [(__bridge id)device isEqual:(id)[anObject device]];
    return [super isEqual:anObject];
}

- (NSUInteger)hash
{
    return [(__bridge id)device hash];
}

- (NSString *)serialNumber
{
    return (__bridge NSString *)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDSerialNumberKey));
}

- (NSString *)manufacturer
{
    return (__bridge NSString *)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDManufacturerKey));
}

- (NSString *)product
{
    return (__bridge NSString *)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductKey));
}

- (NSNumber *)vendorID
{
    return (__bridge NSNumber *)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDVendorIDKey));
}

- (NSNumber *)productID
{
    return (__bridge NSNumber *)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductIDKey));
}

- (NSNumber *)locationID
{
    return (__bridge NSNumber *)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDLocationIDKey));
}

- (IOHIDElementRef)elementForEvent:(OEHIDEvent *)anEvent;
{
    NSMutableDictionary *dict = [NSMutableDictionary dictionary];

    switch([anEvent type])
    {
        case OEHIDEventTypeAxis :
        case OEHIDEventTypeTrigger :
            [dict setObject:@(kHIDPage_GenericDesktop) forKey:@kIOHIDElementUsagePageKey];
            [dict setObject:@([anEvent axis])          forKey:@kIOHIDElementUsageKey];
            break;
        case OEHIDEventTypeButton :
            [dict setObject:@(kHIDPage_Button)         forKey:@kIOHIDElementUsagePageKey];
            [dict setObject:@([anEvent buttonNumber])  forKey:@kIOHIDElementUsageKey];
            break;
        case OEHIDEventTypeHatSwitch :
            [dict setObject:@(kHIDPage_GenericDesktop) forKey:@kIOHIDElementUsagePageKey];
            [dict setObject:@(kHIDUsage_GD_Hatswitch)  forKey:@kIOHIDElementUsageKey];
            break;
        default : return nil;
    }

    NSUInteger cookie = [anEvent cookie];
    if(cookie != NSNotFound) [dict setObject:@(cookie) forKey:@kIOHIDElementCookieKey];

    NSArray *elements = (__bridge_transfer NSArray *)IOHIDDeviceCopyMatchingElements(device, (__bridge CFDictionaryRef)dict, 0);

    return (__bridge IOHIDElementRef)[elements lastObject];
}

- (BOOL)isKeyboardDevice;
{
    return IOHIDDeviceConformsTo(device, kHIDPage_GenericDesktop, kHIDUsage_GD_Keyboard);
}

- (OEHIDEvent *)eventWithHIDValue:(IOHIDValueRef)aValue
{
    IOHIDElementRef  elem     = IOHIDValueGetElement(aValue);
    uint32_t         cookie   = (uint32_t)IOHIDElementGetCookie(elem);
    
    OEHIDEvent *event = [mapTable objectForKey:@(cookie)];
    
    if(event == nil)
    {
        event = [OEHIDEvent eventWithDeviceHandler:self value:aValue];
        [mapTable setObject:event forKey:@(cookie)];
    }
    else NSAssert1([event OE_setupEventWithDeviceHandler:self value:aValue], @"The event setup went wrong for event: %@", event);
    
    return event;
}

- (void)dispatchEventWithHIDValue:(IOHIDValueRef)aValue
{
    OEHIDEvent *event = [self eventWithHIDValue:aValue];
    if([event hasChanges]) [NSApp postHIDEvent:event];
}

- (io_service_t)serviceRef
{
	io_service_t service = MACH_PORT_NULL;
	
#if MAC_OS_X_VERSION_MIN_REQUIRED > MAC_OS_X_VERSION_10_5
	service = IOHIDDeviceGetService(device);
#else
	NSMutableDictionary *matchingDict = (NSMutableDictionary *)IOServiceMatching(kIOHIDDeviceKey);
	if(matchingDict != nil)
	{
		[matchingDict setValue:[self locationID] forKey:(id)CFSTR(kIOHIDLocationIDKey)];
		service = IOServiceGetMatchingService(kIOMasterPortDefault, (CFDictionaryRef)matchingDict);
	}
#endif
	return service;
}

- (BOOL)supportsForceFeedback
{
	BOOL result = NO;
	
	io_service_t service = [self serviceRef];
	if(service != MACH_PORT_NULL)
	{
		HRESULT FFResult = FFIsForceFeedback(service);
		result = (FFResult == FF_OK);
	}
	return result;
}

- (void)enableForceFeedback
{
	if([self supportsForceFeedback])
	{
		io_service_t service = [self serviceRef];
		if(service != MACH_PORT_NULL)
			FFCreateDevice(service, &ffDevice);
	}
}

- (void)disableForceFeedback
{
	if(ffDevice != NULL)
    {
        FFReleaseDevice(ffDevice);
        ffDevice = NULL;
    }
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@ %p deviceIdentifier: '%@' manufacturer: %@ product: %@ serialNumber: %@ deviceNumber: %lu isKeyboard: %@>", [self class], self, [self deviceIdentifier], [self manufacturer], [self product], [self serialNumber], [self deviceNumber], [self isKeyboardDevice] ? @"YES" : @"NO"];
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
