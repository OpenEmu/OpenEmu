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
#import "NSViewController+OEAdditions.h"

#import "OEDBRom.h"
#import "OEDBSystem.h"
#import "OEDBGame.h"

#import "OEGameView.h"
#import "OECorePickerController.h"
#import "OEDOGameCoreHelper.h"
#import "OEDOGameCoreManager.h"
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

#import "NSURL+OELibraryAdditions.h"

#import "OEPreferencesController.h"
#import "OELibraryDatabase.h"

#import <OpenEmuSystem/OpenEmuSystem.h>

NSString *const OEGameVolumeKey = @"volume";
NSString *const OEGameDefaultVideoFilterKey = @"videoFilter";
NSString *const OEGameSystemVideoFilterKeyFormat = @"videoFilter.%@";
NSString *const OEGameCoresInBackgroundKey = @"gameCoreInBackgroundThread";
NSString *const OEDontShowGameTitleInWindowKey = @"dontShowGameTitleInWindow";
NSString *const OEAutoSwitchCoreAlertSuppressionKey = @"changeCoreWhenLoadingStateWitoutConfirmation";
NSString *const OEBackgroundPauseKey = @"backgroundPause";
NSString *const OEForceCorePicker = @"forceCorePicker";
NSString *const OEGameViewControllerEmulationWillFinishNotification = @"OEGameViewControllerEmulationWillFinishNotification";
NSString *const OEGameViewControllerEmulationDidFinishNotification = @"OEGameViewControllerEmulationDidFinishNotification";
NSString *const OEGameViewControllerROMKey = @"OEROM";

#define UDDefaultCoreMappingKeyPrefix   @"defaultCore"
#define UDSystemCoreMappingKeyForSystemIdentifier(_SYSTEM_IDENTIFIER_) [NSString stringWithFormat:@"%@.%@", UDDefaultCoreMappingKeyPrefix, _SYSTEM_IDENTIFIER_]

@interface OEGameViewController () <OEGameViewDelegate>
{
    // Standard game document stuff
    OEGameView *_gameView;
    OEIntSize   _screenSize;
    OEIntSize   _aspectSize;
    BOOL        _pausedByGoingToBackground;
}

@end

@implementation OEGameViewController

- (id)init
{
    if((self = [super init]))
    {
        _controlsWindow = [[OEGameControlsBar alloc] initWithGameViewController:self];
        [_controlsWindow setReleasedWhenClosed:YES];

        NSView *view = [[NSView alloc] initWithFrame:(NSRect){ .size = { 1.0, 1.0 }}];
        [self setView:view];

        _gameView = [[OEGameView alloc] initWithFrame:[[self view] bounds]];
        [_gameView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
        [_gameView setDelegate:self];
        [[self view] addSubview:_gameView];

        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(viewDidChangeFrame:) name:NSViewFrameDidChangeNotification object:_gameView];
    }
    return self;
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];

    [_gameView setDelegate:nil];
    _gameView = nil;

    [_controlsWindow close];
    _controlsWindow = nil;
}

#pragma mark -

- (void)viewDidAppear
{
    [super viewDidAppear];

    if([_controlsWindow parentWindow] != nil)
        [[_controlsWindow parentWindow] removeChildWindow:_controlsWindow];

    NSWindow *window = [self OE_rootWindow];
    if(window == nil) return;

    [window addChildWindow:_controlsWindow ordered:NSWindowAbove];
    [self OE_repositionControlsWindow];
    [_controlsWindow orderFront:self];

    [window makeFirstResponder:_gameView];
}

- (void)viewWillDisappear
{
    [super viewWillDisappear];

    [_controlsWindow hide];
    [[self OE_rootWindow] removeChildWindow:_controlsWindow];
}

#pragma mark - Controlling Emulation

- (BOOL)supportsCheats;
{
    return [[self document] supportsCheats];
}

- (NSString *)coreIdentifier;
{
    return [[self document] coreIdentifier];
}

- (NSString *)systemIdentifier;
{
    return [[self document] systemIdentifier];
}

- (NSImage *)takeNativeScreenshot
{
    return [_gameView nativeScreenshot];
}

- (void)reflectVolume:(float)volume;
{
    [[self controlsWindow] reflectVolume:volume];
}

- (void)reflectEmulationPaused:(BOOL)paused;
{
    [[self controlsWindow] reflectEmulationRunning:!paused];
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

    [_gameView setFilterName:filterName];
    [[NSUserDefaults standardUserDefaults] setObject:filterName forKey:[NSString stringWithFormat:OEGameSystemVideoFilterKeyFormat, [[self document] systemIdentifier]]];
}

