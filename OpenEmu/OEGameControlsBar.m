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
#import "NSWindow+OEFullScreenAdditions.h"

#import "OEButton.h"
#import "OESlider.h"

#import "OEMenu.h"
#import "OEDBRom.h"

#import "OECompositionPlugin.h"
#import "OEShaderPlugin.h"
#import "OECorePlugin.h"
#import "OEGameDocument.h"
#import "OEGameViewController.h"

#import "OEHUDAlert.h"

#import "OEDBSaveState.h"

#import "OEGameIntegralScalingDelegate.h"
#import "OEAudioDeviceManager.h"

#import "OECheats.h"

#pragma mark - Public variables

NSString *const OEGameControlsBarCanDeleteSaveStatesKey = @"HUDBarCanDeleteState";
NSString *const OEGameControlsBarShowsAutoSaveStateKey  = @"HUDBarShowAutosaveState";
NSString *const OEGameControlsBarShowsQuickSaveStateKey = @"HUDBarShowQuicksaveState";
NSString *const OEGameControlsBarHidesOptionButtonKey   = @"HUDBarWithoutOptions";
NSString *const OEGameControlsBarFadeOutDelayKey        = @"fadeoutdelay";

@interface OEHUDControlsBarView : NSView

@property(strong, readonly) OESlider *slider;
@property(strong, readonly) OEButton *fullScreenButton;
@property(strong, readonly) OEButton *pauseButton;

- (void)setupControls;
@end

@interface OEGameControlsBar ()
{
    id              _eventMonitor;
    NSTimer        *_fadeTimer;
    NSArray        *_filterPlugins;
    NSUInteger      _openMenus;
    NSMutableArray *_cheats;
    BOOL            _cheatsLoaded;
}

@property(unsafe_unretained) OEGameViewController *gameViewController;
@property(strong) OEHUDControlsBarView *controlsView;
@property(strong, nonatomic) NSDate *lastMouseMovement;
@end

@implementation OEGameControlsBar

+ (void)initialize
{
    if(self != [OEGameControlsBar class])
        return;

    // Time until hud controls bar fades out
    [[NSUserDefaults standardUserDefaults] registerDefaults:@{
        OEGameControlsBarFadeOutDelayKey : @1.5,
        OEGameControlsBarShowsAutoSaveStateKey : @NO,
        OEGameControlsBarShowsQuickSaveStateKey : @YES,
     }];
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
        [self setCanShow:YES];
        [self setGameViewController:controller];

        OEHUDControlsBarView *barView = [[OEHUDControlsBarView alloc] initWithFrame:NSMakeRect(0, 0, 431 + (hideOptions ? 0 : 50), 45)];
        [[self contentView] addSubview:barView];
        [barView setupControls];

        _eventMonitor = [NSEvent addGlobalMonitorForEventsMatchingMask:NSMouseMovedMask handler:
                        ^(NSEvent *incomingEvent)
                        {
                            if([NSApp isActive] && [[self parentWindow] isMainWindow])
                                [self performSelectorOnMainThread:@selector(mouseMoved:) withObject:incomingEvent waitUntilDone:NO];
                        }];
        _openMenus = 0;
        _controlsView = barView;

        [NSCursor setHiddenUntilMouseMoves:YES];

        // Setup plugins menu
        NSMutableSet   *filterSet     = [NSMutableSet set];
        [filterSet addObjectsFromArray:[OECompositionPlugin allPluginNames]];
        [filterSet addObjectsFromArray:[OEShaderPlugin allPluginNames]];
        [filterSet filterUsingPredicate:[NSPredicate predicateWithFormat:@"NOT SELF beginswith '_'"]];
        _filterPlugins = [[filterSet allObjects] sortedArrayUsingSelector:@selector(caseInsensitiveCompare:)];
    }
    return self;
}

- (void)dealloc
{
    [_fadeTimer invalidate];
    _fadeTimer = nil;
    _gameViewController = nil;

    [NSEvent removeMonitor:_eventMonitor];
}

