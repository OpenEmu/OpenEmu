//
//  OEGBASystemResponderClient.h
//  OpenEmu
//
//  Created by Remy Demarest on 08/05/2011.
//  Copyright 2011 NuLayer Inc. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@protocol OESystemResponderClient;

typedef enum _OEGBAButton
{
    OEGBAButtonA,
    OEGBAButtonB,
    OEGBAButtonSelect,
    OEGBAButtonStart,
    OEGBAButtonRight,
    OEGBAButtonLeft,
    OEGBAButtonUp,
    OEGBAButtonDown,
    OEGBAButtonR,
    OEGBAButtonL,
    OEGBAButtonSpeed,
    OEGBAButtonCapture,
    OEGBAButtonCount
} OEGBAButton;

@protocol OEGBASystemResponderClient <OESystemResponderClient, NSObject>

- (void)didPushGBAButton:(OEGBAButton)button forPlayer:(NSUInteger)player;
- (void)didReleaseGBAButton:(OEGBAButton)button forPlayer:(NSUInteger)player;

@end
