//
//  OEApplicationDelegate.h
//  OpenEmu
//
//  Created by Carl Leimbrock on 19.10.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//


@class OEHIDManager;
@class OEMainWindowController;
@class OECoreInstaller;
@class OECoreUpdater;
@interface OEApplicationDelegate : NSObject <NSApplicationDelegate>
{
}
- (IBAction)showAboutWindow:(id)sender;
- (IBAction)showPreferencesWindow:(id)sender;

- (IBAction)updateBundles:(id)sender;

- (void)updateValidExtensions;
- (void)updateInfoPlist;

- (IBAction)launchGame:(id)sender;
@property (retain) IBOutlet NSWindow *aboutWindow;
@property (retain) IBOutlet NSMenu  *startupMainMenu;
@property (retain) IBOutlet NSMenu *mainMenu;

@property (retain) OEHIDManager *hidManager;
@property (retain) OEMainWindowController *mainWindowController;
@property (readonly) NSString *aboutCreditsPath;
@property (readonly) NSString *appVersion;
@property (readonly) NSAttributedString *projectURL;
@property (retain) NSArray* validExtensions;
@property (retain) OECoreUpdater *coreUpdater;
@end