#pragma mark - Cheats

- (void)OE_loadCheats
{
    // In order to load cheats, we need the game core to be running and, consequently, the ROM to be set.
    // We use -reflectEmulationRunning:, which we receive from OEGameViewController when the emulation
    // starts or resumes
    if([[self gameViewController] supportsCheats])
    {
        NSString *md5Hash = [[[[self gameViewController] document] rom] md5Hash];
        if(md5Hash)
        {
            OECheats *cheatsXML = [[OECheats alloc] initWithMd5Hash:md5Hash];
            _cheats             = [[cheatsXML allCheats] mutableCopy];
            _cheatsLoaded       = YES;
        }
    }
}

#pragma mark -

- (void)show
{
    if([self canShow])
        [[self animator] setAlphaValue:1.0];
}

- (void)hide
{
    [[self animator] setAlphaValue:0.0];
    [_fadeTimer invalidate];
    _fadeTimer = nil;

    [NSCursor setHiddenUntilMouseMoves:YES];
}

- (void)mouseMoved:(NSEvent *)theEvent
{
    NSWindow *parentWindow = [self parentWindow];
    NSPoint mouseLoc = [NSEvent mouseLocation];

    if(!NSPointInRect(mouseLoc, [parentWindow convertRectToScreen:[[[self gameViewController] view] frame]])) return;

    if([self alphaValue] == 0.0)
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
        NSTimeInterval interval = [[NSUserDefaults standardUserDefaults] doubleForKey:OEGameControlsBarFadeOutDelayKey];
        _fadeTimer = [NSTimer scheduledTimerWithTimeInterval:interval target:self selector:@selector(timerDidFire:) userInfo:nil repeats:YES];
    }

    _lastMouseMovement = lastMouseMovementDate;
}

- (void)timerDidFire:(NSTimer *)timer
{
    NSTimeInterval interval = [[NSUserDefaults standardUserDefaults] doubleForKey:OEGameControlsBarFadeOutDelayKey];
    NSDate *hideDate = [_lastMouseMovement dateByAddingTimeInterval:interval];

    if([hideDate timeIntervalSinceNow] <= 0.0)
    {
        if([self canFadeOut])
        {
            [_fadeTimer invalidate];
            _fadeTimer = nil;

            [self hide];
        }
        else
        {
            NSTimeInterval interval = [[NSUserDefaults standardUserDefaults] doubleForKey:OEGameControlsBarFadeOutDelayKey];
            NSDate *nextTime = [NSDate dateWithTimeIntervalSinceNow:interval];

            [_fadeTimer setFireDate:nextTime];
        }
    }
    else [_fadeTimer setFireDate:hideDate];
}

- (NSRect)bounds
{
    NSRect bounds = [self frame];
    bounds.origin = NSMakePoint(0, 0);
    return bounds;
}

- (BOOL)canFadeOut
{
    return _openMenus == 0 && !NSPointInRect([self mouseLocationOutsideOfEventStream], [self bounds]);
}

#pragma mark - Menus

