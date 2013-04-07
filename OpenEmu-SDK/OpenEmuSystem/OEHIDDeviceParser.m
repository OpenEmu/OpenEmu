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

#import "OEHIDDeviceParser.h"

#import "OEControllerDescription.h"
#import "OEControlDescription.h"
#import "OEControlDescription.h"
#import "OEDeviceDescription.h"
#import "OEMultiHIDDeviceHandler.h"
#import "OEPS3HIDDeviceHandler.h"
#import "OEXBox360HIDDeviceHander.h"
#import "OEWiimoteHIDDeviceHandler.h"
#import "OEControllerDescription_Internal.h"

#define ELEM(e) ((__bridge IOHIDElementRef)e)
#define ELEM_TO_VALUE(e) ([NSValue valueWithPointer:e])
#define VALUE_TO_ELEM(e) ((IOHIDElementRef)[e pointerValue])

@interface OEHIDEvent ()
+ (instancetype)OE_eventWithElement:(IOHIDElementRef)element value:(NSInteger)value;
@end

static BOOL OE_isWiimoteControllerName(NSString *name)
{
    return [name hasPrefix:@"Nintendo RVL-CNT-01"];
}

static BOOL OE_isPS3ControllerName(NSString *name)
{
    return [name hasPrefix:@"PLAYSTATION(R)3 Controller"];
}

static BOOL OE_isXboxControllerName(NSString *name)
{
    return [name isEqualToString:@"Controller"];
}

@interface _OEHIDDeviceAttributes : NSObject

- (id)initWithDeviceHandlerClass:(Class)handlerClass;

@property(readonly) Class deviceHandlerClass;

- (void)applyAttributesToElement:(IOHIDElementRef)element;
- (void)setAttributes:(NSDictionary *)attributes forElementCookie:(NSUInteger)cookie;

@property(nonatomic, copy) NSDictionary *subdeviceIdentifiersToDeviceDescriptions;

@end

@interface OEHIDDeviceParser ()
{
    NSMutableDictionary *_controllerDescriptionsToDeviceAttributes;
}
@end

@interface _OEHIDDeviceElementTree : NSObject

- (id)initWithHIDDevice:(IOHIDDeviceRef)device;

- (NSUInteger)numberOfChildrenOfElement:(IOHIDElementRef)element;
- (NSArray *)childrenOfElement:(IOHIDElementRef)element;
- (void)enumerateChildrenOfElement:(IOHIDElementRef)element usingBlock:(void(^)(IOHIDElementRef element, BOOL *stop))block;

@end

@implementation OEHIDDeviceParser

- (id)init
{
    if((self = [super init]))
    {
        _controllerDescriptionsToDeviceAttributes = [[NSMutableDictionary alloc] init];
    }
    return self;
}

- (OEDeviceDescription *)OE_deviceDescriptionForIOHIDDevice:(IOHIDDeviceRef)device
{
    return [OEDeviceDescription deviceDescriptionForVendorID:[(__bridge NSNumber *)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDVendorIDKey)) integerValue]
                                                   productID:[(__bridge NSNumber *)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductIDKey)) integerValue]
                                                        name:(__bridge NSString *)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductKey))];
}

- (Class)OE_deviceHandlerClassForIOHIDDevice:(IOHIDDeviceRef)aDevice
{
    NSString *deviceName = (__bridge id)IOHIDDeviceGetProperty(aDevice, CFSTR(kIOHIDProductKey));

    if(OE_isWiimoteControllerName(deviceName))
        return [OEWiimoteHIDDeviceHandler class];
    else if(OE_isPS3ControllerName(deviceName))
        return [OEPS3HIDDeviceHandler class];
    else if(OE_isXboxControllerName(deviceName))
        return [OEXBox360HIDDeviceHander class];

    return [OEHIDDeviceHandler class];
}

