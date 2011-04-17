//
//  OESystemPlugin.h
//  OpenEmu
//
//  Created by Remy Demarest on 17/04/2011.
//  Copyright 2011 NuLayer Inc. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "OECorePlugin.h"

@class OEGameSystemController;

@interface OEGameSystemPlugin : OEPlugin
{
    NSImage                *icon;
    Class                   inputViewClass;
    OEGameSystemController *controller;
}

@property(readonly) OEGameSystemController *controller;
@property(readonly) NSImage                *icon;
@property(readonly) Class                   inputViewClass;

+ (OEGameSystemPlugin *)systemPluginWithBundleAtPath:(NSString *)bundlePath;

- (NSArray *)availablePreferenceViewControllers;
- (NSViewController *)newPreferenceViewControllerForKey:(NSString *)aKey;

@end