- (void)showOptionsMenu:(id)sender
{
    NSMenu *menu = [[NSMenu alloc] init];

    NSMenuItem *item;
    item = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Edit Game Controls", @"") action:@selector(editControls:) keyEquivalent:@""];
    [menu addItem:item];

    // Setup Cheats Menu
    if([[self gameViewController] supportsCheats])
    {
        NSMenu *cheatsMenu = [[NSMenu alloc] init];
        [cheatsMenu setTitle:NSLocalizedString(@"Select Cheat", @"")];
        item = [[NSMenuItem alloc] init];
        [item setTitle:NSLocalizedString(@"Select Cheat", @"")];
        [menu addItem:item];
        [item setSubmenu:cheatsMenu];

        NSMenuItem *addCheatMenuItem = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Add Cheat…", @"")
                                                                  action:@selector(addCheat:)
                                                           keyEquivalent:@""];
        [addCheatMenuItem setRepresentedObject:_cheats];
        [cheatsMenu addItem:addCheatMenuItem];

        if([_cheats count] != 0)
            [cheatsMenu addItem:[NSMenuItem separatorItem]];

        for(NSDictionary *cheatObject in _cheats)
        {
            NSString *description = [cheatObject objectForKey:@"description"];
            BOOL enabled          = [[cheatObject objectForKey:@"enabled"] boolValue];

            NSMenuItem *cheatsMenuItem = [[NSMenuItem alloc] initWithTitle:description action:@selector(setCheat:) keyEquivalent:@""];
            [cheatsMenuItem setRepresentedObject:cheatObject];
            [cheatsMenuItem setState:enabled ? NSOnState : NSOffState];

            [cheatsMenu addItem:cheatsMenuItem];
        }
    }

    // Setup Core selection menu
    NSMenu *coresMenu = [[NSMenu alloc] init];
    [coresMenu setTitle:NSLocalizedString(@"Select Core", @"")];

    NSString *systemIdentifier = [[self gameViewController] systemIdentifier];
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

            if([[aPlugin bundleIdentifier] isEqual:[[self gameViewController] coreIdentifier]]) [coreItem setState:NSOnState];

            [coresMenu addItem:coreItem];
        }

        item = [[NSMenuItem alloc] init];
        item.title = NSLocalizedString(@"Select Core", @"");
        [item setSubmenu:coresMenu];
        if([[coresMenu itemArray] count] > 1)
            [menu addItem:item];
    }

    // Setup Video Filter Menu
    NSMenu *filterMenu = [[NSMenu alloc] init];
    [filterMenu setTitle:NSLocalizedString(@"Select Filter", @"")];

    NSString *selectedFilter = ([[NSUserDefaults standardUserDefaults] objectForKey:[NSString stringWithFormat:OEGameSystemVideoFilterKeyFormat, systemIdentifier]]
                                ? : [[NSUserDefaults standardUserDefaults] objectForKey:OEGameDefaultVideoFilterKey]);

    for(NSString *aName in _filterPlugins)
    {
        NSMenuItem *filterItem = [[NSMenuItem alloc] initWithTitle:aName action:@selector(selectFilter:) keyEquivalent:@""];

        if([aName isEqualToString:selectedFilter]) [filterItem setState:NSOnState];

        [filterMenu addItem:filterItem];
    }

    item = [[NSMenuItem alloc] init];
    item.title = NSLocalizedString(@"Select Filter", @"");
    [menu addItem:item];
    [item setSubmenu:filterMenu];

    // Setup integral scaling
    id<OEGameIntegralScalingDelegate> integralScalingDelegate = [[self gameViewController] integralScalingDelegate];
    const BOOL hasSubmenu = [integralScalingDelegate shouldAllowIntegralScaling] && [integralScalingDelegate respondsToSelector:@selector(maximumIntegralScale)];

    NSMenu *scaleMenu = [NSMenu new];
    [scaleMenu setTitle:NSLocalizedString(@"Select Scale", @"")];
    item = [NSMenuItem new];
    [item setTitle:[scaleMenu title]];
    [menu addItem:item];
    [item setSubmenu:scaleMenu];

    if(hasSubmenu)
    {
        unsigned int maxScale = [integralScalingDelegate maximumIntegralScale];
        for(unsigned int scale = 1; scale <= maxScale; scale++)
        {
            NSString *scaleTitle  = [NSString stringWithFormat:NSLocalizedString(@"%ux", @"Integral scale menu item title"), scale];
            NSMenuItem *scaleItem = [[NSMenuItem alloc] initWithTitle:scaleTitle action:@selector(changeIntegralScale:) keyEquivalent:@""];
            [scaleItem setRepresentedObject:@(scale)];
            [scaleMenu addItem:scaleItem];
        }
    }
    else
        [item setEnabled:NO];