- (OEHIDDeviceHandler *)deviceHandlerForIOHIDDevice:(IOHIDDeviceRef)device;
{
    Class deviceHandlerClass = [self OE_deviceHandlerClassForIOHIDDevice:device];
    id<OEHIDDeviceParser> parser = [deviceHandlerClass deviceParser];

    if(parser != self) return [parser deviceHandlerForIOHIDDevice:device];

    return [self OE_parseIOHIDDevice:device];
}

- (void)OE_setUpElementsOfIOHIDDevice:(IOHIDDeviceRef)device withAttributes:(NSDictionary *)elementAttributes
{
    NSArray *allElements = (__bridge_transfer NSArray *)IOHIDDeviceCopyMatchingElements(device, NULL, 0);

    for(id e in allElements)
    {
        IOHIDElementRef elem = (__bridge IOHIDElementRef)e;

        NSDictionary *attributes = elementAttributes[@(IOHIDElementGetCookie(elem))];

        [attributes enumerateKeysAndObjectsUsingBlock:
         ^(NSString *key, id attribute, BOOL *stop)
         {
             IOHIDElementSetProperty(elem, (__bridge CFStringRef)key, (__bridge CFTypeRef)attribute);
         }];
    }
}

- (OEHIDDeviceHandler *)OE_parseIOHIDDevice:(IOHIDDeviceRef)device
{
    OEDeviceDescription *deviceDesc = [self OE_deviceDescriptionForIOHIDDevice:device];
    OEControllerDescription *controllerDesc = [deviceDesc controllerDescription];

    _OEHIDDeviceAttributes *attributes = _controllerDescriptionsToDeviceAttributes[controllerDesc];

    if(attributes == nil)
    {
        attributes = [self OE_deviceAttributesForIOHIDDevice:device deviceDescription:deviceDesc];
        _controllerDescriptionsToDeviceAttributes[controllerDesc] = attributes;
    }

    OEHIDDeviceHandler *handler = nil;
    if([[attributes subdeviceIdentifiersToDeviceDescriptions] count] != 0)
        handler = [[[attributes deviceHandlerClass] alloc] initWithIOHIDDevice:device deviceDescription:deviceDesc subdeviceDescriptions:[attributes subdeviceIdentifiersToDeviceDescriptions]];
    else
        handler = [[[attributes deviceHandlerClass] alloc] initWithIOHIDDevice:device deviceDescription:deviceDesc];

    return handler;
}

- (_OEHIDDeviceAttributes *)OE_deviceAttributesForIOHIDDevice:(IOHIDDeviceRef)device deviceDescription:(OEDeviceDescription *)deviceDescription
{
    NSDictionary *representation = [OEControllerDescription OE_dequeueRepresentationForDeviceDescription:deviceDescription];

    _OEHIDDeviceAttributes *attributes = nil;
    if(representation != nil)
        attributes = [self OE_deviceAttributesForKnownIOHIDDevice:device deviceDescription:deviceDescription representations:representation];
    else
        attributes = [self OE_deviceAttributesForUnknownIOHIDDevice:device deviceDescription:deviceDescription];

    return attributes;
}

