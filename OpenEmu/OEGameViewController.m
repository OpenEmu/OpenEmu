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

#import "OEDBRom.h"
#import "OEDBSystem.h"
#import "OEDBGame.h"
#import "OEDBScreenshot.h"

#import "OEGameLayerView.h"
#import "OEGameCoreManager.h"
#import "OEThreadGameCoreManager.h"
#import "OEXPCGameCoreManager.h"

#import "OESystemPlugin.h"
#import "OECorePlugin.h"

#import "OEDBSaveState.h"
#import "OEGameControlsBar.h"

#import "OECoreUpdater.h"

#import "OEGameDocument.h"
#import "OEAudioDeviceManager.h"

#import "OEHUDAlert+DefaultAlertsAdditions.h"

#import "OELibraryDatabase.h"

#import <OpenEmuSystem/OpenEmuSystem.h>

#import "OpenEmu-Swift.h"

NSString *const OEGameVolumeKey = @"volume";
NSString *const OEGameDefaultVideoFilterKey = @"videoFilter";
NSString *const OEGameSystemVideoFilterKeyFormat = @"videoFilter.%@";
NSString *const OEGameCoreDisplayModeKeyFormat = @"displayMode.%@";
NSString *const OEGameCoresInBackgroundKey = @"gameCoreInBackgroundThread";
NSString *const OEAutoSwitchCoreAlertSuppressionKey = @"changeCoreWhenLoadingStateWitoutConfirmation";
NSString *const OEBackgroundPauseKey = @"backgroundPause";
NSString *const OEBackgroundControllerPlayKey = @"backgroundControllerPlay";
NSString *const OEGameViewControllerEmulationWillFinishNotification = @"OEGameViewControllerEmulationWillFinishNotification";
NSString *const OEGameViewControllerEmulationDidFinishNotification = @"OEGameViewControllerEmulationDidFinishNotification";
NSString *const OETakeNativeScreenshots = @"takeNativeScreenshots";
NSString *const OEGameViewControllerROMKey = @"OEROM";
NSString *const OEGameViewBackgroundColorKey = @"gameViewBackgroundColor";

NSString *const OEScreenshotFileFormatKey = @"screenshotFormat";
NSString *const OEScreenshotPropertiesKey = @"screenshotProperties";

#define UDDefaultCoreMappingKeyPrefix   @"defaultCore"
#define UDSystemCoreMappingKeyForSystemIdentifier(_SYSTEM_IDENTIFIER_) [NSString stringWithFormat:@"%@.%@", UDDefaultCoreMappingKeyPrefix, _SYSTEM_IDENTIFIER_]

/*
 TODO Messages to remote layer:
 - Change bounds
 - Start Syphon
 - Native screenshot
 
 Messages from remote layer:
 - Default screen size/aspect size
 */

@interface OEGameViewController () <OEGameViewDelegate>
{
    // Standard game document stuff
    OEGameLayerView *_gameView;
    BOOL        _pausedByGoingToBackground;
}

@property(readonly) OEGameLayerView *gameView;

@end

@implementation OEGameViewController
+ (void)initialize
{
    if([self class] == [OEGameViewController class])
    {
        [[NSUserDefaults standardUserDefaults] registerDefaults:@{
                                                                  OEScreenshotFileFormatKey : @(NSPNGFileType),
                                                                  OEScreenshotPropertiesKey : @{},
                                                                  }];
    }
}

- (id)init
{
    if((self = [super init]))
    {
        _controlsWindow = [[OEGameControlsBar alloc] initWithGameViewController:self];
        [_controlsWindow setReleasedWhenClosed:YES];
        
        NSView *view = [[NSView alloc] initWithFrame:(NSRect){ .size = { 1.0, 1.0 }}];
        [self setView:view];
        
        _gameView = [[OEGameLayerView alloc] initWithFrame:[[self view] bounds]];
        [_gameView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
        [_gameView setDelegate:self];
        
        [[self view] addSubview:_gameView];

        for (OEShaderPlugin *plugin in [OEShaderPlugin allPlugins]) {
            NSLog(@"%@", plugin.path);
        }


        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(viewDidChangeFrame:) name:NSViewFrameDidChangeNotification object:_gameView];
        
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowDidChangeScreen:) name:NSWindowDidMoveNotification object:self];
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

- (void)viewDidLayout
{
    [[self document] setOutputBounds:self.view.bounds];
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

- (NSImage *)nativeScreenshot
{
    NSAssert(0, @"Must send to helper.");
    return nil;
}

- (NSImage *)screenshot
{
    NSAssert(0, @"Must send to helper.");
    return nil;
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
    [[self controlsWindow] setCanShow:[sender state]==NSOffState];
}

#pragma mark - HUD Bar Actions
- (void)selectFilter:(id)sender
{
    NSString *filterName;
    if([sender isKindOfClass:[NSString class]])
        filterName = sender;
    else if([sender respondsToSelector:@selector(representedObject)] && [[sender representedObject] isKindOfClass:[NSString class]])
        filterName = [sender representedObject];
    else if([sender respondsToSelector:@selector(title)] && [[sender title] isKindOfClass:[NSString class]])
        filterName = [sender title];
    else
        DLog(@"Invalid argument passed: %@", sender);

    for (OEShaderPlugin *plugin in [OEShaderPlugin allPlugins]) {
        if ([filterName isEqualToString:plugin.name]) {
            [[self document] gameViewController:self setFilterURL:[NSURL fileURLWithPath:plugin.path]];
        }
    }

    //[[self document] gameViewController:self setFilterURL:url];
    //    [_gameView setFilterName:filterName];
    [[NSUserDefaults standardUserDefaults] setObject:filterName forKey:[NSString stringWithFormat:OEGameSystemVideoFilterKeyFormat, [[self document] systemIdentifier]]];
}

- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
    SEL action = [menuItem action];
    if(action == @selector(toggleControlsVisibility:))
    {
        if([[self controlsWindow] canShow])
            [menuItem setState:NSOffState];
        else
            [menuItem setState:NSOnState];
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
    [_gameView setScreenSize:_screenSize aspectSize:_aspectSize];
}

#pragma mark - Info

// TODO: This is nearly the same as a method in the helper layer.
static NSSize CorrectScreenSizeForAspectSize(OEIntSize screenSize, OEIntSize aspectSize)
{
    // calculate aspect ratio
    CGFloat wr = (CGFloat) aspectSize.width / screenSize.width;
    CGFloat hr = (CGFloat) aspectSize.height / screenSize.height;
    CGFloat ratio = MAX(hr, wr);
    NSSize scaled = NSMakeSize((wr / ratio), (hr / ratio));
    
    CGFloat halfw = scaled.width;
    CGFloat halfh = scaled.height;
    
    NSSize corrected;
    corrected = NSMakeSize(screenSize.width / halfh, screenSize.height / halfw);
    
    return corrected;
}

- (NSSize)defaultScreenSize
{
    NSParameterAssert(_screenSize.width);
    NSParameterAssert(_aspectSize.width);
    
    return CorrectScreenSizeForAspectSize(_screenSize, _aspectSize);
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

- (void)showQuickSaveNotification
{
    // Add new view that does this.
    NSAssert(0, @"Not implemented");
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