#if 0
    // Setup audio output
    NSMenu *audioOutputMenu = [NSMenu new];
    [audioOutputMenu setTitle:NSLocalizedString(@"Select Audio Output Device", @"")];
    item = [NSMenuItem new];
    [item setTitle:[audioOutputMenu title]];
    [menu addItem:item];
    [item setSubmenu:audioOutputMenu];

    NSPredicate *outputPredicate = [NSPredicate predicateWithBlock:^BOOL(OEAudioDevice *device, NSDictionary *bindings) {
        return [device numberOfOutputChannels] > 0;
    }];
    NSArray *audioOutputDevices = [[[OEAudioDeviceManager sharedAudioDeviceManager] audioDevices] filteredArrayUsingPredicate:outputPredicate];
    if([audioOutputDevices count] == 0)
        [item setEnabled:NO];
    else
        for(OEAudioDevice *device in audioOutputDevices)
        {
            NSMenuItem *deviceItem = [[NSMenuItem alloc] initWithTitle:[device deviceName] action:@selector(changeAudioOutputDevice:) keyEquivalent:@""];
            [deviceItem setRepresentedObject:device];
            [audioOutputMenu addItem:deviceItem];
        }
#endif
    // Create OEMenu and display it
    [menu setDelegate:self];

    NSRect targetRect = [sender bounds];
    targetRect.size.width -= 7.0;
    targetRect = NSInsetRect(targetRect, -2.0, 1.0);
    targetRect = [self convertRectToScreen:[sender convertRect:targetRect toView:nil]];

    NSDictionary *options = @{
        OEMenuOptionsStyleKey : @(OEMenuStyleLight),
        OEMenuOptionsArrowEdgeKey : @(OEMinYEdge),
        OEMenuOptionsMaximumSizeKey : [NSValue valueWithSize:NSMakeSize(500, 256)],
        OEMenuOptionsScreenRectKey : [NSValue valueWithRect:targetRect]
    };

    [OEMenu openMenu:menu withEvent:nil forView:sender options:options];
}

