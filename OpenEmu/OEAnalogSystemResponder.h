//
//  OEAnalogSystemResponder.h
//  OpenEmu
//
//  Created by Joshua Weinberg on 12/25/12.
//
//

#import <OpenEmuSystem/OpenEmuSystem.h>

@interface OEAnalogSystemResponder : OEBasicSystemResponder
- (void)changeAnalogEmulatorKey:(OESystemKey *)aKey value:(CGFloat)value;
@end
