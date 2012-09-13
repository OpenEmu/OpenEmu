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
#import "NSImage+OEDrawingAdditions.h"

#import "OEButton.h"
#import "OEHUDSlider.h"

#import "OEMenu.h"
#import "OEDBRom.h"

#import "OECompositionPlugin.h"
#import "OECorePlugin.h"
#import "OEGameViewController.h"

#import "OEHUDAlert.h"

#import "OEDBSaveState.h"

NSString *const OEGameControlsBarCanDeleteSaveStatesKey = @"HUDBarCanDeleteState";
NSString *const OEGameControlsBarHidesOptionButtonKey   = @"HUDBarWithoutOptions";
NSString *const OEGameControlsBarFadeOutDelayKey        = @"fadeoutdelay";

@class OEHUDSlider;
@interface OEHUDControlsBarView : NSView

@property (strong, readonly) OEHUDSlider     *slider;
@property (strong, readonly) OEButton        *fullScreenButton;
@property (strong, readonly) OEButton        *pauseButton;

- (void)setupControls;
@end

@interface OEGameControlsBar ()
@property (strong) OEHUDControlsBarView *controlsView;
@property (strong, nonatomic) NSDate *lastMouseMovement;
@end

@implementation OEGameControlsBar
@synthesize lastMouseMovement;
@synthesize gameViewController;
@synthesize controlsView;

+ (void)initialize
{
    // Time until hud controls bar fades out
    [[NSUserDefaults standardUserDefaults] registerDefaults:@{ OEGameControlsBarFadeOutDelayKey : @1.5 } ];
}

- (id)initWithGameViewController:(OEGameViewController *)controller
{
    BOOL hideOptions = [[NSUserDefaults standardUserDefaults] boolForKey:OEGameControlsBarHidesOptionButtonKey];
    
    self = [super initWithContentRect:NSMakeRect(0, 0, 431 + (hideOptions ? 0 : 50), 45) styleMask:NSBorderlessWindowMask backing:NSWindowBackingLocationDefault defer:YES];
    if(self != nil)
    {
        [self setMovableByWindowBackground:YES];
        [self setOpaque:NO];
        [self setBackgroundColor:[NSColor clearColor]];
        [self setAlphaValue:0.0];
        
        [self setGameViewController:controller];
        
        OEHUDControlsBarView *barView = [[OEHUDControlsBarView alloc] initWithFrame:NSMakeRect(0, 0, 431 + (hideOptions ? 0 : 50), 45)];
        [[self contentView] addSubview:barView];
        [barView setupControls];
        
        eventMonitor = [NSEvent addGlobalMonitorForEventsMatchingMask:NSMouseMovedMask handler:
                        ^(NSEvent *incomingEvent)
                        {
                            if([NSApp isActive] && [[self parentWindow] isMainWindow])
                                [self performSelectorOnMainThread:@selector(mouseMoved:) withObject:incomingEvent waitUntilDone:NO];
                        }];
        openMenus = 0;
        controlsView = barView;
        
        [NSCursor setHiddenUntilMouseMoves:YES];
    }
    return self;
}

- (void)dealloc
{    
    [fadeTimer invalidate];
    fadeTimer = nil;
    
    [self setGameViewController:nil];
    
    [NSEvent removeMonitor:eventMonitor];
}

#pragma mark -
- (void)show
{
    [[self animator] setAlphaValue:1.0];
}

- (void)hide
{
    [[self animator] setAlphaValue:0.0];
    [fadeTimer invalidate];
    fadeTimer = nil;
    
    [NSCursor setHiddenUntilMouseMoves:YES];
}

- (void)mouseMoved:(NSEvent *)theEvent
{
    NSWindow *parentWindow = [self parentWindow];
    NSPoint mouseLoc = [NSEvent mouseLocation];
    
    if(!NSPointInRect(mouseLoc, [parentWindow convertRectToScreen:[[[self gameViewController] view] frame]])) return;
    
    if([self alphaValue] == 0.0)
    {
        lastMouseMovement = [NSDate date];
        [self show];
    }
    
    [self setLastMouseMovement:[NSDate date]];
}

