//
//  OESMSSystemController.m
//  OpenEmu
//
//  Created by Remy Demarest on 23/04/2011.
//  Copyright 2011 NuLayer Inc. All rights reserved.
//

#import "OESMSSystemController.h"
#import "OESMSSystemResponder.h"
#import "OESMSPreferenceView.h"

@implementation OESMSSystemController

- (id)init
{
    self = [super init];
    if (self) {
        // Initialization code here.
    }
    
    return self;
}

- (void)dealloc
{
    [super dealloc];
}

- (NSString *)systemName;
{
    return @"Sega Master System";
}

- (NSUInteger)numberOfPlayers;
{
    return 2;
}

- (NSDictionary *)preferenceViewControllerClasses;
{
    return [NSDictionary dictionaryWithObject:[OESMSPreferenceView class] forKey:OEControlsPreferenceKey];
}

- (Class)responderClass;
{
    return [OESMSSystemResponder class];
}

- (NSArray *)genericSettingNames;
{
    return [super genericSettingNames];
}

- (NSArray *)genericControlNames;
{
    return [NSArray arrayWithObjects:OESMSButtonNameTable count:8];
}

- (NSDictionary*)defaultControls
{
    NSDictionary *controls = [NSDictionary dictionaryWithObjectsAndKeys:
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardUpArrow]   , @"OESMSButtonUp[1]"   ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardRightArrow], @"OESMSButtonRight[1]",
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardLeftArrow] , @"OESMSButtonLeft[1]" ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardDownArrow] , @"OESMSButtonDown[1]" ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardA]         , @"OESMSButtonA[1]"    ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardS]         , @"OESMSButtonB[1]"    ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardSpacebar]  , @"OESMSButtonStart"   ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardEscape]    , @"OESMSButtonReset"   ,
                              nil];
    return controls;
}

@end
