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

#import "OECorePlugin.h"
#import <OpenEmuBase/OpenEmuBase.h>

@class OEGameDocument;

@implementation OECorePlugin
{
    Class _gameCoreClass;
}

@dynamic controller;

static NSArray *_cachedRequiredFiles = nil;

+ (OECorePlugin *)corePluginWithBundleAtPath:(NSString *)bundlePath
{
    return [self pluginWithFileAtPath:bundlePath type:self];
}

+ (OECorePlugin *)corePluginWithBundleIdentifier:(NSString *)identifier
{
    NSArray *cores = [OECorePlugin pluginsForType:self];
    for(OECorePlugin *plugin in cores)
        if([[plugin bundleIdentifier] caseInsensitiveCompare:identifier] == NSOrderedSame)
            return plugin;

    return nil;
}

+ (NSArray *)corePluginsForSystemIdentifier:(NSString *)systemIdentifier;
{
    NSArray *cores = [self allPlugins];
    NSMutableArray *validCores = [NSMutableArray array];

    for(OECorePlugin *plugin in cores)
        if([[plugin systemIdentifiers] containsObject:systemIdentifier])
            [validCores addObject:plugin];

    return validCores;
}

+ (NSArray *)requiredFiles;
{
    if(_cachedRequiredFiles == nil)
    {
        NSMutableArray *files = [[NSMutableArray alloc] init];
        for(OECorePlugin *plugin in [OEPlugin pluginsForType:self])
            if([plugin requiredFiles] != nil) [files addObjectsFromArray:[plugin requiredFiles]];
        
        _cachedRequiredFiles = [files copy];
    }
    
    return _cachedRequiredFiles;
}

- (id)initWithFileAtPath:(NSString *)aPath name:(NSString *)aName
{
    if((self = [super initWithFileAtPath:aPath name:aName]))
    {
        NSString *iconPath = [[self bundle] pathForResource:[[self infoDictionary] objectForKey:@"CFIconName"] ofType:@"icns"];
        _icon = [[NSImage alloc] initWithContentsOfFile:iconPath];

        // invalidate global cache
        _cachedRequiredFiles = nil;
    }

    return self;
}

- (id)newPluginControllerWithClass:(Class)bundleClass
{
    if([bundleClass isSubclassOfClass:[OEGameCoreController class]])
        return [[bundleClass alloc] initWithBundle:[self bundle]];

    return nil;
}

- (NSString *)bundleIdentifier
{
    return [[self infoDictionary] objectForKey:@"CFBundleIdentifier"];
}

- (NSArray *)systemIdentifiers;
{
    return [[self infoDictionary] objectForKey:@"OESystemIdentifiers"];
}

- (NSDictionary *)coreOptions
{
    return [[self infoDictionary] objectForKey:OEGameCoreOptionsKey];
}

- (NSArray *)requiredFiles
{
    id options = [self coreOptions];
    NSMutableArray *allRequiredFiles = [NSMutableArray array];
    
    for (id key in options) {
        id resultDict = [options objectForKey:key];
        if([resultDict objectForKey:@"OERequiredFiles"] != nil)
            [allRequiredFiles addObjectsFromArray:[resultDict objectForKey:@"OERequiredFiles"]];
    }
    
    if([allRequiredFiles count] > 0)
       return [allRequiredFiles copy];
    else
        return nil;
}

- (Class)gameCoreClass
{
    if (_gameCoreClass == Nil)
    {
        [[self bundle] load];
        _gameCoreClass = NSClassFromString([[self infoDictionary] objectForKey:OEGameCoreClassKey]);
    }

    return _gameCoreClass;
}

@end
