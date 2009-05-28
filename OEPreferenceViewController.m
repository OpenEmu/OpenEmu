//
//  OEPreferenceViewController.m
//  OpenEmu
//
//  Created by Remy Demarest on 21/05/2009.
//  Copyright 2009 Psycho Inc.. All rights reserved.
//

#import "OEPreferenceViewController.h"
#import "OEGameCoreController.h"

@implementation OEPreferenceViewController

- (OEGameCoreController *)controller
{
    return (OEGameCoreController *) [self nextResponder];
}

- (void)resetBindingsWithKeys:(NSArray *)keys
{
    for(NSString *key in keys)
    {
        [self willChangeValueForKey:key];
        [self didChangeValueForKey:key];
    }
}

- (void)resetSettingBindings
{
    [self resetBindingsWithKeys:[[self controller] usedSettingNames]];
}

- (id)valueForKey:(NSString *)key
{
    OEGameCoreController *controller = [self controller];
    if([[controller usedSettingNames] containsObject:key])
        return [controller settingForKey:key];
    return [super valueForKey:key];
}

- (void)setValue:(id)value forKey:(NSString *)key
{
    OEGameCoreController *controller = [self controller];
    if([[controller usedSettingNames] containsObject:key]) // should be mutually exclusive
    {
        [self willChangeValueForKey:key];
        [controller registerSetting:value forKey:key];
        [self didChangeValueForKey:key];
    }
    else [super setValue:value forKey:key];
}

@end
