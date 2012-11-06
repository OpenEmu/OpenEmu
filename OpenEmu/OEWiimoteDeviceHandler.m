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
#define DLogInvalid(){ NSLog(@"Invalid for %@", NSStringFromClass([self class])); NSLog(@"%@", [NSThread callStackSymbols]); }
#define DLogInvalidFromClass(){ NSLog(@"Invalid for %@", NSStringFromClass(self)); NSLog(@"%@", [NSThread callStackSymbols]);}
#define DLogNotImplemented() { NSLog(@"Not implemented for %@", NSStringFromClass([self class])); }

@interface OEWiimoteDeviceHandler ()
{
    NSMapTable *eventMap;
}
- (OEHIDEvent*)OE_createEventWithWiiButton:(WiiButtonType)button;
@property Wiimote* wiimote;
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
        [self setWiimote:aWiimote];
    }
    return self;
}

+ (id)deviceHandlerWithDevice:(IOHIDDeviceRef)aDevice
{
    DLogInvalidFromClass();
    return nil;
}

- (id)initWithDevice:(IOHIDDeviceRef)aDevice
{
    return [super initWithDevice:aDevice];
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [[self wiimote] disconnect];
}
#pragma mark -
- (OEHIDEvent *)eventWithHIDValue:(IOHIDValueRef)aValue
{
    DLogInvalid();
    return nil;
}

- (void)dispatchEventWithHIDValue:(IOHIDValueRef)aValue
{
    DLogInvalid();
}
- (io_service_t)serviceRef
{
    DLogInvalid();
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
    DLogInvalid();
    return NULL;
}

FIXME(Return dynamic values below)

- (CGFloat)deadZone
{
    return 0.2;
}

- (NSString*)serialNumber
{
    return [[self wiimote] address];
}

- (NSString*)manufacturer
{
    return @"Nintendo";
}

- (NSString*)product
{
    return [[self wiimote] nameOrAddress];
}

- (NSNumber*)productID
{
    return [[self wiimote] productID];
}

- (NSNumber*)locationID
{
    return [[self wiimote] locationID];
}
#pragma mark - Force Feedback -
- (BOOL)supportsForceFeedback
{
    return NO;
}

- (void)enableForceFeedback
{
    DLogNotImplemented();
}
- (void)disableForceFeedback
{
    DLogNotImplemented();
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

- (OEHIDEvent*)eventWithWiiJoystick:(WiiJoyStickType)joystick
{
    return nil;
}
- (void)dispatchEventWithWiiJoystick:(WiiJoyStickType)joystick tiltX:(CGFloat)tiltX tiltY:(CGFloat)tiltY
{
}
#pragma mark - Handling Disconnect -
- (void)wiimoteDidDisconnection:(NSNotification*)notification
{
    [[OEHIDManager sharedHIDManager] removeDeviceHandler:self];
}
@end
