//
//  OEDeviceDescription.h
//  OpenEmu
//
//  Created by Remy Demarest on 06/01/2013.
//
//

#import <Foundation/Foundation.h>

@class OEDeviceHandler, OEControllerDescription;

@interface OEDeviceDescription : NSObject <NSCopying>

+ (instancetype)deviceDescriptionForDeviceHandler:(OEDeviceHandler *)deviceHandler;

@property(readonly) OEControllerDescription *controllerDescription;

@property(readonly) NSString *name;
@property(readonly) NSUInteger vendorID;
@property(readonly) NSUInteger productID;

@property(readonly) NSString *controllerIdentifier;
@property(readonly) NSString *genericDeviceIdentifier;

@end
