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
#import "OEControllerDescription.h"
#import "OEDeviceDescription.h"
#import "OEControlDescription.h"
#import "OEHIDEvent.h"
#import "OEDeviceManager.h"
#import "OEHIDDeviceParser.h"

@interface OEHIDEvent ()
+ (instancetype)OE_eventWithElement:(IOHIDElementRef)element value:(NSInteger)value;
@end

@interface OEDeviceManager ()
- (void)OE_removeDeviceHandler:(OEDeviceHandler *)handler;
@end

@interface OEHIDDeviceHandler ()
{
    NSMutableDictionary *_latestEvents;

	//force feedback support
	FFDeviceObjectReference _ffDevice;
}

- (void)OE_setupCallbacks;
- (void)OE_removeDeviceHandlerForDevice:(IOHIDDeviceRef)aDevice;

@end

@implementation OEHIDDeviceHandler

+ (id<OEHIDDeviceParser>)deviceParser;
{
    static OEHIDDeviceParser *parser = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        parser = [[OEHIDDeviceParser alloc] init];
    });

    return parser;
}

- (id)init
{
    return nil;
}

- (id)initWithDeviceDescription:(OEDeviceDescription *)deviceDescription
{
    return nil;
}

- (id)initWithIOHIDDevice:(IOHIDDeviceRef)aDevice deviceDescription:(OEDeviceDescription *)deviceDescription;
{
    if(aDevice == NULL) return nil;

    if((self = [super initWithDeviceDescription:deviceDescription]))
    {
        _device = (void *)CFRetain(aDevice);
        NSAssert(deviceDescription != nil || [self isKeyboardDevice], @"Non-keyboard devices must have device descriptions.");
        if(deviceDescription != nil) _latestEvents = [[NSMutableDictionary alloc] initWithCapacity:[[self controllerDescription] numberOfControls]];
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

- (NSUInteger)vendorID
{
    return [(__bridge NSNumber *)IOHIDDeviceGetProperty(_device, CFSTR(kIOHIDVendorIDKey)) integerValue];
}

- (NSUInteger)productID
{
    return [(__bridge NSNumber *)IOHIDDeviceGetProperty(_device, CFSTR(kIOHIDProductIDKey)) integerValue];
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
    if(cookie != OEUndefinedCookie) dict[@kIOHIDElementCookieKey] = @(cookie);

    NSArray *elements = (__bridge_transfer NSArray *)IOHIDDeviceCopyMatchingElements(_device, (__bridge CFDictionaryRef)dict, 0);

    return (__bridge IOHIDElementRef)[elements lastObject];
}

- (BOOL)isKeyboardDevice;
{
    return IOHIDDeviceConformsTo(_device, kHIDPage_GenericDesktop, kHIDUsage_GD_Keyboard);
}

- (void)dispatchEvent:(OEHIDEvent *)event
{
    if(event == nil) return;

    NSNumber   *cookieKey     = @([event cookie]);
    OEHIDEvent *existingEvent = _latestEvents[cookieKey];

    if([event isEqualToEvent:existingEvent]) return;

    _latestEvents[cookieKey] = event;
    [NSApp postHIDEvent:event];
}

- (OEHIDEvent *)eventWithHIDValue:(IOHIDValueRef)aValue
{
    return [OEHIDEvent eventWithDeviceHandler:self value:aValue];
}

- (void)dispatchEventWithHIDValue:(IOHIDValueRef)aValue
{
    [self dispatchEvent:[self eventWithHIDValue:aValue]];
}

- (io_service_t)serviceRef
{
	return IOHIDDeviceGetService(_device);
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