- (_OEHIDDeviceAttributes *)OE_deviceAttributesForKnownIOHIDDevice:(IOHIDDeviceRef)device deviceDescription:(OEDeviceDescription *)deviceDesc representations:(NSDictionary *)controlRepresentations
{
    OEControllerDescription *controllerDesc = [deviceDesc controllerDescription];

    _OEHIDDeviceAttributes *attributes = [[_OEHIDDeviceAttributes alloc] initWithDeviceHandlerClass:[self OE_deviceHandlerClassForIOHIDDevice:device]];

    NSMutableArray *genericDesktopElements = [(__bridge_transfer NSArray *)IOHIDDeviceCopyMatchingElements(device, (__bridge CFDictionaryRef)@{ @kIOHIDElementUsagePageKey : @(kHIDPage_GenericDesktop) }, 0) mutableCopy];
    NSMutableArray *buttonElements = [(__bridge_transfer NSArray *)IOHIDDeviceCopyMatchingElements(device, (__bridge CFDictionaryRef)@{ @kIOHIDElementUsagePageKey : @(kHIDPage_Button) }, 0) mutableCopy];

    [controlRepresentations enumerateKeysAndObjectsUsingBlock:
     ^(NSString *identifier, NSDictionary *rep, BOOL *stop)
     {
         OEHIDEventType type = OEHIDEventTypeFromNSString(rep[@"Type"]);
         NSUInteger cookie = [rep[@"Cookie"] integerValue];
         NSUInteger usage = OEUsageFromUsageStringWithType(rep[@"Usage"], type);

         __block IOHIDElementRef elem = NULL;

         // Find the element for the current description.
         NSMutableArray *targetArray = type == OEHIDEventTypeButton ? buttonElements : genericDesktopElements;
         [targetArray enumerateObjectsUsingBlock:
          ^(id obj, NSUInteger idx, BOOL *stop)
          {
              IOHIDElementRef testedElement = (__bridge IOHIDElementRef)obj;

              if((cookie != OEUndefinedCookie && cookie != IOHIDElementGetCookie(testedElement))
                 || usage != IOHIDElementGetUsage(testedElement))
                  return;

              elem = testedElement;
              // Make sure you stop enumerating right after modifying the array
              // or else it will throw an exception.
              [targetArray removeObjectAtIndex:idx];
              *stop = YES;
          }];

         if(elem == NULL) return;

         cookie = IOHIDElementGetCookie(elem);

         // Create attributes for the element if necessary. We need apply the attributes on
         // the elements because OEHIDEvent depend on them to setup the event.
         switch(type)
         {
             case OEHIDEventTypeTrigger :
                 [attributes setAttributes:@{ @kOEHIDElementIsTriggerKey : @YES } forElementCookie:cookie];
                 [attributes applyAttributesToElement:elem];
                 break;
             case OEHIDEventTypeHatSwitch :
                 [attributes setAttributes:@{ @kOEHIDElementHatSwitchTypeKey : @([self OE_hatSwitchTypeForElement:elem]) } forElementCookie:cookie];
                 [attributes applyAttributesToElement:elem];
                 break;
             default :
                 break;
         }

         // Attempt to create an event for it, dump it if it's not possible.
         OEHIDEvent *genericEvent = [OEHIDEvent OE_eventWithElement:elem value:0];
         if(genericEvent == nil) return;

         // Add the control description.
         [controllerDesc addControlWithIdentifier:identifier name:rep[@"Name"] event:genericEvent valueRepresentations:rep[@"Values"]];

     }];

    [genericDesktopElements enumerateObjectsWithOptions:NSEnumerationConcurrent | NSEnumerationReverse usingBlock:
     ^(id elem, NSUInteger idx, BOOL *stop)
     {
         if([OEHIDEvent OE_eventWithElement:(__bridge IOHIDElementRef)elem value:0] == nil)
             [genericDesktopElements removeObjectAtIndex:idx];
     }];

    if([genericDesktopElements count] > 0)
        NSLog(@"WARNING: There are %ld generic desktop elements unaccounted for in %@", [genericDesktopElements count], [deviceDesc name]);

    if([buttonElements count] > 0)
        NSLog(@"WARNING: There are %ld button elements unaccounted for.", [buttonElements count]);

    return attributes;
}

