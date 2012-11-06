//
//  OEWiimoteDeviceHandler.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 06.11.12.
//
//

#import "OEWiimoteDeviceHandler.h"
#import "OEHIDManager.h"
#import "OEHIDEvent.h"
#import "NSApplication+OEHIDAdditions.h"
#pragma mark - Debugging -
#define DLogInvalid(){ /* DLog(@"Invalid for %@", NSStringFromClass([self class])); DLog(@"%@", [NSThread callStackSymbols]); */}
#define DLogInvalidFromClass(){ /* DLog(@"Invalid for %@", NSStringFromClass(self)); DLog(@"%@", [NSThread callStackSymbols]);*/}
#define DLogNotImplemented() {/* DLog(@"Not implemented for %@", NSStringFromClass([self class])) */}

@interface OEWiimoteDeviceHandler ()
{
    NSMapTable *eventMap;
}
- (OEHIDEvent*)OE_createEventWithWiiButton:(WiiButtonType)button;
@end
@implementation OEWiimoteDeviceHandler
+ (id)deviceHandlerWithWiimote:(Wiimote*)aWiimote
{
    return [[self alloc] initWithWiimote:aWiimote];
}

- (id)initWithWiimote:(Wiimote*)aWiimote
{
    if((self = [super init]))
    {
        eventMap = [[NSMapTable alloc] initWithKeyOptions:NSPointerFunctionsOpaqueMemory | NSPointerFunctionsIntegerPersonality valueOptions:NSPointerFunctionsObjectPersonality capacity:10];
    }
    return self;
}

+ (id)deviceHandlerWithDevice:(IOHIDDeviceRef)aDevice
{
    // DLogInvalidFromClass();
    return nil;
}

- (id)initWithDevice:(IOHIDDeviceRef)aDevice
{
    return [super initWithDevice:aDevice];
}

- (OEHIDEvent *)eventWithHIDValue:(IOHIDValueRef)aValue
{
    // DLogInvalid();
    return nil;
}
- (void)dispatchEventWithHIDValue:(IOHIDValueRef)aValue
{
    // DLogInvalid();
}
- (io_service_t)serviceRef
{
    // DLogInvalid();
    return 0;
}
#pragma mark -
- (BOOL)isKeyboardDevice
{
    return NO;
}
#pragma mark - Device Info -
- (IOHIDDeviceRef)device
{
    // DLogInvalid();
    return NULL;
}

FIXME(Return dynamic values below)

- (NSUInteger)deviceNumber
{
    return 100;
}

- (CGFloat)deadZone
{
    return 0.2;
}

- (NSString*)serialNumber
{
    // return bt mac address
    return @"bt mac address";
}

- (NSString*)manufacturer
{
    return @"Nintendo";
}

- (NSString*)product
{
    return @"Wiimote";
}

- (NSNumber*)productID
{
    return @(1);
}

- (NSNumber*)locationID
{
    return @(2);
}
#pragma mark - Force Feedback -
- (BOOL)supportsForceFeedback
{
    return NO;
}

- (void)enableForceFeedback
{
    // DLogNotImplemented();
}
- (void)disableForceFeedback
{
    // DLogNotImplemented();
}
#pragma mark - Custom Event Handling -
- (OEHIDEvent*)eventWithWiiButton:(WiiButtonType)button
{
    OEHIDEvent *event = [eventMap objectForKey:@((int)button)];
    if(event == nil)
        event = [self OE_createEventWithWiiButton:button];
    return event;
}

- (OEHIDEvent*)OE_createEventWithWiiButton:(WiiButtonType)button
{
    OEHIDEvent *event = [OEHIDEvent buttonEventWithPadNumber:[self deviceNumber] timestamp:[NSDate timeIntervalSinceReferenceDate] buttonNumber:button state:OEHIDEventStateOff cookie:button];
    [eventMap setObject:event forKey:@((int)button)];
    return event;
}

- (void)dispatchEventWithWiiButton:(WiiButtonType)button state:(BOOL)state
{
    OEHIDEvent *event = [self eventWithWiiButton:button];
    [event setState:state];
    [NSApp postHIDEvent:event];
}
#pragma mark - Wiimote Delegate -
- (void)wiimoteDidConnect:(Wiimote*)theWiimote
{
    // DLog();
}
- (void)wiimoteDidDisconnect:(Wiimote*)theWiimote
{
    // DLog();
    [[OEHIDManager sharedHIDManager] removeDeviceHandler:self];
}

- (void)wiimote:(Wiimote*)theWiimote didNotConnect:(NSError*)err
{
    // DLog();
}
- (void)wiimote:(Wiimote*)theWiimote didNotDisconnect:(NSError*)err
{
    // DLog();
}

- (void)wiimoteReportsExpansionPortChanged:(Wiimote*)theWiimote
{
    // DLog();
}
- (void)wiimote:(Wiimote*)theWiimote reportsButtonChanged:(WiiButtonType)type isPressed:(BOOL)isPressed
{
    [self dispatchEventWithWiiButton:type state:isPressed];
}
- (void)wiimote:(Wiimote*)theWiimote reportsIrPointMovedX:(float)px Y:(float)py
{
    // DLog();
}

- (void)wiimote:(Wiimote*)theWiimote reportsJoystickChanged:(WiiJoyStickType)type tiltX:(unsigned short)tiltX tiltY:(unsigned short)tiltY
{
    // DLog();
}
@end
