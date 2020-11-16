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

#import "OEGameViewController.h"

@import OpenEmuKit;
@import OpenEmuSystem;

#import "OEGameControlsBar.h"

#import "OEGameDocument.h"

#import "OpenEmu-Swift.h"

NSNotificationName const OEGameViewControllerEmulationWillFinishNotification = @"OEGameViewControllerEmulationWillFinishNotification";
NSNotificationName const OEGameViewControllerEmulationDidFinishNotification = @"OEGameViewControllerEmulationDidFinishNotification";
NSString *const OEGameVolumeKey = @"volume";
NSString *const OEGameDefaultVideoShaderKey = @"videoShader";
NSString *const OEGameSystemVideoShaderKeyFormat = @"videoShader.%@";
NSString *const OEGameCoreDisplayModeKeyFormat = @"displayMode.%@";
NSString *const OEGameCoresInBackgroundKey = @"gameCoreInBackgroundThread";
NSString *const OEBackgroundPauseKey = @"backgroundPause";
NSString *const OEBackgroundControllerPlayKey = @"backgroundControllerPlay";
NSString *const OEPopoutGameWindowAlwaysOnTopKey        = @"OEPopoutGameWindowAlwaysOnTop";
NSString *const OEPopoutGameWindowIntegerScalingOnlyKey = @"OEPopoutGameWindowIntegerScalingOnly";
NSString *const OETakeNativeScreenshots = @"takeNativeScreenshots";
NSString *const OEGameViewControllerROMKey = @"OEROM";
NSString *const OEGameViewBackgroundColorKey = @"gameViewBackgroundColor";

NSString *const OEScreenshotFileFormatKey = @"screenshotFormat";
NSString *const OEScreenshotPropertiesKey = @"screenshotProperties";

#define UDDefaultCoreMappingKeyPrefix   @"defaultCore"
#define UDSystemCoreMappingKeyForSystemIdentifier(_SYSTEM_IDENTIFIER_) [NSString stringWithFormat:@"%@.%@", UDDefaultCoreMappingKeyPrefix, _SYSTEM_IDENTIFIER_]

// arbitrary default screen size with 4:3 ratio
CGFloat const DEFAULT_WIDTH = 400.0;
CGFloat const DEFAULT_HEIGHT = 300.0;

/*
 TODO Messages to remote layer:
 - Change bounds
 - Start Syphon
 - Native screenshot
 
 Messages from remote layer:
 - Default screen size/aspect size - DONE?
 */

@interface OEGameViewController () <OEGameViewDelegate>
{
    // Standard game document stuff
    OEScaledGameLayerView   *_scaledView;
    OEGameLayerView         *_gameView;
    NSArray<NSLayoutConstraint *> *_gameViewContraints;
    
    OEGameLayerNotificationView   *_notificationView;
    BOOL        _pausedByGoingToBackground;
    OEShaderParametersWindowController *_controller;
}

@end

@implementation OEGameViewController
+ (void)initialize
{
    if([self class] == [OEGameViewController class])
    {
        [[NSUserDefaults standardUserDefaults] registerDefaults:@{
                                                                  OEScreenshotFileFormatKey : @(NSBitmapImageFileTypePNG),
                                                                  OEScreenshotPropertiesKey : @{},
                                                                  }];
    }
}

