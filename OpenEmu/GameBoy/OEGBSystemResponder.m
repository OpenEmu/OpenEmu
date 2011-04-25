//
//  OESMSSystemResponder.m
//  OpenEmu
//
//  Created by Remy Demarest on 23/04/2011.
//  Copyright 2011 NuLayer Inc. All rights reserved.
//

#import "OEGBSystemResponder.h"

NSString *OEGBButtonNameTable[] =
{
    @"OEGBButtonA",
    @"OEGBButtonB",
    @"OEGBButtonUp",
    @"OEGBButtonDown",
    @"OEGBButtonLeft",
    @"OEGBButtonRight",
    @"OEGBButtonStart",
    @"OEGBButtonSelect"
};

@implementation OEGBSystemResponder
@dynamic client;

+ (Protocol *)gameSystemResponderClientProtocol;
{
    return @protocol(OEGBSystemResponderClient);
}

- (OEEmulatorKey)emulatorKeyForKeyIndex:(NSUInteger)index player:(NSUInteger)thePlayer
{
    return OEMakeEmulatorKey(0, index);
}

- (void)pressEmulatorKey:(OEEmulatorKey)aKey
{
    [[self client] didPushButton:(OEGBButton)aKey.key];
}

- (void)releaseEmulatorKey:(OEEmulatorKey)aKey
{
    [[self client] didReleaseButton:(OEGBButton)aKey.key];
}

@end
