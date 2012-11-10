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
#import "OESystemController.h"

#import "OELibraryDatabase.h"
#import "OEDBSystem.h"

@implementation OESystemPlugin
@synthesize responderClass, bundleIcon, systemName, systemIcon, systemIdentifier;
@dynamic controller;

static NSMutableDictionary *pluginsBySystemIdentifiers = nil;
static NSArray *cachedSupportedTypeExtensions = nil;

+ (void)initialize
{
    if(self == [OESystemPlugin class])
    {
        pluginsBySystemIdentifiers = [[NSMutableDictionary alloc] init];
    }
}

+ (OESystemPlugin *)gameSystemPluginForIdentifier:(NSString *)gameSystemIdentifier;
{
    return [pluginsBySystemIdentifiers objectForKey:gameSystemIdentifier];
}

+ (void)registerGameSystemPlugin:(OESystemPlugin *)plugin forIdentifier:(NSString *)gameSystemIdentifier;
{
    [pluginsBySystemIdentifiers setObject:plugin forKey:gameSystemIdentifier];
    
    OELibraryDatabase *db = [OELibraryDatabase defaultDatabase];
    
    if(db == nil) NSLog(@"system plugins not registered in database, because the db does not exist yet!");
    else          [OEDBSystem systemForPlugin:plugin inDatabase:db];
    
    // Invalidate supported type extenesions cache
    cachedSupportedTypeExtensions = nil;
}

+ (NSArray *)supportedTypeExtensions;
{
    if(cachedSupportedTypeExtensions == nil)
    {
        NSMutableSet *extensions = [NSMutableSet set];
        for(OESystemPlugin *plugin in [OEPlugin pluginsForType:self])
            [extensions addObjectsFromArray:[plugin supportedTypeExtensions]];
        
        cachedSupportedTypeExtensions = [extensions allObjects];
    }
    
    return cachedSupportedTypeExtensions;
}

+ (OESystemPlugin *)systemPluginWithBundleAtPath:(NSString *)bundlePath;
{
    return [self pluginWithBundleAtPath:bundlePath type:self];
}

- (id)initWithBundle:(NSBundle *)aBundle
{
    if((self = [super initWithBundle:aBundle]))
    {
        systemIdentifier = [[self infoDictionary] objectForKey:OESystemIdentifier];
        responderClass   = [[self controller] responderClass];
        
        NSString *iconPath = [[self bundle] pathForResource:[[self infoDictionary] objectForKey:@"CFIconName"] ofType:@"icns"];
        
        bundleIcon = [[NSImage alloc] initWithContentsOfFile:iconPath];
        
        [[self class] registerGameSystemPlugin:self forIdentifier:systemIdentifier];
    }
    
    return self;
}

- (id<OEPluginController>)newPluginControllerWithClass:(Class)bundleClass
{
    if(![bundleClass isSubclassOfClass:[OESystemController class]]) return nil;
    
    return [[bundleClass alloc] initWithBundle:[self bundle]];
}

- (NSString *)systemName
{
    return [[self controller] systemName];
}

- (NSImage *)systemIcon
{
    return [[self controller] systemIcon];
}

- (NSArray *)supportedTypeExtensions;
{
    return [[self controller] fileTypes];
}

@end
