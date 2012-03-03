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

@interface OEHIDEvent ()
- (BOOL)OE_setupEventWithDeviceHandler:(OEHIDDeviceHandler *)aDeviceHandler value:(IOHIDValueRef)aValue;
@end


@implementation OEHIDDeviceHandler

@synthesize device, deviceNumber, deadZone;

+ (id)deviceHandlerWithDevice:(IOHIDDeviceRef)aDevice
{
    return [[self alloc] initWithDevice:aDevice];
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
    if(self)
    {
        mapTable = [[NSMapTable alloc] initWithKeyOptions:NSPointerFunctionsOpaqueMemory | NSPointerFunctionsIntegerPersonality valueOptions:NSPointerFunctionsObjectPersonality capacity:10];
        
        if(aDevice == NULL)
        {
            if(nilHandler == nil)
            {
                device = NULL;
                deviceNumber = 0;
                deadZone = 0.0;
                nilHandler = self;
            }
            else
            {
                return nilHandler;
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

- (void)dealloc
{
	if(ffDevice) FFReleaseDevice(ffDevice);
    
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

- (NSString *)manufacturer
{
    return (__bridge NSString *)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDManufacturerKey));
}

- (NSString *)product
{
    return (__bridge NSString *)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductKey));
}

- (NSNumber *)productID
{
    return (__bridge NSNumber *)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductIDKey));
}

- (NSNumber *)locationID
{
    return (__bridge NSNumber *)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDLocationIDKey));
}

- (OEHIDEvent *)eventWithHIDValue:(IOHIDValueRef)aValue
{
    IOHIDElementRef elem   = IOHIDValueGetElement(aValue);
    uint32_t      cookie = (uint32_t)IOHIDElementGetCookie(elem);
    NSNumber        *nscookie = [NSNumber numberWithUnsignedInt:cookie];
    
    
    OEHIDEvent *event = [mapTable objectForKey:nscookie];
    
    if(event == nil)
    {
        event = [OEHIDEvent eventWithDeviceHandler:self value:aValue];
        [mapTable setObject:event forKey:nscookie];
    }
    else NSAssert1([event OE_setupEventWithDeviceHandler:self value:aValue], @"The event setup went wrong for event: %@", event);
    
    return event;
}

- (void)dispatchEventWithHIDValue:(IOHIDValueRef)aValue
{
    [NSApp postHIDEvent:[self eventWithHIDValue:aValue]];
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

@end
