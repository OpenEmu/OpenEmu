//
//  OESMSSystemController.m
//  OpenEmu
//
//  Created by Remy Demarest on 23/04/2011.
//  Copyright 2011 NuLayer Inc. All rights reserved.
//

#import "OESNESSystemController.h"
#import "OESNESSystemResponder.h"
#import "OESNESPreferenceViewController.h"

@implementation OESNESSystemController

- (NSUInteger)numberOfPlayers;
{
    return 8;
}

- (NSDictionary *)preferenceViewControllerClasses;
{
    return [NSDictionary dictionaryWithObject:[OESNESPreferenceViewController class] forKey:OEControlsPreferenceKey];
}

- (Class)responderClass;
{
    return [OESNESSystemResponder class];
}

- (NSArray *)genericSettingNames;
{
    return [super genericSettingNames];
}

- (NSArray *)genericControlNames;
{
    return [NSArray arrayWithObjects:OESNESButtonNameTable count:OESNESButtonCount];
}

- (NSDictionary *)defaultControls
{
    NSDictionary *controls = [NSDictionary dictionaryWithObjectsAndKeys:
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardUpArrow]   , @"OESNESButtonUp[1]"          ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardDownArrow] , @"OESNESButtonDown[1]"        ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardLeftArrow] , @"OESNESButtonLeft[1]"        ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardRightArrow], @"OESNESButtonRight[1]"       ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardD]         , @"OESNESButtonA[1]"           ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardS]         , @"OESNESButtonB[1]"           ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardW]         , @"OESNESButtonX[1]"           ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardA]         , @"OESNESButtonY[1]"           ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardE]         , @"OESNESButtonTriggerLeft[1]" ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardQ]         , @"OESNESButtonTriggerRight[1]",
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardSpacebar]  , @"OESNESButtonStart[1]"       ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardEscape]    , @"OESNESButtonSelect[1]"      ,
                              nil];
    return controls;
}

@end
