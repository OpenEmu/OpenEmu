//
//  OEGamePreferenceController.h
//  OpenEmu
//
//  Created by Remy Demarest on 25/02/2009.
//  Copyright 2009 Psycho Inc.. All rights reserved.
//

#import <Cocoa/Cocoa.h>

extern NSString *const OEControlsPreferenceKey;
extern NSString *const OEAdvancedPreferenceKey;

@class PluginInfo;

@interface OEGamePreferenceController : NSWindowController
{
    IBOutlet NSDrawer          *pluginDrawer;
    IBOutlet NSArrayController *pluginController;
    NSDictionary               *preferencePanels;
    NSArray                    *plugins;
    NSString                   *currentViewIdentifier;
    NSPredicate                *availablePluginsPredicate;
    NSIndexSet                 *selectedPlugins;
    PluginInfo                 *currentPlugin;
    NSViewController           *currentViewController;
}

@property(readonly) NSArray     *plugins;
@property(retain)   NSPredicate *availablePluginsPredicate;
@property(retain)   NSIndexSet  *selectedPlugins;

@end
