//
//  OEControlDescription.m
//  OpenEmu
//
//  Created by Remy Demarest on 06/01/2013.
//
//

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

@interface OEControlDescription ()
{
    OEHIDEvent *_genericEvent;
}

@end

@implementation OEControlDescription

- (id)OE_initWithIdentifier:(NSString *)identifier representation:(NSDictionary *)representation genericEvent:(OEHIDEvent *)genericEvent
{
    if((self = [super init]))
    {
        NSAssert((identifier == nil) == (representation == nil), @"Either identifier and representation are both nil or neither are, you screwed up, dude!");

        _isGenericControl = identifier == nil;
        _genericEvent = [genericEvent copy];
        _identifier = [identifier copy] ? : OEControlGenericIdentifierFromEvent(_genericEvent);
        _name = representation[@"Name"] ? : [_genericEvent displayDescription];

        if(identifier == nil) [self OE_setupGenericControlValuesForEvent];
        else [self OE_setupControlValuesWithRepresentations:representation[@"Values"]];
    }

    return self;
}

- (void)OE_setupControlValuesWithRepresentations:(NSDictionary *)representations;
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

- (void)OE_setupGenericControlValuesForEvent;
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
        _identifier      = [identifier copy];
        _name            = [name copy];
        _event           = [event copy];
        _valueIdentifier = @([event genericIdentifier]);
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
    