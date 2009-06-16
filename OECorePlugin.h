//
//  OECorePlugin.h
//  OpenEmu
//
//  Created by Remy Demarest on 15/06/2009.
//  Copyright 2009 Psycho Inc.. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "OEPlugin.h"

@class OEGameCoreController;

@interface OECorePlugin : OEPlugin
{
    NSImage              *icon;
    NSDictionary         *supportedTypes;
    NSArray              *supportedTypeExtensions;
    Class                 gameCoreClass;
    OEGameCoreController *controller;
}

@property(readonly) OEGameCoreController *controller;
@property(readonly) NSString             *details;
@property(readonly) NSImage              *icon;
@property(readonly) NSDictionary         *supportedTypes;
@property(readonly) NSArray              *supportedTypeNames;
@property(readonly) NSArray              *supportedTypeExtensions;
@property(readonly) Class                 gameCoreClass;

- (NSArray *)availablePreferenceViewControllers;
- (NSViewController *)newPreferenceViewControllerForKey:(NSString *)aKey;
- (NSArray *)extensionsForTypeName:(NSString *)aTypeName;
- (BOOL)supportsFileExtension:(NSString *)extension;

- (void)updateBundle:(id)sender;

@end