- (void)showSaveMenu:(id)sender
{
    NSMenu *menu = [[NSMenu alloc] init];

    NSMenuItem *newSaveItem = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Save Current Game", @"") action:@selector(saveState:) keyEquivalent:@""];
    [menu setDelegate:self];
    [menu addItem:newSaveItem];

    OEDBRom *rom = [[[self gameViewController] document] rom];
    [rom removeMissingStates];

    if(rom != nil)
    {
        BOOL includeAutoSaveState = [[NSUserDefaults standardUserDefaults] boolForKey:OEGameControlsBarShowsAutoSaveStateKey];
        BOOL includeQuickSaveState = [[NSUserDefaults standardUserDefaults] boolForKey:OEGameControlsBarShowsQuickSaveStateKey];
        BOOL useQuickSaveSlots = [[NSUserDefaults standardUserDefaults] boolForKey:OESaveStateUseQuickSaveSlotsKey];
        NSArray *saveStates = [rom normalSaveStatesByTimestampAscending:YES];

        if(includeQuickSaveState && !useQuickSaveSlots && [rom quickSaveStateInSlot:0] != nil)
            saveStates = [@[[rom quickSaveStateInSlot:0]] arrayByAddingObjectsFromArray:saveStates];

        if(includeAutoSaveState && [rom autosaveState] != nil)
            saveStates = [@[[rom autosaveState]] arrayByAddingObjectsFromArray:saveStates];

        if([saveStates count] != 0 || (includeQuickSaveState && useQuickSaveSlots))
        {
            [menu addItem:[NSMenuItem separatorItem]];

            // Build Quck Load item with submenu
            if(includeQuickSaveState && useQuickSaveSlots)
            {
                NSString *loadTitle   = NSLocalizedString(@"Quick Load", @"Quick load menu title");
                //NSString *saveTitle   = NSLocalizedString(@"Quick Save", @"Quick save menu title");

                NSMenuItem *loadItem  = [[NSMenuItem alloc] initWithTitle:loadTitle action:NULL keyEquivalent:@""];
                //NSMenuItem *saveItem  = [[NSMenuItem alloc] initWithTitle:saveTitle action:NULL keyEquivalent:@""];
                //[saveItem setKeyEquivalentModifierMask:NSAlternateKeyMask];
                //[saveItem setAlternate:YES];

                NSMenu *loadSubmenu = [[NSMenu alloc] initWithTitle:loadTitle];
                //NSMenu *saveSubmenu = [[NSMenu alloc] initWithTitle:saveTitle];

                for(NSInteger i = 1; i <= 9; i++)
                {
                    OEDBSaveState *state = [rom quickSaveStateInSlot:i];

                    loadTitle = [NSString stringWithFormat:NSLocalizedString(@"Slot %d", @"Quick load menu item title"), i];
                    NSMenuItem *loadItem = [[NSMenuItem alloc] initWithTitle:loadTitle action:@selector(quickLoad:) keyEquivalent:@""];
                    [loadItem setEnabled:state != nil];
                    [loadItem setRepresentedObject:@(i)];
                    [loadSubmenu addItem:loadItem];

                    //saveTitle  = [NSString stringWithFormat:NSLocalizedString(@"Save to Slot %d", @"Quick save menu item title"), i];
                    //NSMenuItem *saveItem = [[NSMenuItem alloc] initWithTitle:saveTitle action:@selector(quickSave:) keyEquivalent:@""];
                    //[saveItem setRepresentedObject:@(i)];
                    //[saveSubmenu addItem:saveItem];
                }

                [loadItem setSubmenu:loadSubmenu];
                [menu addItem:loadItem];

                //[saveItem setSubmenu:saveSubmenu];
                //[menu addItem:saveItem];
            }

            // Add 'normal' save states
            for(OEDBSaveState *saveState in saveStates)
            {
                NSString *itemTitle = [saveState displayName];

                if(!itemTitle || [itemTitle isEqualToString:@""])
                    itemTitle = [NSString stringWithFormat:@"%@", [saveState timestamp]];

                NSMenuItem *item = [[NSMenuItem alloc] initWithTitle:itemTitle action:@selector(loadState:) keyEquivalent:@""];
                [item setRepresentedObject:saveState];
                [menu addItem:item];

                if([[NSUserDefaults standardUserDefaults] boolForKey:OEGameControlsBarCanDeleteSaveStatesKey])
                {
                    NSMenuItem *deleteStateItem = [[NSMenuItem alloc] initWithTitle:itemTitle action:@selector(deleteSaveState:) keyEquivalent:@""];
                    [deleteStateItem setAlternate:YES];
                    [deleteStateItem setKeyEquivalentModifierMask:NSAlternateKeyMask];
                    [deleteStateItem setRepresentedObject:saveState];
                    [menu addItem:deleteStateItem];
                }
            }
        }
    }

    NSRect targetRect = [sender bounds];
    targetRect.size.width -= 7.0;
    targetRect = NSInsetRect(targetRect, -2.0, 1.0);
    targetRect = [self convertRectToScreen:[sender convertRect:targetRect toView:nil]];

    NSDictionary *options = @{
        OEMenuOptionsStyleKey : @(OEMenuStyleLight),
        OEMenuOptionsArrowEdgeKey : @(OEMinYEdge),
        OEMenuOptionsMaximumSizeKey : [NSValue valueWithSize:NSMakeSize(500, 256)],
        OEMenuOptionsScreenRectKey : [NSValue valueWithRect:targetRect]
    };

    [OEMenu openMenu:menu withEvent:nil forView:sender options:options];
}

#pragma mark - OEMenuDelegate Implementation

- (void)menuWillOpen:(NSMenu *)menu
{
    _openMenus++;
}

