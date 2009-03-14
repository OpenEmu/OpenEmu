//
//  AppData.m
//  ImageTextCellSample
//
//  Created by Martin Kahr on 04.05.07.
//  Copyright 2007 CASE Apps. All rights reserved.
//

#import "PluginInfo.h"
#import "GameCore.h"
#import "GameDocument.h"
#import "OEGameCoreController.h"
#import <Sparkle/Sparkle.h>

NSString *OEControlsPreferencesClassName = @"OEControlsPreferencesClassName";

@implementation PluginInfo

@synthesize bundle, icon, supportedTypes, supportedTypeExtensions, gameCoreClass, controller;

- (id)init
{
    [self release];
    return nil;
}

+ (id)pluginInfoWithBundleAtPath:(NSString*)path
{
    return [[[self alloc] initWithBundleAtPath:path] autorelease];
}

- (id)initWithBundleAtPath: (NSString*) path {
	bundle = [NSBundle bundleWithPath: path];
	if (bundle == nil) {
		[self release];
		return nil;
	}
	
    // Recovers the GameCore class of the plugin bundle
    gameCoreClass = [bundle principalClass];
    if([gameCoreClass isSubclassOfClass:[OEGameCoreController class]])
    {
        controller = [[gameCoreClass alloc] init];
        gameCoreClass = [controller gameCoreClass];
    }
        
    NSDictionary* infoDict = [bundle infoDictionary];
	
	NSString * iconPath = [bundle pathForResource:[infoDict objectForKey:@"CFIconName"] ofType:@"icns"];
	return [self initWithInfoDictionary:[bundle infoDictionary] icon: [[[NSImage alloc] initWithContentsOfFile:iconPath] autorelease]];
}

- (id)initWithInfoDictionary:(NSDictionary*)infoDict icon:(NSImage*)image
{
	if (self = [super init])
    {
		infoDictionary = [infoDict retain];
		icon = [image retain];
        
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
	[infoDictionary release];
    [supportedTypes release];
    [supportedTypeExtensions release];
	[super dealloc];
}

// When an instance is assigned as objectValue to a NSCell, the NSCell creates a copy.
// Therefore we have to implement the NSCopying protocol
- (id)copyWithZone:(NSZone *)zone
{
    PluginInfo *copy = [[[self class] allocWithZone: zone] initWithInfoDictionary:infoDictionary icon: icon];
	return copy;
}

- (id)newGameCoreWithDocument:(GameDocument *)aDocument
{
    return [[gameCoreClass alloc] initWithDocument:aDocument];
}

- (NSViewController *)newControlsPreferencesViewController
{
    NSViewController *viewController = [[[controller controlsPreferencesClass] alloc] initWithNibName:[controller controlsPreferencesNibName] bundle:[self bundle]];
    
    [viewController loadView];
    [viewController setNextResponder:controller];
        
    return viewController;
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

- (NSString *)displayName
{
	NSString *displayName = [infoDictionary objectForKey: @"CFBundleName"];
	if (displayName) return displayName;
	return [infoDictionary objectForKey: @"CFBundleExecutable"];
}

- (NSString *)details
{
	return [NSString stringWithFormat: @"Version %@", [infoDictionary objectForKey: @"CFBundleVersion"]];
}

- (void)updateBundle:(id)sender
{
    [[SUUpdater updaterForBundle:bundle] resetUpdateCycle];
	[[SUUpdater updaterForBundle:bundle] checkForUpdates:self];
}

@end
