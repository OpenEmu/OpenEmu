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

#import "OEGameControlsBar.h"

#import "OEDBRom.h"

@import OpenEmuKit;
#import "OEGameDocument.h"
#import "OEPopoutGameWindowController.h"

#import "OEDBSaveState.h"

#import "OEAudioDeviceManager.h"

@import QuartzCore;

#import "OpenEmu-Swift.h"

#pragma mark - Public variables

NSString *const OEGameControlsBarShowsAutoSaveStateKey  = @"HUDBarShowAutosaveState";
NSString *const OEGameControlsBarShowsQuickSaveStateKey = @"HUDBarShowQuicksaveState";
NSString *const OEGameControlsBarFadeOutDelayKey        = @"fadeoutdelay";
NSString *const OEGameControlsBarShowsAudioOutput       = @"HUDBarShowAudioOutput";

@interface OEGameControlsBar () <CAAnimationDelegate>
@property (strong) id eventMonitor;
@property (strong) NSTimer *fadeTimer;
//@property (strong) NSMutableArray *cheats;
//@property          BOOL cheatsLoaded;

@property (unsafe_unretained) OEGameViewController *gameViewController;
//@property (strong) OEGameControlsBarView *controlsView;
@property (strong, nonatomic) NSDate *lastMouseMovement;
@end

@implementation OEGameControlsBar
{
    NSRect _lastGameWindowFrame;
}

+ (void)initialize
{
    if(self != [OEGameControlsBar class])
        return;

    // Time until hud controls bar fades out
    [NSUserDefaults.standardUserDefaults registerDefaults:@{
        OEGameControlsBarFadeOutDelayKey : @1.5,
        OEGameControlsBarShowsAutoSaveStateKey : @NO,
        OEGameControlsBarShowsQuickSaveStateKey : @NO,
        OEGameControlsBarShowsAudioOutput : @NO,
     }];
}

- (id)initWithGameViewController:(OEGameViewController *)controller
{
    BOOL useNew = [NSUserDefaults.standardUserDefaults integerForKey:OEHUDBarAppearancePreferenceKey] == OEHUDBarAppearancePreferenceValueVibrant;
    
    if(useNew)
        self = [super initWithContentRect:NSMakeRect(0, 0, 442, 42) styleMask:NSWindowStyleMaskTitled backing:NSBackingStoreBuffered defer:YES];
    else
        self = [super initWithContentRect:NSMakeRect(0, 0, 481, 45) styleMask:NSWindowStyleMaskBorderless backing:NSBackingStoreBuffered defer:YES];
    
    if(self != nil)
    {
        self.movableByWindowBackground = YES;
        self.animationBehavior = NSWindowAnimationBehaviorNone;
        
        self.canShow = YES;
        self.gameViewController = controller;
        
        if(useNew)
        {
            self.titlebarAppearsTransparent = YES;
            self.titleVisibility = NSWindowTitleHidden;
            self.styleMask |= NSWindowStyleMaskFullSizeContentView;
            self.appearance = [NSAppearance appearanceNamed:NSAppearanceNameVibrantDark];
            
            NSVisualEffectView *veView = [NSVisualEffectView new];
            veView.material = NSVisualEffectMaterialHUDWindow;
            veView.state = NSVisualEffectStateActive;
            self.contentView = veView;
        }
        else
        {
            self.backgroundColor = NSColor.clearColor;
            self.alphaValue = 0.0;
        }
        
        NSRect barRect;
        if(useNew)
            barRect = NSMakeRect(0, 0, 442, 42);
        else
            barRect = NSMakeRect(0, 0, 442, 45);
        
        OEGameControlsBarView *barView = [[OEGameControlsBarView alloc] initWithFrame:barRect];
        
        [self.contentView addSubview:barView];
        
        __weak __auto_type weakSelf = self;
        _eventMonitor = [NSEvent addLocalMonitorForEventsMatchingMask:NSEventMaskMouseMoved handler:^NSEvent*(NSEvent* e)
                         {
                             OEGameControlsBar *self = weakSelf;
                             if (self == nil)
                             {
                                 return e;
                             }
                
                             if(NSApp.isActive && self.gameWindow.isMainWindow)
                                 [self performSelectorOnMainThread:@selector(mouseMoved:) withObject:e waitUntilDone:NO];
                             return e;
                         }];
        _controlsView = barView;

        [NSCursor setHiddenUntilMouseMoves:YES];

        NSNotificationCenter *nc = NSNotificationCenter.defaultCenter;
        // Show HUD when switching back from other applications
        [nc addObserver:self selector:@selector(mouseMoved:) name:NSApplicationDidBecomeActiveNotification object:nil];
        [nc addObserver:self selector:@selector(willMove:) name:NSWindowWillMoveNotification object:self];
        [nc addObserver:self selector:@selector(didMove:) name:NSWindowDidMoveNotification object:self];
    }
    return self;
}

