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

#import "OEGameView.h"
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
#import "NSColor+OEAdditions.h"

#import "OELibraryDatabase.h"

#import <OpenEmuSystem/OpenEmuSystem.h>

#import "OpenEmu-Swift.h"

NSString *const OEGameVolumeKey = @"volume";
NSString *const OEGameDefaultVideoFilterKey = @"videoFilter";
NSString *const OEGameSystemVideoFilterKeyFormat = @"videoFilter.%@";
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

@interface OEGameViewController () <OEGameViewDelegate>
{
    // Standard game document stuff
    OEGameView *_gameView;
    OEIntSize   _screenSize;
    OEIntSize   _aspectSize;
    BOOL        _pausedByGoingToBackground;
    NSUInteger  _discCount;
}

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

        _gameView = [[OEGameView alloc] initWithFrame:[[self view] bounds]];
        [_gameView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
        [_gameView setDelegate:self];

        NSString *backgroundColorName = [[NSUserDefaults standardUserDefaults] objectForKey:OEGameViewBackgroundColorKey];
        if(backgroundColorName != nil)
        {
            NSColor *color = [NSColor colorFromString:backgroundColorName];
            [_gameView setBackgroundColor:color];
        }
        
        [[self view] addSubview:_gameView];

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

- (NSUInteger)discCount
{
    return _discCount;
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

    [_gameView setFilterName:filterName];
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

#pragma mark - Taking Screenshots
- (void)takeScreenshot:(id)sender
{
    NSImage *screenshotImage;
    bool takeNativeScreenshots = [[NSUserDefaults standardUserDefaults] boolForKey:OETakeNativeScreenshots];
    screenshotImage = takeNativeScreenshots ? [_gameView nativeScreenshot] : [_gameView screenshot];
    NSData *TIFFData = [screenshotImage TIFFRepresentation];

    NSUserDefaults *standardUserDefaults = [NSUserDefaults standardUserDefaults];
    NSBitmapImageFileType type = [standardUserDefaults integerForKey:OEScreenshotFileFormatKey];
    NSDictionary *properties = [standardUserDefaults dictionaryForKey:OEScreenshotPropertiesKey];
    NSBitmapImageRep *bitmapImageRep = [NSBitmapImageRep imageRepWithData:TIFFData];
    NSData *imageData = [bitmapImageRep representationUsingType:type properties:properties];

    NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
    [dateFormatter setDateFormat:@"yyyy-MM-dd HH.mm.ss"];
    NSString *timeStamp = [dateFormatter stringFromDate:[NSDate date]];

    // Replace forward slashes in the game title with underscores because forward slashes aren't allowed in filenames.
    NSMutableString *displayName = [self.document.rom.game.displayName mutableCopy];
    [displayName replaceOccurrencesOfString:@"/" withString:@"_" options:0 range:NSMakeRange(0, displayName.length)];
    
    NSString *fileName = [NSString stringWithFormat:@"%@ %@.png", displayName, timeStamp];
    NSString *temporaryPath = [NSTemporaryDirectory() stringByAppendingPathComponent:fileName];
    NSURL *temporaryURL = [NSURL fileURLWithPath:temporaryPath];

    __autoreleasing NSError *error;
    if(![imageData writeToURL:temporaryURL options:NSDataWritingAtomic error:&error])
    {
        NSLog(@"Could not save screenshot at URL: %@, with error: %@", temporaryURL, error);
    } else {
        OEDBRom *rom = [[self document] rom];
        OEDBScreenshot *screenshot = [OEDBScreenshot createObjectInContext:[rom managedObjectContext] forROM:rom withFile:temporaryURL];
        [screenshot save];
        [[self gameView] showScreenShotNotification];
    }
}

#pragma mark - OEGameCoreOwner methods

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

- (void)setDiscCount:(NSUInteger)discCount
{
    _discCount = discCount;
}

#pragma mark - Info

- (NSSize)defaultScreenSize
{
    if(OEIntSizeIsEmpty(_screenSize) || OEIntSizeIsEmpty(_aspectSize))
        return NSMakeSize(400, 300);

    NSSize corrected = [_gameView correctScreenSize:_screenSize forAspectSize:_aspectSize returnVertices:NO];
    return corrected;
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

- (void)gameView:(OEGameView *)gameView didReceiveMouseEvent:(OEEvent *)event
{
    [[self document] gameViewController:self didReceiveMouseEvent:event];
}

@end