#pragma mark - Taking Screenshots

- (void)takeScreenshot:(id)sender
{
    NSImage *screenshotImage = [_gameView screenshot];
    NSData *TIFFData = [screenshotImage TIFFRepresentation];
    NSBitmapImageRep *bitmapImageRep = [NSBitmapImageRep imageRepWithData:TIFFData];
    NSData *PNGData = [bitmapImageRep representationUsingType:NSPNGFileType properties:nil];

    NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
    [dateFormatter setDateFormat:@"yyyy-MM-dd HH.mm.ss"];
    NSString *timeStamp = [dateFormatter stringFromDate:[NSDate date]];
    
    NSURL *screenshotFolderURL = [[OELibraryDatabase defaultDatabase] screenshotFolderURL];
    NSURL *screenshotURL = [screenshotFolderURL URLByAppendingPathComponent:[NSString stringWithFormat:@"%@ %@.png", [[[[self document] rom] game] displayName], timeStamp]];

    __autoreleasing NSError *error;
    if(![PNGData writeToURL:screenshotURL options:NSDataWritingAtomic error:&error])
        NSLog(@"Could not save screenshot at URL: %@, with error: %@", screenshotURL, error);
}

#pragma mark - OEGameCoreDisplayHelper methods

- (void)setEnableVSync:(BOOL)enable;
{
    [_gameView setEnableVSync:enable];
}

- (void)setScreenSize:(OEIntSize)newScreenSize aspectSize:(OEIntSize)newAspectSize withIOSurfaceID:(IOSurfaceID)newSurfaceID
{
    _screenSize = newScreenSize;
    _aspectSize = newAspectSize;
    [_gameView setScreenSize:_screenSize aspectSize:_aspectSize withIOSurfaceID:newSurfaceID];
}

- (void)setScreenSize:(OEIntSize)newScreenSize withIOSurfaceID:(IOSurfaceID)newSurfaceID;
{
    _screenSize = newScreenSize;
    [_gameView setScreenSize:newScreenSize withIOSurfaceID:newSurfaceID];
}

- (void)setAspectSize:(OEIntSize)newAspectSize;
{
    _aspectSize = newAspectSize;
    [_gameView setAspectSize:newAspectSize];
}

#pragma mark - Info

- (NSSize)defaultScreenSize
{
    if(OEIntSizeIsEmpty(_screenSize) || OEIntSizeIsEmpty(_aspectSize))
        return NSMakeSize(400, 300);

    CGFloat wr = (CGFloat) _aspectSize.width / _screenSize.width;
    CGFloat hr = (CGFloat) _aspectSize.height / _screenSize.height;
    CGFloat ratio = MAX(hr, wr);
    NSSize scaled = NSMakeSize((wr / ratio), (hr / ratio));
    
    CGFloat halfw = scaled.width;
    CGFloat halfh = scaled.height;
    
    return NSMakeSize(_screenSize.width / halfh, _screenSize.height / halfw);
}

#pragma mark - Private Methods

- (void)OE_repositionControlsWindow
{
    NSWindow *gameWindow = [self OE_rootWindow];
    if(gameWindow == nil) return;

    static const CGFloat _OEControlsMargin = 19;

    NSRect gameViewFrameInWindow = [_gameView convertRect:[_gameView frame] toView:nil];
    NSPoint origin = [gameWindow convertRectToScreen:gameViewFrameInWindow].origin;

    origin.x += ([_gameView frame].size.width - [_controlsWindow frame].size.width) / 2;

    // If the controls bar fits, it sits over the window
    if([_gameView frame].size.width >= [_controlsWindow frame].size.width)
        origin.y += _OEControlsMargin;
    else
    {
        // Otherwise, it sits below the window
        origin.y -= ([_controlsWindow frame].size.height + _OEControlsMargin);

        // Unless below the window means it being off-screen, in which case it sits above the window
        if(origin.y < NSMinY([[gameWindow screen] visibleFrame]))
            origin.y = NSMaxY([gameWindow frame]) + _OEControlsMargin;
    }

    [_controlsWindow setFrameOrigin:origin];
}

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
    [self OE_repositionControlsWindow];
}

#pragma mark - OEGameViewDelegate Protocol

- (void)gameView:(OEGameView *)gameView didReceiveMouseEvent:(OEEvent *)event
{
    [[self document] gameViewController:self didReceiveMouseEvent:event];
}

- (void)gameView:(OEGameView *)gameView setDrawSquarePixels:(BOOL)drawSquarePixels
{
    [[self document] gameViewController:self setDrawSquarePixels:drawSquarePixels];
}

@end
