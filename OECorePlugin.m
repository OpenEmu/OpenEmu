/*
 Copyright (c) 2009, OpenEmu Team
 All rights reserved.
 
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
