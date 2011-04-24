//
//  OESMSSystemResponder.m
//  OpenEmu
//
//  Created by Remy Demarest on 23/04/2011.
//  Copyright 2011 NuLayer Inc. All rights reserved.
//

#import "OESNESSystemResponder.h"

NSString *OESNESButtonNameTable[] =
{
    @"OESNESButtonA[@]",
    @"OESNESButtonB[@]",
    @"OESNESButtonX[@]",
    @"OESNESButtonY[@]",
    @"OESNESButtonUp[@]",
    @"OESNESButtonDown[@]",
    @"OESNESButtonLeft[@]",
    @"OESNESButtonRight[@]",
    @"OESNESButtonStart[@]",
    @"OESNESButtonSelect[@]",
    @"OESNESButtonTriggerLeft[@]",
    @"OESNESButtonTriggerRight[@]",
};

@implementation OESNESSystemResponder
@dynamic client;

+ (Protocol *)gameSystemResponderClientProtocol;
{
    return @protocol(OESNESSystemResponderClient);
}

- (OEEmulatorKey)emulatorKeyForKeyIndex:(NSUInteger)index player:(NSUInteger)thePlayer
{
    return OEMakeEmulatorKey(thePlayer, index);
}

- (void)pressEmulatorKey:(OEEmulatorKey)aKey
{
    [[self client] didPushButton:(OESNESButton)aKey.key forPlayer:aKey.player];
}

- (void)releaseEmulatorKey:(OEEmulatorKey)aKey
{
    [[self client] didReleaseButton:(OESNESButton)aKey.key forPlayer:aKey.player];
}

@end
