//
//  OENESSystemResponder.m
//  OpenEmu
//
//  Created by Remy Demarest on 24/04/2011.
//  Copyright 2011 NuLayer Inc. All rights reserved.
//

#import "OENESSystemResponder.h"

NSString *const OENESControlNameTable[] =
{
    @"OENESButtonA[@]",
    @"OENESButtonB[@]",
    @"OENESButtonUp[@]",
    @"OENESButtonDown[@]",
    @"OENESButtonLeft[@]",
    @"OENESButtonRight[@]",
    @"OENESButtonStart[@]",
    @"OENESButtonSelect[@]"
};

@implementation OENESSystemResponder
@dynamic client;

+ (Protocol *)gameSystemResponderClientProtocol;
{
    return @protocol(OENESSystemResponderClient);
}

- (OEEmulatorKey)emulatorKeyForKeyIndex:(NSUInteger)index player:(NSUInteger)thePlayer
{
    return OEMakeEmulatorKey(thePlayer, index);
}

- (void)pressEmulatorKey:(OEEmulatorKey)aKey
{
    [[self client] didPushButton:(OENESButton)aKey.key forPlayer:aKey.player];
}

- (void)releaseEmulatorKey:(OEEmulatorKey)aKey
{
    [[self client] didReleaseButton:(OENESButton)aKey.key forPlayer:aKey.player];
}

@end