- (void)setLastMouseMovement:(NSDate *)lastMouseMovementDate
{
    if(!fadeTimer)
    {
        NSTimeInterval interval = [[NSUserDefaults standardUserDefaults] doubleForKey:OEGameControlsBarFadeOutDelayKey];
        fadeTimer = [NSTimer scheduledTimerWithTimeInterval:interval target:self selector:@selector(timerDidFire:) userInfo:nil repeats:YES];
    }
    
    
    lastMouseMovement = lastMouseMovementDate;
}

- (void)timerDidFire:(NSTimer *)timer
{
    NSTimeInterval interval = [[NSUserDefaults standardUserDefaults] doubleForKey:OEGameControlsBarFadeOutDelayKey];
    NSDate *hideDate = [lastMouseMovement dateByAddingTimeInterval:interval];
    
    if([hideDate timeIntervalSinceNow] <= 0.0)
    {
        if([self canFadeOut])
        {
            [fadeTimer invalidate];
            fadeTimer = nil;
            
            [self hide];
        }
        else
        {
            NSTimeInterval interval = [[NSUserDefaults standardUserDefaults] doubleForKey:OEGameControlsBarFadeOutDelayKey];
            NSDate *nextTime = [NSDate dateWithTimeIntervalSinceNow:interval];
            
            [fadeTimer setFireDate:nextTime];
        }
    }
    else [fadeTimer setFireDate:hideDate];
}

- (NSRect)bounds
{
    NSRect bounds = [self frame];
    bounds.origin = NSMakePoint(0, 0);
    return bounds;
}

- (BOOL)canFadeOut
{
    return openMenus == 0 && !NSPointInRect([self mouseLocationOutsideOfEventStream], [self bounds]);
}

#pragma mark - Menus
- (void)showOptionsMenu:(id)sender
{
    NSMenu *menu = [[NSMenu alloc] init];
    
    NSMenuItem *item;
    item = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Edit Game Controls", @"") action:@selector(editControls:) keyEquivalent:@""];
    [menu addItem:item];
    
    // Setup Core selection menu
    NSMenu* coresMenu = [[NSMenu alloc] init];
    [coresMenu setTitle:NSLocalizedString(@"Select Core", @"")];
    
    NSString* systemIdentifier = [[self gameViewController] systemIdentifier];
    NSArray *corePlugins = [OECorePlugin corePluginsForSystemIdentifier:systemIdentifier];
    if([corePlugins count] > 1)
    {
        corePlugins = [corePlugins sortedArrayUsingComparator:^NSComparisonResult(id obj1, id obj2) {
            return [[obj1 displayName] compare:[obj2 displayName]];
        }];
        
        for(OECorePlugin *aPlugin in corePlugins)
        {
            NSMenuItem *coreItem = [[NSMenuItem alloc] initWithTitle:[aPlugin displayName] action:@selector(switchCore:) keyEquivalent:@""];
            [coreItem setRepresentedObject:aPlugin];
            
            if([[aPlugin bundleIdentifier] isEqualTo:[[self gameViewController] coreIdentifier]]) [coreItem setState:NSOnState];
            
            [coresMenu addItem:coreItem];
        }
        
        item = [[NSMenuItem alloc] init];
        item.title = NSLocalizedString(@"Select Core", @"");
        [item setSubmenu:coresMenu];
        if([[coresMenu itemArray] count]>1)
            [menu addItem:item];
    }
    
    // Setup Video Filter Menu
    NSMenu *filterMenu = [[NSMenu alloc] init];
    [filterMenu setTitle:NSLocalizedString(@"Select Filter", @"")];
    // Setup plugins menu
    NSArray *filterPlugins = [[OECompositionPlugin allPluginNames] sortedArrayUsingSelector:@selector(caseInsensitiveCompare:)];
    // These filters are loaded and run by GL, and do not rely on QTZs
    NSArray *filterNames = [filterPlugins arrayByAddingObjectsFromArray:OEOpenGLFilterNameArray];
    
    NSString *selectedFilter = [[NSUserDefaults standardUserDefaults] objectForKey:OEGameVideoFilterKey];
    for(NSString *aName in filterNames)
    {
        NSMenuItem *filterItem = [[NSMenuItem alloc] initWithTitle:aName action:@selector(selectFilter:) keyEquivalent:@""];
        
        if([aName isEqualToString:selectedFilter]) [filterItem setState:NSOnState];
        
        [filterMenu addItem:filterItem];
    }
    
    item = [[NSMenuItem alloc] init];
    item.title = NSLocalizedString(@"Select Filter", @"");
    [menu addItem:item];
    [item setSubmenu:filterMenu];

    // Create OEMenu and display it
    [menu setDelegate:self];

    NSRect targetRect = [sender bounds];
    targetRect.size.width -= 7.0;
    targetRect = NSInsetRect(targetRect, 0.0, 3.0);
    targetRect = [self convertRectToScreen:[sender convertRect:targetRect toView:nil]];

    NSDictionary *options = @{ OEMenuOptionsStyleKey : @(OEMenuStyleLight),
                           OEMenuOptionsArrowEdgeKey : @(OEMinYEdge),
                         OEMenuOptionsMaximumSizeKey : [NSValue valueWithSize:NSMakeSize(500, 256)],
                          OEMenuOptionsScreenRectKey : [NSValue valueWithRect:targetRect] };
    
    [OEMenu openMenu:menu withEvent:nil forView:sender options:options];
}

