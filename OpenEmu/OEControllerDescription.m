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

#import "OEControllerDescription.h"
#import "OEControllerDescription_Internal.h"
#import "OEDeviceHandler.h"
#import "OEHIDDeviceHandler.h"
#import "OEHIDEvent.h"
#import <IOKit/hid/IOHIDUsageTables.h>

OEHIDEventType OEHIDEventTypeFromNSString(NSString *string);
NSUInteger OEUsageFromUsageStringWithType(NSString *usageString, OEHIDEventType type);

@interface OEHIDEvent ()
+ (instancetype)OE_eventWithElement:(IOHIDElementRef)element value:(NSInteger)value;
@end

static NSNumber *_OEDeviceIdentifierKey(id obj)
{
    return @([obj vendorID] << 32 | [obj productID]);
}

@interface OEControllerDescription ()
{
    NSString     *_identifier;
    NSString     *_name;
    NSArray      *_devices;

    NSDictionary *_controls;
    NSDictionary *_identifierToControlValue;
    NSDictionary *_valueIdentifierToControlValue;
    NSArray      *_axisControls;
    NSArray      *_buttonControls;
    NSArray      *_hatSwitchControls;
    NSArray      *_triggerControls;
}

- (id)OE_initWithIdentifier:(NSString *)identifier representation:(NSDictionary *)representation __attribute__((objc_method_family(init)));
- (id)OE_initWithDevice:(IOHIDDeviceRef)device __attribute__((objc_method_family(init)));
- (BOOL)OE_needsControlSetup;

@end

// Device representations stay in there for as long as no actual device of their type were plugged in.
static NSMutableDictionary *_mappingRepresentations;

static NSMutableDictionary *_identifierToControllerDescriptions;
static NSMutableDictionary *_deviceIDToDeviceDescriptions;

@implementation OEControllerDescription

+ (void)initialize
{
    if(self == [OEControllerDescription class])
    {
        NSString *identifierPath = [[NSBundle mainBundle] pathForResource:@"Controller-Database" ofType:@"plist"];
        NSDictionary *representations = [NSPropertyListSerialization propertyListWithData:[NSData dataWithContentsOfFile:identifierPath options:NSDataReadingMappedIfSafe error:NULL] options:0 format:NULL error:NULL];

        _identifierToControllerDescriptions = [NSMutableDictionary dictionary];
        _deviceIDToDeviceDescriptions = [NSMutableDictionary dictionary];

        NSMutableDictionary *mappingReps = [NSMutableDictionary dictionaryWithCapacity:[representations count]];

        [representations enumerateKeysAndObjectsUsingBlock:
         ^(NSString *identifier, NSDictionary *rep, BOOL *stop) {
             [self OE_installControllerDescription:[[OEControllerDescription alloc] OE_initWithIdentifier:identifier representation:rep]];

             mappingReps[identifier] = rep[@"OEControllerMappings"] ? : @{};
         }];

        _mappingRepresentations = mappingReps;
    }
}

+ (instancetype)controllerDescriptionForControllerIdentifier:(NSString *)controllerIdentifier
{
    return _identifierToControllerDescriptions[controllerIdentifier];
}

+ (instancetype)controllerDescriptionForDeviceHandler:(OEDeviceHandler *)deviceHandler
{
    return [[OEDeviceDescription deviceDescriptionForDeviceHandler:deviceHandler] controllerDescription];
}

+ (OEDeviceDescription *)OE_deviceDescriptionForDeviceHandler:(OEHIDDeviceHandler *)deviceHandler;
{
    NSAssert([deviceHandler isKindOfClass:[OEHIDDeviceHandler class]], @"Device Handle type not handled.");

    OEDeviceDescription *ret = _deviceIDToDeviceDescriptions[_OEDeviceIdentifierKey(deviceHandler)];
    OEControllerDescription *ctrlDesc = [ret controllerDescription];

    if(ret == nil)
    {
        [self OE_installControllerDescription:[[OEControllerDescription alloc] OE_initWithDevice:[deviceHandler device]]];
        ret = _deviceIDToDeviceDescriptions[_OEDeviceIdentifierKey(deviceHandler)];
    }
    else if([ctrlDesc OE_needsControlSetup])
    {
        [ctrlDesc OE_setupControlsWithMappings:_mappingRepresentations[[ctrlDesc identifier]] IOHIDDevice:[deviceHandler device]];
        [_mappingRepresentations removeObjectForKey:[ctrlDesc identifier]];
    }

    return ret;
}

+ (void)OE_installControllerDescription:(OEControllerDescription *)controllerDescription
{
    _identifierToControllerDescriptions[[controllerDescription identifier]] = controllerDescription;

    for(OEDeviceDescription *device in [controllerDescription devices])
        _deviceIDToDeviceDescriptions[_OEDeviceIdentifierKey(device)] = device;
}

