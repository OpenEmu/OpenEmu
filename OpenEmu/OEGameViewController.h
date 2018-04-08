/*
 Copyright (c) 2011, OpenEmu Team
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the OpenEmu Team nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

@import Cocoa;
#import "OEGameCoreHelper.h"

extern NSString *const OEGameVolumeKey;
extern NSString *const OEGameDefaultVideoFilterKey;
extern NSString *const OEGameSystemVideoFilterKeyFormat;
extern NSString *const OEGameCoresInBackgroundKey;
extern NSString *const OEAutoSwitchCoreAlertSuppressionKey;
extern NSString *const OEGameViewControllerEmulationWillFinishNotification;
extern NSString *const OEGameViewControllerEmulationDidFinishNotification;
extern NSString *const OEGameViewControllerROMKey;
extern NSString *const OEBackgroundPauseKey;
extern NSString *const OEBackgroundControllerPlayKey;
extern NSString *const OEGameViewBackgroundColorKey;
extern NSString *const OETakeNativeScreenshots;

extern NSString *const OEScreenshotFileFormatKey;
extern NSString *const OEScreenshotPropertiesKey;


@class OEDBRom;
@class OEDBGame;
@class OEDBSaveState;

@class OEGameView;

@class OEGameControlsBar;
@class OEGameDocument;
@class OEDBSaveState;

@class OECorePlugin;

@protocol OEGameIntegralScalingDelegate;

@interface OEGameViewController : NSViewController

#pragma mark -

@property(strong) OEGameControlsBar *controlsWindow;
@property(readonly) OEGameView *gameView;

@property(unsafe_unretained) id<OEGameIntegralScalingDelegate> integralScalingDelegate;

@property(weak) OEGameDocument *document;

@property(readonly) BOOL supportsCheats;
@property(readonly) BOOL supportsSaveStates;
@property(readonly) BOOL supportsMultipleDiscs;
@property(readonly) NSUInteger discCount;
@property(readonly) NSString *coreIdentifier;
@property(readonly) NSString *systemIdentifier;

- (NSImage *)takeNativeScreenshot;

- (void)reflectVolume:(float)volume;
- (void)reflectEmulationPaused:(BOOL)paused;
#pragma mark - HUD Bar Actions
// switchCore:: expects sender or [sender representedObject] to be an OECorePlugin object and prompts the user for confirmation
- (void)selectFilter:(id)sender;
- (void)toggleControlsVisibility:(id)sender;

#pragma mark - Taking Screenshots
- (IBAction)takeScreenshot:(id)sender;

#pragma mark - Info
@property(readonly) NSSize defaultScreenSize;

- (void)setEnableVSync:(BOOL)enable;
- (void)setAspectSize:(OEIntSize)newAspectSize;
- (void)setScreenSize:(OEIntSize)newScreenSize withIOSurfaceID:(IOSurfaceID)newSurfaceID;
- (void)setScreenSize:(OEIntSize)newScreenSize aspectSize:(OEIntSize)newAspectSize withIOSurfaceID:(IOSurfaceID)newSurfaceID;
- (void)setDiscCount:(NSUInteger)discCount;

@end