- (void)showSaveMenu:(id)sender
{
    NSMenu *menu = [[NSMenu alloc] init];
    
    NSMenuItem *item;
    item = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Save Current Game", @"") action:@selector(saveState:) keyEquivalent:@""];
    [menu addItem:item];
    
    NSArray *saveStates = nil;
    
    [[[self gameViewController] rom] removeMissingStates];
    if([[self gameViewController] rom] != nil && (saveStates = [[[self gameViewController] rom] normalSaveStatesByTimestampAscending:YES]) && [saveStates count] != 0)
    {
        [menu addItem:[NSMenuItem separatorItem]];
        for(OEDBSaveState* saveState in saveStates)
        {
            NSString *itemTitle = [saveState name];
            if(!itemTitle || [itemTitle isEqualToString:@""])
                itemTitle = [NSString stringWithFormat:@"%@", [saveState timestamp]];
            
            item = [[NSMenuItem alloc] initWithTitle:itemTitle action:@selector(loadState:) keyEquivalent:@""];
            [item setRepresentedObject:saveState];
            [menu addItem:item];

            if([[NSUserDefaults standardUserDefaults] boolForKey:OEGameControlsBarCanDeleteSaveStatesKey])
            {
                NSMenuItem *alternateItem = [[NSMenuItem alloc] initWithTitle:itemTitle action:@selector(deleteSaveState:) keyEquivalent:@""];
                [alternateItem setAlternate:YES];
                [alternateItem setKeyEquivalentModifierMask:NSAlternateKeyMask];
                [alternateItem setRepresentedObject:saveState];
                [menu addItem:alternateItem];
            }
        }
    }

    [menu setDelegate:self];

    NSRect targetRect = [sender bounds];
    targetRect.size.width -= 7.0;
    targetRect = NSInsetRect(targetRect, 0.0, 3.0);
    targetRect = [self convertRectToScreen:[sender convertRect:targetRect toView:nil]];
    

    NSDictionary *options = @{ OEMenuOptionsStyleKey : @(OEMenuStyleLight),
    OEMenuOptionsArrowEdgeKey : @(OEMinYEdge),
    OEMenuOptionsMaximumSizeKey : [NSValue valueWithSize:NSMakeSize(500, 256)],
    OEMenuOptionsScreenRectKey : [NSValue valueWithRect:targetRect] };

    [OEMenu openMenu:menu withEvent:nil forView:sender options:options];
}

