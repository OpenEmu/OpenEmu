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
        _latestEvents = [[NSMutableDictionary alloc] initWithCapacity:10];
        _device = (void *)CFRetain(aDevice);
        _deadZone = 0.2;

        if(![self isKeyboardDevice])
        {
            NSLog(@"Device: %@", self);
            
            // Set the device's hat switch and trigger elements so the event creation works properly.
            OEControllerDescription *controller = [self controllerDescription];
            NSMutableArray *genericDesktopElements = [(__bridge_transfer NSArray *)IOHIDDeviceCopyMatchingElements(_device, (__bridge CFDictionaryRef)@{ @kIOHIDElementUsagePageKey : @(kHIDPage_GenericDesktop) }, 0) mutableCopy];

            for(id e in genericDesktopElements)
            {
                IOHIDElementRef elem = (__bridge IOHIDElementRef)e;
                OEHIDEvent *event = [[controller controlValueDescriptionForEvent:[OEHIDEvent OE_eventWithElement:elem value:0]] event];
                switch([event type])
                {
                    case OEHIDEventTypeHatSwitch :
                        IOHIDElementSetProperty(elem, CFSTR(kOEHIDElementHatSwitchTypeKey), (__bridge CFTypeRef)@([event hatSwitchType]));
                        break;
                    case OEHIDEventTypeTrigger :
                        IOHIDElementSetProperty(elem, CFSTR(kOEHIDElementIsTriggerKey), (__bridge CFTypeRef)@YES);
                        break;
                    default :
                        break;
                }
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

- (OEHIDEvent *)eventWithHIDValue:(IOHIDValueRef)aValue
{
    OEHIDEvent *event = [OEHIDEvent eventWithDeviceHandler:self value:aValue];
    if(event == nil) return nil;

    NSNumber   *cookieKey     = @([event cookie]);
    OEHIDEvent *existingEvent = _latestEvents[cookieKey];

    if([event isEqualToEvent:existingEvent])
        return nil;

    return _latestEvents[cookieKey] = event;
}

- (void)dispatchEventWithHIDValue:(IOHIDValueRef)aValue
{
    OEHIDEvent *event = [self eventWithHIDValue:aValue];
    if(event != nil) [NSApp postHIDEvent:event];
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

    [[NSNotificationCenter defaultCenter] postNotificationName:OEInputDeviceDisconnectNotification object:self];
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

#pragma mark - Controller Description Set Up

- (void)setUpControllerDescription:(OEControllerDescription *)description usingRepresentation:(NSDictionary *)controlRepresentations
{
    NSMutableArray *genericDesktopElements = [(__bridge_transfer NSArray *)IOHIDDeviceCopyMatchingElements(_device, (__bridge CFDictionaryRef)@{ @kIOHIDElementUsagePageKey : @(kHIDPage_GenericDesktop) }, 0) mutableCopy];
    NSMutableArray *buttonElements = [(__bridge_transfer NSArray *)IOHIDDeviceCopyMatchingElements(_device, (__bridge CFDictionaryRef)@{ @kIOHIDElementUsagePageKey : @(kHIDPage_Button) }, 0) mutableCopy];

    void (^addControl)(NSString *, NSDictionary *, IOHIDElementRef) =
    ^(NSString *identifier, NSDictionary *representation, IOHIDElementRef elem)
    {
        OEHIDEvent *genericEvent = [OEHIDEvent OE_eventWithElement:elem value:0];
        if(genericEvent == nil) return;

        [description addControlWithIdentifier:identifier name:representation[@"Name"] event:genericEvent valueRepresentations:representation[@"Values"]];
    };

    [controlRepresentations enumerateKeysAndObjectsUsingBlock:
     ^(NSString *identifier, NSDictionary *rep, BOOL *stop)
     {
         IOHIDElementRef elem = [self OE_elementForRepresentation:rep inGenericDesktopElements:genericDesktopElements andButtonElements:buttonElements];
         addControl(identifier, rep, elem);
     }];

    if([[description controls] count] > 0)
    {
        [genericDesktopElements enumerateObjectsWithOptions:NSEnumerationConcurrent | NSEnumerationReverse usingBlock:
         ^(id elem, NSUInteger idx, BOOL *stop)
         {
             if([OEHIDEvent OE_eventWithElement:(__bridge IOHIDElementRef)elem value:0] == nil)
                 [genericDesktopElements removeObjectAtIndex:idx];
         }];

        if([genericDesktopElements count] > 0)
            NSLog(@"WARNING: There are %ld generic desktop elements unaccounted for in %@", [genericDesktopElements count], [description name]);

        if([buttonElements count] > 0)
            NSLog(@"WARNING: There are %ld button elements unaccounted for.", [buttonElements count]);
    }

    for(id e in buttonElements) addControl(nil, nil, (__bridge IOHIDElementRef)e);

    [self OE_setUpGenericDesktopElements:genericDesktopElements];

    for(id e in genericDesktopElements) addControl(nil, nil, (__bridge IOHIDElementRef)e);
}

- (IOHIDElementRef)OE_elementForRepresentation:(NSDictionary *)representation inGenericDesktopElements:(NSMutableArray *)genericDesktopElements andButtonElements:(NSMutableArray *)buttonElements
{
    OEHIDEventType type = OEHIDEventTypeFromNSString(representation[@"Type"]);
    NSUInteger cookie = [representation[@"Cookie"] integerValue];
    NSUInteger usage = OEUsageFromUsageStringWithType(representation[@"Usage"], type);

    __block IOHIDElementRef ret = NULL;

    NSMutableArray *targetArray = type == OEHIDEventTypeButton ? buttonElements : genericDesktopElements;
    [targetArray enumerateObjectsUsingBlock:
     ^(id obj, NSUInteger idx, BOOL *stop)
     {
         IOHIDElementRef elem = (__bridge IOHIDElementRef)obj;

         if((cookie != OEUndefinedCookie && cookie != IOHIDElementGetCookie(elem))
            || usage != IOHIDElementGetUsage(elem))
             return;

         ret = elem;
         // Make sure you stop enumerating right after modifying the array
         // or else it will throw an exception.
         [targetArray removeObjectAtIndex:idx];
         *stop = YES;
     }];

    switch(type)
    {
        case OEHIDEventTypeTrigger :
            IOHIDElementSetProperty(ret, CFSTR(kOEHIDElementIsTriggerKey), (__bridge CFTypeRef)@YES);
            break;
        case OEHIDEventTypeHatSwitch :
            [self OE_setUpHatSwitchElement:ret];
            break;
        default :
            break;
    }

    return ret;
}

- (void)OE_setUpGenericDesktopElements:(NSArray *)genericDesktopElements;
{
    NSMutableArray *posNegElements = [NSMutableArray array];
    NSMutableArray *posElements    = [NSMutableArray array];

    for(id element in genericDesktopElements)
    {
        IOHIDElementRef elem  = (__bridge IOHIDElementRef)element;
        uint32_t        usage = IOHIDElementGetUsage(elem);

        if(usage == kHIDUsage_GD_Hatswitch)
            [self OE_setUpHatSwitchElement:elem];
        else if(kHIDUsage_GD_X <= usage && usage <= kHIDUsage_GD_Rz)
        {
            CFIndex minimum = IOHIDElementGetLogicalMin(elem);
            CFIndex maximum = IOHIDElementGetLogicalMax(elem);

            if(minimum == 0) [posElements addObject:element];
            else if(minimum < 0 && maximum > 0) [posNegElements addObject:element];
        }
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

- (void)OE_setUpHatSwitchElement:(IOHIDElementRef)element
{
    NSInteger count = IOHIDElementGetLogicalMax(element) - IOHIDElementGetLogicalMin(element) + 1;
    OEHIDEventHatSwitchType type = OEHIDEventHatSwitchTypeUnknown;
    switch(count)
    {
        case 4 : type = OEHIDEventHatSwitchType4Ways; break;
        case 8 : type = OEHIDEventHatSwitchType8Ways; break;
    }
    IOHIDElementSetProperty(element, CFSTR(kOEHIDElementHatSwitchTypeKey), (__bridge CFTypeRef)@(type));
}

@end