- (void)dealloc
{
    [_fadeTimer invalidate];
    _fadeTimer = nil;
    _gameViewController = nil;

    [NSEvent removeMonitor:_eventMonitor];

    _gameWindow = nil;
}

- (NSRect)bounds
{
    NSRect bounds = self.frame;
    bounds.origin = NSMakePoint(0, 0);
    return bounds;
}


#pragma mark - Manage Visibility
- (void)show
{
    if(self.canShow)
        [self animator].alphaValue = 1;
}

- (void)hideAnimated:(BOOL)animated
{
    [NSCursor setHiddenUntilMouseMoves:YES];

    // only hide if 'docked' to game window (aka on the same screen)
    if(self.parentWindow)
    {
        if(animated)
            [self animator].alphaValue = 0;
        else
            self.alphaValue = 0;
    }

    [_fadeTimer invalidate];
    _fadeTimer = nil;
}

- (void)mouseMoved:(NSEvent *)theEvent
{
    [self _performMouseMoved:theEvent];
}

- (void)_performMouseMoved:(NSEvent *)theEvent
{
    NSWindow *gameWindow = self.gameWindow;
    if(gameWindow == nil) return;

    NSView *gameView = self.gameViewController.view;
    NSRect viewFrame = gameView.frame;
    NSPoint mouseLoc = NSEvent.mouseLocation;

    NSRect viewFrameOnScreen = [gameWindow convertRectToScreen:viewFrame];
    if(!NSPointInRect(mouseLoc, viewFrameOnScreen)) return;

    if(self.alphaValue == 0.0)
    {
        _lastMouseMovement = [NSDate date];
        [self show];
    }

    [self setLastMouseMovement:[NSDate date]];
}

- (void)setLastMouseMovement:(NSDate *)lastMouseMovementDate
{
    if(!_fadeTimer)
    {
        NSTimeInterval interval = [NSUserDefaults.standardUserDefaults doubleForKey:OEGameControlsBarFadeOutDelayKey];
        _fadeTimer = [NSTimer scheduledTimerWithTimeInterval:interval target:self selector:@selector(timerDidFire:) userInfo:nil repeats:YES];
    }

    _lastMouseMovement = lastMouseMovementDate;
}

- (void)timerDidFire:(NSTimer *)timer
{
    NSTimeInterval interval = [NSUserDefaults.standardUserDefaults doubleForKey:OEGameControlsBarFadeOutDelayKey];
    NSDate *hideDate = [_lastMouseMovement dateByAddingTimeInterval:interval];

    if([hideDate timeIntervalSinceNow] <= 0.0)
    {
        if([self canFadeOut])
        {
            [_fadeTimer invalidate];
            _fadeTimer = nil;

            [self hideAnimated:YES];
        }
        else
        {
            NSTimeInterval interval = [NSUserDefaults.standardUserDefaults doubleForKey:OEGameControlsBarFadeOutDelayKey];
            NSDate *nextTime = [NSDate dateWithTimeIntervalSinceNow:interval];

            _fadeTimer.fireDate = nextTime;
        }
    }
    else _fadeTimer.fireDate = hideDate;
}

- (BOOL)canFadeOut
{
    return !NSPointInRect(self.mouseLocationOutsideOfEventStream, [self bounds]);
}

