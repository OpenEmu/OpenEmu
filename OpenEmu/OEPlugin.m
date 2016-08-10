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
@synthesize controller = _controller;

static NSMutableDictionary *_allPlugins              = nil;
static NSMutableDictionary *_pluginFolders           = nil;
static NSMutableDictionary *_needsReload             = nil;
static NSMutableSet        *_allPluginClasses        = nil;
static NSMutableDictionary *_pluginsForPathsByTypes  = nil;
static NSMutableDictionary *_pluginsForNamesByTypes  = nil;

+ (void)initialize
{
    if(self == [OEPlugin class])
    {
        _allPlugins             = [[NSMutableDictionary alloc] init];
        _pluginFolders          = [[NSMutableDictionary alloc] init];
        _needsReload            = [[NSMutableDictionary alloc] init];
        _allPluginClasses       = [[NSMutableSet alloc] init];
        _pluginsForPathsByTypes = [[NSMutableDictionary alloc] init];
        _pluginsForNamesByTypes = [[NSMutableDictionary alloc] init];
    }
    else [self registerPluginClass:self];
}

+ (NSSet *)pluginClasses;
{
    return [_allPluginClasses copy];
}

+ (void)registerPluginClass;
{
    [_allPluginClasses addObject:self];

    [self pluginsForType:self];
}

+ (void)registerPluginClass:(Class)pluginClass;
{
    NSAssert1([pluginClass isPluginClass], @"Class %@ is not a subclass of OEPlugin.", pluginClass);

    [pluginClass registerPluginClass];
}

+ (NSString *)pluginType
{
    return NSStringFromClass(self);
}

+ (NSString *)pluginFolder
{
    NSString *ret = [_pluginFolders objectForKey:self];
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
        [_pluginFolders setObject:ret forKey:(id<NSCopying>)self];
    }
    return ret;
}

+ (NSString *)pluginExtension
{
    return [[self pluginType] lowercaseString];
}

+ (Class)typeForExtension:(NSString *)anExtension
{
    for(Class cls in _allPlugins)
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
    return nil;
}

+ (NSMutableDictionary *)OE_pluginsForNamesOfType:(Class)cls createIfNeeded:(BOOL)create
{
    NSMutableDictionary *plugins = [_pluginsForNamesByTypes objectForKey:cls];
    if(plugins == nil && create)
    {
        plugins = [NSMutableDictionary dictionary];
        _pluginsForNamesByTypes[(id)cls] = plugins;
    }
    return plugins;
}

+ (NSMutableDictionary *)OE_pluginsForPathsOfType:(Class)cls createIfNeeded:(BOOL)create
{
    NSMutableDictionary *plugins = [_pluginsForPathsByTypes objectForKey:cls];
    if(plugins == nil && create)
    {
        plugins = [NSMutableDictionary dictionary];
        _pluginsForPathsByTypes[(id)cls] = plugins;
    }
    return plugins;
}

// When an instance is assigned as objectValue to a NSCell, the NSCell creates a copy.
// Therefore we have to implement the NSCopying protocol
// No need to make an actual copy, we can consider each OECorePlugin instances like a singleton for their bundle
- (id)copyWithZone:(NSZone *)zone
{
    return self;
}

- (void)OE_setupWithBundleAtPath:(NSString *)aPath;
{
    NSBundle *bundle = [NSBundle bundleWithPath:aPath];
    if(bundle == nil) return;

    _bundle         = bundle;
    _path           = [_bundle bundlePath];
    _infoDictionary = [_bundle infoDictionary];
    _version        = [_infoDictionary objectForKey:@"CFBundleVersion"];
    _displayName    = [_infoDictionary objectForKey:@"CFBundleName"] ? : [_infoDictionary objectForKey:@"CFBundleExecutable"];
}

- (id)initWithFileAtPath:(NSString *)aPath name:(NSString *)aName;
{
    if(aPath == nil && aName == nil) return nil;

    id existing = (   [[self class] OE_pluginsForNamesOfType:[self class] createIfNeeded:NO][aName]
                  ? : [[self class] OE_pluginsForPathsOfType:[self class] createIfNeeded:NO][aPath]);
    if(existing != nil) return existing;

    if((self = [super init]))
    {
        if(aPath != nil && ![[aPath pathExtension] isEqualToString:[[self class] pluginExtension]])
            return nil;

        _path = [aPath copy];
        _name = [aName copy] ? : [[_path lastPathComponent] stringByDeletingPathExtension];

        if(_path != nil)
        {
            [self OE_setupWithBundleAtPath:aPath];
            [[self class] OE_pluginsForPathsOfType:[self class] createIfNeeded:YES][_path] = self;
        }

        [[self class] OE_pluginsForNamesOfType:[self class] createIfNeeded:YES][_name] = self;
    }
    return self;
}

