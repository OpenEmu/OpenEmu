//
//  OENESSystemResponderClient.h
//  OpenEmu
//
//  Created by Remy Demarest on 24/04/2011.
//  Copyright 2011 NuLayer Inc. All rights reserved.
//

#import <Cocoa/Cocoa.h>

typedef enum _OENESButton
{
    OENESButtonA,
    OENESButtonB,
    OENESButtonUp,
    OENESButtonDown,
    OENESButtonLeft,
    OENESButtonRight,
    OENESButtonStart,
    OENESButtonSelect,
    OENESButtonCount
} OENESButton;

@protocol OENESSystemResponderClient <OESystemResponderClient, NSObject>

- (void)didPushNESButton:(OENESButton)button forPlayer:(NSUInteger)player;
- (void)didReleaseNESButton:(OENESButton)button forPlayer:(NSUInteger)player;

@end