- (void)repositionOnGameWindow
{
    if(!_gameWindow || !self.parentWindow) return;

    static const CGFloat _OEControlsMargin = 19;

    NSView *gameView = self.gameViewController.view;
    NSRect gameViewFrame = gameView.frame;
    NSRect gameViewFrameInWindow = [gameView convertRect:gameViewFrame toView:nil];
    NSPoint origin = [_gameWindow convertRectToScreen:gameViewFrameInWindow].origin;

    origin.x += (NSWidth(gameViewFrame) - NSWidth(self.frame)) / 2;

    // If the controls bar fits, it sits over the window
    if(NSWidth(gameViewFrame) >= NSWidth(self.frame))
        origin.y += _OEControlsMargin;
    else
    {
        // Otherwise, it sits below the window
        origin.y -= (NSHeight(self.frame) + _OEControlsMargin);

        // Unless below the window means it being off-screen, in which case it sits above the window
        if(origin.y < NSMinY(_gameWindow.screen.visibleFrame))
            origin.y = NSMaxY(_gameWindow.frame) + _OEControlsMargin;
    }

    [self setFrameOrigin:origin];
}
#pragma mark -

- (void)willMove:(NSNotification *)notification
{
    if (self.parentWindow)
        _lastGameWindowFrame = self.parentWindow.frame;
}

- (void)didMove:(NSNotification*)notification
{
    BOOL userMoved = NO;
    if (!self.parentWindow)
        userMoved = YES;
    else
        userMoved = NSEqualRects(self.parentWindow.frame, _lastGameWindowFrame);
    [self adjustWindowAttachment:userMoved];
}

- (void)adjustWindowAttachment:(BOOL)userMovesGameWindow
{
    NSWindow *gameWindow = self.gameWindow;
    NSScreen *barScreen  = self.screen;
    NSScreen *gameScreen = gameWindow.screen;

    BOOL screensDiffer = barScreen != gameScreen;

    if(userMovesGameWindow && screensDiffer && self.parentWindow != nil && barScreen != nil)
    {
        NSRect f = self.frame;
        [self orderOut:nil];
        [self setFrame:NSZeroRect display:NO];
        [self setFrame:f display:NO];
        [self orderFront:self];
    }
    else if(!screensDiffer && self.parentWindow == nil)
    {
        // attach to window and center the controls bar
        [self.gameWindow addChildWindow:self ordered:NSWindowAbove];
        [self repositionOnGameWindow];
    }
}

- (void)mouseUp:(NSEvent *)theEvent
{
    [super mouseUp:theEvent];
    [self adjustWindowAttachment:NO];
}

#pragma mark - Updating UI States

- (void)setGameWindow:(NSWindow *)gameWindow
{
    NSNotificationCenter *nc = NSNotificationCenter.defaultCenter;

    // un-register notifications for parent window
    if(self.parentWindow != nil)
    {
        [nc removeObserver:self name:NSWindowDidEnterFullScreenNotification object:_gameWindow];
        [nc removeObserver:self name:NSWindowWillExitFullScreenNotification object:_gameWindow];
        [nc removeObserver:self name:NSWindowDidChangeScreenNotification    object:_gameWindow];
    }

    // remove from parent window if there was one, and attach to to the new game window
    if((!_gameWindow || self.parentWindow) && gameWindow != self.parentWindow)
    {
        [self.parentWindow removeChildWindow:self];
        [gameWindow addChildWindow:self ordered:NSWindowAbove];
    }

    _gameWindow = gameWindow;

    // register notifications and update state of the fullscreen button
    if(gameWindow != nil)
    {
        [nc addObserver:self selector:@selector(gameWindowDidEnterFullScreen:) name:NSWindowDidEnterFullScreenNotification object:gameWindow];
        [nc addObserver:self selector:@selector(gameWindowWillExitFullScreen:) name:NSWindowWillExitFullScreenNotification object:gameWindow];

        [self.controlsView reflectFullScreen:gameWindow.isFullScreen];
    }
}

@end
