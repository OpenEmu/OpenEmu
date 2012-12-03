//
//  OEWiimoteDeviceHandler.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 06.11.12.
//
//

TODO("Remove this code where we are sure that we don't need it anymore.");
#if 0
#import <OpenEmuSystem/OpenEmuSystem.h>
#import "Wiimote.h"
#import "WiimoteTypes.h"

@class OEHIDEvent, Wiimote;

@interface OEWiimoteDeviceHandler_old : OEDeviceHandler

+ (id)deviceHandlerWithWiimote:(Wiimote *)aWiimote;
- (id)initWithWiimote:(Wiimote *)aWiimote;

- (OEHIDEvent *)eventWithWiiButton:(WiiButtonType)button;
- (void)dispatchEventWithWiiButton:(WiiButtonType)button state:(BOOL)state;

- (OEHIDEvent *)eventWithWiiJoystick:(WiiJoyStickType)joystick;
- (void)dispatchEventWithWiiJoystick:(WiiJoyStickType)joystick tiltX:(CGFloat)tiltX tiltY:(CGFloat)tiltY;
- (void)dispatchEventWithWiiTrigger:(WiiTriggerType)trigger value:(CGFloat)triggerVal;

@end
#endif
