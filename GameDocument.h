//
//  MyDocument.h
//  OpenEmu
//
//  Created by Josh Weinberg on 9/1/08.
//  Copyright __MyCompanyName__ 2008 . All rights reserved.
//


#import <Cocoa/Cocoa.h>
#import <QuartzCore/CoreAnimation.h>

@class GameCore;
@class GameAudio;
@class OEGameLayer;
@class GameQTRecorder;

@interface GameDocument : NSDocument
{
	NSTimer           *frameTimer;
	CALayer           *rootLayer;
	OEGameLayer       *gameLayer;
	GameQTRecorder    *recorder;
	BOOL               keyedOnce;
	GameCore          *gameCore;
	GameAudio         *audio;
	IBOutlet NSWindow *gameWindow;
	IBOutlet NSView   *view;
}

@property(readonly) GameCore *gameCore;

- (void)scrambleBytesInRam:(NSUInteger)bytes;
- (BOOL)isFullScreen;
- (void)refresh;
- (void)saveStateToFile:(NSString *)fileName;
- (void)loadStateFromFile:(NSString *)fileName;

- (IBAction)loadState:(id)sender;
- (IBAction)resetGame:(id)sender;
- (IBAction)saveState:(id)sender;
- (IBAction)scrambleRam:(id)sender;
- (IBAction)toggleFullScreen:(id)sender;

- (BOOL)backgroundPauses;
- (BOOL)defaultsToFullScreenMode;

- (void)savePanelDidEnd:(NSSavePanel *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo;
- (void)openPanelDidEnd:(NSOpenPanel *)panel returnCode:(int)returnCode contextInfo:(void *)contextInfo;

- (NSBitmapImageRep *)screenshot;

@end
