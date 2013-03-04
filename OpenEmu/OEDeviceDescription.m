//
//  OEDeviceDescription.m
//  OpenEmu
//
//  Created by Remy Demarest on 06/01/2013.
//
//

#import "OEDeviceDescription.h"
#import "OEControllerDescription_Internal.h"

@implementation OEDeviceDescription

+ (instancetype)deviceDescriptionForDeviceHandler:(OEDeviceHandler *)deviceHandler;
{
    return [OEControllerDescription OE_deviceDescriptionForDeviceHandler:deviceHandler];
}

- (id)OE_initWithRepresentation:(NSDictionary *)representation;
{
    if((self = [super init]))
    {
        _name = [representation[@"OEControllerDeviceName"] copy];
        _vendorID = [representation[@"OEControllerVendorID"] integerValue];
        _productID = [representation[@"OEControllerProductID"] integerValue];
        _genericDeviceIdentifier = [NSString stringWithFormat:@"OEGenericDeviceIdentifier_%ld_%ld", _vendorID, _productID];
    }

    return self;
}

- (id)copyWithZone:(NSZone *)zone
{
    return self;
}

- (NSUInteger)hash;
{
    return _vendorID << 32 | _productID;
}

- (BOOL)isEqual:(OEDeviceDescription *)object;
{
    if(self == object) return YES;

    if([object isKindOfClass:[OEDeviceDescription class]])
        return _vendorID == object->_vendorID && _productID == object->_productID;

    return NO;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@ %@ %ld %ld>", _name, [[self controllerDescription] name], _vendorID, _productID];
}

- (NSString *)controllerIdentifier
{
    return [[self controllerDescription] identifier];
}

@end
