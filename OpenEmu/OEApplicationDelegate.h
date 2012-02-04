//
//  OEApplicationDelegate.h
//  OpenEmu
//
//  Created by Carl Leimbrock on 19.10.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//


@class OEHIDManager;
@class OEMainWindowController;
@interface OEApplicationDelegate : NSObject <NSApplicationDelegate>
{
}
- (IBAction)showAboutWindow:(id)sender;
- (IBAction)showPreferencesWindow:(id)sender;

- (IBAction)updateBundles:(id)sender;

- (void)updateInfoPlist;

@property (retain) IBOutlet NSWindow *aboutWindow;

@property (retain) OEHIDManager *hidManager;
@property (retain) IBOutlet OEMainWindowController *mainWindowController;
@property (readonly) NSString *aboutCreditsPath;
@property (readonly) NSString *appVersion;
@property (readonly) NSAttributedString *projectURL;
@end
