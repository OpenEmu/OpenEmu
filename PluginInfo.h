//
//  AppData.h
//  ImageTextCellSample
//
//  Created by Martin Kahr on 04.05.07.
//  Copyright 2007 CASE Apps. All rights reserved.
//

#import <Cocoa/Cocoa.h>

extern NSString *OEControlsPreferencesClassName;

@class OEGameCoreController;

@interface PluginInfo : NSObject <NSCopying> {
	NSDictionary         *infoDictionary;
	NSImage              *icon;
	NSBundle             *bundle;
    NSString             *version;
    NSString             *displayName;
    NSDictionary         *supportedTypes;
    NSArray              *supportedTypeExtensions;
    Class                 gameCoreClass;
    OEGameCoreController *controller;
}

@property(readonly) OEGameCoreController *controller;
@property(readonly) NSBundle *bundle;
@property(readonly) NSString *displayName, *details, *version;
@property(readonly) NSImage *icon;
@property(readonly) NSDictionary *supportedTypes;
@property(readonly) NSArray *supportedTypeNames;
@property(readonly) NSArray *supportedTypeExtensions;
@property(readonly) Class gameCoreClass;

+ (id)pluginInfoWithBundleAtPath:(NSString*)path;

- (NSArray *)availablePreferenceViewControllers;
- (id)initWithBundleAtPath:(NSString*)path;
- (id)initWithInfoDictionary:(NSDictionary*)infoDict icon:(NSImage*)image;
- (NSViewController *)newPreferenceViewControllerForKey:(NSString *)aKey;
- (NSArray *)extensionsForTypeName:(NSString *)aTypeName;
- (BOOL)supportsFileExtension:(NSString *)extension;

- (void)updateBundle:(id)sender;

@end
