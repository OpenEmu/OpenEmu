/*
 Copyright (c) 2012, OpenEmu Team

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 *Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 *Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 *Neither the name of the OpenEmu Team nor the
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

#import "OEPopoutGameWindowController.h"

#import "OEHUDWindow.h"
#import "OEGameDocument.h"
#import "OEGameViewController.h"
#import "OEGameView.h"
#import "OEGameControlsBar.h"
#import "OEUtilities.h"

#import "OEDBRom.h"
#import "OEDBGame.h"
#import "OEDBSystem+CoreDataProperties.h"

@import QuartzCore;

#import "OpenEmu-Swift.h"

#pragma mark - Private variables

static const NSSize       _OEPopoutGameWindowMinSize = {100, 100};
static const NSSize       _OEScreenshotWindowMinSize = {100, 100};
static const unsigned int _OEFitToWindowScale        = 0;

// User defaults
static NSString *const _OESystemIntegralScaleKeyFormat = @"OEIntegralScale.%@";
static NSString *const _OEIntegralScaleKey             = @"integralScale";
static NSString *const _OELastWindowSizeKey            = @"lastPopoutWindowSize";

typedef enum
{
    _OEPopoutGameWindowFullScreenStatusNonFullScreen = 0,
    _OEPopoutGameWindowFullScreenStatusFullScreen,
    _OEPopoutGameWindowFullScreenStatusEntering,
    _OEPopoutGameWindowFullScreenStatusExiting,
} OEPopoutGameWindowFullScreenStatus;



@interface OEScreenshotWindow : NSWindow
@property(nonatomic, unsafe_unretained) NSImageView *screenshotView;
@property(nonatomic, unsafe_unretained) NSImage     *screenshot;
@end



@implementation OEPopoutGameWindowController
{
    NSScreen                           *_screenBeforeWindowMove;
    unsigned int                        _integralScale;

    // Full screen
    NSRect                              _frameForNonFullScreenMode;
    OEScreenshotWindow                 *_screenshotWindow;
    OEPopoutGameWindowFullScreenStatus  _fullScreenStatus;
    BOOL                                _resumePlayingAfterFullScreenTransition;
}

#pragma mark - NSWindowController overridden methods

- (id)initWithWindow:(NSWindow *)window
{
    self = [super initWithWindow:window];
    if(!self)
        return nil;

    [window setDelegate:self];
    [window setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
    [window setAnimationBehavior:NSWindowAnimationBehaviorDocumentWindow];
    [window setMinSize:_OEPopoutGameWindowMinSize];

    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(OE_constrainIntegralScaleIfNeeded) name:NSApplicationDidChangeScreenParametersNotification object:nil];

    return self;
}

- (void)dealloc
{
    [[self window] setDelegate:nil];
    [self setWindow:nil];
}

- (BOOL)windowShouldClose:(id)sender
{
    return [self document] != nil;
}

- (void)setDocument:(NSDocument *)document
{
    NSAssert(!document || [document isKindOfClass:[OEGameDocument class]], @"OEPopoutGameWindowController accepts OEGameDocument documents only");

    [super setDocument:document];

    if(document != nil)
    {
        OEGameViewController *gameViewController = [[self OE_gameDocument] gameViewController];
        NSString *systemIdentifier               = [[[[[gameViewController document] rom] game] system] systemIdentifier];
        NSUserDefaults *defaults                 = [NSUserDefaults standardUserDefaults];
        unsigned int maxScale                    = [self maximumIntegralScale];
        NSDictionary *integralScaleInfo          = [defaults objectForKey:[NSString stringWithFormat:_OESystemIntegralScaleKeyFormat, systemIdentifier]];
        NSNumber *lastScaleNumber                = [integralScaleInfo objectForKey:_OEIntegralScaleKey];
        NSSize windowSize;

        _integralScale = ([lastScaleNumber respondsToSelector:@selector(unsignedIntValue)] ?
                          MIN([lastScaleNumber unsignedIntValue], maxScale) :
                          maxScale);

        if(_integralScale == _OEFitToWindowScale)
        {
            windowSize = NSSizeFromString([integralScaleInfo objectForKey:_OELastWindowSizeKey]);
            if(windowSize.width == 0 || windowSize.height == 0)
                windowSize = [self OE_windowSizeForGameViewIntegralScale:maxScale];
        }
        else
            windowSize = [self OE_windowSizeForGameViewIntegralScale:_integralScale];

        OEHUDWindow *window     = (OEHUDWindow *)[self window];
        const NSRect windowRect = {NSZeroPoint, windowSize};

        [gameViewController setIntegralScalingDelegate:self];

        [window setFrame:windowRect display:NO animate:NO];
        [window center];
        [window setContentAspectRatio:[gameViewController defaultScreenSize]];
        [window setMainContentView:[gameViewController view]];

        [self OE_buildScreenshotWindow];
    }
}

- (void)showWindow:(id)sender
{
    NSWindow *window = [self window];
    const BOOL needsToggleFullScreen = (!![self isWindowFullScreen] != !![window isFullScreen]);

    if(![window isVisible])
    {
        OEGameViewController *gameViewController = [[self OE_gameDocument] gameViewController];

        // We disable window animation if we need to toggle full screen because two parallel animations
        // (window being ordered front and toggling full-screen) looks painfully ugly. The animation
        // behaviour is restored in -windowDidExitFullScreen:.
        if(needsToggleFullScreen)
            [window setAnimationBehavior:NSWindowAnimationBehaviorNone];
        
        [window makeKeyAndOrderFront:sender];

        [gameViewController viewWillAppear];
        [gameViewController viewDidAppear];
    }

    if(needsToggleFullScreen)
        [window toggleFullScreen:self];
}

#pragma mark - Actions

- (IBAction)changeIntegralScale:(id)sender;
{
    if(![sender respondsToSelector:@selector(representedObject)])
        return;
    if(![[sender representedObject] respondsToSelector:@selector(unsignedIntValue)])
        return;

    const unsigned int newScale = [[sender representedObject] unsignedIntValue];
    if(newScale > [self maximumIntegralScale])
        return;

    [self OE_changeGameViewIntegralScale:newScale];
}

#pragma mark - OEGameIntegralScalingDelegate

- (unsigned int)maximumIntegralScale
{
    NSScreen *screen            = ([[self window] screen] ? : [NSScreen mainScreen]);
    const NSSize maxContentSize = [OEHUDWindow mainContentRectForFrameRect:[screen visibleFrame]].size;
    const NSSize defaultSize    = [[[self OE_gameDocument] gameViewController] defaultScreenSize];
    const unsigned int maxScale = MAX(MIN(floor(maxContentSize.height / defaultSize.height), floor(maxContentSize.width / defaultSize.width)), 1);

    return maxScale;
}

- (unsigned int)currentIntegralScale
{
    return [[self window] isFullScreen] ? _OEFitToWindowScale : _integralScale;
}

- (BOOL)shouldAllowIntegralScaling
{
    return ![[self window] isFullScreen];
}

#pragma mark - Private methods

- (NSSize)OE_windowContentSizeForGameViewIntegralScale:(unsigned int)gameViewIntegralScale
{
    const NSSize defaultSize = [[[self OE_gameDocument] gameViewController] defaultScreenSize];
    const NSSize contentSize = OEScaleSize(defaultSize, (CGFloat)gameViewIntegralScale);

    return contentSize;
}

- (NSSize)OE_windowSizeForGameViewIntegralScale:(unsigned int)gameViewIntegralScale
{
    const NSSize contentSize = [self OE_windowContentSizeForGameViewIntegralScale:gameViewIntegralScale];
    const NSSize windowSize  = [OEHUDWindow frameRectForMainContentRect:(NSRect){.size = contentSize}].size;

    return windowSize;
}

- (void)OE_changeGameViewIntegralScale:(unsigned int)newScale
{
    if(_fullScreenStatus != _OEPopoutGameWindowFullScreenStatusNonFullScreen)
        return;
    
    _integralScale = newScale;

    if(newScale != _OEFitToWindowScale)
    {
        const NSRect screenFrame = [[[self window] screen] visibleFrame];
        NSRect currentWindowFrame = [[self window] frame];
        NSRect newWindowFrame = { .size = [self OE_windowSizeForGameViewIntegralScale:newScale] };
        
        newWindowFrame.origin.y = roundf(NSMidY(currentWindowFrame)-newWindowFrame.size.height/2);
        newWindowFrame.origin.x = roundf(NSMidX(currentWindowFrame)-newWindowFrame.size.width/2);
        
        // Make sure the entire window is visible, centering it in case it isn’t
        if(NSMinY(newWindowFrame) < NSMinY(screenFrame) || NSMaxY(newWindowFrame) > NSMaxY(screenFrame))
            newWindowFrame.origin.y = NSMinY(screenFrame) + ((screenFrame.size.height - newWindowFrame.size.height) / 2);

        if(NSMinX(newWindowFrame) < NSMinX(screenFrame) || NSMaxX(newWindowFrame) > NSMaxX(screenFrame))
            newWindowFrame.origin.x = NSMinX(screenFrame) + ((screenFrame.size.width - newWindowFrame.size.width) / 2);

        [[[self window] animator] setFrame:newWindowFrame display:YES];
    }
}

- (void)OE_constrainIntegralScaleIfNeeded
{
    if(_fullScreenStatus != _OEPopoutGameWindowFullScreenStatusNonFullScreen || _integralScale == _OEFitToWindowScale)
        return;

    const unsigned int newMaxScale = [self maximumIntegralScale];
    const NSRect newScreenFrame    = [[[self window] screen] visibleFrame];
    const NSRect currentFrame      = [[self window] frame];

    if(newScreenFrame.size.width < currentFrame.size.width || newScreenFrame.size.height < currentFrame.size.height)
        [self OE_changeGameViewIntegralScale:newMaxScale];
}

- (OEGameDocument *)OE_gameDocument
{
    return (OEGameDocument *)[self document];
}

- (void)OE_buildScreenshotWindow
{
    NSRect windowFrame = {.size = _OEScreenshotWindowMinSize};
    NSScreen *mainScreen                     = [[NSScreen screens] objectAtIndex:0];
    const NSRect screenFrame                 = [mainScreen frame];
    _screenshotWindow  = [[OEScreenshotWindow alloc] initWithContentRect:screenFrame
                                                               styleMask:NSBorderlessWindowMask
                                                                 backing:NSBackingStoreBuffered
                                                                   defer:NO];
    [_screenshotWindow setBackgroundColor:[NSColor clearColor]];
    [_screenshotWindow setOpaque:NO];
    [_screenshotWindow setAnimationBehavior:NSWindowAnimationBehaviorNone];

    
    const NSRect  contentFrame = {NSZeroPoint, windowFrame.size};
    NSImageView  *imageView    = [[NSImageView alloc] initWithFrame:contentFrame];
    [[imageView cell] setImageAlignment:NSImageAlignBottomLeft];
    [[imageView cell] setImageScaling:NSImageScaleAxesIndependently];
    [imageView setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
    [imageView setWantsLayer:YES];
    [imageView setLayerContentsRedrawPolicy:NSViewLayerContentsRedrawOnSetNeedsDisplay];
    [imageView.layer setOpaque:YES];
    _screenshotWindow.screenshotView = imageView;

    [_screenshotWindow setContentView:imageView];
}

- (NSRect)OE_screenshotWindowFrameForOriginalFrame:(NSRect)frame
{
    OEGameViewController *gameViewController    = [[self OE_gameDocument] gameViewController];
    const NSSize          gameSize              = [gameViewController defaultScreenSize];
    const float           widthRatio            = gameSize.width  / frame.size.width;
    const float           heightRatio           = gameSize.height / frame.size.height;
    const float           dominantRatioInverse  = 1 / MAX(widthRatio, heightRatio);
    const NSSize          gameViewportSize      = OEScaleSize(gameSize, dominantRatioInverse);
    const NSPoint         gameViewportOrigin    =
    {
        NSMinX(frame) + ((frame.size.width  - gameViewportSize.width ) / 2),
        NSMinY(frame) + ((frame.size.height - gameViewportSize.height) / 2),
    };
    const NSRect          screenshotWindowFrame = {gameViewportOrigin, gameViewportSize};
    
    return screenshotWindowFrame;
}

- (void)OE_hideScreenshotWindow
{
    [_screenshotWindow orderOut:self];

    // Reduce the memory footprint of the screenshot window when it’s not visible
    [_screenshotWindow setScreenshot:nil];
    [_screenshotWindow.screenshotView.layer setFrame:(NSRect){.size = _OEScreenshotWindowMinSize}];
}

- (void)OE_forceLayerReposition:(CALayer *)layer toFrame:(NSRect)frame
{
    // This forces the CALayer to reposition
    // without this we see the previous state for a split second
    CABasicAnimation *moveToPosition = [CABasicAnimation animationWithKeyPath:@"position"];
    moveToPosition.fromValue = [NSValue valueWithPoint:frame.origin];
    moveToPosition.toValue = [NSValue valueWithPoint:frame.origin];
    moveToPosition.duration = 0;
    moveToPosition.fillMode = kCAFillModeForwards;
    moveToPosition.removedOnCompletion = NO;

    CABasicAnimation *scaleToSize = [CABasicAnimation animationWithKeyPath:@"bounds.size"];
    scaleToSize.fromValue = [NSValue valueWithSize:frame.size];
    scaleToSize.toValue = [NSValue valueWithSize:frame.size];
    scaleToSize.duration = 0;
    scaleToSize.fillMode = kCAFillModeForwards;
    scaleToSize.removedOnCompletion = NO;

    [_screenshotWindow.screenshotView.layer addAnimation:moveToPosition forKey:@"moveToPosition"];
    [_screenshotWindow.screenshotView.layer addAnimation:scaleToSize forKey:@"scaleToSize"];
}

#pragma mark - NSWindowDelegate

- (void)windowWillMove:(NSNotification *)notification
{
    if(_fullScreenStatus != _OEPopoutGameWindowFullScreenStatusNonFullScreen)
        return;

    _screenBeforeWindowMove = [[self window] screen];
}

- (void)windowDidMove:(NSNotification *)notification
{
    if(_fullScreenStatus != _OEPopoutGameWindowFullScreenStatusNonFullScreen)
        return;

    if(_screenBeforeWindowMove != [[self window] screen])
        [self OE_constrainIntegralScaleIfNeeded];

    _screenBeforeWindowMove = nil;
}

- (void)windowDidChangeScreen:(NSNotification *)notification
{
    if(_fullScreenStatus != _OEPopoutGameWindowFullScreenStatusNonFullScreen)
        return;

    [self OE_constrainIntegralScaleIfNeeded];
}

- (void)windowWillClose:(NSNotification *)notification
{
    OEGameViewController *gameViewController = [[self OE_gameDocument] gameViewController];

    const NSSize windowSize         = ([[self window] isFullScreen] ? _frameForNonFullScreenMode.size : [[self window] frame].size);
    NSString *systemIdentifier      = [[[[[gameViewController document] rom] game] system] systemIdentifier];
    NSUserDefaults *userDefaults    = [NSUserDefaults standardUserDefaults];
    NSString *systemKey             = [NSString stringWithFormat:_OESystemIntegralScaleKeyFormat, systemIdentifier];
    NSDictionary *integralScaleInfo = @{
        _OEIntegralScaleKey  : @(_integralScale),
        _OELastWindowSizeKey : NSStringFromSize(windowSize),
    };
    [userDefaults setObject:integralScaleInfo forKey:systemKey];

    [gameViewController viewWillDisappear];
    [gameViewController viewDidDisappear];
}

- (NSSize)windowWillResize:(NSWindow *)sender toSize:(NSSize)frameSize
{
    _integralScale = _OEFitToWindowScale;
    const NSSize windowSize  = [OEHUDWindow frameRectForMainContentRect:(NSRect){.size = frameSize}].size;

    return windowSize;
}

- (void)cancelOperation:(id)sender
{
    if([[self window] isFullScreen])
        [[self window] toggleFullScreen:self];
}

#pragma mark - NSWindowDelegate Full Screen

/* Since resizing OEGameView produces choppy animation, we do the following:
 *
 * - Take a screenshot of the game viewport inside OEGameView and build a borderless window from that
 * - The screenshot window is the one whose resizing to/from full screen is animated
 * - The actual window is faded out and resized to its final animation size
 * - When the animation ends, the actual window is faded in and the screenshot window is removed
 *
 * Emulation is paused when the animation begins and resumed when the animation ends (unless emulation
 * was already paused in the first place).
 */

