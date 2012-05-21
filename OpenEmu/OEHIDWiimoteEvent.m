//
//  OEHIDWiimoteEvent.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 21.05.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "OEHIDWiimoteEvent.h"
#import "OEWiimoteHandler.h"

@implementation OEHIDWiimoteEvent
- (NSString*)stringForButtonNumber:(NSUInteger)buttonNumber
{
    switch (buttonNumber) {
        case 0: return @"A";
        case 1: return @"B";
        case 2: return @"1";
        case 3: return @"2";
        case 4: return @"-";
        case 5: return @"Home";
        case 6: return @"+";
            
        case 7: return @"Up";
        case 8: return @"Down";
        case 9: return @"Left";
        case 10: return @"Right";
            
        default:
            break;
    }
    return [NSString stringWithFormat:@"%ld", buttonNumber];
}
- (NSString*)displayDescription
{
    return [NSString stringWithFormat:@"Wiimote %ld %@", [self padNumber]-WiimoteBasePadNumber+1, [self stringForButtonNumber:[self buttonNumber]]];
}
@end
