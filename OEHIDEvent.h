//
//  OEHIDEvent.h
//  OpenEmu
//
//  Created by Remy Demarest on 25/02/2009.
//  Copyright 2009 Psycho Inc.. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <IOKit/hid/IOHIDLib.h>

#define OEGlobalEventsKey @"OEGlobalEventsKey"

typedef enum OEHIDEventAxis {
    OEHIDEventAxisNone = 0x00,
    OEHIDEventAxisX    = 0x30,
    OEHIDEventAxisY    = 0x31,
    OEHIDEventAxisZ    = 0x32,
    OEHIDEventAxisRx   = 0x33,
    OEHIDEventAxisRy   = 0x34,
    OEHIDEventAxisRz   = 0x35
} OEHIDEventAxis;

    
// Dummy class to easily dispatch an event as an action through the responder chains.
@interface OEHIDEvent : NSObject <NSCoding>
{
    IOHIDDeviceRef device;
    NSUInteger buttonNumber;
    OEHIDEventAxis axis;
    CGFloat value;
}

- (NSString *)axisName;

+ (id)eventWithDevice:(IOHIDDeviceRef)aDevice page:(uint32_t)aPage usage:(uint32_t)aUsage value:(CGFloat)aValue;
- (id)initWithDevice:(IOHIDDeviceRef)aDevice page:(uint32_t)aPage usage:(uint32_t)aUsage value:(CGFloat)aValue;
@property(readonly) IOHIDDeviceRef device;
@property(readonly) NSUInteger buttonNumber;
@property(readonly) OEHIDEventAxis axis;
@property(readonly) CGFloat value;
@end
