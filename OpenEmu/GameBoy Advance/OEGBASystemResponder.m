//
//  OEGBASystemResponder.m
//  OpenEmu
//
//  Created by Remy Demarest on 08/05/2011.
//  Copyright 2011 NuLayer Inc. All rights reserved.
//

#import "OEGBASystemResponder.h"
#import "OEGBASystemResponderClient.h"

NSString *OEGBAButtonNameTable[] =
{
    @"OEGBAButtonA[@]",
    @"OEGBAButtonB[@]",
    @"OEGBAButtonSelect[@]",
    @"OEGBAButtonStart[@]",
    @"OEGBAButtonRight[@]",
    @"OEGBAButtonLeft[@]",
    @"OEGBAButtonUp[@]",
    @"OEGBAButtonDown[@]",
    @"OEGBAButtonR[@]",
    @"OEGBAButtonL[@]",
    @"OEGBAButtonSpeed[@]",
    @"OEGBAButtonCapture[@]"
};

@implementation OEGBASystemResponder
@dynamic client;

+ (Protocol *)gameSystemResponderClientProtocol;
{
    return @protocol(OEGBASystemResponderClient);
}

- (OEEmulatorKey)emulatorKeyForKeyIndex:(NSUInteger)index player:(NSUInteger)thePlayer
{
    return OEMakeEmulatorKey(thePlayer, index);
}

- (void)pressEmulatorKey:(OEEmulatorKey)aKey
{
    [[self client] didPushGBAButton:(OEGBAButton)aKey.key forPlayer:aKey.player];
}

- (void)releaseEmulatorKey:(OEEmulatorKey)aKey
{
    [[self client] didReleaseGBAButton:(OEGBAButton)aKey.key forPlayer:aKey.player];
}

@end