- (void)menuDidClose:(NSMenu *)menu
{
    _openMenus--;
}

- (void)setVolume:(CGFloat)value
{
    _volume = value;
    [self reflectVolume:value];
}

#pragma mark - Updating UI States

- (void)reflectVolume:(CGFloat)volume
{
    OEHUDControlsBarView *view   = [[[self contentView] subviews] lastObject];
    OESlider             *slider = [view slider];

    [[slider animator] setDoubleValue:volume];
}

- (void)reflectEmulationRunning:(BOOL)isEmulationRunning
{
    OEHUDControlsBarView *view        = [[[self contentView] subviews] lastObject];
    NSButton             *pauseButton = [view pauseButton];
    [pauseButton setState:!isEmulationRunning];

    if(isEmulationRunning && !_cheatsLoaded)
        [self OE_loadCheats];
}

- (void)parentWindowDidEnterFullScreen:(NSNotification *)notification;
{
    OEHUDControlsBarView *view = [[[self contentView] subviews] lastObject];
    [[view fullScreenButton] setState:NSOnState];
}

- (void)parentWindowWillExitFullScreen:(NSNotification *)notification;
{
    OEHUDControlsBarView *view = [[[self contentView] subviews] lastObject];
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

        OEHUDControlsBarView *view = [[[self contentView] subviews] lastObject];
        [[view fullScreenButton] setState:[window isFullScreen] ? NSOnState : NSOffState];
    }
}

@end

