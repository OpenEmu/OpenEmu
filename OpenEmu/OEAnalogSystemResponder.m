//
//  OEAnalogSystemResponder.m
//  OpenEmu
//
//  Created by Joshua Weinberg on 12/25/12.
//
//

#import "OEAnalogSystemResponder.h"

@implementation OEAnalogSystemResponder

- (void)axisMoved:(OEHIDEvent *)anEvent
{
    OESystemKey             *negativeKey = nil;
    OESystemKey             *positiveKey = nil;
    OEHIDEventAxisDirection  direction   = [anEvent direction];
    
    negativeKey = [[self keyMap] systemKeyForEvent:[anEvent axisEventWithDirection:OEHIDEventAxisDirectionNegative]];
    positiveKey = [[self keyMap] systemKeyForEvent:[anEvent axisEventWithDirection:OEHIDEventAxisDirectionPositive]];

    FIXME("We shouldn't need to call this method twice in a row to support the feature, find a better to deal with it.");
    CGFloat value = [anEvent value];
    [self changeAnalogEmulatorKey:positiveKey value:direction == OEHIDEventAxisDirectionPositive ? value : 0.0f];
    [self changeAnalogEmulatorKey:negativeKey value:direction == OEHIDEventAxisDirectionNegative ? value : 0.0f];
}

- (void)changeAnalogEmulatorKey:(OESystemKey *)aKey value:(CGFloat)value
{
    
}

@end