- (id)OE_initWithIdentifier:(NSString *)identifier representation:(NSDictionary *)representation
{
    if((self = [super init]))
    {
        _identifier = [identifier copy];
        _name = representation[@"OEControllerName"];

        [self OE_setupDevicesWithRepresentations:[representation objectForKey:@"OEControllerDevices"]];
    }

    return self;
}

- (id)OE_initWithDevice:(IOHIDDeviceRef)device
{
    if((self = [super init]))
    {
        _isGeneric = YES;
        _name = (__bridge NSString *)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductKey));
        OEDeviceDescription *desc = [[OEDeviceDescription alloc] OE_initWithRepresentation:
                                     @{
                                         @"OEControllerDeviceName" : _name,
                                         @"OEControllerVendorID"   : (__bridge NSNumber *)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDVendorIDKey)),
                                         @"OEControllerProductID"  : (__bridge NSNumber *)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductIDKey))
                                     }];
        [desc setControllerDescription:self];

        _identifier = [desc genericDeviceIdentifier];

        _devices = @[ desc ];
        [self OE_setupControlsWithMappings:nil IOHIDDevice:device];
    }

    return self;
}

- (id)copyWithZone:(NSZone *)zone
{
    return self;
}

- (void)OE_setupDevicesWithRepresentations:(NSArray *)representations
{
    NSMutableArray *devices = [[NSMutableArray alloc] initWithCapacity:[representations count]];

    for(NSDictionary *rep in representations)
    {
        OEDeviceDescription *desc = [[OEDeviceDescription alloc] OE_initWithRepresentation:rep];
        [desc setControllerDescription:self];
        [devices addObject:desc];
    }

    _devices = [devices copy];
}

- (void)OE_setupHatSwitchElement:(IOHIDElementRef)element
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
            [self OE_setupHatSwitchElement:ret];
            break;
        default :
            break;
    }

    return ret;
}

- (void)OE_parseGenericDesktopElements:(NSArray *)genericDesktopElements usingBlock:(void(^)(IOHIDElementRef))block;
{
    NSMutableArray *posNegElements = [NSMutableArray array];
    NSMutableArray *posElements    = [NSMutableArray array];

    for(id element in genericDesktopElements)
    {
        IOHIDElementRef elem  = (__bridge IOHIDElementRef)element;
        uint32_t        usage = IOHIDElementGetUsage(elem);

        if(usage == kHIDUsage_GD_Hatswitch)
        {
            [self OE_setupHatSwitchElement:elem];
            block(elem);
        }
        else if(kHIDUsage_GD_X <= usage && usage <= kHIDUsage_GD_Rz)
        {
            CFIndex minimum = IOHIDElementGetLogicalMin(elem);
            CFIndex maximum = IOHIDElementGetLogicalMax(elem);

            if(minimum == 0) [posElements addObject:element];
            else if(minimum < 0 && maximum > 0) [posNegElements addObject:element];
        }
    }

    NSArray *axisElements = [posElements count] > 0 ? posElements : posNegElements;

    if([posNegElements count] != 0 && [posElements count] != 0)
    {
        for(id element in posElements)
        {
            IOHIDElementRef elem  = (__bridge IOHIDElementRef)element;
            IOHIDElementSetProperty(elem, CFSTR(kOEHIDElementIsTriggerKey), (__bridge CFTypeRef)@YES);
            block(elem);
        }

        axisElements = posNegElements;
    }

    for(id element in posElements) block((__bridge IOHIDElementRef)element);
}

