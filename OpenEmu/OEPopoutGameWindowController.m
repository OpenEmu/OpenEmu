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

#import "OEMainWindowController.h"
#import "OEGameDocument.h"
#import "OEGameViewController.h"
#import "OEGameControlsBar.h"
#import "OEUtilities.h"

#import "OEDBRom.h"
#import "OEDBGame.h"
#import "OEDBSystem+CoreDataProperties.h"

@import QuartzCore;

#import "OpenEmu-Swift.h"

#pragma mark - Private variables

static const NSSize       _OEPopoutGameWindowMinSize = {100, 100};
static const unsigned int _OEFitToWindowScale        = 0;

// User defaults
static NSString *const _OESystemIntegralScaleKeyFormat = @"OEIntegralScale.%@";
static NSString *const _OEIntegralScaleKey             = @"integralScale";
static NSString *const _OELastWindowSizeKey            = @"lastPopoutWindowSize";

typedef NS_ENUM(NSInteger, OEPopoutGameWindowFullScreenStatus)
{
    _OEPopoutGameWindowFullScreenStatusNonFullScreen = 0,
    _OEPopoutGameWindowFullScreenStatusFullScreen,
    _OEPopoutGameWindowFullScreenStatusEntering,
    _OEPopoutGameWindowFullScreenStatusExiting,
};



@interface OEScreenshotWindow : NSWindow
@property(nonatomic, unsafe_unretained) NSImageView *screenshotView;
- (void)setScreenshot:(NSImage *)screenshot;
@end



@implementation OEPopoutGameWindowController
{
    NSScreen                           *_screenBeforeWindowMove;
    unsigned int                        _integralScale;

    // Full screen
    OEScreenshotWindow                 *_screenshotWindow;
    OEPopoutGameWindowFullScreenStatus  _fullScreenStatus;
    BOOL                                _resumePlayingAfterFullScreenTransition;
    BOOL                                _snapResize;
    BOOL                                _shouldSnapResize;
    OEIntegralWindowResizingDelegate   *_snapDelegate;
    // State prior to entering full screen
    NSRect                              _windowedFrame;
    unsigned int                        _windowedIntegralScale;
}

#pragma mark - NSWindowController overridden methods

