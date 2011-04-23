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

extern NSString *const OEGameSystemPluginName;

@interface OEGameSystemPlugin : OEPlugin

@property(readonly) OEGameSystemController<OEPluginController> *controller;

@property(readonly) NSString *gameSystemName;
@property(readonly) NSImage  *icon;
@property(readonly) Class     responderClass;

+ (OEGameSystemPlugin *)gameSystemPluginForName:(NSString *)gameSystemName;
+ (void)registerGameSystemPlugin:(OEGameSystemPlugin *)plugin forName:(NSString *)gameSystemName;

+ (OEGameSystemPlugin *)systemPluginWithBundleAtPath:(NSString *)bundlePath;

@end
