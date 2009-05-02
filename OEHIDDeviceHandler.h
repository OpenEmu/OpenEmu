//
//  OEHIDManager.h
//  OpenEmu
//
//  Created by Remy Demarest on 25/02/2009.
//  Copyright 2009 Psycho Inc.. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <IOKit/hid/IOHIDLib.h>
#import <IOKit/hid/IOHIDUsageTables.h>

@class OEHIDEvent;

@interface OEHIDDeviceHandler : NSObject
{
    IOHIDDeviceRef device;
    NSUInteger     deviceNumber;
    CGFloat        deadZone;
}

@property(readonly) IOHIDDeviceRef device;
@property(readonly) NSUInteger     deviceNumber;
@property           CGFloat        deadZone;

@property(readonly) NSString *manufacturer;
@property(readonly) NSString *product;
@property(readonly) NSNumber *productID;
@property(readonly) NSNumber *locationID;

+ (id)deviceHandlerWithDevice:(IOHIDDeviceRef)aDevice;

- (id)initWithDevice:(IOHIDDeviceRef)aDevice;
- (OEHIDEvent *)eventWithHIDValue:(IOHIDValueRef)aValue;
- (void)dispatchEventWithHIDValue:(IOHIDValueRef)aValue;

@end
