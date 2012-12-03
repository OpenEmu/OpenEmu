/*
 Copyright (c) 2012, OpenEmu Team


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
#import "OEDeviceManager.h"

@interface OEHIDEvent ()
- (BOOL)OE_setupEventWithDeviceHandler:(OEHIDDeviceHandler *)aDeviceHandler value:(IOHIDValueRef)aValue;
@end

@interface OEDeviceHandler ()
- (void)OE_setupDeviceIdentifier;
@end

@interface OEDeviceManager ()
- (void)OE_removeDeviceHandler:(OEDeviceHandler *)handler;
@end

@interface OEHIDDeviceHandler ()
{
    NSMutableDictionary *_reusableEventMap;

	//force feedback support
	FFDeviceObjectReference _ffDevice;
}

- (void)OE_setupCallbacks;
- (void)OE_removeDeviceHandlerForDevice:(IOHIDDeviceRef)aDevice;

@end

@implementation OEHIDDeviceHandler

+ (instancetype)deviceHandlerWithIOHIDDevice:(IOHIDDeviceRef)aDevice;
{
    return [[self alloc] initWithIOHIDDevice:aDevice];
}

- (id)init
{
    return nil;
}

- (id)initWithIOHIDDevice:(IOHIDDeviceRef)aDevice;
{
    if(aDevice == NULL) return nil;

    if((self = [super init]))
    {
        _reusableEventMap = [[NSMutableDictionary alloc] initWithCapacity:10];
        _device = (void *)CFRetain(aDevice);
        _deadZone = 0.2;

        if(![self isKeyboardDevice])
        {
            [self OE_setupDeviceIdentifier];

            NSArray *elements = (__bridge_transfer NSArray *)IOHIDDeviceCopyMatchingElements(_device, (__bridge CFDictionaryRef)@{ @kIOHIDElementUsagePageKey : @(kHIDPage_GenericDesktop) }, 0);

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
                for(id element in posElements)
                {
                    IOHIDElementRef elem  = (__bridge IOHIDElementRef)element;
                    IOHIDElementSetProperty(elem, CFSTR(kOEHIDElementIsTriggerKey), (__bridge CFTypeRef)@YES);
            }
        }

        [self OE_setupCallbacks];
    }

    return self;
}

- (void)dealloc
{
    [self OE_removeDeviceHandlerForDevice:_device];

    if(_device != NULL) CFRelease(_device);
    if(_ffDevice != NULL) FFReleaseDevice(_ffDevice);
}

- (NSString *)serialNumber
{
    return (__bridge NSString *)IOHIDDeviceGetProperty(_device, CFSTR(kIOHIDSerialNumberKey));
}

- (NSString *)manufacturer
{
    return (__bridge NSString *)IOHIDDeviceGetProperty(_device, CFSTR(kIOHIDManufacturerKey));
}

- (NSString *)product
{
    return (__bridge NSString *)IOHIDDeviceGetProperty(_device, CFSTR(kIOHIDProductKey));
}

- (NSNumber *)vendorID
{
    return (__bridge NSNumber *)IOHIDDeviceGetProperty(_device, CFSTR(kIOHIDVendorIDKey));
}

- (NSNumber *)productID
{
    return (__bridge NSNumber *)IOHIDDeviceGetProperty(_device, CFSTR(kIOHIDProductIDKey));
}

- (NSNumber *)locationID
{
    return (__bridge NSNumber *)IOHIDDeviceGetProperty(_device, CFSTR(kIOHIDLocationIDKey));
}

- (IOHIDElementRef)elementForEvent:(OEHIDEvent *)anEvent;
{
    NSMutableDictionary *dict = [NSMutableDictionary dictionary];

    switch([anEvent type])
    {
        case OEHIDEventTypeAxis :
        case OEHIDEventTypeTrigger :
            dict[@kIOHIDElementUsagePageKey] = @(kHIDPage_GenericDesktop);
            dict[@kIOHIDElementUsageKey]     = @([anEvent axis]);
            break;
        case OEHIDEventTypeButton :
            dict[@kIOHIDElementUsagePageKey] = @(kHIDPage_Button);
            dict[@kIOHIDElementUsageKey]     = @([anEvent buttonNumber]);
            break;
        case OEHIDEventTypeHatSwitch :
            dict[@kIOHIDElementUsagePageKey] = @(kHIDPage_GenericDesktop);
            dict[@kIOHIDElementUsageKey]     = @(kHIDUsage_GD_Hatswitch);
            break;
        default : return nil;
    }

    NSUInteger cookie = [anEvent cookie];
    if(cookie != NSNotFound) dict[@kIOHIDElementCookieKey] = @(cookie);

    NSArray *elements = (__bridge_transfer NSArray *)IOHIDDeviceCopyMatchingElements(_device, (__bridge CFDictionaryRef)dict, 0);

    return (__bridge IOHIDElementRef)[elements lastObject];
}

- (BOOL)isKeyboardDevice;
{
    return IOHIDDeviceConformsTo(_device, kHIDPage_GenericDesktop, kHIDUsage_GD_Keyboard);
}

- (OEHIDEvent *)eventWithHIDValue:(IOHIDValueRef)aValue
{
    IOHIDElementRef  elem   = IOHIDValueGetElement(aValue);
    uint32_t         cookie = (uint32_t)IOHIDElementGetCookie(elem);
    OEHIDEvent      *event  = [_reusableEventMap objectForKey:@(cookie)];

    if(event == nil)
    {
        event = [OEHIDEvent eventWithDeviceHandler:self value:aValue];
        if(event == nil) return nil;

        [_reusableEventMap setObject:event forKey:@(cookie)];
    }
    else NSAssert([event OE_setupEventWithDeviceHandler:self value:aValue], @"The event setup went wrong for event: %@", event);

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
	service = IOHIDDeviceGetService(_device);
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
			FFCreateDevice(service, &_ffDevice);
	}
}

- (void)disableForceFeedback
{
	if(_ffDevice != NULL)
    {
        FFReleaseDevice(_ffDevice);
        _ffDevice = NULL;
    }
}

- (void)OE_setupCallbacks;
{
    // Register for removal
    IOHIDDeviceRegisterRemovalCallback(_device, OEHandle_DeviceRemovalCallback, (__bridge void *)self);

    // Register for input
    IOHIDDeviceRegisterInputValueCallback(_device, OEHandle_InputValueCallback, (__bridge void *)self);

    // Attach to the runloop
    IOHIDDeviceScheduleWithRunLoop(_device, CFRunLoopGetMain(), kCFRunLoopDefaultMode);
}

- (void)OE_removeDeviceHandlerForDevice:(IOHIDDeviceRef)aDevice
{
    NSAssert(aDevice == _device, @"Device remove callback called on the wrong object.");

	IOHIDDeviceUnscheduleFromRunLoop(_device, CFRunLoopGetMain(), kCFRunLoopDefaultMode);

    [[OEDeviceManager sharedDeviceManager] OE_removeDeviceHandler:self];
}

static void OEHandle_InputValueCallback(void *inContext, IOReturn inResult, void *inSender, IOHIDValueRef inIOHIDValueRef)
{
    [(__bridge OEHIDDeviceHandler *)inContext dispatchEventWithHIDValue:inIOHIDValueRef];
}

static void OEHandle_DeviceRemovalCallback(void *inContext, IOReturn inResult, void *inSender)
{
	IOHIDDeviceRef hidDevice = (IOHIDDeviceRef)inSender;

	[(__bridge OEHIDDeviceHandler *)inContext OE_removeDeviceHandlerForDevice:hidDevice];
}

@end
