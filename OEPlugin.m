//
//  OEPlugin.m
//  OpenEmu
//
//  Created by Remy Demarest on 15/06/2009.
//  Copyright 2009 Psycho Inc.. All rights reserved.
//

#import "OEPlugin.h"

@implementation NSObject (OEPlugin)
+ (BOOL)isPluginClass
{
    return [self isSubclassOfClass:[OEPlugin class]];
}
@end

@implementation OEPlugin

@synthesize infoDictionary, version, displayName, bundle;

static NSMutableDictionary *allPlugins = nil;
static NSMutableDictionary *pluginFolders = nil;

+ (void)initialize
{
    if(self == [OEPlugin class])
    {
        allPlugins = [[NSMutableDictionary alloc] init];
        pluginFolders = [[NSMutableDictionary alloc] init];
    }
}

+ (NSString *)pluginType
{
    return NSStringFromClass(self);
}

+ (NSString *)pluginFolder
{
    NSString *ret = [pluginFolders objectForKey:self];
    if(ret == nil)
    {
        ret = [self pluginType];
        NSRange c = [ret rangeOfCharacterFromSet:[NSCharacterSet lowercaseLetterCharacterSet]];
        if(c.location != NSNotFound && c.location > 0)
        {
            ret = [ret substringFromIndex:c.location - 1];
            if([ret hasSuffix:@"Plugin"])
                ret = [ret substringToIndex:[ret length] - 6];
        }
        ret = [ret stringByAppendingString:@"s"];
        [pluginFolders setObject:ret forKey:self];
    }
    return ret;
}

+ (NSString *)pluginExtension
{
    return [[self pluginType] lowercaseString];
}

+ (BOOL)isPluginClass
{
    return YES;
}

- (id)init
{
    [self release];
    return nil;
}

// When an instance is assigned as objectValue to a NSCell, the NSCell creates a copy.
// Therefore we have to implement the NSCopying protocol
// No need to make an actual copy, we can consider each OECorePlugin instances like a singleton for their bundle
- (id)copyWithZone:(NSZone *)zone
{
	return [self retain];
}

- (id)initWithBundle:(NSBundle *)aBundle
{
    if(aBundle == nil)
    {
        [self release];
        return nil;
    }
    
    if(self = [super init])
    {
        bundle = [aBundle retain];
        infoDictionary = [[bundle infoDictionary] retain];
        version = [[infoDictionary objectForKey:@"CFBundleVersion"] retain];
        displayName = [[infoDictionary objectForKey:@"CFBundleName"] retain];
        if(displayName == nil)
            displayName = [[infoDictionary objectForKey:@"CFBundleExecutable"] retain];
    }
    return self;
}

- (void) dealloc
{
    [bundle         release];
    [version        release];
    [displayName    release];
    [infoDictionary release];
    [super          dealloc];
}

static NSString *OE_pluginPathForNameType(NSString *aName, Class aType)
{
    NSString *folderName = [aType pluginFolder];
    NSString *extension  = [aType pluginExtension];
    
    NSString *openEmuSearchPath = [@"OpenEmu" stringByAppendingPathComponent:
                                   [folderName stringByAppendingPathComponent:
                                    [aName stringByAppendingPathExtension:extension]]];
    
    NSString *ret = nil;
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSAllDomainsMask, YES);
    
    NSFileManager *manager = [NSFileManager defaultManager];
    for(NSString *path in paths)
    {
        NSString *tested = [path stringByAppendingPathComponent:openEmuSearchPath];
        if([manager fileExistsAtPath:tested])
        {
            ret = tested;
            break;
        }
    }
    
    if(ret == nil) ret = [[NSBundle mainBundle] pathForResource:aName ofType:extension inDirectory:folderName];
    return ret;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"Type: %@, Bundle: %@, version: %@", [[self class] pluginType], displayName, version];
}

NSInteger OE_compare(OEPlugin *obj1, OEPlugin *obj2, void *ctx)
{
    return [[obj1 displayName] compare:[obj2 displayName]];
}

+ (NSArray *)pluginsForType:(Class)aType
{
    NSArray *ret = nil;
    if([aType isPluginClass])
    {
        NSMutableDictionary *plugins = [allPlugins objectForKey:aType];
        if(plugins == nil)
        {
            NSString *folder = [aType pluginFolder];
            NSString *extension = [aType pluginExtension];
            
            NSString *openEmuSearchPath = [@"OpenEmu" stringByAppendingPathComponent:folder];
            
            NSArray *paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSAllDomainsMask, YES);
            
            NSFileManager *manager = [NSFileManager defaultManager];
            
            for(NSString *path in paths)
            {
                NSString *subpath = [path stringByAppendingPathComponent:openEmuSearchPath];
                NSArray  *subpaths = [manager directoryContentsAtPath:subpath];
                for(NSString *bundlePath in subpaths)
                    if([extension isEqualToString:[bundlePath pathExtension]])
                        [self pluginWithBundleAtPath:[subpath stringByAppendingPathComponent:bundlePath] type:aType];
            }
            
            paths = [[NSBundle mainBundle] pathsForResourcesOfType:extension inDirectory:folder];
            for(NSString *path in paths)
                [self pluginWithBundleAtPath:path type:aType];
            
            plugins = [allPlugins objectForKey:aType];
        }
        
        NSMutableSet *set = [NSMutableSet setWithArray:[plugins allValues]];
        [set removeObject:[NSNull null]];
        ret = [[set allObjects] sortedArrayUsingFunction:OE_compare context:nil];
    }
    return ret;
}

+ (NSArray *)allPlugins
{
    NSArray *ret = nil;
    if(self == [OEPlugin class])
    {
        ret = [NSMutableArray array];
        for(Class key in allPlugins)
            [(NSMutableArray *)ret addObjectsFromArray:[self pluginsForType:key]];
    }
    else ret = [self pluginsForType:self];
    
    return ret;
}

+ (id)pluginWithBundleAtPath:(NSString *)bundlePath type:(Class)aType
{
    if(bundlePath == nil || ![aType isPluginClass]) return nil;
    
    NSMutableDictionary *plugins = [allPlugins objectForKey:aType];
    if(plugins == nil)
    {
        plugins = [NSMutableDictionary dictionary];
        [allPlugins setObject:plugins forKey:aType];
    }
    
    NSString *aName = [[bundlePath stringByDeletingPathExtension] lastPathComponent];
    id ret = [plugins objectForKey:aName];
    if(ret == nil)
    {
        NSBundle *theBundle = [NSBundle bundleWithPath:bundlePath];
        if(bundlePath != nil && theBundle != nil)
            ret = [[[aType alloc] initWithBundle:theBundle] autorelease];
        else ret = [NSNull null];
        
        [plugins setObject:ret forKey:aName];
    }
    
    if(ret == [NSNull null]) ret = nil;
    
    return ret;
}

+ (id)pluginWithBundleName:(NSString *)aName type:(Class)aType
{
    return [self pluginWithBundleAtPath:OE_pluginPathForNameType(aName, aType) type:aType];
}

@end
