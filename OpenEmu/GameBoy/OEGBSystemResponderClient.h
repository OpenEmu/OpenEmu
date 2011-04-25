//
//  OESNESSystemResponderClient.h
//  OpenEmu
//
//  Created by Remy Demarest on 24/04/2011.
//  Copyright 2011 NuLayer Inc. All rights reserved.
//

#import <Foundation/Foundation.h>

@protocol OESystemResponderClient;

typedef enum _OEGBButton
{
    OEGBButtonA,
    OEGBButtonB,
    OEGBButtonUp,
    OEGBButtonDown,
    OEGBButtonLeft,
    OEGBButtonRight,
    OEGBButtonStart,
    OEGBButtonSelect,
    OEGBButtonCount,
} OEGBButton;

@protocol OEGBSystemResponderClient <OESystemResponderClient, NSObject>

- (void)didPushButton:(OEGBButton)button;
- (void)didReleaseButton:(OEGBButton)button;

@end
