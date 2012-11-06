//
//  OEWiimoteDeviceHandler.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 06.11.12.
//
//

#import <OpenEmuSystem/OpenEmuSystem.h>
#import "Wiimote.h"

@class OEHIDEvent;
@interface OEWiimoteDeviceHandler : OEHIDDeviceHandler <WiimoteDelegate>
+ (id)deviceHandlerWithWiimote:(Wiimote*)aWiimote;
- (id)initWithWiimote:(Wiimote*)aWiimote;

- (OEHIDEvent*)eventWithWiiButton:(WiiButtonType)button;
- (void)dispatchEventWithWiiButton:(WiiButtonType)button state:(BOOL)state;
@end
