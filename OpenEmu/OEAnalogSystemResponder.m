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
    OESystemKey             *negativeKey       = nil;
    OESystemKey             *positiveKey       = nil;
    OEHIDEventAxisDirection  direction         = [anEvent direction];
    
    negativeKey = [[self keyMap] systemKeyForEvent:[anEvent axisEventWithDirection:OEHIDEventAxisDirectionNegative]];
    positiveKey = [[self keyMap] systemKeyForEvent:[anEvent axisEventWithDirection:OEHIDEventAxisDirectionPositive]];

    NSInteger posValue = MIN([anEvent value], [anEvent maximum]);
    NSInteger negValue = MAX([anEvent value], [anEvent minimum]);
    
    CGFloat posScaler = ((CGFloat)posValue)/[anEvent maximum];
    CGFloat negScaler = ((CGFloat)negValue)/[anEvent minimum];
    
    [self changeAnalogEmulatorKey:positiveKey value:(direction == OEHIDEventAxisDirectionPositive)?posScaler:0.0f];
    [self changeAnalogEmulatorKey:negativeKey value:(direction == OEHIDEventAxisDirectionNegative)?negScaler:0.0f];
}

- (void)changeAnalogEmulatorKey:(OESystemKey *)aKey value:(CGFloat)value
{
    
}

@end
