//
//  OESystemPlugin.h
//  OpenEmu
//
//  Created by Remy Demarest on 17/04/2011.
//  Copyright 2011 NuLayer Inc. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "OECorePlugin.h"

@class OESystemController;

extern NSString *const OESystemPluginName;

@interface OESystemPlugin : OEPlugin

@property(readonly) OESystemController<OEPluginController> *controller;

@property(readonly) NSString *gameSystemName;
@property(readonly) NSImage  *icon;
@property(readonly) Class     responderClass;

+ (OESystemPlugin *)gameSystemPluginForName:(NSString *)gameSystemName;
+ (void)registerGameSystemPlugin:(OESystemPlugin *)plugin forName:(NSString *)gameSystemName;

+ (OESystemPlugin *)systemPluginWithBundleAtPath:(NSString *)bundlePath;

@end