- (_OEHIDDeviceAttributes *)OE_deviceAttributesForUnknownIOHIDDevice:(IOHIDDeviceRef)device deviceDescription:(OEDeviceDescription *)deviceDesc
{
    OEControllerDescription *controllerDesc = [deviceDesc controllerDescription];

    _OEHIDDeviceElementTree *tree = [[_OEHIDDeviceElementTree alloc] initWithHIDDevice:device];

    NSMutableArray *rootJoysticks = [NSMutableArray array];
    [tree enumerateChildrenOfElement:nil usingBlock:
     ^(IOHIDElementRef element, BOOL *stop)
     {
         if(IOHIDElementGetUsagePage(element) == kHIDPage_GenericDesktop &&
            IOHIDElementGetUsage(element) == kHIDUsage_GD_Joystick)
             [rootJoysticks addObject:ELEM_TO_VALUE(element)];
     }];

    if([rootJoysticks count] == 0)
        return nil;

    if([rootJoysticks count] == 1)
    {
        _OEHIDDeviceAttributes *attributes = [[_OEHIDDeviceAttributes alloc] initWithDeviceHandlerClass:[OEHIDDeviceHandler class]];

        [self OE_parseJoystickElement:(__bridge IOHIDElementRef)rootJoysticks[0] intoControllerDescription:controllerDesc attributes:attributes deviceIdentifier:nil usingElementTree:tree];

        return attributes;
    }

    _OEHIDDeviceAttributes *attributes = [[_OEHIDDeviceAttributes alloc] initWithDeviceHandlerClass:[OEMultiHIDDeviceHandler class]];

    const NSUInteger subdeviceVendorID = [deviceDesc vendorID] << 32;
    const NSUInteger subdeviceProductIDBase = [deviceDesc productID] << 32;
    NSUInteger lastDeviceIndex = 0;

    NSMutableDictionary *deviceIdentifiers = [[NSMutableDictionary alloc] initWithCapacity:[rootJoysticks count]];

    for(id e in rootJoysticks)
    {
        id deviceIdentifier = @(++lastDeviceIndex);

        OEDeviceDescription *subdeviceDesc = [OEDeviceDescription deviceDescriptionForVendorID:subdeviceVendorID productID:subdeviceProductIDBase | lastDeviceIndex name:[[controllerDesc name] stringByAppendingFormat:@" %@", deviceIdentifier]];

        [self OE_parseJoystickElement:VALUE_TO_ELEM(e) intoControllerDescription:[subdeviceDesc controllerDescription] attributes:attributes deviceIdentifier:deviceIdentifier usingElementTree:tree];

        deviceIdentifiers[deviceIdentifier] = subdeviceDesc;
    }

    [attributes setSubdeviceIdentifiersToDeviceDescriptions:deviceIdentifiers];

    return attributes;
}

