//
//  OESaveStateController.h
//  OpenEmu
//
//  Created by Joshua Weinberg on 7/28/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class OECorePlugin, GameDocumentController;

@interface OESaveStateController : NSWindowController {
	NSPredicate                *availablePluginsPredicate;
    NSIndexSet                 *selectedPlugins;
    OECorePlugin               *currentPlugin;	
	GameDocumentController	   *docController;
    IBOutlet NSArrayController *pluginController;
	IBOutlet NSPredicate *selectedRomPredicate;
}

@property(retain) GameDocumentController *docController;
@property(readonly) NSArray     *plugins;
@property(retain)   NSPredicate *availablePluginsPredicate;
@property(retain)   NSIndexSet  *selectedPlugins;
@property(retain)	NSPredicate *selectedRomPredicate;
@end
