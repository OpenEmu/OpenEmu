//
//  GamePreferencesController.h
//  OpenEmu
//
//  Created by Josh Weinberg on 9/10/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>


#include <IOKit/hid/IOHIDLib.h>
#include <IOKit/hid/IOHIDUsageTables.h>



extern NSString * const OEGameControls;
extern NSString * const OEFullScreen;
extern NSString * const OEFilter;
extern NSString * const OEGamepadControls;
extern NSString * const OEVolume;
extern NSString * const OEPauseBackground;
extern NSString * const OEQTFrameRate;

@class GameDocumentController, GameButton, KeyboardButton;


@interface GamePreferencesController : NSWindowController {
	
	IBOutlet NSTableView* controlsTableView;
	IBOutlet NSTableView* gamepadTableView;
	IBOutlet NSTableView* pluginsTableView;
	
	IBOutlet NSWindow* keyPanel;
	IBOutlet NSWindow* gamepadPanel;
	
	IBOutlet NSView* controlsView;
	IBOutlet NSView* videoView;
	IBOutlet NSView* pluginsView;
	IBOutlet NSView* audioView;
	
	NSString* currentViewIdentifier;
	
	NSArray* bundles;
	
	NSDictionary* customViews;
	NSDictionary* customIcons;
	
	
	GameDocumentController* docController;
}

@property (readonly) NSArray* bundles;

- (NSRect)newFrameForNewContentView:(NSView *)view;

@end

@interface GamePreferencesController (Recording)

- (int) qtFrameRate;

@end


@interface GamePreferencesController (Controls)

- (void) bindKeyFromSheet: (int) key;
- (void) doubleClickedKeyboard;

- (BOOL) bindButtonFromSheet: (GameButton*) button;
- (void) doubleClickedGamepad;

- (NSString*) keyForIndex:(int) index;
- (NSString*) friendlyDescription: (NSArray*) keyArray;

- (void) setKeyboardControl: (KeyboardButton*) button;
- (void) setGamepadControl: (GameButton*) button;
- (void) setControl: (NSString*) dictKey withKey: (int) keyCode;


- (IBAction) restoreDefaultControls: (id) sender;

@end

@interface GamePreferencesController (Video)

- (BOOL) fullScreen;
- (IBAction) switchFilter: (id) sender;
- (int) filter;
- (BOOL) pauseBackground;
@end

@interface GamePreferencesController (Audio)

- (IBAction) setVolume: (id) sender;
- (float) volume;

@end


@interface GamePreferencesController (Plugins)



@end



@interface GamePreferencesController (UserDefaults)

- (NSDictionary*) gamepadControls;
- (NSDictionary*) gameControls;

@end

@interface GamePreferencesController (Toolbar)

- (void)setupToolbarForWindow:(NSWindow *)theWindow;
- (IBAction) switchView: (id) sender;
-(NSView *)viewForIdentifier:(NSString*)identifier;

@end


@interface GamepadRebindView : NSView {
	IBOutlet NSWindow* window;
	IBOutlet GamePreferencesController* controller;
	
	IOHIDManagerRef hidManager;
}
- (IBAction) close: (id) sender;
- (void) launchHIDThread;

@property(readonly) GamePreferencesController* controller;


@end

@interface ControlRebindView : NSView {
	IBOutlet NSWindow* window;
	IBOutlet GamePreferencesController* controller;
}


@end
