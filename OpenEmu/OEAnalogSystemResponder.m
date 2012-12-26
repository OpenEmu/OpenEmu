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

    [self changeAnalogEmulatorKey:positiveKey value:(direction == OEHIDEventAxisDirectionPositive)?((CGFloat)[anEvent value])/[anEvent maximum]:0.0f];
    [self changeAnalogEmulatorKey:negativeKey value:(direction == OEHIDEventAxisDirectionNegative)?((CGFloat)[anEvent value])/[anEvent minimum]:0.0f];
}

- (void)changeAnalogEmulatorKey:(OESystemKey *)aKey value:(CGFloat)value
{
    
}

@end