- (void)OE_parseJoystickElement:(IOHIDElementRef)rootElement intoControllerDescription:(OEControllerDescription *)desc attributes:(_OEHIDDeviceAttributes *)attributes deviceIdentifier:(id)deviceIdentifier usingElementTree:(_OEHIDDeviceElementTree *)elementTree
{
    NSMutableArray *buttonElements      = [NSMutableArray array];
    NSMutableArray *hatSwitchElements   = [NSMutableArray array];
    NSMutableArray *groupedAxisElements = [NSMutableArray array];

    NSMutableArray *posNegAxisElements  = [NSMutableArray array];
    NSMutableArray *posAxisElements     = [NSMutableArray array];

    // Adds the element to the right list based on its usage.
    void (^addElement)(IOHIDElementRef element) =
    ^(IOHIDElementRef element)
    {
        id elem = (__bridge id)element;
        NSUInteger page = IOHIDElementGetUsagePage(element);

        switch(page)
        {
            case kHIDPage_GenericDesktop :
            {
                NSUInteger usage = IOHIDElementGetUsage(element);
                if(usage == kHIDUsage_GD_Hatswitch)
                    [hatSwitchElements addObject:elem];
                else if(kHIDUsage_GD_X <= usage && usage <= kHIDUsage_GD_Rz)
                {
                    // Postpone setup of these elements since they might be triggers.
                    CFIndex minimum = IOHIDElementGetLogicalMin(element);
                    CFIndex maximum = IOHIDElementGetLogicalMax(element);

                    if(minimum == 0) [posAxisElements addObject:elem];
                    else if(minimum < 0 && maximum > 0) [posNegAxisElements addObject:elem];
                }
            }
                break;
            case kHIDPage_Button :
                [buttonElements addObject:elem];
                break;
            default :
                break;
        }
    };

    // Enumerate children elements to move them into the right lists.
    [elementTree enumerateChildrenOfElement:rootElement usingBlock:
     ^(IOHIDElementRef element, BOOL *stop)
     {
         if(IOHIDElementGetType(element) != kIOHIDElementTypeCollection
            || IOHIDElementGetUsagePage(element) != kHIDPage_GenericDesktop)
             return;

         [elementTree enumerateChildrenOfElement:element usingBlock:
          ^(IOHIDElementRef element, BOOL *stop)
          {
              if(IOHIDElementGetType(element) == kIOHIDElementTypeCollection)
              {
                  NSArray *children = [elementTree childrenOfElement:element];
                  BOOL foundNonAxisElement =
                  [children indexOfObjectPassingTest:
                   ^ BOOL (id obj, NSUInteger idx, BOOL *stop)
                   {
                       return (IOHIDElementGetUsagePage(element) != kHIDPage_GenericDesktop
                               || IOHIDElementGetUsage(element) < kHIDUsage_GD_X
                               || IOHIDElementGetUsage(element) > kHIDUsage_GD_Rz);
                   }] != NSNotFound;

                  if(!foundNonAxisElement) [groupedAxisElements addObjectsFromArray:children];
                  else for(id e in children)
                      addElement(ELEM(e));
              }
              else addElement(element);
          }];
     }];

    // Setup HatSwitch element attributes and create a control in the controller description.
    for(id e in hatSwitchElements)
    {
        IOHIDElementRef elem = ELEM(e);

        NSDictionary *attr = [NSDictionary dictionaryWithObjectsAndKeys:
                              @([self OE_hatSwitchTypeForElement:elem]), @kOEHIDElementHatSwitchTypeKey,
                              deviceIdentifier, @kOEHIDElementDeviceIdentifierKey,
                              nil];

        [attributes setAttributes:attr forElementCookie:IOHIDElementGetCookie(elem)];
        [attributes applyAttributesToElement:elem];

        OEHIDEvent *genericEvent = [OEHIDEvent OE_eventWithElement:elem value:0];
        if(genericEvent != nil) [desc addControlWithIdentifier:nil name:nil event:genericEvent];
    }

    // Setup events that only have the device identifier as attribute.
    void(^setUpControlsInArray)(NSArray *) =
    ^(NSArray *elements)
    {
        for(id e in elements)
        {
            IOHIDElementRef elem = ELEM(e);
            OEHIDEvent *genericEvent = [OEHIDEvent OE_eventWithElement:elem value:0];
            if(genericEvent == nil) continue;

            if(deviceIdentifier != nil)
            {
                [attributes setAttributes:@{ @kOEHIDElementDeviceIdentifierKey : deviceIdentifier } forElementCookie:IOHIDElementGetCookie(elem)];
                [attributes applyAttributesToElement:elem];
            }
            [desc addControlWithIdentifier:nil name:nil event:genericEvent];
        }
    };

    // We assume that axis events that have only positive values when
    // other axis are grouped or have positive and negative values.
    if(([posNegAxisElements count] + [groupedAxisElements count]) != 0 && [posAxisElements count] != 0)
    {
        for(id e in posAxisElements)
        {
            IOHIDElementRef elem = ELEM(e);

            NSDictionary *attr = [NSDictionary dictionaryWithObjectsAndKeys:
                                  @YES, @kOEHIDElementIsTriggerKey,
                                  deviceIdentifier, @kOEHIDElementDeviceIdentifierKey,
                                  nil];

            [attributes setAttributes:attr forElementCookie:IOHIDElementGetCookie(elem)];
            [attributes applyAttributesToElement:elem];

            OEHIDEvent *genericEvent = [OEHIDEvent OE_eventWithElement:elem value:0];
            if(genericEvent != nil) [desc addControlWithIdentifier:nil name:nil event:genericEvent];
        }
    }
    else setUpControlsInArray(posAxisElements);

    setUpControlsInArray(buttonElements);
    setUpControlsInArray(groupedAxisElements);
    setUpControlsInArray(posNegAxisElements);
}

