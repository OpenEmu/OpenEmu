//
//  OEWiimoteDeviceHandler.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 06.11.12.
//
//

#import <OpenEmuSystem/OpenEmuSystem.h>
#import "Wiimote.h"
#import "WiimoteTypes.h"

@class OEHIDEvent, Wiimote;

@interface OEWiimoteDeviceHandler : OEHIDDeviceHandler

+ (id)deviceHandlerWithWiimote:(Wiimote *)aWiimote;
- (id)initWithWiimote:(Wiimote *)aWiimote;

- (OEHIDEvent *)eventWithWiiButton:(WiiButtonType)button;
- (void)dispatchEventWithWiiButton:(WiiButtonType)button state:(BOOL)state;

- (OEHIDEvent *)eventWithWiiJoystick:(WiiJoyStickType)joystick;
- (void)dispatchEventWithWiiJoystick:(WiiJoyStickType)joystick tiltX:(CGFloat)tiltX tiltY:(CGFloat)tiltY;
- (void)dispatchEventWithWiiTrigger:(WiiTriggerType)trigger value:(CGFloat)triggerVal;

@end
