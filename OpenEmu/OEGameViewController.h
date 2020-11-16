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
@import OpenEmuKit;

extern NSNotificationName const OEGameViewControllerEmulationWillFinishNotification;
extern NSNotificationName const OEGameViewControllerEmulationDidFinishNotification;
extern NSString *const OEGameVolumeKey;
extern NSString *const OEGameDefaultVideoShaderKey;
extern NSString *const OEGameSystemVideoShaderKeyFormat;
extern NSString *const OEGameCoreDisplayModeKeyFormat;
extern NSString *const OEGameCoresInBackgroundKey;
extern NSString *const OEGameViewControllerROMKey;
extern NSString *const OEBackgroundPauseKey;
extern NSString *const OEBackgroundControllerPlayKey;
extern NSString *const OEGameViewBackgroundColorKey;
extern NSString *const OEPopoutGameWindowAlwaysOnTopKey;
extern NSString *const OEPopoutGameWindowIntegerScalingOnlyKey;
extern NSString *const OETakeNativeScreenshots;

extern NSString *const OEScreenshotFileFormatKey;
extern NSString *const OEScreenshotPropertiesKey;


@class OEDBRom;
@class OEDBGame;
@class OEDBSaveState;

@class OEGameLayerView;

@class OEGameControlsBar;
@class OEGameDocument;
@class OEDBSaveState;

@class OECorePlugin;
@class OEGameViewController;

@class OEShaderParamGroupValue;

@protocol OEGameIntegralScalingDelegate;

@protocol OEGameViewControllerDelegate <NSObject>
- (void)gameViewController:(OEGameViewController *)sender didReceiveMouseEvent:(OEEvent *)event;
- (void)gameViewController:(OEGameViewController *)sender updateBounds:(CGRect)newBounds;
- (void)gameViewController:(OEGameViewController *)sender updateBackingScaleFactor:(CGFloat)newScaleFactor;
- (void)gameViewController:(OEGameViewController *)sender setShaderURL:(NSURL *)url completionHandler:(void (^)(BOOL success, NSError *error))block;
- (void)gameViewController:(OEGameViewController *)sender shaderParamGroupsWithCompletionHandler:(void (^)(NSArray<OEShaderParamGroupValue *> *))handler;
- (void)gameViewController:(OEGameViewController *)sender setShaderParameterValue:(CGFloat)value atIndex:(NSUInteger)index atGroupIndex:(NSUInteger)group;

@end

@interface OEGameViewController : NSViewController <NSMenuItemValidation>

#pragma mark -

@property(strong) OEGameControlsBar *controlsWindow;

@property(unsafe_unretained) id<OEGameIntegralScalingDelegate> integralScalingDelegate;

@property(weak) OEGameDocument *document;

@property(readonly) BOOL supportsCheats;
@property(readonly) BOOL supportsSaveStates;
@property(readonly) BOOL supportsMultipleDiscs;
@property(readonly) BOOL supportsFileInsertion;
@property(readonly) BOOL supportsDisplayModeChange;
@property(readonly) NSString *coreIdentifier;
@property(readonly) NSString *systemIdentifier;

#pragma mark - Game View control

@property (nonatomic, readonly) OEGameLayerView *gameView;
- (void)gameViewSetIntegralSize:(NSSize)size animated:(BOOL)animated;
- (void)gameViewFillSuperView;

- (void)setScreenSize:(OEIntSize)newScreenSize aspectSize:(OEIntSize)newAspectSize;

- (void)reflectVolume:(float)volume;
- (void)reflectEmulationPaused:(BOOL)paused;

#pragma mark - HUD Bar Actions
// switchCore:: expects sender or [sender representedObject] to be an OECorePlugin object and prompts the user for confirmation
- (void)selectShader:(id)sender;
- (void)configureShader:(id)sender;
- (void)toggleControlsVisibility:(id)sender;

#pragma mark - Screenshots support
- (NSImage *)screenshot;
- (IBAction)takeScreenshot:(id)sender;

#pragma mark - Info
@property (readonly) NSSize defaultScreenSize;
@property (readonly) OEIntSize aspectSize;
@property (readonly) OEIntSize screenSize;
@property (nonatomic) NSUInteger discCount;
@property (nonatomic) NSArray <NSDictionary <NSString *, id> *> *displayModes;
@property (nonatomic) NSUInteger remoteContextID;

@end

@interface OEGameViewController (Notifications)

- (void)showQuickSaveNotification;
- (void)showScreenShotNotification;
- (void)showFastForwardNotification:(BOOL)enable;
- (void)showRewindNotification:(BOOL)enable;
- (void)showStepForwardNotification;
- (void)showStepBackwardNotification;

@end