- (id)initWithBundle:(NSBundle *)aBundle
{
    return [self initWithFileAtPath:[aBundle bundlePath] name:nil];
}

- (void)dealloc
{
    [_bundle unload];
}

- (id)controller
{
    if (_controller == nil)
    {
        Class principalClass = [[self bundle] principalClass];
        _controller = [self newPluginControllerWithClass:principalClass];
    }
    return _controller;
}

- (id)newPluginControllerWithClass:(Class)bundleClass
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
    return _bundle != nil ? [NSString stringWithFormat:@"Version %@", [self version]] : nil;
}

- (NSString *)description
{
    return (  _bundle != nil
            ? [NSString stringWithFormat:@"Type: %@, Bundle: %@, Version: %@", [[self class] pluginType], _displayName, _version]
            : [NSString stringWithFormat:@"Type: %@, Path: %@", [[self class] pluginType], _path]);
}

NSInteger OE_compare(OEPlugin *obj1, OEPlugin *obj2, void *ctx)
{
    return [[obj1 displayName] caseInsensitiveCompare:[obj2 displayName]];
}

+ (NSArray *)pluginsForType:(Class)aType
{
    NSArray *ret = nil;
    if([aType isPluginClass] && [_allPluginClasses containsObject:aType])
    {
        NSMutableDictionary *plugins = [_allPlugins objectForKey:aType];
        if(plugins == nil)
        {
            NSString *folder = [aType pluginFolder];
            NSString *extension = [aType pluginExtension];

            if(extension == nil) return nil;

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
                        [self pluginWithFileAtPath:[subpath stringByAppendingPathComponent:bundlePath] type:aType forceReload:YES];
            }

            NSString *pluginFolderPath = [[[NSBundle mainBundle] builtInPlugInsPath] stringByAppendingPathComponent:folder];
            paths = [manager contentsOfDirectoryAtPath:pluginFolderPath error:NULL];
            for(NSString *path in paths)
                if([extension isEqualToString:[path pathExtension]])
                    [self pluginWithFileAtPath:[pluginFolderPath stringByAppendingPathComponent:path] type:aType];

            plugins = [_allPlugins objectForKey:aType];
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
        for(Class key in _allPlugins)
            [temp addObjectsFromArray:[self pluginsForType:key]];

        ret = [temp copy];
    }
    else ret = [self pluginsForType:self];

    return ret;
}

+ (NSArray *)allPluginNames;
{
    return [[self allPlugins] valueForKey:@"name"];
}

+ (instancetype)pluginWithBundle:(NSBundle *)aBundle type:(Class)aType forceReload:(BOOL)reload
{
    return [self pluginWithFileAtPath:[aBundle bundlePath] type:aType forceReload:reload];
}

+ (instancetype)pluginWithFileAtPath:(NSString *)filePath type:(Class)aType forceReload:(BOOL)reload
{
    if(filePath == nil || ![aType isPluginClass]) return nil;

    NSMutableDictionary *plugins = [_allPlugins objectForKey:aType];
    if(plugins == nil)
    {
        plugins = [NSMutableDictionary dictionary];
        [_allPlugins setObject:plugins forKey:(id<NSCopying>)aType];
    }

    NSString *pluginName = [[filePath lastPathComponent] stringByDeletingPathExtension];
    id ret = [plugins objectForKey:pluginName];

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

        ret = [[aType alloc] initWithFileAtPath:filePath name:pluginName];

        // If ret is still nil at this point, it means the plugin can't be loaded (old-style version for example)
        if(ret == nil) ret = [NSNull null];

        [plugins setObject:ret forKey:pluginName];
        [aType didChangeValueForKey:@"allPlugins"];
        [OEPlugin didChangeValueForKey:@"allPlugins"];
    }

    if(ret == [NSNull null]) ret = nil;

    return ret;
}

+ (instancetype)pluginWithFileAtPath:(NSString *)aPath type:(Class)aType
{
    return [self pluginWithFileAtPath:aPath type:aType forceReload:NO];
}

+ (instancetype)pluginWithName:(NSString *)aName
{
    if(self == [OEPlugin class]) return nil;

    return [self pluginWithName:aName type:self];
}

+ (instancetype)pluginWithName:(NSString *)aName type:(Class)aType
{
    return (    [self OE_pluginsForNamesOfType:aType createIfNeeded:NO][aName]
            ? : [self pluginWithFileAtPath:OE_pluginPathForNameType(aName, aType) type:aType]);
}

- (NSArray *)availablePreferenceViewControllerKeys
{
    return _bundle != nil ? [[self controller] availablePreferenceViewControllerKeys] : nil;
}

@end
