//
//  OESMSSystemResponderClient.h
//  OpenEmu
//
//  Created by Remy Demarest on 24/04/2011.
//  Copyright 2011 NuLayer Inc. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@protocol OESystemResponderClient;

typedef enum _OESMSButton
{
    OESMSButtonUp,
    OESMSButtonDown,
    OESMSButtonLeft,
    OESMSButtonRight,
    OESMSButtonA,
    OESMSButtonB,
    OESMSButtonStart,
    OESMSButtonReset,
    OESMSButtonCount,
} OESMSButton;

@protocol OESMSSystemResponderClient <OESystemResponderClient, NSObject>

- (void)didPushSMSButton:(OESMSButton)button forPlayer:(NSUInteger)player;
- (void)didReleaseSMSButton:(OESMSButton)button forPlayer:(NSUInteger)player;

- (void)didPushSMSStartButton;
- (void)didReleaseSMSStartButton;

- (void)didPushSMSResetButton;
- (void)didReleaseSMSResetButton;

@end