- (id)init
{
    if((self = [super init]))
    {
        _controlsWindow = [[OEGameControlsBar alloc] initWithGameViewController:self];
        [_controlsWindow setReleasedWhenClosed:NO];
        
        _controller = [[OEShaderParametersWindowController alloc] initWithGameViewController:self];
        
        _defaultScreenSize = NSMakeSize(DEFAULT_WIDTH, DEFAULT_HEIGHT);
        
        _scaledView = [[OEScaledGameLayerView alloc] initWithFrame:(NSRect){ .size = { 1.0, 1.0 }}];
        [self setView:_scaledView];
        
        _gameView = [[OEGameLayerView alloc] initWithFrame:[[self view] bounds]];
        [_gameView setDelegate:self];
        _scaledView.contentView = _gameView;
        [_scaledView setContentViewSizeFillWithAnimated:NO];
        
        _notificationView = [[OEGameLayerNotificationView alloc] initWithFrame:NSMakeRect(0, 0, 28, 28)];
        _notificationView.translatesAutoresizingMaskIntoConstraints = NO;
        _notificationView.cell.accessibilityElement = NO;
        [self.view addSubview:_notificationView];
        
        [NSLayoutConstraint activateConstraints:@[
            [_notificationView.widthAnchor constraintEqualToConstant:28],
            [_notificationView.heightAnchor constraintEqualToConstant:28],
            [_notificationView.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor constant:60],
            [_notificationView.topAnchor constraintEqualToAnchor:self.view.topAnchor constant:10],
        ]];

        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(viewDidChangeFrame:) name:NSViewFrameDidChangeNotification object:_gameView];
    }
    return self;
}

- (void)dealloc
{
    [_gameView setDelegate:nil];
    _gameView = nil;
    
    [_controlsWindow setGameWindow:nil];
    [_controlsWindow close];
    _controlsWindow = nil;
    
    [_controller.window close];
    _controller = nil;
}

#pragma mark -

- (void)viewDidAppear
{
    [super viewDidAppear];
    
    NSWindow *window = [self OE_rootWindow];
    if(window == nil) return;
    
    [_controlsWindow setGameWindow:window];
    [_controlsWindow repositionOnGameWindow];
    
    [window makeFirstResponder:_gameView];
}

- (void)viewWillDisappear
{
    [super viewWillDisappear];

    [_controlsWindow hideAnimated:NO];
    [_controlsWindow setGameWindow:nil];
    [[self OE_rootWindow] removeChildWindow:_controlsWindow];
}
#pragma mark - Game View control

- (void)gameViewSetIntegralSize:(NSSize)size animated:(BOOL)animated
{
    [_scaledView setContentViewSize:size animated:animated];
}

- (void)gameViewFillSuperView
{
    [_scaledView setContentViewSizeFillWithAnimated:NO];
}

- (void)viewDidLayout
{
    [[self document] setOutputBounds:_gameView.bounds];
}

#pragma mark - Controlling Emulation
- (BOOL)supportsCheats;
{
    return [[self document] supportsCheats];
}

- (BOOL)supportsSaveStates
{
    return [[self document] supportsSaveStates];
}

- (BOOL)supportsMultipleDiscs
{
    return [[self document] supportsMultipleDiscs];
}

- (BOOL)supportsFileInsertion
{
    return [[self document] supportsFileInsertion];
}

- (BOOL)supportsDisplayModeChange
{
    return [[self document] supportsDisplayModeChange];
}

- (NSString *)coreIdentifier;
{
    return [[self document] coreIdentifier];
}

- (NSString *)systemIdentifier;
{
    return [[self document] systemIdentifier];
}

- (NSImage *)screenshot
{
    return [self.document screenshot];
}

- (IBAction)takeScreenshot:(id)sender
{
    [[self document] takeScreenshot:sender];
}

- (void)reflectVolume:(float)volume;
{
    [[self controlsWindow] reflectVolume:volume];
}

- (void)reflectEmulationPaused:(BOOL)paused;
{
    [[self controlsWindow] reflectEmulationRunning:!paused];
}

- (void)toggleControlsVisibility:(NSMenuItem*)sender
{
    [sender setState:![sender state]];
    [[self controlsWindow] setCanShow:[sender state]==NSControlStateValueOff];
}

#pragma mark - HUD Bar Actions
- (void)selectShader:(id)sender
{
    NSString *shaderName;
    if([sender respondsToSelector:@selector(title)] && [[sender title] isKindOfClass:[NSString class]])
        shaderName = [sender title];
    else
        DLog(@"Invalid argument passed: %@", sender);

    OEShaderModel *shader = [OEShadersModel.shared shaderWithName:shaderName];
    if (shader) {
        [[self document] gameViewController:self setShaderURL:shader.url completionHandler:^(BOOL success, NSError *error) {
            if (success)
            {
                [self didLoadShader:shader];
            }
            else if (error != nil)
            {
                NSAlert *alert = [NSAlert alertWithError:error];
                [alert runModal];
            }
        }];
    }

    [NSUserDefaults.standardUserDefaults setObject:shaderName forKey:[NSString stringWithFormat:OEGameSystemVideoShaderKeyFormat, self.document.systemIdentifier]];
}