- (OEHIDEventHatSwitchType)OE_hatSwitchTypeForElement:(IOHIDElementRef)element
{
    NSInteger count = IOHIDElementGetLogicalMax(element) - IOHIDElementGetLogicalMin(element) + 1;
    OEHIDEventHatSwitchType type = OEHIDEventHatSwitchTypeUnknown;
    switch(count)
    {
        case 4 : type = OEHIDEventHatSwitchType4Ways; break;
        case 8 : type = OEHIDEventHatSwitchType8Ways; break;
    }
    return type;
}

@end

@implementation _OEHIDDeviceAttributes
{
    NSMutableDictionary *_elementAttributes;
}

- (id)init
{
    return nil;
}

- (id)initWithDeviceHandlerClass:(Class)handlerClass;
{
    if((self = [super init]))
    {
        _deviceHandlerClass = handlerClass;
        _elementAttributes = [[NSMutableDictionary alloc] init];
    }
    return self;
}

- (void)applyAttributesToElement:(IOHIDElementRef)element;
{
    NSDictionary *attributes = _elementAttributes[@(IOHIDElementGetCookie(element))];
    [attributes enumerateKeysAndObjectsUsingBlock:
     ^(NSString *key, id attribute, BOOL *stop)
     {
         IOHIDElementSetProperty(element, (__bridge CFStringRef)key, (__bridge CFTypeRef)attribute);
     }];
}

- (void)setAttributes:(NSDictionary *)attributes forElementCookie:(NSUInteger)cookie;
{
    _elementAttributes[@(cookie)] = [attributes copy];
}

@end

@implementation _OEHIDDeviceElementTree
{
    IOHIDDeviceRef _device;
    CFArrayRef _elements;
    NSDictionary *_elementTree;
}

- (void)dealloc
{
    CFRelease(_device);
    CFRelease(_elements);
}

- (id)init
{
    return nil;
}

- (id)initWithHIDDevice:(IOHIDDeviceRef)device;
{
    if((self = [super init]))
    {
        _device = (IOHIDDeviceRef)CFRetain(device);
        _elements = IOHIDDeviceCopyMatchingElements(_device, NULL, 0);

        NSMutableDictionary *elementTree = [NSMutableDictionary dictionary];
        for(id e in (__bridge NSArray *)_elements)
        {
            IOHIDElementRef elem = ELEM(e);
            IOHIDElementRef parent = IOHIDElementGetParent(elem);

            elementTree[ELEM_TO_VALUE(elem)] = ELEM_TO_VALUE(parent);
        }
        
        _elementTree = [elementTree copy];
    }
    return self;
}

- (NSArray *)childrenOfElement:(IOHIDElementRef)element
{
    NSArray *children = [_elementTree allKeysForObject:ELEM_TO_VALUE(element)];

    return [children sortedArrayUsingComparator:
            ^ NSComparisonResult (id obj1, id obj2)
            {
                return [@(IOHIDElementGetCookie(VALUE_TO_ELEM(obj1))) compare:@(IOHIDElementGetCookie(VALUE_TO_ELEM(obj2)))];
            }];
}

- (NSUInteger)numberOfChildrenOfElement:(IOHIDElementRef)element;
{
    return [[_elementTree allKeysForObject:ELEM_TO_VALUE(element)] count];
}

- (void)enumerateChildrenOfElement:(IOHIDElementRef)element usingBlock:(void(^)(IOHIDElementRef element, BOOL *stop))block;
{
    [[self childrenOfElement:element] enumerateObjectsUsingBlock:
     ^(id obj, NSUInteger idx, BOOL *stop)
     {
         block(VALUE_TO_ELEM(obj), stop);
     }];
}

- (NSString *)description
{
    NSMutableString *string = [NSMutableString stringWithFormat:@"<%@ %p {", [self class], self];
    [_elementTree enumerateKeysAndObjectsUsingBlock:
     ^(NSValue *key, NSValue *obj, BOOL *stop)
     {
         [string appendFormat:@"\t%p --> %p\n", [obj pointerValue], [key pointerValue]];
     }];
    [string appendString:@"}>"];
    return string;
}

@end
