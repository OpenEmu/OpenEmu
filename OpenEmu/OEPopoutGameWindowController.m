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
#import "NSViewController+OEAdditions.h"
#import "NSWindow+OEFullScreenAdditions.h"
#import <QuartzCore/QuartzCore.h>
#import "OEUtilities.h"

#import "OEDBRom.h"
#import "OEDBGame.h"
#import "OEDBSystem.h"

#pragma mark - Private variables

static const NSSize _OEPopoutGameWindowMinSize = {100, 100};
static const NSSize _OEPopoutGameWindowMaxSize = {10000, 10000};
static const unsigned int _OEFitToWindowScale  = 0;

// user defaults
static NSString *const _OESystemIntegralScaleKeyFormat = @"OEIntegralScale.%@";
static NSString *const _OEIntegralScaleKey             = @"integralScale";
static NSString *const _OELastWindowSizeKey            = @"lastPopoutWindowSize";

@implementation OEPopoutGameWindowController
{
    NSScreen     *_screenBeforeWindowMove;
    unsigned int  _integralScale;
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

    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(OE_updateMaximumIntegralScale) name:NSApplicationDidChangeScreenParametersNotification object:nil];

    return self;
}

- (void)dealloc
{
    [[self window] setDelegate:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:self name:NSApplicationDidChangeScreenParametersNotification object:nil];
}

- (void)setDocument:(NSDocument *)document
{
    NSAssert(!document || [document isKindOfClass:[OEGameDocument class]], @"OEPopoutGameWindowController accepts OEGameDocument documents only");

    [super setDocument:document];

    if(document)
    {
        OEGameViewController *gameViewController = [[self OE_gameDocument] gameViewController];
        NSString *systemIdentifier               = [[[[gameViewController rom] game] system] systemIdentifier];
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

        NSWindow *window        = [self window];
        const NSRect windowRect = {NSZeroPoint, windowSize};

        [gameViewController setIntegralScalingDelegate:self];

        [window setFrame:windowRect display:YES animate:NO];
        [window center];
        [window setContentView:[gameViewController view]];
        [self OE_updateWindowMinMaxSizes];
    }
}

- (void)showWindow:(id)sender
{
    NSWindow *window = [self window];
    const BOOL needsToggleFullScreen = (!![self isWindowFullScreen] != !![window OE_isFullScreen]);

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

- (IBAction)changeIntegralScale:(id)sender
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
    NSScreen *screen             = ([[self window] screen] ? : [NSScreen mainScreen]);
    const NSSize maxContentSize  = [[self window] contentRectForFrameRect:[screen visibleFrame]].size;
    const NSSize defaultSize     = [[[self OE_gameDocument] gameViewController] defaultScreenSize];
    const unsigned int maxScale  = MAX(MIN(floor(maxContentSize.height / defaultSize.height), floor(maxContentSize.width / defaultSize.width)), 1);

    return maxScale;
}

- (unsigned int)currentIntegralScale
{
    return [[self window] OE_isFullScreen] ? _OEFitToWindowScale : _integralScale;
}

- (BOOL)allowsIntegralScaling
{
    return ![[self window] OE_isFullScreen];
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
    const CGFloat windowTitleBarHeight = 21.0;
    const NSSize contentSize           = [self OE_windowContentSizeForGameViewIntegralScale:gameViewIntegralScale];
    const NSSize windowSize            = {contentSize.width, contentSize.height + windowTitleBarHeight};

    return windowSize;
}