#pragma mark -
#pragma mark OEMenuDelegate Implementation
- (void)menuWillOpen:(NSMenu *)menu
{
    openMenus++;
}

- (void)menuDidClose:(NSMenu *)menu
{
    openMenus--;
}

#pragma mark - Updating UI States
- (void)reflectVolume:(float)volume
{
    OEHUDControlsBarView    *view   = [[[self contentView] subviews] lastObject];
    OEHUDSlider             *slider = [view slider];
    
    if(volume == [slider floatValue]) return;
    [[slider animator] setFloatValue:volume];
}

- (void)reflectEmulationRunning:(BOOL)flag
{
    OEHUDControlsBarView    *view        = [[[self contentView] subviews] lastObject];
    NSButton                *pauseButton = [view pauseButton];
    [pauseButton setState:flag];
}

- (void)parentWindowDidEnterFullScreen:(NSNotification *)notification;
{
    OEHUDControlsBarView    *view        = [[[self contentView] subviews] lastObject];

    [[view fullScreenButton] setState:NSOnState];
}

- (void)parentWindowWillExitFullScreen:(NSNotification *)notification;
{
    OEHUDControlsBarView    *view        = [[[self contentView] subviews] lastObject];

    [[view fullScreenButton] setState:NSOffState];
}

- (void)setParentWindow:(NSWindow *)window
{
    NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
    
    if([self parentWindow] != nil)
    {
        [nc removeObserver:self name:NSWindowDidEnterFullScreenNotification object:[self parentWindow]];
        [nc removeObserver:self name:NSWindowWillExitFullScreenNotification object:[self parentWindow]];
    }
    
    [super setParentWindow:window];
    
    if(window != nil)
    {
        [nc addObserver:self selector:@selector(parentWindowDidEnterFullScreen:) name:NSWindowDidEnterFullScreenNotification object:window];
        [nc addObserver:self selector:@selector(parentWindowWillExitFullScreen:) name:NSWindowWillExitFullScreenNotification object:window];
    
        if(([window styleMask] & NSFullScreenWindowMask) == NSFullScreenWindowMask)
        {
            OEHUDControlsBarView    *view        = [[[self contentView] subviews] lastObject];

            [[view fullScreenButton] setImage:[NSImage imageNamed:@"hud_exit_fullscreen_glyph_normal"]];
            [[view fullScreenButton] setAlternateImage:[NSImage imageNamed:@"hud_exit_fullscreen_glyph_pressed"]];
        }
    }
}

@end

@implementation OEHUDControlsBarView
@synthesize slider, fullScreenButton, pauseButton;

- (id)initWithFrame:(NSRect)frame
{
    if((self = [super initWithFrame:frame]))
        [self setWantsLayer:YES];
    return self;
}

- (BOOL)isOpaque
{
    return NO;
}

#pragma mark -
- (void)drawRect:(NSRect)dirtyRect
{
    NSImage *barBackground = [NSImage imageNamed:@"hud_bar"];
    [barBackground drawInRect:[self bounds] fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil leftBorder:15 rightBorder:15 topBorder:0 bottomBorder:0];
}

