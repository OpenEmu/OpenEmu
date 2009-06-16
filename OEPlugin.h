//
//  OEPlugin.h
//  OpenEmu
//
//  Created by Remy Demarest on 15/06/2009.
//  Copyright 2009 Psycho Inc.. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface NSObject (OEPlugin)
+ (BOOL)isPluginClass;
@end


@interface OEPlugin : NSObject <NSCopying>
{
    NSDictionary *infoDictionary;
    NSBundle     *bundle;
    NSString     *displayName;
    NSString     *version;
}

@property(readonly) NSDictionary *infoDictionary;
@property(readonly) NSBundle     *bundle;
@property(readonly) NSString     *displayName;
@property(readonly) NSString     *version;

// All plugins should be get with this method
// It ensures a plugin is loaded only once
+ (id)pluginWithBundleName:(NSString *)aName type:(Class)pluginType;
+ (id)pluginWithBundleAtPath:(NSString *)bundlePath type:(Class)aType;
+ (NSArray *)pluginsForType:(Class)aType;
+ (NSArray *)allPlugins;

- (id)initWithBundle:(NSBundle *)aBundle;

+ (NSString *)pluginType;
+ (NSString *)pluginFolder;
+ (NSString *)pluginExtension;

@end
