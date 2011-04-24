//
//  OESMSSystemResponder.h
//  OpenEmu
//
//  Created by Remy Demarest on 23/04/2011.
//  Copyright 2011 NuLayer Inc. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <OpenEmuSystem/OpenEmuSystem.h>

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

- (void)didPushButton:(OESMSButton)button forPlayer:(NSUInteger)player;
- (void)didReleaseButton:(OESMSButton)button forPlayer:(NSUInteger)player;

- (void)didPushStartButton;
- (void)didReleaseStartButton;

- (void)didPushResetButton;
- (void)didReleaseResetButton;

@end

extern NSString *OESMSButtonNameTable[];

@interface OESMSSystemResponder : OEBasicSystemResponder

@property(nonatomic, assign) id<OESMSSystemResponderClient> client;

@end
