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

@import Cocoa;

NS_ERROR_ENUM(OEGameCoreErrorDomain) {
    OEGameCorePluginAlreadyLoadedError = -1000,
    OEGameCorePluginInvalidError       = -1001,
    OEGameCorePluginOutOfSupportError  = -1002,
};

@interface NSObject (OEPlugin)
@property(class, readonly) BOOL isPluginClass;
@end

@interface OEPlugin : NSObject <NSCopying>

@property(class, readonly) NSSet <Class> *pluginClasses;

+ (void)registerPluginClass;
+ (void)registerPluginClass:(Class)pluginClass;

// Subclass hook to perform checks or setups of the controller.
- (id)newPluginControllerWithClass:(Class)bundleClass;

@property(readonly) NSString *path;
@property(readonly) NSString *name;

// Properties only available when dealing with bundle-based plugins.
@property(readonly) id controller; // Main Class of the bundle, can be nil.
@property(readonly) NSString *displayName;
@property(readonly) NSBundle *bundle;
@property(readonly) NSDictionary *infoDictionary;
@property(readonly) NSString *details;
@property(readonly) NSString *version;

// All plugins should be retrieved with this method
// Ensuring a plugin is loaded only once
+ (instancetype)pluginWithName:(NSString *)aName; // Do not call on OEPlugin, only call on subclasses.
+ (instancetype)pluginWithName:(NSString *)aName type:(Class)pluginType;
+ (instancetype)pluginWithFileAtPath:(NSString *)aPath type:(Class)aType;
+ (instancetype)pluginWithFileAtPath:(NSString *)aPath type:(Class)aType forceReload:(BOOL)reload;
+ (instancetype)pluginWithFileAtPath:(NSString *)aPath type:(Class)aType forceReload:(BOOL)reload error:(NSError *__autoreleasing *)outError;
+ (instancetype)pluginWithBundle:(NSBundle *)aBundle type:(Class)aType forceReload:(BOOL)reload;
+ (NSArray *)pluginsForType:(Class)aType;
@property(class, readonly) NSArray <__kindof OEPlugin *> *allPlugins;
@property(class, readonly) NSArray <NSString *> *allPluginNames;

// Designated initializer for plugin files that do not use bundles.
- (instancetype)init NS_UNAVAILABLE;
- (instancetype)initWithFileAtPath:(NSString *)aPath name:(NSString *)aName error:(NSError *__autoreleasing *)outError NS_DESIGNATED_INITIALIZER;
- (instancetype)initWithFileAtPath:(NSString *)aPath name:(NSString *)aName;
- (instancetype)initWithBundle:(NSBundle *)aBundle;

@property(class, readonly) NSString *pluginType;
@property(class, readonly) NSString *pluginFolder;
@property(class, readonly) NSString *pluginExtension;
+ (Class)typeForExtension:(NSString *)anExtension;

@property(readonly) NSArray <NSString *> *availablePreferenceViewControllerKeys;

@property (readonly, getter=isDeprecated) BOOL deprecated;
// When YES, the plugin is automatically removed
@property (readonly, getter=isOutOfSupport) BOOL outOfSupport;

@end