- (void)setupControls
{
    OEButton *stopButton = [[OEButton alloc] init];
    [stopButton setThemeKey:@"hud_button_power"];
    [stopButton setTitle:nil];
    [stopButton setAction:@selector(terminateEmulation)];
    [stopButton setFrame:NSMakeRect(10, 13, 51, 23)];
    [stopButton setAutoresizingMask:NSViewMaxXMargin | NSViewMinYMargin];
    [self addSubview:stopButton];
        
    pauseButton = [[OEButton alloc] init];
    [pauseButton setButtonType:NSToggleButton];
    [pauseButton setThemeKey:@"hud_button_toggle_pause"];
    [pauseButton setAction:@selector(toggleEmulationPaused)];
    [pauseButton setFrame:NSMakeRect(82, 9, 32, 32)];
    [pauseButton setAutoresizingMask:NSViewMaxXMargin | NSViewMinYMargin];
    [self addSubview:pauseButton];
    
    OEButton *restartButton = [[OEButton alloc] init];
    [restartButton setThemeKey:@"hud_button_restart"];
    [restartButton setAction:@selector(resetGame)];
    [restartButton setFrame:NSMakeRect(111, 9, 32, 32)];
    [restartButton setAutoresizingMask:NSViewMaxXMargin | NSViewMinYMargin];
    [self addSubview:restartButton];
    
    OEButton *saveButton = [[OEButton alloc] init];
    [saveButton setThemeKey:@"hud_button_save"];
    [saveButton setTarget:[self window]];
    [saveButton setAction:@selector(showSaveMenu:)];
    [saveButton setFrame:NSMakeRect(162, 6, 32, 32)];
    [saveButton setAutoresizingMask:NSViewMaxXMargin | NSViewMinYMargin];
    [self addSubview:saveButton];
    
    BOOL hideOptions = [[NSUserDefaults standardUserDefaults] boolForKey:OEGameControlsBarHidesOptionButtonKey];
    if(!hideOptions)
    {
        OEButton *optionsButton = [[OEButton alloc] init];
        [optionsButton setThemeKey:@"hud_button_options"];
        [optionsButton setTarget:[self window]];
        [optionsButton setAction:@selector(showOptionsMenu:)];
        [optionsButton setFrame:NSMakeRect(212, 6, 32, 32)];
        [optionsButton setAutoresizingMask:NSViewMaxXMargin | NSViewMinYMargin];
        [self addSubview:optionsButton];
    }
    
    OEButton *volumeDownButton = [[OEButton alloc] initWithFrame:NSMakeRect(223 + (hideOptions ? 0 : 50), 17, 13, 14)];
    [volumeDownButton setThemeKey:@"hud_button_volume_down"];
    [volumeDownButton setAction:@selector(mute:)];
    [self addSubview:volumeDownButton];
    
    OEButton *volumeUpButton = [[OEButton alloc] initWithFrame:NSMakeRect(320 + (hideOptions? 0 : 50), 17, 15, 14)];
    [volumeUpButton setThemeKey:@"hud_button_volume_up"];
    [volumeUpButton setAction:@selector(unmute:)];
    [self addSubview:volumeUpButton];
    
    slider = [[OEHUDSlider alloc] initWithFrame:NSMakeRect(240 + (hideOptions ? 0 : 50), 13, 80, 23)];
    
    OEHUDSliderCell *sliderCell = [[OEHUDSliderCell alloc] init];
    [slider setCell:sliderCell];
    [slider setContinuous:YES];
    [slider setMaxValue:1.0];
    [slider setMinValue:0.0];
    [slider setFloatValue:[[NSUserDefaults standardUserDefaults] floatForKey:OEGameVolumeKey]];
    [slider setAction:@selector(changeVolume:)];
    
    CABasicAnimation *animation     = [CABasicAnimation animation];
    animation.timingFunction    = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseOut];
    animation.delegate          = self;

    [slider setAnimations:[NSDictionary dictionaryWithObject:animation forKey:@"floatValue"]];
    [self addSubview:slider];

    fullScreenButton = [[OEButton alloc] init];
    [fullScreenButton setTitle:nil];
    [fullScreenButton setThemeKey:@"hud_button_fullscreen"];
    [fullScreenButton setButtonType:NSPushOnPushOffButton];
    [fullScreenButton setAction:@selector(toggleFullScreen:)];
    [fullScreenButton setFrame:NSMakeRect(370 + (hideOptions ? 0 : 50), 13, 51, 23)];
    [fullScreenButton setAutoresizingMask:NSViewMaxXMargin | NSViewMinYMargin];
    [self addSubview:fullScreenButton];
}
@end
