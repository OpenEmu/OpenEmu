/*
 Copyright (c) 2013, OpenEmu Team

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

@interface OEHIDEvent ()
+ (instancetype)OE_eventWithElement:(IOHIDElementRef)element value:(NSInteger)value;
@end

static NSNumber *_OEDeviceIdentifierKey(id obj)
{
    return @([obj vendorID] << 32 | [obj productID]);
}

@interface OEControllerDescription ()
{
    NSString            *_identifier;
    NSString            *_name;
    NSArray             *_devices;

    NSMutableDictionary *_controls;
    NSMutableDictionary *_identifierToControlValue;
    NSMutableDictionary *_valueIdentifierToControlValue;
}

- (id)OE_initWithVendorID:(NSUInteger)vendorID productID:(NSUInteger)productID name:(NSString *)deviceName __attribute__((objc_method_family(init)));
- (id)OE_initWithIdentifier:(NSString *)identifier representation:(NSDictionary *)representation __attribute__((objc_method_family(init)));

@end

// Device representations stay in there for as long as no actual device of their type were plugged in.
static NSMutableDictionary *_mappingRepresentations;

static NSMutableDictionary *_deviceIDToDeviceDescriptions;

@implementation OEControllerDescription

+ (void)initialize
{
    if(self == [OEControllerDescription class])
    {
        NSString *identifierPath = [[NSBundle mainBundle] pathForResource:@"Controller-Database" ofType:@"plist"];
        NSDictionary *representations = [NSPropertyListSerialization propertyListWithData:[NSData dataWithContentsOfFile:identifierPath options:NSDataReadingMappedIfSafe error:NULL] options:0 format:NULL error:NULL];

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

+ (OEDeviceDescription *)OE_deviceDescriptionForVendorID:(NSUInteger)vendorID productID:(NSUInteger)productID name:(NSString *)deviceName
{
    OEDeviceDescription *ret = _deviceIDToDeviceDescriptions[[self OE_deviceDescriptionKeyForDeviceVendorID:vendorID productID:productID]];

    if(ret == nil)
    {
        OEControllerDescription *desc = [[OEControllerDescription alloc] OE_initWithVendorID:vendorID productID:productID name:deviceName];
        [self OE_installControllerDescription:desc];
        ret = _deviceIDToDeviceDescriptions[[self OE_deviceDescriptionKeyForDeviceVendorID:vendorID productID:productID]];
    }

    return ret;
}

+ (NSDictionary *)OE_dequeueRepresentationForDeviceDescription:(OEDeviceDescription *)deviceDescription;
{
    OEControllerDescription *ctrlDesc = [deviceDescription controllerDescription];
    NSDictionary *ret = _mappingRepresentations[[ctrlDesc identifier]];
    [_mappingRepresentations removeObjectForKey:[ctrlDesc identifier]];
    return ret;
}

+ (NSString *)OE_deviceDescriptionKeyForDeviceVendorID:(NSUInteger)vendorID productID:(NSUInteger)productID
{
    return [NSString stringWithFormat:@"<%#lx %#lx>", vendorID, productID];
}

+ (NSString *)OE_deviceDescriptionKeyForDevice:(id)device
{
    return [self OE_deviceDescriptionKeyForDeviceVendorID:[device vendorID] productID:[device productID]];
}

+ (void)OE_installControllerDescription:(OEControllerDescription *)controllerDescription
{
    for(OEDeviceDescription *device in [controllerDescription devices])
        _deviceIDToDeviceDescriptions[[self OE_deviceDescriptionKeyForDevice:device]] = device;
}

- (id)OE_initWithIdentifier:(NSString *)identifier representation:(NSDictionary *)representation
{
    if((self = [super init]))
    {
        _identifier = [identifier copy];
        _name = representation[@"OEControllerName"];
        _controls = [NSMutableDictionary dictionary];
        _identifierToControlValue = [NSMutableDictionary dictionary];
        _valueIdentifierToControlValue = [NSMutableDictionary dictionary];

        [self OE_setUpDevicesWithRepresentations:[representation objectForKey:@"OEControllerDevices"]];
    }

    return self;
}

- (id)OE_initWithVendorID:(NSUInteger)vendorID productID:(NSUInteger)productID name:(NSString *)deviceName
{
    if((self = [super init]))
    {
        _isGeneric = YES;
        _name = deviceName;
        _controls = [NSMutableDictionary dictionary];
        _identifierToControlValue = [NSMutableDictionary dictionary];
        _valueIdentifierToControlValue = [NSMutableDictionary dictionary];

        OEDeviceDescription *desc = [[OEDeviceDescription alloc] OE_initWithRepresentation:
                                     @{
                                         @"OEControllerDeviceName" : _name,
                                         @"OEControllerVendorID"   : @(vendorID),
                                         @"OEControllerProductID"  : @(productID)
                                     }];
        [desc setControllerDescription:self];

        _identifier = [desc genericDeviceIdentifier];
        _devices = @[ desc ];
    }

    return self;
}

- (id)copyWithZone:(NSZone *)zone
{
    return self;
}

- (void)OE_setUpDevicesWithRepresentations:(NSArray *)representations
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

- (NSUInteger)numberOfControls
{
    return [_controls count];
}

- (NSArray *)controls
{
    return [_controls allValues];
}

- (OEControlValueDescription *)controlValueDescriptionForEvent:(OEHIDEvent *)event;
{
    return _valueIdentifierToControlValue[@([event controlValueIdentifier])];
}

- (OEControlValueDescription *)controlValueDescriptionForIdentifier:(NSString *)controlIdentifier;
{
    return _identifierToControlValue[controlIdentifier];
}

- (OEControlValueDescription *)controlValueDescriptionForValueIdentifier:(NSNumber *)controlValueIdentifier;
{
    return _valueIdentifierToControlValue[controlValueIdentifier];
}

- (OEControlDescription *)addControlWithIdentifier:(NSString *)identifier name:(NSString *)name event:(OEHIDEvent *)event;
{
    OEControlDescription *desc = [[OEControlDescription alloc] OE_initWithIdentifier:identifier name:name genericEvent:event];
    NSAssert(_controls[[desc identifier]] == nil, @"There is already a control %@ with the identifier %@", _controls[[desc identifier]], identifier);

    [desc setControllerDescription:self];
    _controls[[desc identifier]] = desc;

    return desc;
}

- (OEControlDescription *)addControlWithIdentifier:(NSString *)identifier name:(NSString *)name event:(OEHIDEvent *)event valueRepresentations:(NSDictionary *)valueRepresentations;
{
    OEControlDescription *desc = [self addControlWithIdentifier:identifier name:name event:event];
    [desc setUpControlValuesUsingRepresentations:valueRepresentations];

    return desc;
}


- (void)OE_controlDescription:(OEControlDescription *)control didAddControlValue:(OEControlValueDescription *)valueDesc;
{
    _identifierToControlValue[[valueDesc identifier]] = valueDesc;
    _valueIdentifierToControlValue[[valueDesc valueIdentifier]] = valueDesc;
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
