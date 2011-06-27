//
//  OEGameViewController.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 25.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class OEDBGame;
@class GameQTRecorder;
@protocol OEGameCoreHelper;
@class OEGameView;
@class OEGameCoreManager;
@class OEGameCoreController;
#import "OETaskWrapper.h"
@interface OEGameViewController : NSViewController <OETaskWrapperController>{
@private
    OEDBGame* game;
    IBOutlet OEGameView* gameView;
    
    // IPC from our OEHelper
    id<OEGameCoreHelper>  rootProxy;
    OEGameCoreManager    *gameCoreManager;
    
    // Standard game document stuff
    NSTimer              *frameTimer;
    GameQTRecorder       *recorder;
    NSString             *emulatorName;
    OEGameCoreController *gameController;
    NSToolbarItem        *playPauseToolbarItem;
    BOOL                  keyedOnce;
}
- (BOOL)loadFromURL:(NSURL*)url error:(NSError**)error;

- (void)terminateEmulation;

- (void)scrambleBytesInRam:(NSUInteger)bytes;
- (void)refresh DEPRECATED_ATTRIBUTE;
- (void)saveStateToFile:(NSString *)fileName;
- (void)loadStateFromFile:(NSString *)fileName;

- (void)captureScreenshotUsingBlock:(void(^)(NSImage *img))block;

- (IBAction)loadState:(id)sender;
- (IBAction)resetGame:(id)sender;
- (IBAction)saveState:(id)sender;
- (IBAction)scrambleRam:(id)sender;
- (IBAction)toggleFullScreen:(id)sender;
- (IBAction)playPauseGame:(id)sender;

- (BOOL)backgroundPauses;
- (BOOL)defaultsToFullScreenMode;

- (NSImage *)screenShot DEPRECATED_ATTRIBUTE;
- (void)applicationWillTerminate:(NSNotification *)aNotification;

@property (nonatomic, retain) OEDBGame* game;
@property(retain) IBOutlet OEGameView    *gameView;

@property(getter=isEmulationPaused) BOOL pauseEmulation;
@property(readonly) BOOL isFullScreen;
@property(readonly) NSString *emulatorName;
@end
