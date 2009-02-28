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
@class GameLayer;
@class GameBuffer;
@class GameQTRecorder;

@interface GameDocument : NSDocument
{
	NSTimer* frameTimer;
	CALayer *rootLayer;
	GameLayer *gameLayer;
	GameBuffer *gameBuffer;
	GameQTRecorder *recorder;
	BOOL keyedOnce;
	GameCore * gameCore;
	GameAudio * audio;
	IBOutlet NSWindow *gameWindow;
	IBOutlet NSView *view;
}

- (void) scrambleRam:(int) bytes;
- (BOOL) isFullScreen;
- (void) switchFullscreen;
- (void) refresh;
- (void) refreshAudio;
- (void) saveStateToFile: (NSString *) fileName;
- (void) loadStateFromFile: (NSString *) fileName;

- (IBAction)loadState:(id)sender;
- (IBAction)saveState:(id)sender;

- (void)savePanelDidEnd:(NSSavePanel *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo;
- (void)openPanelDidEnd:(NSOpenPanel *)panel returnCode:(int)returnCode contextInfo:(void *)contextInfo;

- (NSBitmapImageRep*) getRawScreenshot;

- (void) setVolume: (float) volume;

- (void) resetFilter;

@property(readonly) GameCore* gameCore;
@end
