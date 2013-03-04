//
//  OEControllerDescription_Internal.h
//  OpenEmu
//
//  Created by Remy Demarest on 06/01/2013.
//
//

#import "OEControllerDescription.h"
#import "OEControlDescription.h"
#import "OEDeviceDescription.h"

@interface OEControllerDescription ()
+ (OEDeviceDescription *)OE_deviceDescriptionForDeviceHandler:(OEDeviceHandler *)deviceHandler;
@end

@interface OEDeviceDescription ()
- (id)OE_initWithRepresentation:(NSDictionary *)representation __attribute__((objc_method_family(init)));
@property(readwrite) OEControllerDescription *controllerDescription;
@end

@interface OEControlDescription ()
- (id)OE_initWithIdentifier:(NSString *)identifier representation:(NSDictionary *)representation genericEvent:(OEHIDEvent *)genericEvent __attribute__((objc_method_family(init)));
@property(readwrite) OEControllerDescription *controllerDescription;
@end
