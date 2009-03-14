//
//  OEGamePreferenceController.h
//  OpenEmu
//
//  Created by Remy Demarest on 25/02/2009.
//  Copyright 2009 Psycho Inc.. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class PluginInfo;

@interface OEGamePreferenceController : NSWindowController
{
    IBOutlet NSDrawer *pluginDrawer;
    NSDictionary *preferencePanels;
    NSArray *plugins;
    NSString *currentViewIdentifier;
    NSIndexSet *selectedPlugins;
    PluginInfo *currentPlugin;
    NSViewController *currentViewController;
}

//@property(readonly) NSArray *controlPlugins;
@property(readonly) NSArray *plugins;
@property(retain) NSIndexSet *selectedPlugins;

@end