- (void)OE_changeGameViewIntegralScale:(unsigned int)newScale
{
    _integralScale = newScale;

    // It turns out we can’t change the style mask’s NSResizableWindowMask bit.
    // If we clear it because of integral scaling and set it because of free scaling,
    // NSWindow shrinks its frame size by a factor equal to title bar height.
    // Instead of changing the NSResizableWindowMask bit, we set window min/max sizes.

    if(newScale != _OEFitToWindowScale)
    {
        const NSRect currentFrame = [[self window] frame];
        const NSRect screenFrame  = [[[self window] screen] visibleFrame];

        NSRect newWindowFrame     = currentFrame;
        newWindowFrame.size       = [self OE_windowSizeForGameViewIntegralScale:newScale];

        const CGFloat heightDelta = newWindowFrame.size.height - currentFrame.size.height;

        // Keep the same distance from the top of the screen...
        newWindowFrame.origin.y -= heightDelta;

        // ...and make sure the title bar is always entirely visible
        if(NSMaxY(newWindowFrame) > NSMaxY(screenFrame))
            newWindowFrame.origin.y = NSMaxY(screenFrame) - newWindowFrame.size.height;
        
        [[self window] setFrame:newWindowFrame display:YES];
    }

    [self OE_updateWindowMinMaxSizes];
}

- (void)OE_constrainIntegralScaleIfNeeded
{
    if(_integralScale == _OEFitToWindowScale) return;

    const unsigned int newMaxScale = [self maximumIntegralScale];
    const NSRect newScreenFrame    = [[[self window] screen] visibleFrame];
    const NSRect currentFrame      = [[self window] frame];

    if(newScreenFrame.size.width < currentFrame.size.width || newScreenFrame.size.height < currentFrame.size.height)
        [self OE_changeGameViewIntegralScale:newMaxScale];
}

// Assumes both _integralScale and [[self window] frame] have been set
- (void)OE_updateWindowMinMaxSizes
{
    NSWindow *window = [self window];

    if(_integralScale == _OEFitToWindowScale)
    {
        [window setMinSize:_OEPopoutGameWindowMinSize];
        [window setMaxSize:_OEPopoutGameWindowMaxSize];
    }
    else
    {
        NSSize size = [window frame].size;
        [window setMinSize:size];
        [window setMaxSize:size];
    }
}

- (OEGameDocument *)OE_gameDocument
{
    return (OEGameDocument *)[self document];
}

#pragma mark - NSWindowDelegate

- (void)windowWillMove:(NSNotification *)notification
{
    _screenBeforeWindowMove = [[self window] screen];
}

- (void)windowDidMove:(NSNotification *)notification
{
    if(_screenBeforeWindowMove != [[self window] screen])
        [self OE_constrainIntegralScaleIfNeeded];

    _screenBeforeWindowMove = nil;
}

- (void)windowDidChangeScreen:(NSNotification *)notification
{
    [self OE_constrainIntegralScaleIfNeeded];
}

- (void)windowWillClose:(NSNotification *)notification
{
    OEGameViewController *gameViewController = [[self OE_gameDocument] gameViewController];

    NSString *systemIdentifier      = [[[[gameViewController rom] game] system] systemIdentifier];
    NSUserDefaults *userDefaults    = [NSUserDefaults standardUserDefaults];
    NSString *systemKey             = [NSString stringWithFormat:_OESystemIntegralScaleKeyFormat, systemIdentifier];
    NSDictionary *integralScaleInfo = (@{
                                       _OEIntegralScaleKey   : @(_integralScale),
                                       _OELastWindowSizeKey : NSStringFromSize([[self window] frame].size),
                                       });
    [userDefaults setObject:integralScaleInfo forKey:systemKey];
    [userDefaults synchronize]; // needed whilst AppKit isn’t fixed to synchronise defaults in -_deallocHardCore:

    [gameViewController viewWillDisappear];
    [gameViewController viewDidDisappear];
}

- (void)windowWillEnterFullScreen:(NSNotification *)notification
{
    [[self window] setMaxSize:_OEPopoutGameWindowMaxSize];
}

- (void)windowDidExitFullScreen:(NSNotification *)notification
{
    [self OE_updateWindowMinMaxSizes];
    [[self window] setAnimationBehavior:NSWindowAnimationBehaviorDocumentWindow];
}

@end
