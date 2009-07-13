//
//  OECompositionPlugin.m
//  OpenEmu
//
//  Created by Remy Demarest on 01/07/2009.
//  Copyright 2009 Psycho Inc.. All rights reserved.
//

#import "OECompositionPlugin.h"


@interface OECompositionPlugin ()
+ (void)OE_addPluginWithPath:(NSString *)aPath;
- (NSComparisonResult)OE_compare:(OECompositionPlugin *)value;
- (id)initWithCompositionAtPath:(NSString *)aPath;
@end


@implementation OECompositionPlugin

@synthesize composition, name, path;

+ (NSString *)pluginFolder
{
    return @"Filters";
}
+ (NSString *)pluginExtension
{
    return @"qtz";
}

- (NSComparisonResult)OE_compare:(OECompositionPlugin *)value
{
    return [[self name] caseInsensitiveCompare:[value name]];
}

static NSMutableDictionary *plugins = nil;

+ (void)OE_addPluginWithPath:(NSString *)aPath
{
    OECompositionPlugin *plugin = [[[self alloc] initWithCompositionAtPath:aPath] autorelease];
    if(plugin != nil) [plugins setObject:plugin forKey:[plugin name]];
}

+ (NSArray *)allPluginNames
{
    if(plugins == nil)
    {
        plugins = [[NSMutableDictionary alloc] init];
        
        NSString *folder = [self pluginFolder];
        NSString *extension = [self pluginExtension];
        
        NSString *openEmuSearchPath = [@"OpenEmu" stringByAppendingPathComponent:folder];
        
        NSArray *paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSAllDomainsMask, YES);
        
        NSFileManager *manager = [NSFileManager defaultManager];
        
        for(NSString *path in paths)
        {
            NSString *subpath = [path stringByAppendingPathComponent:openEmuSearchPath];
            NSArray  *subpaths = [manager directoryContentsAtPath:subpath];
            for(NSString *bundlePath in subpaths)
                if([extension isEqualToString:[bundlePath pathExtension]])
                    [self OE_addPluginWithPath:[subpath stringByAppendingPathComponent:bundlePath]];
        }
        
        paths = [[NSBundle mainBundle] pathsForResourcesOfType:extension inDirectory:folder];
		NSLog(@"Bundle path is : %@", paths);
        for(NSString *path in paths) [self OE_addPluginWithPath:path];
    }
    
    return [plugins allKeys];
}

+ (id)compositionPluginWithName:(NSString *)aName
{
    return [plugins objectForKey:aName];
}

- (id)init
{
    return [self initWithCompositionAtPath:nil];
}

- (id)initWithCompositionAtPath:(NSString *)aPath
{
    if(self = [super init])
    {
        if(![[aPath pathExtension] isEqualToString:@"qtz"])
        {
            [self release];
            return nil;
        }
        path = [aPath copy];
        composition = [[QCComposition compositionWithFile:path] retain];
        name = [[[composition attributes] objectForKey:QCCompositionAttributeNameKey] retain];
        if(name == nil)
            name = [[[composition attributes] objectForKey:@"name"] retain];
        if(name == nil)
            name = [[[path lastPathComponent] stringByDeletingPathExtension] retain];
    }
    return self;
}

- (void) dealloc
{
    [path release];
    [name release];
    [composition release];
    [super dealloc];
}

- (BOOL)isEqual:(id)object
{
    if([object isKindOfClass:[OECompositionPlugin class]])
        return [[self name] isEqualToString:[object name]];
    else if([object isKindOfClass:[NSString class]])
        return [[self name] isEqualToString:object];
    return [super isEqual:object];
}

- (NSUInteger)hash
{
    return [[self name] hash];
}

- (NSString *)description
{
    return [[composition attributes] objectForKey:QCCompositionAttributeDescriptionKey];
}

- (NSString *)copyright
{
    return [[composition attributes] objectForKey:QCCompositionAttributeCopyrightKey];
}
- (BOOL)isBuiltIn
{
    return [[[composition attributes] objectForKey:QCCompositionAttributeBuiltInKey] boolValue];
}
- (BOOL)isTimeDependent
{
    return [[[composition attributes] objectForKey:@"QCCompositionAttributeTimeDependentKey"] boolValue];
}
- (BOOL)hasConsumers
{
    return [[[composition attributes] objectForKey:QCCompositionAttributeHasConsumersKey] boolValue];
}
- (NSString *)category
{
    return [[composition attributes] objectForKey:QCCompositionAttributeCategoryKey];
}

@end
