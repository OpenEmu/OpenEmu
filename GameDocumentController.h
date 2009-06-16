//
//  GameDocumentController.h
//  Gambatte
//
//  Created by Ben on 24/08/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <IOKit/hid/IOHIDLib.h>
#import <IOKit/hid/IOHIDUsageTables.h>
#import "OEHIDDeviceHandler.h"

@class GameCore, OECorePlugin;
@class GameDocument, OEGamePreferenceController;

@interface GameDocumentController : NSDocumentController {
	GameDocument               *currentGame;
    OEGamePreferenceController *preferences;
    NSArray                    *filterNames;
    NSArray                    *plugins;
	NSArray                    *validExtensions;
	BOOL                        gameLoaded;
    
    IOHIDManagerRef             hidManager;
    NSMutableArray             *deviceHandlers;
}

@property(readonly) NSArray *filterNames;
@property(readonly) NSArray *plugins;
@property(readwrite) BOOL gameLoaded;

- (GameDocument *)currentDocument;

- (OECorePlugin *)pluginForType:(NSString *)type;
- (void) updateInfoPlist;

- (GameCore *)currentGame;

- (IBAction)updateBundles:(id)sender;
- (BOOL)isGameKey;

- (IBAction)openPreferenceWindow:(id)sender;

@end