- (void)OE_setupControlsWithMappings:(NSDictionary *)mappings IOHIDDevice:(IOHIDDeviceRef)device
{
    NSMutableArray *genericDesktopElements = [(__bridge_transfer NSArray *)IOHIDDeviceCopyMatchingElements(device, (__bridge CFDictionaryRef)@{ @kIOHIDElementUsagePageKey : @(kHIDPage_GenericDesktop) }, 0) mutableCopy];
    NSMutableArray *buttonElements = [(__bridge_transfer NSArray *)IOHIDDeviceCopyMatchingElements(device, (__bridge CFDictionaryRef)@{ @kIOHIDElementUsagePageKey : @(kHIDPage_Button) }, 0) mutableCopy];

    NSMutableDictionary *controls                      = [NSMutableDictionary dictionaryWithCapacity:[mappings count]];
    NSMutableDictionary *identifierToControlValue      = [NSMutableDictionary dictionaryWithCapacity:[mappings count] * 2];
    NSMutableDictionary *valueIdentifierToControlValue = [NSMutableDictionary dictionaryWithCapacity:[mappings count] * 2];
    NSMutableArray      *axisControls                  = [NSMutableArray arrayWithCapacity:6];
    NSMutableArray      *buttonControls                = [NSMutableArray arrayWithCapacity:32];
    NSMutableArray      *hatSwitchControls             = [NSMutableArray arrayWithCapacity:1];
    NSMutableArray      *triggerControls               = [NSMutableArray arrayWithCapacity:2];

    void (^addControl)(NSString *, NSDictionary *, IOHIDElementRef) =
    ^(NSString *identifier, NSDictionary *representation, IOHIDElementRef elem)
    {
        OEHIDEvent *genericEvent = [OEHIDEvent OE_eventWithElement:elem value:0];
        OEControlDescription *desc = [[OEControlDescription alloc] OE_initWithIdentifier:identifier representation:representation genericEvent:genericEvent];
        [desc setControllerDescription:self];

        controls[[desc identifier]] = desc;

        switch([genericEvent type])
        {
            case OEHIDEventTypeAxis      : [axisControls      addObject:desc]; break;
            case OEHIDEventTypeButton    : [buttonControls    addObject:desc]; break;
            case OEHIDEventTypeHatSwitch : [hatSwitchControls addObject:desc]; break;
            case OEHIDEventTypeTrigger   : [triggerControls   addObject:desc]; break;
            default : return;
        }

        for(OEControlValueDescription *value in [desc controlValues])
        {
            identifierToControlValue[[value identifier]] = value;
            valueIdentifierToControlValue[[value valueIdentifier]] = value;
        }
    };

    [mappings enumerateKeysAndObjectsUsingBlock:
     ^(NSString *identifier, NSDictionary *rep, BOOL *stop)
     {
         IOHIDElementRef elem = [self OE_elementForRepresentation:rep inGenericDesktopElements:genericDesktopElements andButtonElements:buttonElements];
         addControl(identifier, rep, elem);
     }];

    if([controls count] > 0)
    {
        [genericDesktopElements enumerateObjectsWithOptions:NSEnumerationConcurrent | NSEnumerationReverse usingBlock:
         ^(id elem, NSUInteger idx, BOOL *stop)
         {
             if([OEHIDEvent OE_eventWithElement:(__bridge IOHIDElementRef)elem value:0] == nil)
                 [genericDesktopElements removeObjectAtIndex:idx];
         }];

        if([genericDesktopElements count] > 0)
            NSLog(@"WARNING: There are %ld generic desktop elements unaccounted for in %@", [genericDesktopElements count], _name);

        if([buttonElements count] > 0)
            NSLog(@"WARNING: There are %ld button elements unaccounted for.", [buttonElements count]);
    }

    for(id e in buttonElements) addControl(nil, nil, (__bridge IOHIDElementRef)e);

    [self OE_parseGenericDesktopElements:genericDesktopElements usingBlock:
     ^(IOHIDElementRef elem)
     {
         addControl(nil, nil, elem);
     }];

    _controls = [controls copy];
    _identifierToControlValue = [identifierToControlValue copy];
    _valueIdentifierToControlValue = [valueIdentifierToControlValue copy];
    _axisControls = [axisControls copy];
    _buttonControls = [buttonControls copy];
    _hatSwitchControls = [hatSwitchControls copy];
    _triggerControls = [triggerControls copy];
}

- (NSArray *)controls
{
    return [_controls allValues];
}

- (OEControlDescription *)controlDescriptionForIdentifier:(NSString *)controlIdentifier;
{
    return _controls[controlIdentifier];
}

- (OEControlDescription *)controlDescriptionForIOHIDElement:(IOHIDElementRef)element
{
    return [[self controlValueDescriptionForEvent:[OEHIDEvent OE_eventWithElement:element value:0]] controlDescription];
}

- (OEControlValueDescription *)controlValueDescriptionForEvent:(OEHIDEvent *)event;
{
    return _valueIdentifierToControlValue[@([event genericIdentifier])];
}

- (OEControlValueDescription *)controlValueDescriptionForIdentifier:(NSString *)controlIdentifier;
{
    return _identifierToControlValue[controlIdentifier];
}

- (OEControlValueDescription *)controlValueDescriptionForValueIdentifier:(NSNumber *)controlValueIdentifier;
{
    return _valueIdentifierToControlValue[controlValueIdentifier];
}

- (BOOL)OE_needsControlSetup
{
    return _controls == nil;
}

@end

OEHIDEventType OEHIDEventTypeFromNSString(NSString *string)
{
    static NSDictionary *namesToValue = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        namesToValue = @{
            @"Axis"      : @(OEHIDEventTypeAxis),
            @"Button"    : @(OEHIDEventTypeButton),
            @"HatSwitch" : @(OEHIDEventTypeHatSwitch),
            @"Trigger"   : @(OEHIDEventTypeTrigger),
        };
    });

    return [[namesToValue objectForKey:string] integerValue];
}

NSUInteger OEUsageFromUsageStringWithType(NSString *usageString, OEHIDEventType type)
{
    switch(type)
    {
        case OEHIDEventTypeButton :
            return [usageString integerValue];
        case OEHIDEventTypeAxis :
        case OEHIDEventTypeTrigger :
            return OEHIDEventAxisFromNSString(usageString);
        case OEHIDEventTypeHatSwitch :
            return kHIDUsage_GD_Hatswitch;
        default :
            break;
    }

    return 0;
}
