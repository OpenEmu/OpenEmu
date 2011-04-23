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

static NSMutableDictionary *pluginsBySystemNames = nil;

+ (void)initialize
{
    if(self == [OEGameSystemPlugin class])
    {
        pluginsBySystemNames = [[NSMutableDictionary alloc] init];
    }
}

+ (OEGameSystemPlugin *)gameSystemPluginForName:(NSString *)gameSystemName;
{
    return [pluginsBySystemNames objectForKey:gameSystemName];
}

+ (void)registerGameSystemPlugin:(OEGameSystemPlugin *)plugin forName:(NSString *)gameSystemName;
{
    [pluginsBySystemNames setObject:plugin forKey:gameSystemName];
}

@synthesize responderClass, controller, icon, gameSystemName;

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
        
        gameSystemName = [[self infoDictionary] objectForKey:gameSystemName];
        controller     = [[mainClass alloc] init];
        responderClass = [controller responderClass];
        
        NSString *iconPath = [[self bundle] pathForResource:[[self infoDictionary] objectForKey:@"CFIconName"] ofType:@"icns"];
        
        icon = [[NSImage alloc] initWithContentsOfFile:iconPath];
        
        [[self class] registerGameSystemPlugin:self forName:gameSystemName];
    }
    return self;
}

- (void)dealloc
{
    [gameSystemName release];
    [icon           release];
    [controller     release];
    [super          dealloc];
}

@end