- (void)windowWillEnterFullScreen:(NSNotification *)notification
{
    OEGameViewController *gameViewController = [[self OE_gameDocument] gameViewController];

    _fullScreenStatus                       = _OEPopoutGameWindowFullScreenStatusEntering;
    _frameForNonFullScreenMode              = [[self window] frame];

    _resumePlayingAfterFullScreenTransition = ![[self document] isEmulationPaused];
    [[self document] setEmulationPaused:YES];

    [NSCursor hide];
    [[gameViewController controlsWindow] setCanShow:NO];
    [[gameViewController controlsWindow] hideAnimated:YES];
}

- (NSArray *)customWindowsToEnterFullScreenForWindow:(NSWindow *)window
{
    return @[[self window], _screenshotWindow];
}

- (void)window:(NSWindow *)window startCustomAnimationToEnterFullScreenWithDuration:(NSTimeInterval)duration
{
    OEGameViewController *gameViewController = [[self OE_gameDocument] gameViewController];
    OEGameView *gameView                     = [gameViewController gameView];
    CALayer *layer                           = [[_screenshotWindow screenshotView] layer];
    NSView *contentView                      = [(OEHUDWindow *)window mainContentView];
    NSScreen *mainScreen                     = [[NSScreen screens] objectAtIndex:0];
    const NSRect screenFrame                 = [mainScreen frame];
    const NSTimeInterval hideBorderDuration  = duration / 4;
    const NSTimeInterval resizeDuration      = duration - hideBorderDuration;
    const NSRect contentFrame                = [OEHUDWindow mainContentRectForFrameRect:[window frame]];
    const NSRect screenshotWindowFrame       = [self OE_screenshotWindowFrameForOriginalFrame:contentFrame];
    const NSRect fullScreenWindowFrame       = [self OE_screenshotWindowFrameForOriginalFrame:screenFrame];


    [_screenshotWindow setScreenshot:[gameView screenshot]];
    [self OE_forceLayerReposition:layer toFrame:screenshotWindowFrame];
    [_screenshotWindow orderFront:self];

    CABasicAnimation *moveToPosition = [CABasicAnimation animationWithKeyPath:@"position"];
    moveToPosition.fromValue = [NSValue valueWithPoint:screenshotWindowFrame.origin];
    moveToPosition.toValue = [NSValue valueWithPoint:fullScreenWindowFrame.origin];
    moveToPosition.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseInEaseOut];
    moveToPosition.duration = resizeDuration;
    moveToPosition.fillMode = kCAFillModeForwards;
    moveToPosition.removedOnCompletion = NO;

    CABasicAnimation *scaleToSize = [CABasicAnimation animationWithKeyPath:@"bounds.size"];
    scaleToSize.fromValue = [NSValue valueWithSize:screenshotWindowFrame.size];
    scaleToSize.toValue = [NSValue valueWithSize:fullScreenWindowFrame.size];
    scaleToSize.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseInEaseOut];
    scaleToSize.duration = resizeDuration;
    scaleToSize.fillMode = kCAFillModeForwards;
    scaleToSize.removedOnCompletion = NO;

    // Fade the real window out
    [NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {
        [context setDuration:hideBorderDuration];
        [[window animator] setAlphaValue:0.0];
        [[[(OEHUDWindow *)window borderWindow] animator] setAlphaValue:0.0];
    } completionHandler:^{
        [window setFrame:screenFrame display:YES];
        [contentView setFrame:(NSRect){.size = screenFrame.size}]; // ignore title bar area

        // Resize the screenshot window to fullscreen
        [CATransaction begin];
        [CATransaction setCompletionBlock:^{
            [window setAlphaValue:1.0];
            [self OE_hideScreenshotWindow];
        }];

        [layer addAnimation:moveToPosition forKey:@"position"];
        [layer addAnimation:scaleToSize forKey:@"scale"];
        
        [CATransaction commit];
    }];
}

