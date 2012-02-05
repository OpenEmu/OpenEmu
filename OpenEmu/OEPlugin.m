/*
 Copyright (c) 2009, OpenEmu Team
 
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the OpenEmu Team nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#import "OEPlugin.h"
#import <objc/runtime.h>

@implementation NSObject (OEPlugin)
+ (BOOL)isPluginClass
{
    return [self isSubclassOfClass:[OEPlugin class]];
}
@end

NSInteger OE_compare(OEPlugin *obj1, OEPlugin *obj2, void *ctx);

@implementation OEPlugin

@synthesize infoDictionary, version, displayName, bundle, controller;

static NSMutableDictionary *allPlugins       = nil;
static NSMutableDictionary *pluginFolders    = nil;
static NSMutableDictionary *needsReload      = nil;
static NSMutableSet        *allPluginClasses = nil;

+ (void)initialize
{
    if(self == [OEPlugin class])
    {
        allPlugins       = [[NSMutableDictionary alloc] init];
        pluginFolders    = [[NSMutableDictionary alloc] init];
        needsReload      = [[NSMutableDictionary alloc] init];
        allPluginClasses = [[NSMutableSet alloc] init];
    }
}

+ (NSSet *)pluginClasses;
{
    return [[allPluginClasses copy] autorelease];
}

+ (void)registerPluginClass:(Class)pluginClass;
{
    NSAssert1([pluginClass isPluginClass], @"Class %@ is not a subclass of OEPlugin.", pluginClass);
    
    [allPluginClasses addObject:pluginClass];
    
    [self pluginsForType:pluginClass];
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

+ (Class)typeForExtension:(NSString *)anExtension
{
    for(Class cls in allPlugins)
        if([[cls pluginExtension] isEqualToString:anExtension])
            return cls;
    
    NSInteger len = [anExtension length] - 8;
    if(len > 0) return NSClassFromString([NSString stringWithFormat:@"OE%@Plugin",
                                          [[anExtension substringWithRange:NSMakeRange(2, len)]
                                           capitalizedString]]);
    return Nil;
}

+ (BOOL)isPluginClass
{
    return YES;
}

- (id)init
{
	self = [super init];
	// TODO: CHECK IF THIS WAS NEEDED ANYWHERE
  //  [self release];
    return self;
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
    
    if((self = [super init]))
    {
        bundle         = [aBundle retain];
        infoDictionary = [[bundle infoDictionary] retain];
        version        = [[infoDictionary objectForKey:@"CFBundleVersion"] retain];
        displayName    = ([[infoDictionary objectForKey:@"CFBundleName"] retain] ? : [[infoDictionary objectForKey:@"CFBundleExecutable"] retain]);
        
        Class principalClass = [[self bundle] principalClass];
        
        if(principalClass != Nil && ![principalClass conformsToProtocol:@protocol(OEPluginController)])
        {
            [self release];
            return nil;
        }
        
        controller = [self newPluginControllerWithClass:principalClass];
        
        if(controller == nil && principalClass != Nil)
        {
            [self release];
            return nil;
        }
    }
    return self;
}

- (void)dealloc
{
    [bundle         unload];
    [bundle         release];
    [version        release];
    [controller     release];
    [displayName    release];
    [infoDictionary release];
    [super          dealloc];
}

- (id<OEPluginController>)newPluginControllerWithClass:(Class)bundleClass
{
    return [[bundleClass alloc] init];
}

static NSString *OE_pluginPathForNameType(NSString *aName, Class aType)
{
    NSString *folderName = [aType pluginFolder];
    NSString *extension  = [aType pluginExtension];
    
    NSString *openEmuSearchPath = [@"OpenEmu" stringByAppendingPathComponent:
                                   [folderName stringByAppendingPathComponent:
                                    [aName stringByAppendingPathExtension:extension]]];
    
    NSString *ret = nil;
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
    
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

- (NSString *)details
{
    return [NSString stringWithFormat:@"Version %@", [self version]];
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
            
            NSArray *paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
            NSFileManager *manager = [NSFileManager defaultManager];
            
            for(NSString *path in paths)
            {
                NSString *subpath = [path stringByAppendingPathComponent:openEmuSearchPath];
                NSArray  *subpaths = [manager contentsOfDirectoryAtPath:subpath error:NULL];
                for(NSString *bundlePath in subpaths)
                    if([extension isEqualToString:[bundlePath pathExtension]])
                        // If a plugin fails to load, another plugin with the same name in deeper paths can take its spot
                        // Typical case: an old-style plugin in ~/Library should fail to load,
                        // and its new-style counter-part in /Library will take its place and load properly
                        [self pluginWithBundleAtPath:[subpath stringByAppendingPathComponent:bundlePath] type:aType forceReload:YES];
            }
            
            NSString *pluginFolderPath = [[[NSBundle mainBundle] builtInPlugInsPath] stringByAppendingPathComponent:folder];
            paths = [[NSFileManager defaultManager] contentsOfDirectoryAtPath:pluginFolderPath error:NULL];
            for(NSString *path in paths) [self pluginWithBundleAtPath:[pluginFolderPath stringByAppendingPathComponent:path] type:aType];
            
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
        NSMutableArray *temp = [NSMutableArray array];
        for(Class key in allPlugins)
            [temp addObjectsFromArray:[self pluginsForType:key]];
        
        ret = [[temp copy] autorelease];
    }
    else ret = [self pluginsForType:self];
    
    return ret;
}

+ (id)pluginWithBundle:(NSBundle *)aBundle type:(Class)aType forceReload:(BOOL)reload
{
    if(aBundle == nil || ![aType isPluginClass]) return nil;
    
    NSMutableDictionary *plugins = [allPlugins objectForKey:aType];
    if(plugins == nil)
    {
        plugins = [NSMutableDictionary dictionary];
        [allPlugins setObject:plugins forKey:aType];
    }
    
    NSString *aName = [[[aBundle bundlePath] stringByDeletingPathExtension] lastPathComponent];
    id ret = [plugins objectForKey:aName];
    
    if(reload)
    {
        // Will override a previous failed attempt at loading a plugin
        if(ret == [NSNull null]) ret = nil;
        // A plugin was already successfully loaded
        else if(ret != nil) return nil;
    }
    
    // No plugin with such name, attempt to actually load the file at the given path
    if(ret == nil)
    {
        [OEPlugin willChangeValueForKey:@"allPlugins"];
        [aType willChangeValueForKey:@"allPlugins"];
        
        if(aBundle != nil) ret = [[[aType alloc] initWithBundle:aBundle] autorelease];
        
        // If ret is still nil at this point, it means the plugin can't be loaded (old-style version for example)
        if(ret == nil) ret = [NSNull null];
        
        [plugins setObject:ret forKey:aName];
        [aType didChangeValueForKey:@"allPlugins"];
        [OEPlugin didChangeValueForKey:@"allPlugins"];
    }
    
    if(ret == [NSNull null]) ret = nil;
    
    return ret;
}

+ (id)pluginWithBundleAtPath:(NSString *)bundlePath type:(Class)aType forceReload:(BOOL)reload
{
    return [self pluginWithBundle:[NSBundle bundleWithPath:bundlePath] type:aType forceReload:reload];
}

+ (id)pluginWithBundleAtPath:(NSString *)bundlePath type:(Class)aType
{
    return [self pluginWithBundleAtPath:bundlePath type:aType forceReload:NO];
}

+ (id)pluginWithBundleName:(NSString *)aName type:(Class)aType
{
    return [self pluginWithBundleAtPath:OE_pluginPathForNameType(aName, aType) type:aType];
}

- (NSArray *)availablePreferenceViewControllerKeys
{
    return [controller availablePreferenceViewControllerKeys];
}
@end