- (id)initWithWindow:(NSWindow *)window
{
    self = [super initWithWindow:window];
    if(!self)
        return nil;

    _snapDelegate = [OEIntegralWindowResizingDelegate new];
    
    [NSUserDefaults.standardUserDefaults addObserver:self
                                          forKeyPath:OEPopoutGameWindowIntegerScalingOnlyKey
                                             options:NSKeyValueObservingOptionNew
                                             context:NULL];
    
    if ([NSUserDefaults.standardUserDefaults boolForKey:OEPopoutGameWindowIntegerScalingOnlyKey])
        _shouldSnapResize = YES;
    else
        _shouldSnapResize = NO;
    
    [window setDelegate:self];
    [window setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
    [window setAnimationBehavior:NSWindowAnimationBehaviorDocumentWindow];
    [window setMinSize:_OEPopoutGameWindowMinSize];
    window.tabbingMode = NSWindowTabbingModeDisallowed;
    
    NSUserDefaults *ud = [NSUserDefaults standardUserDefaults];
    NSString *bgColorStr = [ud objectForKey:OEGameViewBackgroundColorKey];
    NSColor *bgColor = bgColorStr ? [[NSColor alloc] colorFromHexString:bgColorStr] : [NSColor blackColor];
    window.backgroundColor = bgColor;
    
    if([ud boolForKey:OEPopoutGameWindowAlwaysOnTopKey] == YES)
    {
        window.level = NSFloatingWindowLevel;
    }

    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(OE_constrainIntegralScaleIfNeeded) name:NSApplicationDidChangeScreenParametersNotification object:nil];

    return self;
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object
    change:(NSDictionary<NSKeyValueChangeKey, id> *)change context:(void *)context
{
    if (object == NSUserDefaults.standardUserDefaults && [keyPath isEqualToString:OEPopoutGameWindowIntegerScalingOnlyKey])
        _shouldSnapResize = _shouldSnapResize ? NO : YES;
    else
        [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
}

- (void)dealloc
{
    [[self window] setDelegate:nil];
    [self setWindow:nil];
    [NSUserDefaults.standardUserDefaults removeObserver:self forKeyPath:OEPopoutGameWindowIntegerScalingOnlyKey];
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

        unsigned int lastScale = ([lastScaleNumber respondsToSelector:@selector(unsignedIntValue)] ?
                                  MIN([lastScaleNumber unsignedIntValue], maxScale) :
                                  maxScale);

        if(lastScale == _OEFitToWindowScale)
        {
            windowSize = NSSizeFromString([integralScaleInfo objectForKey:_OELastWindowSizeKey]);
            if(windowSize.width == 0 || windowSize.height == 0)
                windowSize = [self OE_windowSizeForGameViewIntegralScale:maxScale];
        }
        else
            windowSize = [self OE_windowSizeForGameViewIntegralScale:lastScale];

        NSWindow *window        = self.window;
        const NSRect windowRect = {NSZeroPoint, windowSize};

        [gameViewController setIntegralScalingDelegate:self];

        window.contentViewController = gameViewController;
        [window setFrame:windowRect display:NO animate:NO];
        [window center];
        [window setContentAspectRatio:gameViewController.defaultScreenSize];
        
        _integralScale = lastScale;

        [self OE_buildScreenshotWindow];
    }
}

- (void)showWindow:(id)sender
{
    NSWindow *window = [self window];
    const BOOL needsToggleFullScreen = (!![self isWindowFullScreen] != !![window isFullScreen]);

    if(![window isVisible])
    {
        // We disable window animation if we need to toggle full screen because two parallel animations
        // (window being ordered front and toggling full-screen) looks painfully ugly. The animation
        // behaviour is restored in -windowDidExitFullScreen:.
        if(needsToggleFullScreen)
            [window setAnimationBehavior:NSWindowAnimationBehaviorNone];
        
        [window makeKeyAndOrderFront:sender];
    }

    if(needsToggleFullScreen)
        [window toggleFullScreen:self];
}

#pragma mark - Menu Items

- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
    SEL action = menuItem.action;
    if(action == @selector(floatOnTop:))
    {
        if(self.window.level == NSFloatingWindowLevel)
            menuItem.state = NSControlStateValueOn;
        else
            menuItem.state = NSControlStateValueOff;
    }
    
    return YES;
}

- (IBAction)floatOnTop:(NSMenuItem *)sender
{
    if(self.window.level == NSNormalWindowLevel)
    {
        self.window.level = NSFloatingWindowLevel;
    }
    else
    {
        self.window.level = NSNormalWindowLevel;
    }
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
    return _integralScale;
}

