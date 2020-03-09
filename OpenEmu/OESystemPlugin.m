/*
 Copyright (c) 2011, OpenEmu Team

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

#import "OESystemPlugin.h"

#if SUPPORTS_LIBRARY_REGISTRATION
#import "OEDBSystem.h"
#import "OELibraryDatabase.h"
#endif

#import <OpenEmuSystem/OpenEmuSystem.h>

@implementation OESystemPlugin
@dynamic controller;

static NSMutableDictionary *_pluginsBySystemIdentifiers = nil;
static NSArray *_cachedSupportedTypeExtensions = nil;
static NSArray *_cachedSupportedSystemTypes = nil;

+ (void)initialize
{
    if(self == [OESystemPlugin class])
    {
        _pluginsBySystemIdentifiers = [[NSMutableDictionary alloc] init];
        [self registerPluginClass:self];
    }
}

+ (OESystemPlugin *)systemPluginForIdentifier:(NSString *)gameSystemIdentifier;
{
    return [_pluginsBySystemIdentifiers objectForKey:gameSystemIdentifier];
}

+ (void)registerGameSystemPlugin:(OESystemPlugin *)plugin forIdentifier:(NSString *)gameSystemIdentifier;
{
    [_pluginsBySystemIdentifiers setObject:plugin forKey:gameSystemIdentifier];

#if SUPPORTS_LIBRARY_REGISTRATION
    OELibraryDatabase *db = [OELibraryDatabase defaultDatabase];
    if(db == nil) NSLog(@"system plugins not registered in database, because the db does not exist yet!");
    else          [OEDBSystem systemForPlugin:plugin inContext:[db mainThreadContext]];
#endif

    // Invalidate supported type extenesions cache
    _cachedSupportedTypeExtensions = nil;
    _cachedSupportedSystemTypes = nil;
}

+ (NSArray *)supportedTypeExtensions;
{
    if(_cachedSupportedTypeExtensions == nil)
    {
        NSMutableSet *extensions = [NSMutableSet set];
        for(OESystemPlugin *plugin in [OEPlugin pluginsForType:self])
            [extensions addObjectsFromArray:[plugin supportedTypeExtensions]];

        _cachedSupportedTypeExtensions = [extensions allObjects];
    }

    return _cachedSupportedTypeExtensions;
}

+ (NSArray *)supportedSystemTypes;
{
    if(_cachedSupportedSystemTypes == nil)
    {
        NSMutableSet *extensions = [NSMutableSet set];
        for(OESystemPlugin *plugin in [OEPlugin pluginsForType:self])
            [extensions addObject:plugin.systemType];

        _cachedSupportedSystemTypes = extensions.allObjects;
    }

    return _cachedSupportedSystemTypes;
}

+ (OESystemPlugin *)systemPluginWithBundleAtPath:(NSString *)bundlePath;
{
    return [self pluginWithFileAtPath:bundlePath type:self];
}

- (id)initWithFileAtPath:(NSString *)aPath name:(NSString *)aName error:(NSError *__autoreleasing *)outError
{
    if((self = [super initWithFileAtPath:aPath name:aName error:outError]))
    {
        _systemIdentifier = [[self infoDictionary] objectForKey:OESystemIdentifier];
        _responderClass   = [[self controller] responderClass];

        NSString *iconPath = [[self bundle] pathForResource:[[self infoDictionary] objectForKey:@"CFIconName"] ofType:@"icns"];

        _bundleIcon = [[NSImage alloc] initWithContentsOfFile:iconPath];

        [[self class] registerGameSystemPlugin:self forIdentifier:_systemIdentifier];
    }

    return self;
}

- (id)newPluginControllerWithClass:(Class)bundleClass
{
    if(![bundleClass isSubclassOfClass:[OESystemController class]]) return nil;

    return [[bundleClass alloc] initWithBundle:[self bundle]];
}

- (NSString *)systemName
{
    return [[self controller] systemName];
}

- (NSString *)systemType
{
    return [[self controller] systemType];
}

- (NSImage *)systemIcon
{
    return [[self controller] systemIcon];
}

- (NSArray *)supportedTypeExtensions;
{
    return [[self controller] fileTypes];
}

- (BOOL)supportsDiscs
{
    return [[self controller] supportsDiscs];
}

- (CGFloat)coverAspectRatio
{
    return [[self controller] coverAspectRatio];
}

- (BOOL)isOutOfSupport
{
    /* system plugins are shipped inside the application bundle; all
     * plugins located in the application support directory must be removed. */
    NSURL *bundlePath = [[[self bundle] bundleURL] baseURL];
    NSFileManager *fm = [NSFileManager defaultManager];
    NSURL *systemsDirectory = [[fm URLsForDirectory:NSApplicationSupportDirectory inDomains:NSUserDomainMask] lastObject];
    systemsDirectory = [systemsDirectory URLByAppendingPathComponent:@"OpenEmu/Systems"];
    if ([systemsDirectory.path isEqual:bundlePath.path])
        return YES;
        
    return NO;
}

@end
