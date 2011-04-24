//
//  OESNESSystemResponderClient.h
//  OpenEmu
//
//  Created by Remy Demarest on 24/04/2011.
//  Copyright 2011 NuLayer Inc. All rights reserved.
//

#import <Foundation/Foundation.h>

@protocol OESystemResponderClient;

typedef enum _OESNESButton
{
    OESNESButtonA,
    OESNESButtonB,
    OESNESButtonX,
    OESNESButtonY,
    OESNESButtonUp,
    OESNESButtonDown,
    OESNESButtonLeft,
    OESNESButtonRight,
    OESNESButtonStart,
    OESNESButtonSelect,
    OESNESButtonTriggerLeft,
    OESNESButtonTriggerRight,
    OESNESButtonCount,
} OESNESButton;

@protocol OESNESSystemResponderClient <OESystemResponderClient, NSObject>

- (void)didPushButton:(OESNESButton)button forPlayer:(NSUInteger)player;
- (void)didReleaseButton:(OESNESButton)button forPlayer:(NSUInteger)player;

@end