@implementation OEHUDControlsBarView

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
    [stopButton setAction:@selector(performClose:)];
    [stopButton setFrame:NSMakeRect(10, 13, 51, 23)];
    [stopButton setAutoresizingMask:NSViewMaxXMargin | NSViewMinYMargin];
    [stopButton setToolTip:NSLocalizedString(@"Stop Emulation", @"Tooltip")];
    [stopButton setToolTipStyle:OEToolTipStyleHUD];
    [self addSubview:stopButton];

    _pauseButton = [[OEButton alloc] init];
    [_pauseButton setButtonType:NSToggleButton];
    [_pauseButton setThemeKey:@"hud_button_toggle_pause"];
    [_pauseButton setTitle:nil];
    [_pauseButton setAction:@selector(toggleEmulationPaused:)];
    [_pauseButton setFrame:NSMakeRect(82, 9, 32, 32)];
    [_pauseButton setAutoresizingMask:NSViewMaxXMargin | NSViewMinYMargin];
    [_pauseButton setToolTip:NSLocalizedString(@"Pause Gameplay", @"Tooltip")];
    [_pauseButton setToolTipStyle:OEToolTipStyleHUD];
    [self addSubview:_pauseButton];

    OEButton *restartButton = [[OEButton alloc] init];
    [restartButton setThemeKey:@"hud_button_restart"];
    [restartButton setTitle:nil];
    [restartButton setAction:@selector(resetEmulation:)];
    [restartButton setFrame:NSMakeRect(111, 9, 32, 32)];
    [restartButton setAutoresizingMask:NSViewMaxXMargin | NSViewMinYMargin];
    [restartButton setToolTip:NSLocalizedString(@"Restart System", @"Tooltip")];
    [restartButton setToolTipStyle:OEToolTipStyleHUD];
    [self addSubview:restartButton];

    OEButton *saveButton = [[OEButton alloc] init];
    [saveButton setThemeKey:@"hud_button_save"];
    [saveButton setTitle:nil];
    [saveButton setTarget:[self window]];
    [saveButton setAction:@selector(showSaveMenu:)];
    [saveButton setFrame:NSMakeRect(162, 6, 32, 32)];
    [saveButton setAutoresizingMask:NSViewMaxXMargin | NSViewMinYMargin];
    [saveButton setToolTip:NSLocalizedString(@"Create Save State", @"Tooltip")];
    [saveButton setToolTipStyle:OEToolTipStyleHUD];
    [self addSubview:saveButton];

    BOOL hideOptions = [[NSUserDefaults standardUserDefaults] boolForKey:OEGameControlsBarHidesOptionButtonKey];
    if(!hideOptions)
    {
        OEButton *optionsButton = [[OEButton alloc] init];
        [optionsButton setThemeKey:@"hud_button_options"];
        [optionsButton setTitle:nil];
        [optionsButton setTarget:[self window]];
        [optionsButton setAction:@selector(showOptionsMenu:)];
        [optionsButton setFrame:NSMakeRect(212, 6, 32, 32)];
        [optionsButton setAutoresizingMask:NSViewMaxXMargin | NSViewMinYMargin];
        [optionsButton setToolTip:NSLocalizedString(@"Options", @"Tooltip")];
        [optionsButton setToolTipStyle:OEToolTipStyleHUD];
        [self addSubview:optionsButton];
    }

    OEButton *volumeDownButton = [[OEButton alloc] initWithFrame:NSMakeRect(223 + (hideOptions ? 0 : 50), 17, 13, 14)];
    [volumeDownButton setTitle:nil];
    [volumeDownButton setThemeKey:@"hud_button_volume_down"];
    [volumeDownButton setAction:@selector(mute:)];
    [volumeDownButton setToolTip:NSLocalizedString(@"Mute Audio", @"Tooltip")];
    [volumeDownButton setToolTipStyle:OEToolTipStyleHUD];
    [self addSubview:volumeDownButton];

    OEButton *volumeUpButton = [[OEButton alloc] initWithFrame:NSMakeRect(320 + (hideOptions? 0 : 50), 17, 15, 14)];
    [volumeUpButton setTitle:nil];
    [volumeUpButton setThemeKey:@"hud_button_volume_up"];
    [volumeUpButton setAction:@selector(unmute:)];
    [volumeUpButton setToolTip:NSLocalizedString(@"Unmute Audio", @"Tooltip")];
    [volumeUpButton setToolTipStyle:OEToolTipStyleHUD];
    [self addSubview:volumeUpButton];

    _slider = [[OESlider alloc] initWithFrame:NSMakeRect(240 + (hideOptions ? 0 : 50), 13, 80, 23)];

    OESliderCell *sliderCell = [[OESliderCell alloc] init];
    [_slider setCell:sliderCell];
    [_slider setContinuous:YES];
    [_slider setMaxValue:1.0];
    [_slider setMinValue:0.0];
    [_slider setThemeKey:@"hud_slider"];
    [_slider setFloatValue:[[NSUserDefaults standardUserDefaults] floatForKey:OEGameVolumeKey]];
    [_slider setToolTip:NSLocalizedString(@"Change Volume", @"Tooltip")];
    [_slider setToolTipStyle:OEToolTipStyleHUD];
    [_slider setAction:@selector(changeVolume:)];

    CABasicAnimation *animation = [CABasicAnimation animation];
    animation.timingFunction    = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseOut];
    animation.delegate          = self;

    [_slider setAnimations:[NSDictionary dictionaryWithObject:animation forKey:@"floatValue"]];
    [self addSubview:_slider];

    _fullScreenButton = [[OEButton alloc] init];
    [_fullScreenButton setTitle:nil];
    [_fullScreenButton setThemeKey:@"hud_button_fullscreen"];
    [_fullScreenButton setButtonType:NSPushOnPushOffButton];
    [_fullScreenButton setAction:@selector(toggleFullScreen:)];
    [_fullScreenButton setFrame:NSMakeRect(370 + (hideOptions ? 0 : 50), 13, 51, 23)];
    [_fullScreenButton setAutoresizingMask:NSViewMaxXMargin | NSViewMinYMargin];
    [_fullScreenButton setToolTip:NSLocalizedString(@"Toggle Fullscreen", @"Tooltip")];
    [_fullScreenButton setToolTipStyle:OEToolTipStyleHUD];
    [self addSubview:_fullScreenButton];
}

@end
