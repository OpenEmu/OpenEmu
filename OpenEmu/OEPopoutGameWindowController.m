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

#import "OEGameDocument.h"
#import "OEGameViewController.h"
#import "OEGameControlsBar.h"
#import "OEUtilities.h"
#import "NSColor+OEAdditions.h"

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
- (void)setScreenshot:(NSImage *)screenshot;
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
    BOOL                                _snapResize;
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
    
    NSUserDefaults *ud = [NSUserDefaults standardUserDefaults];
    NSString *bgColorStr = [ud objectForKey:OEGameViewBackgroundColorKey];
    NSColor *bgColor = bgColorStr ? [NSColor colorFromString:bgColorStr] : [NSColor blackColor];
    window.backgroundColor = bgColor;

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

        NSWindow *window        = self.window;
        const NSRect windowRect = {NSZeroPoint, windowSize};

        [gameViewController setIntegralScalingDelegate:self];

        [window setFrame:windowRect display:NO animate:NO];
        [window center];
        [window setContentAspectRatio:[gameViewController defaultScreenSize]];
        window.contentView = gameViewController.view;

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
    const NSSize maxContentSize = [self.window contentRectForFrameRect:screen.visibleFrame].size;
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
    const NSSize windowSize  = [self.window frameRectForContentRect:(NSRect){.size = contentSize}].size;

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
                                                               styleMask:NSWindowStyleMaskBorderless
                                                                 backing:NSBackingStoreBuffered
                                                                   defer:NO];
    [_screenshotWindow setBackgroundColor:self.window.backgroundColor];
    [_screenshotWindow setOpaque:NO];
    [_screenshotWindow setAnimationBehavior:NSWindowAnimationBehaviorNone];

    const NSRect  contentFrame = {NSZeroPoint, windowFrame.size};
    NSImageView  *imageView    = [[NSImageView alloc] initWithFrame:contentFrame];
    imageView.imageAlignment = NSImageAlignCenter;
    imageView.imageScaling = NSImageScaleProportionallyUpOrDown;
    [imageView setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
    _screenshotWindow.screenshotView = imageView;

    [_screenshotWindow setContentView:imageView];
}

