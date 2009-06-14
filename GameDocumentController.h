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

@class GameCore, PluginInfo;
@class GameDocument, OEGamePreferenceController;

@interface GameDocumentController : NSDocumentController {
	GameDocument *currentGame;
    OEGamePreferenceController *preferences;
    NSArray *plugins;
	NSArray *bundles;
	NSArray *validExtensions;
	BOOL gameLoaded;
    
    IOHIDManagerRef hidManager;
    NSMutableArray *deviceHandlers;
}

@property(readonly) NSArray *plugins;
@property(readonly) NSArray *bundles;
@property(readwrite) BOOL gameLoaded;

- (GameDocument *)currentDocument;

- (PluginInfo *)pluginForType:(NSString *)type;
- (void) updateInfoPlist;

- (IBAction)changeVideoFilter:(id)sender;
- (IBAction)changeVolume:(id)sender;

- (GameCore *)currentGame;

- (IBAction)saveState: (id) sender;
- (IBAction)loadState: (id) sender;

- (IBAction)resetGame: (id) sender;

- (IBAction)scrambleRam: (id) sender;

- (IBAction)updateBundles: (id) sender;
- (BOOL)isGameKey;

- (IBAction)openPreferenceWindow:(id)sender;

- (IOHIDDeviceRef)deviceWithManufacturer:(NSString *)aManufacturer productID:(NSNumber *)aProductID locationID:(NSNumber *)aLocationID;

@end