- (void)didLoadShader:(OEShaderModel *)shader
{
    _controller.shader = shader;
    if (_controller.window.isVisible)
    {
        [self configureShader:nil];
    }
}

- (void)configureShader:(id)sender
{
    if (_controller.shader == nil)
    {
        _controller.shader = [OEShadersModel.shared shaderForSystem:self.document.systemIdentifier];
    }
    
    [self.document gameViewController:self shaderParamGroupsWithCompletionHandler:^(NSArray<OEShaderParamGroupValue *> *groups) {
        self->_controller.groups = groups;
        [self->_controller showWindow:self->_controller];
    }];

}

- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
    SEL action = [menuItem action];
    if(action == @selector(toggleControlsVisibility:))
    {
        if([[self controlsWindow] canShow])
            [menuItem setState:NSControlStateValueOff];
        else
            [menuItem setState:NSControlStateValueOn];
    }
    
    
    return YES;
}

#pragma mark - OEGameCoreOwner methods

- (void)setRemoteContextID:(NSUInteger)remoteContextID
{
    _gameView.remoteContextID = (CAContextID)remoteContextID;
}

- (void)setScreenSize:(OEIntSize)newScreenSize aspectSize:(OEIntSize)newAspectSize
{
    _screenSize = newScreenSize;
    _aspectSize = newAspectSize;
    // Should never happen
    if (OEIntSizeIsEmpty(newScreenSize) && OEIntSizeIsEmpty(newAspectSize))
    {
        _defaultScreenSize = NSMakeSize(DEFAULT_WIDTH, DEFAULT_HEIGHT);
    }
    else
    {
        // Some cores may initially report a 0x0 screenRect on launch, so use aspectSize instead.
        if (OEIntSizeIsEmpty(newScreenSize)) {
            _screenSize = _aspectSize;
        }
        OEIntSize correct  = OECorrectScreenSizeForAspectSize(_screenSize, _aspectSize);
        _defaultScreenSize = NSMakeSize(correct.width, correct.height);
    }
    
    [_gameView setScreenSize:_screenSize aspectSize:_aspectSize];
}

#pragma mark - Private Methods
- (NSWindow *)OE_rootWindow
{
    NSWindow *window = [[self gameView] window];
    while([window parentWindow])
        window = [window parentWindow];
    return window;
}

#pragma mark - Notifications

- (void)viewDidChangeFrame:(NSNotification*)notification
{
    [_controlsWindow repositionOnGameWindow];
}

#pragma mark - OEGameViewDelegate Protocol

- (void)gameView:(OEGameLayerView *)gameView didReceiveMouseEvent:(OEEvent *)event
{
    [[self document] gameViewController:self didReceiveMouseEvent:event];
}

- (void)gameView:(OEGameLayerView *)gameView updateBounds:(CGRect)newBounds
{
    [[self document] gameViewController:self updateBounds:newBounds];
}

- (void)gameView:(OEGameLayerView *)gameView updateBackingScaleFactor:(CGFloat)newScaleFactor
{
    [[self document] gameViewController:self updateBackingScaleFactor:newScaleFactor];
}

@end

@implementation OEGameViewController (Notifications)

- (void)showQuickSaveNotification
{
    [_notificationView showQuickSave];
}

- (void)showScreenShotNotification
{
    [_notificationView showScreenShot];
}

- (void)showFastForwardNotification:(BOOL)enable
{
    [_notificationView showFastForwardWithEnabled:enable];
}

- (void)showRewindNotification:(BOOL)enable
{
    [_notificationView showRewindWithEnabled:enable];
}

- (void)showStepForwardNotification
{
    [_notificationView showStepForward];
}

- (void)showStepBackwardNotification
{
    [_notificationView showStepBackward];
}

@end