- (void)windowDidEnterFullScreen:(NSNotification *)notification
{
    _fullScreenStatus = _OEPopoutGameWindowFullScreenStatusFullScreen;

    OEGameViewController *gameViewController = [[self OE_gameDocument] gameViewController];
    if(_resumePlayingAfterFullScreenTransition)
        [[self document] setEmulationPaused:NO];

    [[gameViewController controlsWindow] hideAnimated:YES];
    [[gameViewController controlsWindow] setCanShow:YES];
    [NSCursor unhide];
}

- (void)windowWillExitFullScreen:(NSNotification *)notification
{
    OEGameViewController *gameViewController = [[self OE_gameDocument] gameViewController];

    _fullScreenStatus                        = _OEPopoutGameWindowFullScreenStatusExiting;
    _resumePlayingAfterFullScreenTransition  = ![[self document] isEmulationPaused];
    
    [[self document] setEmulationPaused:YES];
    
    [NSCursor hide];
    [[gameViewController controlsWindow] setCanShow:NO];
    [[gameViewController controlsWindow] hideAnimated:YES];
}

- (NSArray *)customWindowsToExitFullScreenForWindow:(NSWindow *)window
{
    return @[[self window], _screenshotWindow];
}

- (void)window:(NSWindow *)window startCustomAnimationToExitFullScreenWithDuration:(NSTimeInterval)duration
{
    OEGameViewController *gameViewController = [[self OE_gameDocument] gameViewController];
    OEGameView *gameView                     = [gameViewController gameView];
    CALayer *layer                           = [[_screenshotWindow screenshotView] layer];
    NSView *contentView                      = [(OEHUDWindow *)window mainContentView];
    NSScreen *mainScreen                     = [[NSScreen screens] objectAtIndex:0];
    const NSRect screenFrame                 = [mainScreen frame];
    const NSRect fullScreenGameArea          = [self OE_screenshotWindowFrameForOriginalFrame:screenFrame];
    const NSTimeInterval showBorderDuration  = duration / 4;
    const NSTimeInterval resizeDuration      = duration - showBorderDuration;

    [_screenshotWindow setScreenshot:[gameView screenshot]];
    [self OE_forceLayerReposition:layer toFrame:fullScreenGameArea];
    [_screenshotWindow orderFront:self];

    const NSRect contentFrame          = [OEHUDWindow mainContentRectForFrameRect:_frameForNonFullScreenMode];
    const NSRect screenshotWindowFrame = [self OE_screenshotWindowFrameForOriginalFrame:contentFrame];

    // Restore the window to its original frame
    {
        [window setAlphaValue:0.0];
        [window setFrame:_frameForNonFullScreenMode display:YES];

        // Resize the content view so that it takes window decoration into account
        NSRect contentRect = [window convertRectFromScreen:[OEHUDWindow mainContentRectForFrameRect:_frameForNonFullScreenMode]];
        [contentView setFrame:contentRect];
    }

    CABasicAnimation *moveToPosition = [CABasicAnimation animationWithKeyPath:@"position"];
    moveToPosition.fromValue = [NSValue valueWithPoint:fullScreenGameArea.origin];
    moveToPosition.toValue = [NSValue valueWithPoint:screenshotWindowFrame.origin];
    moveToPosition.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseInEaseOut];
    moveToPosition.duration = resizeDuration;
    moveToPosition.fillMode = kCAFillModeForwards;
    moveToPosition.removedOnCompletion = NO;

    CABasicAnimation *scaleToSize = [CABasicAnimation animationWithKeyPath:@"bounds.size"];
    scaleToSize.fromValue = [NSValue valueWithSize:fullScreenGameArea.size];
    scaleToSize.toValue = [NSValue valueWithSize:screenshotWindowFrame.size];
    scaleToSize.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseInEaseOut];
    scaleToSize.duration = resizeDuration;
    scaleToSize.fillMode = kCAFillModeForwards;
    scaleToSize.removedOnCompletion = NO;

    // Scale the screenshot window down
    [CATransaction begin];
    [CATransaction setCompletionBlock:^{
        // Fade the real window back in after the scaling is done
        [NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {
            [context setDuration:showBorderDuration];

            [[window animator] setAlphaValue:1.0];
            [[[(OEHUDWindow *)[self window] borderWindow] animator] setAlphaValue:1.0];
        } completionHandler:^{
            [self OE_hideScreenshotWindow];
        }];
    }];

    [_screenshotWindow.screenshotView.layer addAnimation:moveToPosition forKey:@"moveToPosition"];
    [_screenshotWindow.screenshotView.layer addAnimation:scaleToSize forKey:@"scaleToSize"];

    [CATransaction commit];
}

