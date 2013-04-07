//
//  OEXBox360HIDDeviceHander.m
//  OpenEmu
//
//  Created by Joshua Weinberg on 12/30/12.
//
//

#import "OEXBox360HIDDeviceHander.h"

@interface OEDeviceHandler ()
@property(readwrite) NSUInteger deviceNumber;
@end

@implementation OEXBox360HIDDeviceHander

- (void)setDeviceNumber:(NSUInteger)deviceNumber
{
    // see: http://tattiebogle.net/index.php/ProjectRoot/Xbox360Controller/UsbInfo#toc3
    [super setDeviceNumber:deviceNumber];

    NSUInteger pattern = deviceNumber + 0x6;
    
    IOHIDDeviceSetReport([self device],
                         kIOHIDReportTypeOutput,
                         0x0,
                         (uint8_t[]){ 0x1, 0x3, pattern },
                         3);
}

@end