- (void)OE_hideScreenshotWindow
{
    [_screenshotWindow orderOut:self];

    // Reduce the memory footprint of the screenshot window when it’s not visible
    [_screenshotWindow setScreenshot:nil];
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

- (void)flagsChanged:(NSEvent *)event
{
    if (event.modifierFlags & NSEventModifierFlagShift)
    {
        _snapResize = YES;
        [self.window setContentAspectRatio:CGSizeZero];
    }
    else
    {
        _snapResize = NO;
        [self.window setContentAspectRatio:[self OE_gameDocument].gameViewController.defaultScreenSize];
    }
}

- (NSSize)windowWillResize:(NSWindow *)sender toSize:(NSSize)frameSize
{
    if (_snapResize) {
        CGSize existing = sender.frame.size;
        CGSize size = [self OE_gameDocument].gameViewController.defaultScreenSize;
        NSInteger proposedScale = IntegralScaleForProposedSize(existing, frameSize, size);
        if (proposedScale != _integralScale && proposedScale <= self.maximumIntegralScale)
        {
            _integralScale = (unsigned int)proposedScale;
            CGSize newSize = [self OE_windowSizeForGameViewIntegralScale:_integralScale];
            return newSize;
        }
        return existing;
    }
    
    _integralScale = _OEFitToWindowScale;
    return frameSize;
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
    
    // move the screenshot window to the same screen as the game window
    // otherwise it will be shown in the system full-screen animation
    NSScreen *mainScreen = self.window.screen;
    [_screenshotWindow setFrameOrigin:mainScreen.frame.origin];

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
    NSScreen *mainScreen                     = window.screen;
    const NSRect screenFrame                 = [mainScreen frame];
    const NSTimeInterval hideBorderDuration  = duration / 4;
    const NSTimeInterval resizeDuration      = duration - hideBorderDuration;
    const NSRect contentFrame                = [window contentRectForFrameRect:window.frame];

    [_screenshotWindow setScreenshot:[gameViewController screenshot]];
    [_screenshotWindow setFrame:contentFrame display:YES];
    [_screenshotWindow orderFront:self];

    // Fade the real window out
    [CATransaction begin];
    [CATransaction setAnimationDuration:hideBorderDuration];
    [CATransaction setAnimationTimingFunction:[CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseInEaseOut]];
    [CATransaction setCompletionBlock:^{
        [window setFrame:screenFrame display:NO];

        // Resize the screenshot window to fullscreen
        [CATransaction begin];
        [CATransaction setAnimationDuration:resizeDuration];
        [CATransaction setAnimationTimingFunction:[CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseInEaseOut]];
        [CATransaction setCompletionBlock:^{
            [window setAlphaValue:1.0];
            [self OE_hideScreenshotWindow];
        }];
        
        [self->_screenshotWindow.animator setFrame:screenFrame display:YES];
        [CATransaction commit];
    }];
    
    [window.animator setAlphaValue:0.0];
    [CATransaction commit];
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
    NSScreen *mainScreen                     = window.screen;
    const NSRect screenFrame                 = [mainScreen frame];
    const NSTimeInterval showBorderDuration  = duration / 4;
    const NSTimeInterval resizeDuration      = duration - showBorderDuration;
    /* a window in full-screen mode does not have a title bar, and thus we have
     * to explicitly specify the style mask to compute the correct content
     * frame */
    const NSRect contentFrame = [NSWindow contentRectForFrameRect:_frameForNonFullScreenMode styleMask:NSWindowStyleMaskTitled];
    const NSRect targetWindowFrame = [window frameRectForContentRect:contentFrame];

    [_screenshotWindow setScreenshot:[gameViewController screenshot]];
    [_screenshotWindow setFrame:screenFrame display:YES];
    [_screenshotWindow orderFront:self];
    [window setAlphaValue:0.0];

    // Scale the screenshot window down
    [CATransaction begin];
    [CATransaction setAnimationDuration:resizeDuration];
    [CATransaction setAnimationTimingFunction:[CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseInEaseOut]];
    [CATransaction setCompletionBlock:^{
        // Restore the window to its original frame.
        [window setFrame:targetWindowFrame display:NO];
        window.styleMask = window.styleMask & ~NSWindowStyleMaskFullScreen;
        
        // Fade the real window back in after the scaling is done
        [CATransaction begin];
        [CATransaction setAnimationDuration:showBorderDuration];
        [CATransaction setAnimationTimingFunction:[CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseInEaseOut]];
        [CATransaction setCompletionBlock:^{
            [self OE_hideScreenshotWindow];
        }];
        
        [window.animator setAlphaValue:1.0];
        [CATransaction commit];
    }];
    
    [self->_screenshotWindow.animator setFrame:contentFrame display:YES];
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
    /* The game view uses the kCGColorSpaceITUR_709 color space, while the
     * screenshots we get use a sRGB color space for exactly the same pixel
     * data. We must fix this mismatch to make the screenshot window look
     * exactly the same as the game view. */
    CGImageRef cgimg = [screenshot CGImageForProposedRect:NULL context:NULL hints:nil];
    if (cgimg) {
        NSBitmapImageRep *rep = [[NSBitmapImageRep alloc] initWithCGImage:cgimg];
        CGColorSpaceRef cgcs = CGColorSpaceCreateWithName(kCGColorSpaceITUR_709);
        NSColorSpace *cs = [[NSColorSpace alloc] initWithCGColorSpace:cgcs];
        CGColorSpaceRelease(cgcs);
        rep = [rep bitmapImageRepByRetaggingWithColorSpace:cs];
        screenshot = [[NSImage alloc] init];
        [screenshot addRepresentation:rep];
    }
    
    [[self screenshotView] setImage:screenshot];
}

@end
