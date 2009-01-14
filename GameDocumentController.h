//
//  GameDocumentController.h
//  Gambatte
//
//  Created by Ben on 24/08/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "GameCore.h"

@class GameDocument, GamePreferencesController;

@interface GameDocumentController : NSDocumentController {
	GameDocument* currentGame;
	IBOutlet GamePreferencesController* preferenceController;
	NSArray* bundles;
	NSArray* validExtensions;
	BOOL gameLoaded;
}

@property(readonly) NSArray* bundles;
@property(readwrite) BOOL gameLoaded;

+ (NSDictionary*) defaultControls;
+ (NSDictionary*) defaultGamepadControls;

-(NSBundle*) bundleForType:(NSString*) type;

- (id <GameCore>) currentGame;
- (IBAction) closeWindow: (id) sender;
- (IBAction) switchFullscreen: (id) sender;

- (IBAction) saveState: (id) sender;
- (IBAction) loadState: (id) sender;

- (IBAction) resetGame: (id) sender;

- (IBAction) scrambleRam: (id) sender;

- (IBAction) updateBundles: (id) sender;
-(BOOL) isGameKey;

-(void) setCurrentGame: (GameDocument*) game;
-(GamePreferencesController*) preferenceController;

@end