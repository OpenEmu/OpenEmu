//
//  OEPluginController.h
//  OpenEmu
//
//  Created by Remy Demarest on 23/04/2011.
//  Copyright 2011 NuLayer Inc. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@protocol OEPluginController <NSObject>

// Support for preference view controllers
- (NSArray *)availablePreferenceViewControllerKeys;
- (NSViewController *)preferenceViewControllerForKey:(NSString *)aKey;

@end