- (void)windowDidExitFullScreen:(NSNotification *)notification
{
    OEGameViewController *gameViewController = [[self OE_gameDocument] gameViewController];

    _fullScreenStatus = _OEPopoutGameWindowFullScreenStatusNonFullScreen;

    if(_resumePlayingAfterFullScreenTransition)
        [[self document] setEmulationPaused:NO];

    [[gameViewController controlsWindow] hideAnimated:YES];
    [[gameViewController controlsWindow] setCanShow:YES];
    [NSCursor unhide];
}

- (void)windowDidFailToEnterFullScreen:(NSWindow *)window
{
    OEGameViewController *gameViewController = [[self OE_gameDocument] gameViewController];

    _fullScreenStatus = _OEPopoutGameWindowFullScreenStatusNonFullScreen;

    if(_resumePlayingAfterFullScreenTransition)
        [[self document] setEmulationPaused:NO];

    [[gameViewController controlsWindow] setCanShow:YES];
    [NSCursor unhide];
}

- (void)windowDidFailToExitFullScreen:(NSWindow *)window
{
    OEGameViewController *gameViewController = [[self OE_gameDocument] gameViewController];

    _fullScreenStatus = _OEPopoutGameWindowFullScreenStatusFullScreen;

    if(_resumePlayingAfterFullScreenTransition)
        [[self document] setEmulationPaused:NO];

    [[gameViewController controlsWindow] setCanShow:YES];
    [NSCursor unhide];
}

@end

@implementation OEScreenshotWindow

- (void)setScreenshot:(NSImage *)screenshot
{
    [[self screenshotView] setImage:screenshot];
}

@end
