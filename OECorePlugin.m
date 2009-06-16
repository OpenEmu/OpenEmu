//
//  OECorePlugin.m
//  OpenEmu
//
//  Created by Remy Demarest on 15/06/2009.
//  Copyright 2009 Psycho Inc.. All rights reserved.
//

#import "OECorePlugin.h"
#import <Sparkle/Sparkle.h>
#import "OEGameCoreController.h"
#import "GameCore.h"
#import "GameDocument.h"

@implementation OECorePlugin

@synthesize icon, supportedTypes, supportedTypeExtensions, gameCoreClass, controller;

- (id)initWithBundle:(NSBundle *)aBundle
{
    if(self = [super initWithBundle:aBundle])
    {
        gameCoreClass = [bundle principalClass];
        
        if([gameCoreClass isSubclassOfClass:[OEGameCoreController class]])
        {
            controller = [[gameCoreClass alloc] init];
            gameCoreClass = [controller gameCoreClass];
        }
        
        NSString *iconPath = [bundle pathForResource:[infoDictionary objectForKey:@"CFIconName"] ofType:@"icns"];
        icon = [[NSImage alloc] initWithContentsOfFile:iconPath];
        
        NSMutableDictionary *tempTypes = [[NSMutableDictionary alloc] init];
        NSMutableArray *tempExts = [[NSMutableArray alloc] init];
        NSArray *types = [infoDictionary objectForKey:@"CFBundleDocumentTypes"];
        for(NSDictionary *type in types)
        {
            NSArray *exts = [type objectForKey:@"CFBundleTypeExtensions"];
            NSMutableArray *reExts = [[NSMutableArray alloc] initWithCapacity:[exts count]];
            for(NSString *ext in exts) [reExts addObject:[ext lowercaseString]];
            [tempTypes setObject:reExts forKey:[type objectForKey:@"CFBundleTypeName"]];
            [tempExts addObjectsFromArray:reExts];
            [reExts release];
        }
                
        supportedTypes = [tempTypes copy];
        supportedTypeExtensions = [tempExts copy];
        
        [tempExts release];
        [tempTypes release];
    }
    return self;
}

- (void)dealloc
{
	[icon release];
    [controller release];
    [supportedTypes release];
    [supportedTypeExtensions release];
	[super dealloc];
}

- (id)newGameCoreWithDocument:(GameDocument *)aDocument
{
    return [[gameCoreClass alloc] initWithDocument:aDocument];
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

- (BOOL)supportsFileExtension:(NSString *)extension
{
    NSString *ext = [extension lowercaseString];
    return [supportedTypeExtensions containsObject:ext];
}

- (NSArray *)supportedTypeNames
{
    return [supportedTypes allKeys];
}

- (NSArray *)extensionsForTypeName:(NSString *)aTypeName
{
    return [supportedTypes objectForKey:aTypeName];
}

- (NSString *)details
{
	return [NSString stringWithFormat: @"Version %@", [self version]];
}

- (NSString *)description
{
    return [[super description] stringByAppendingFormat:@", supported types: %@", supportedTypes];
}

- (void)updateBundle:(id)sender
{
    [[SUUpdater updaterForBundle:bundle] resetUpdateCycle];
	[[SUUpdater updaterForBundle:bundle] checkForUpdates:self];
}

@end
