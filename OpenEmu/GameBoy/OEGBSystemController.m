//
//  OESMSSystemController.m
//  OpenEmu
//
//  Created by Remy Demarest on 23/04/2011.
//  Copyright 2011 NuLayer Inc. All rights reserved.
//

#import "OEGBSystemController.h"
#import "OEGBSystemResponder.h"
#import "OEGBPreferenceView.h"

@implementation OEGBSystemController

- (NSUInteger)numberOfPlayers;
{
    return 8;
}

- (NSDictionary *)preferenceViewControllerClasses;
{
    return [NSDictionary dictionaryWithObject:[OEGBPreferenceView class] forKey:OEControlsPreferenceKey];
}

- (Class)responderClass;
{
    return [OEGBSystemResponder class];
}

- (NSArray *)genericSettingNames;
{
    return [super genericSettingNames];
}

- (NSArray *)genericControlNames;
{
    return [NSArray arrayWithObjects:OEGBButtonNameTable count:OEGBButtonCount];
}

- (NSDictionary *)defaultControls
{
    NSDictionary *controls = [NSDictionary dictionaryWithObjectsAndKeys:
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardUpArrow]   , @"OEGBButtonUp"    ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardDownArrow] , @"OEGBButtonDown"  ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardLeftArrow] , @"OEGBButtonLeft"  ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardRightArrow], @"OEGBButtonRight" ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardA]         , @"OEGBButtonA"     ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardS]         , @"OERBButtonB"     ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardSpacebar]  , @"OEGBButtonStart" ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardEscape]    , @"OEGBButtonSelect",
                              nil];
    return controls;
}

@end
