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

- (id)initWithFileAtPath:(NSString *)aPath name:(NSString *)aName error:(NSError *__autoreleasing *)outError
{
    if((self = [super initWithFileAtPath:aPath name:aName error:outError]))
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
    return [[self infoDictionary] objectForKey:OEGameCoreSystemIdentifiersKey];
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
        if([resultDict objectForKey:OEGameCoreRequiredFilesKey] != nil)
            [allRequiredFiles addObjectsFromArray:[resultDict objectForKey:OEGameCoreRequiredFilesKey]];
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

- (BOOL)_isMarkedDeprecatedInInfoPlist
{
    if (![self.infoDictionary[OEGameCoreDeprecatedKey] boolValue])
        return NO;
        
    NSString *minOSXVer = self.infoDictionary[OEGameCoreDeprecatedMinMacOSVersionKey];
    if (!minOSXVer)
        return YES;
    NSArray *osxVerComponents = [minOSXVer componentsSeparatedByString:@"."];
    if (osxVerComponents.count < 2)
        return YES;
    NSOperatingSystemVersion minOsxVerParsed = (NSOperatingSystemVersion){
        atoi([osxVerComponents[0] UTF8String]),
        atoi([osxVerComponents[1] UTF8String]),
        osxVerComponents.count > 2 ? atoi([osxVerComponents[2] UTF8String]) : 0};
    if ([[NSProcessInfo processInfo] isOperatingSystemAtLeastVersion:minOsxVerParsed])
        return YES;
    return NO;
}

- (BOOL)isDeprecated
{
    if (self.isOutOfSupport)
        return YES;
    return [self _isMarkedDeprecatedInInfoPlist];
}

- (BOOL)isOutOfSupport
{
    /* plugins deprecated 2017-11-04 */
    NSString *bundleName = [[[self bundle] bundleURL] lastPathComponent];
    NSArray *deprecatedPlugins = @[
        @"NeoPop.oecoreplugin",
        @"TwoMbit.oecoreplugin",
        @"VisualBoyAdvance.oecoreplugin",
        @"Yabause.oecoreplugin"];
    if ([deprecatedPlugins containsObject:bundleName])
        return YES;
        
    /* beta-era plugins */
    NSString *appcastURL = self.infoDictionary[@"SUFeedURL"];
    if ([appcastURL containsString:@"openemu.org/update"])
        return YES;
        
    /* plugins marked as deprecated in the Info.plist keys */
    if ([self _isMarkedDeprecatedInInfoPlist]) {
        NSDate *deadline = self.infoDictionary[OEGameCoreSupportDeadlineKey];
        if (!deadline) return NO;
        if ([[NSDate date] compare:deadline] == NSOrderedDescending) {
            // we are past the support deadline; return YES to remove the core
            [self _prepareForRemoval];
            return YES;
        }
    }
    
    return NO;
}

- (void)_prepareForRemoval
{
    NSDictionary *replacements = self.infoDictionary[OEGameCoreSuggestedReplacement];

    NSUserDefaults *ud = [NSUserDefaults standardUserDefaults];
    for (NSString *system in self.systemIdentifiers) {
        NSString *replacement = [replacements objectForKey:system];
        NSString *prefKey = [@"defaultCore." stringByAppendingString:system];
        NSString *currentCore = [ud stringForKey:prefKey];
        if (currentCore && [currentCore isEqual:self.bundleIdentifier]) {
            if (replacement)
                [ud setObject:replacement forKey:prefKey];
            else
                [ud removeObjectForKey:prefKey];
        }
    }
}

@end
