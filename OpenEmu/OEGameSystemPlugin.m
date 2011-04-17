//
//  OESystemPlugin.m
//  OpenEmu
//
//  Created by Remy Demarest on 17/04/2011.
//  Copyright 2011 NuLayer Inc. All rights reserved.
//

#import "OEGameSystemPlugin.h"
#import "OEGameSystemController.h"

@implementation OEGameSystemPlugin

@synthesize inputViewClass, controller;

+ (OEGameSystemPlugin *)systemPluginWithBundleAtPath:(NSString *)bundlePath;
{
    return [self pluginWithBundleAtPath:bundlePath type:self];
}

- (id)initWithBundle:(NSBundle *)aBundle
{
    if((self = [super initWithBundle:aBundle]))
    {
        Class mainClass = [[self bundle] principalClass];
        
        if(![mainClass isSubclassOfClass:[OEGameSystemController class]])
        {
            [self release];
            return nil;
        }
        
        controller = [[mainClass alloc] init];
        inputViewClass = [controller inputViewClass];
        
        NSString *iconPath = [[self bundle] pathForResource:[[self infoDictionary] objectForKey:@"CFIconName"] ofType:@"icns"];
        
        icon = [[NSImage alloc] initWithContentsOfFile:iconPath];
    }
    return self;
}

- (void)dealloc
{
    [icon       release];
    [controller release];
    [super      dealloc];
}

- (NSViewController *)newPreferenceViewControllerForKey:(NSString *)aKey
{
    NSViewController *ret = [controller newPreferenceViewControllerForKey:aKey];
    if(ret == nil) ret = [[NSViewController alloc] initWithNibName:@"UnimplementedPreference" bundle:[NSBundle mainBundle]];
    return ret;
}

- (NSArray *)availablePreferenceViewControllers
{
    return [controller availablePreferenceViewControllers];
}

@end
