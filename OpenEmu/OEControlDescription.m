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

#import "OEControlDescription.h"
#import "OEControllerDescription_Internal.h"
#import "OEHIDEvent.h"

static NSString *OEControlGenericIdentifierFromEvent(OEHIDEvent *event)
{
    NSString *ret = @"OEUnknownTypeWithCookie";
    switch([event type])
    {
        case OEHIDEventTypeAxis :
        {
            NSString *dir = @"";
            switch([event direction])
            {
                case OEHIDEventAxisDirectionNegative : dir = @"Negative"; break;
                case OEHIDEventAxisDirectionPositive : dir = @"Positive"; break;
                default : break;
            }

            ret = [NSString stringWithFormat:@"OEGenericControlAxis%@%@", NSStringFromOEHIDEventAxis([event axis]), dir];
        }
            break;
        case OEHIDEventTypeTrigger :
            ret = [NSString stringWithFormat:@"OEGenericControlTrigger%@", NSStringFromOEHIDEventAxis([event axis])];
            break;
        case OEHIDEventTypeButton :
            ret = [NSString stringWithFormat:@"OEGenericControlButton%ld", [event buttonNumber]];
            break;
        case OEHIDEventTypeHatSwitch :
            ret = @"OEGenericControlHatSwitch";
            break;
        default :
            break;
    }

    return [ret stringByAppendingFormat:@"%ld", [event cookie]];
}

@interface OEControlValueDescription ()
- (id)OE_initWithIdentifier:(NSString *)identifier name:(NSString *)name event:(OEHIDEvent *)event __attribute__((objc_method_family(init)));
@property(readwrite, weak) OEControlDescription *controlDescription;
@end

@implementation OEControlDescription

- (id)OE_initWithIdentifier:(NSString *)identifier name:(NSString *)name genericEvent:(OEHIDEvent *)genericEvent
{
    if((self = [super init]))
    {
        _isGenericControl = identifier == nil;
        _genericEvent     = [genericEvent copy];
        _identifier       = [identifier copy] ? : OEControlGenericIdentifierFromEvent(_genericEvent);
        _name             = name              ? : [_genericEvent displayDescription];
    }
    return self;
}

- (void)setUpControlValuesUsingRepresentations:(NSDictionary *)representations;
{
    if(_isGenericControl) [self OE_setUpGenericControlValuesForEvent];
    else [self OE_setUpControlValuesWithRepresentations:representations];

    [_controlValues enumerateObjectsUsingBlock:
     ^(OEControlValueDescription *obj, NSUInteger idx, BOOL *stop)
     {
         [_controllerDescription OE_controlDescription:self didAddControlValue:obj];
     }];
}

- (void)OE_setUpControlValuesWithRepresentations:(NSDictionary *)representations;
{
    NSMutableArray *controlValues = [NSMutableArray array];

    switch([_genericEvent type])
    {
        case OEHIDEventTypeAxis :
        {
            NSAssert([representations count] == 2, @"Incorrect number of axis values, expected 2 got: %@", representations);

            [representations enumerateKeysAndObjectsUsingBlock:
             ^(NSString *identifier, NSDictionary *rep, BOOL *stop)
             {
                 [controlValues addObject:[[OEControlValueDescription alloc] OE_initWithIdentifier:identifier name:rep[@"Name"] event:[_genericEvent axisEventWithDirection:[rep[@"Direction"] integerValue]]]];
             }];
        }
            break;
        case OEHIDEventTypeHatSwitch :
        {
            NSAssert([representations count] == 4, @"Incorrect number of hat switch values, expected 4 got: %@", representations);
            [representations enumerateKeysAndObjectsUsingBlock:
             ^(NSString *identifier, NSDictionary *rep, BOOL *stop)
             {
                 [controlValues addObject:[[OEControlValueDescription alloc] OE_initWithIdentifier:identifier name:rep[@"Name"] event:[_genericEvent hatSwitchEventWithDirection:OEHIDEventHatDirectionFromNSString(rep[@"Direction"])]]];
             }];
        }
            break;
        default :
            NSAssert([representations count] == 0, @"Event type %@ should have no control values, got %@", NSStringFromOEHIDEventType([_genericEvent type]), representations);
            [controlValues addObject:[[OEControlValueDescription alloc] OE_initWithIdentifier:_identifier name:_name event:_genericEvent]];
            break;
    }

    [controlValues setValue:self forKey:@"controlDescription"];
    _controlValues = [controlValues copy];
}

