//
//  OEGBASystemController.m
//  OpenEmu
//
//  Created by Remy Demarest on 08/05/2011.
//  Copyright 2011 NuLayer Inc. All rights reserved.
//

#import "OEGBASystemController.h"
#import "OEGBAControlsPreference.h"
#import "OEGBASystemResponder.h"
#import "OEGBASystemResponderClient.h"

@implementation OEGBASystemController

- (NSUInteger)numberOfPlayers;
{
    return 4;
}

- (NSDictionary *)preferenceViewControllerClasses;
{
    return [NSDictionary dictionaryWithObject:[OEGBAControlsPreference class] forKey:OEControlsPreferenceKey];
}

- (Class)responderClass;
{
    return [OEGBASystemResponder class];
}

- (NSArray *)genericSettingNames;
{
    return [super genericSettingNames];
}

- (NSArray *)genericControlNames;
{
    return [NSArray arrayWithObjects:OEGBAButtonNameTable count:OEGBAButtonCount];
}

- (NSDictionary *)defaultControls
{
    NSDictionary *controls = [NSDictionary dictionaryWithObjectsAndKeys:
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardUpArrow]   , @"OEGBAButtonUp[1]"    ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardDownArrow] , @"OEGBAButtonDown[1]"  ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardLeftArrow] , @"OEGBAButtonLeft[1]"  ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardRightArrow], @"OEGBAButtonRight[1]" ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardD]         , @"OEGBAButtonA[1]"     ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardS]         , @"OEGBAButtonB[1]"     ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardE]         , @"OEGBAButtonL[1]"     ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardQ]         , @"OEGBAButtonR[1]"     ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardSpacebar]  , @"OEGBAButtonStart[1]" ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardEscape]    , @"OEGBAButtonSelect[1]",
                              nil];
    return controls;
}

@end
