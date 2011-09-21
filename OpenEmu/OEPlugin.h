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

#import <Cocoa/Cocoa.h>

#import "OEPluginController.h"



@interface NSObject (OEPlugin)
+ (BOOL)isPluginClass;
@end



@interface OEPlugin : NSObject <NSCopying>
{
@private
    NSDictionary           *infoDictionary;
    NSBundle               *bundle;
    NSString               *displayName;
    NSString               *version;
    id<OEPluginController>  controller;
}

+ (NSSet *)pluginClasses;
+ (void)registerPluginClass:(Class)pluginClass;

// Subclass hook to perform checks or setups of the controller.
- (id<OEPluginController>)newPluginControllerWithClass:(Class)bundleClass;

@property(readonly) id<OEPluginController>  controller; // Main Class of the bundle, can be nil
@property(readonly) NSDictionary           *infoDictionary;
@property(readonly) NSBundle               *bundle;
@property(readonly) NSString               *details;
@property(readonly) NSString               *displayName;
@property(readonly) NSString               *version;

// All plugins should be retrieved with this method
// Ensuring a plugin is loaded only once
+ (id)pluginWithBundleName:(NSString *)aName type:(Class)pluginType;
+ (id)pluginWithBundleAtPath:(NSString *)bundlePath type:(Class)aType;
+ (id)pluginWithBundleAtPath:(NSString *)bundlePath type:(Class)aType forceReload:(BOOL)reload;
+ (NSArray *)pluginsForType:(Class)aType;
+ (NSArray *)allPlugins;

- (id)initWithBundle:(NSBundle *)aBundle;

+ (NSString *)pluginType;
+ (NSString *)pluginFolder;
+ (NSString *)pluginExtension;
+ (Class)typeForExtension:(NSString *)anExtension;

- (NSArray *)availablePreferenceViewControllerKeys;
@end