- (void)OE_setUpGenericControlValuesForEvent;
{
    NSMutableArray *controlValues = [NSMutableArray array];

    void (^addEvent)(OEHIDEvent *) =
    ^(OEHIDEvent *event)
    {
        [controlValues addObject:[[OEControlValueDescription alloc] OE_initWithIdentifier:OEControlGenericIdentifierFromEvent(event) name:[event displayDescription] event:event]];
    };

    switch([_genericEvent type])
    {
        case OEHIDEventTypeAxis :
            addEvent([_genericEvent axisEventWithDirection:OEHIDEventAxisDirectionNegative]);
            addEvent([_genericEvent axisEventWithDirection:OEHIDEventAxisDirectionPositive]);
            break;
        case OEHIDEventTypeHatSwitch :
            addEvent([_genericEvent hatSwitchEventWithDirection:OEHIDEventHatDirectionNorth]);
            addEvent([_genericEvent hatSwitchEventWithDirection:OEHIDEventHatDirectionEast]);
            addEvent([_genericEvent hatSwitchEventWithDirection:OEHIDEventHatDirectionSouth]);
            addEvent([_genericEvent hatSwitchEventWithDirection:OEHIDEventHatDirectionWest]);
            break;
        default :
            addEvent(_genericEvent);
            break;
    }

    [controlValues setValue:self forKey:@"controlDescription"];
    _controlValues = [controlValues copy];
}

- (OEControlValueDescription *)addControlValueWithIdentifier:(NSString *)identifier name:(NSString *)name event:(OEHIDEvent *)event
{
    NSAssert([[_controlValues indexesOfObjectsPassingTest:
              ^ BOOL (OEControlValueDescription *obj, NSUInteger idx, BOOL *stop)
              {
                  return [[obj identifier] isEqualToString:identifier] || [[obj event] isEqualToEvent:event];
              }] count] == 0,
             @"Control Description %@ already contains a value with identifier %@ or event %@",
             self, identifier, event);

    NSAssert([event isUsageEqualToEvent:_genericEvent], @"The provided event %@ does not correspond to the even %@ of the receiver %@", event, _genericEvent, self);

    OEControlValueDescription *desc = [[OEControlValueDescription alloc] OE_initWithIdentifier:identifier name:name event:event];
    [desc setControlDescription:self];

    _controlValues = [(_controlValues ? : @[]) arrayByAddingObject:desc];

    [[self controllerDescription] OE_controlDescription:self didAddControlValue:desc];

    return desc;
}

- (NSUInteger)controlIdentifier
{
    return [_genericEvent controlIdentifier];
}

- (OEHIDEventType)type
{
    return [_genericEvent type];
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@ %@ %@>", _identifier, _name, _controlValues];
}

@end

@implementation OEControlValueDescription

- (id)OE_initWithIdentifier:(NSString *)identifier name:(NSString *)name event:(OEHIDEvent *)event
{
    if((self = [super init]))
    {
        _identifier      = [identifier copy] ? : OEControlGenericIdentifierFromEvent(event);
        _name            = [name copy]       ? : [event displayDescription];
        _event           = [event copy];
        _valueIdentifier = @([event controlValueIdentifier]);
    }

    return self;
}

- (OEControlValueDescription *)associatedControlValueDescriptionForEvent:(OEHIDEvent *)anEvent;
{
    OEControlValueDescription *ret = [[[self controlDescription] controllerDescription] controlValueDescriptionForEvent:anEvent];

    NSAssert([ret controlDescription] == [self controlDescription], @"Requested %@ event is not associated with %@", anEvent, [self event]);

    return ret;
}

- (id)representationIdentifier
{
    return [_controlDescription isGenericControl] ? _valueIdentifier : _identifier;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@ %@ %@ %@>", _identifier, _name, _event, _valueIdentifier];
}

@end