- (BOOL)shouldAllowIntegralScaling
{
    return YES;
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
    if (_integralScale == newScale) return;
    
    _integralScale = newScale;
    
    if (_fullScreenStatus == _OEPopoutGameWindowFullScreenStatusNonFullScreen)
    {
        if (newScale == _OEFitToWindowScale)
            return;

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
    else
    {
        OEGameViewController *gv = [self OE_gameDocument].gameViewController;
        if (newScale == _OEFitToWindowScale)
        {
            [gv gameViewSetIntegralSize:CGSizeZero animated:YES];
        }
        else
        {
            NSSize newSize = [self OE_windowSizeForGameViewIntegralScale:newScale];
            [gv gameViewSetIntegralSize:newSize animated:YES];
        }
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
    NSScreen *mainScreen                     = [[NSScreen screens] objectAtIndex:0];
    const NSRect screenFrame                 = [mainScreen frame];
    _screenshotWindow  = [[OEScreenshotWindow alloc] initWithContentRect:screenFrame
                                                               styleMask:NSWindowStyleMaskBorderless
                                                                 backing:NSBackingStoreBuffered
                                                                   defer:NO];
    [_screenshotWindow setBackgroundColor:self.window.backgroundColor];
    [_screenshotWindow setOpaque:NO];
    [_screenshotWindow setAnimationBehavior:NSWindowAnimationBehaviorNone];
    
    NSView *view = _screenshotWindow.contentView;

    NSImageView  *imageView  = [[NSImageView alloc] initWithFrame:(NSRect){.origin = NSZeroPoint, .size = view.frame.size}];
    imageView.imageAlignment = NSImageAlignCenter;
    imageView.imageScaling   = NSImageScaleProportionallyUpOrDown;
    [imageView setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
    _screenshotWindow.screenshotView = imageView;
    [view addSubview:imageView];
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

    const NSSize windowSize         = ([[self window] isFullScreen] ? _windowedFrame.size : [[self window] frame].size);
    NSString *systemIdentifier      = [[[[[gameViewController document] rom] game] system] systemIdentifier];
    NSUserDefaults *userDefaults    = [NSUserDefaults standardUserDefaults];
    NSString *systemKey             = [NSString stringWithFormat:_OESystemIntegralScaleKeyFormat, systemIdentifier];
    NSDictionary *integralScaleInfo = @{
        _OEIntegralScaleKey  : @(_integralScale),
        _OELastWindowSizeKey : NSStringFromSize(windowSize),
    };
    [userDefaults setObject:integralScaleInfo forKey:systemKey];
}

- (void)windowWillStartLiveResize:(NSNotification *)notification
{
    if (_fullScreenStatus != _OEPopoutGameWindowFullScreenStatusNonFullScreen)
        return;
    
    BOOL shiftPressed = (NSEvent.modifierFlags & NSEventModifierFlagShift) != 0;
    if ((_shouldSnapResize && !shiftPressed) || (!_shouldSnapResize && shiftPressed))
    {
        _snapResize = YES;
    }
    
    if (_snapResize)
    {
        // necessary to prevent erratic resize behavior when window is
        // resized via top or bottom only (Y-axis)
        [self.window setContentAspectRatio:CGSizeZero];
        _snapDelegate.currentScale = _integralScale;
        _snapDelegate.screenSize = [self OE_gameDocument].gameViewController.defaultScreenSize;
        [_snapDelegate windowWillStartLiveResize:notification];
    }
}

- (NSSize)windowWillResize:(NSWindow *)sender toSize:(NSSize)frameSize
{
    if (_fullScreenStatus != _OEPopoutGameWindowFullScreenStatusNonFullScreen) {
        return frameSize;
    }
    
    if (_snapResize) {
        return [_snapDelegate windowWillResize:sender toSize:frameSize];
    }

    _integralScale = _OEFitToWindowScale;
    
    return frameSize;
}

- (void)windowDidEndLiveResize:(NSNotification *)notification
{
    if (_snapResize)
    {
        [self.window setContentAspectRatio:[self OE_gameDocument].gameViewController.defaultScreenSize];
        [_snapDelegate windowDidEndLiveResize:notification];
        _integralScale = (unsigned int)_snapDelegate.currentScale;
        _snapResize = NO;
    }
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

    _fullScreenStatus       = _OEPopoutGameWindowFullScreenStatusEntering;
    _windowedFrame          = self.window.frame;
    _windowedIntegralScale  = _integralScale;

    _resumePlayingAfterFullScreenTransition = ![[self document] isEmulationPaused];
    [[self document] setEmulationPaused:YES];
    
    // move the screenshot window to the same screen as the game window
    // otherwise it will be shown in the system full-screen animation
    NSScreen *mainScreen = self.window.screen;
    [_screenshotWindow setFrameOrigin:mainScreen.frame.origin];

    [[gameViewController controlsWindow] setCanShow:NO];
    [[gameViewController controlsWindow] hideAnimated:YES];
    if (_integralScale != _OEFitToWindowScale)
    {
        CGSize size = [self OE_windowContentSizeForGameViewIntegralScale:_integralScale];
        [gameViewController gameViewSetIntegralSize:size animated:NO];
    }
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
    
    NSRect screenshotFrame = screenFrame;
    if (_integralScale != _OEFitToWindowScale)
    {
        NSPoint origin = NSMakePoint((screenFrame.size.width  - contentFrame.size.width)  / 2.0,
                                     (screenFrame.size.height - contentFrame.size.height) / 2.0);
        origin.x += screenFrame.origin.x;
        origin.y += screenFrame.origin.y;
        screenshotFrame = (NSRect){ .origin = origin, .size = contentFrame.size };
        screenshotFrame = NSIntegralRect(screenshotFrame);
    }

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
        
        [self->_screenshotWindow.animator setFrame:screenshotFrame display:YES];
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
}

- (void)windowWillExitFullScreen:(NSNotification *)notification
{
    OEGameViewController *gameViewController = [[self OE_gameDocument] gameViewController];

    _fullScreenStatus                        = _OEPopoutGameWindowFullScreenStatusExiting;
    _resumePlayingAfterFullScreenTransition  = ![[self document] isEmulationPaused];
    
    [[self document] setEmulationPaused:YES];
    
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
    
    /* a window in full-screen mode does not have a title bar, thus we have
     * to explicitly specify the style mask to compute the correct content
     * frame */
    const NSRect contentFrame = [NSWindow contentRectForFrameRect:_windowedFrame styleMask:window.styleMask & ~NSWindowStyleMaskFullScreen];
    
    NSSize fullScreenContentSize    = gameViewController.gameView.frame.size;
    NSRect screenshotWindowedFrame  = contentFrame;
    NSPoint targetWindowOrigin      = _windowedFrame.origin;

    NSRect screenshotFrame = screenFrame;
    if (_integralScale != _OEFitToWindowScale)
    {
        NSPoint origin = NSMakePoint((screenFrame.size.width  - fullScreenContentSize.width)  / 2.0,
                                     (screenFrame.size.height - fullScreenContentSize.height) / 2.0);
        origin.x += screenFrame.origin.x;
        origin.y += screenFrame.origin.y;
        screenshotFrame = (NSRect){ .origin = origin, .size = fullScreenContentSize };
        screenshotFrame = NSIntegralRect(screenshotFrame);
    }

    [_screenshotWindow setScreenshot:[gameViewController screenshot]];
    [_screenshotWindow setFrame:screenshotFrame display:YES];
    [_screenshotWindow orderFront:self];
    [window setAlphaValue:0.0];

    // Scale the screenshot window down
    [CATransaction begin];
    [CATransaction setAnimationDuration:resizeDuration];
    [CATransaction setAnimationTimingFunction:[CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseInEaseOut]];
    [CATransaction setCompletionBlock:^{
        // Restore the window to its original frame.
        //   We do it using -setContentSize: instead of -setFrame:display:
        // because -setFrame:display: does not obey the new style mask correctly
        // for some reason
        window.styleMask = window.styleMask & ~NSWindowStyleMaskFullScreen;
        [window setContentSize:contentFrame.size];
        [window setFrameOrigin:targetWindowOrigin];
        
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
    
    [_screenshotWindow.animator setFrame:screenshotWindowedFrame display:YES];
    [CATransaction commit];
}

- (void)windowDidExitFullScreen:(NSNotification *)notification
{
    OEGameViewController *gameViewController = [[self OE_gameDocument] gameViewController];

    _fullScreenStatus = _OEPopoutGameWindowFullScreenStatusNonFullScreen;
    _integralScale    = _windowedIntegralScale;

    if(_resumePlayingAfterFullScreenTransition)
        [[self document] setEmulationPaused:NO];

    [[gameViewController controlsWindow] hideAnimated:YES];
    [[gameViewController controlsWindow] setCanShow:YES];
    [gameViewController gameViewFillSuperView];
}

- (void)windowDidFailToEnterFullScreen:(NSWindow *)window
{
    OEGameViewController *gameViewController = [[self OE_gameDocument] gameViewController];

    _fullScreenStatus = _OEPopoutGameWindowFullScreenStatusNonFullScreen;

    if(_resumePlayingAfterFullScreenTransition)
        [[self document] setEmulationPaused:NO];

    [[gameViewController controlsWindow] setCanShow:YES];
}

- (void)windowDidFailToExitFullScreen:(NSWindow *)window
{
    OEGameViewController *gameViewController = [[self OE_gameDocument] gameViewController];

    _fullScreenStatus = _OEPopoutGameWindowFullScreenStatusFullScreen;

    if(_resumePlayingAfterFullScreenTransition)
        [[self document] setEmulationPaused:NO];

    [[gameViewController controlsWindow] setCanShow:YES];
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
