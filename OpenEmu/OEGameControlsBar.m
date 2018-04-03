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

#import "OEButton.h"
#import "OESlider.h"
#import "OESliderCell.h"

#import "OEMenu.h"
#import "OEDBRom.h"

#import "OEShaderPlugin.h"
#import "OECorePlugin.h"
#import "OEGameDocument.h"
#import "OEGameViewController.h"
#import "OEPopoutGameWindowController.h"

#import "OEHUDAlert.h"

#import "OEDBSaveState.h"

#import "OEAudioDeviceManager.h"

#import "OETheme.h"

@import QuartzCore;

#import "OpenEmu-Swift.h"

#pragma mark - Public variables

NSString *const OEGameControlsBarCanDeleteSaveStatesKey = @"HUDBarCanDeleteState";
NSString *const OEGameControlsBarShowsAutoSaveStateKey  = @"HUDBarShowAutosaveState";
NSString *const OEGameControlsBarShowsQuickSaveStateKey = @"HUDBarShowQuicksaveState";
NSString *const OEGameControlsBarHidesOptionButtonKey   = @"HUDBarWithoutOptions";
NSString *const OEGameControlsBarFadeOutDelayKey        = @"fadeoutdelay";
NSString *const OEGameControlsBarShowsAudioOutput       = @"HUDBarShowAudioOutput";

@interface OEHUDControlsBarView : NSView <CAAnimationDelegate>

@property(strong, readonly) OESlider *slider;
@property(strong, readonly) OEButton *fullScreenButton;
@property(strong, readonly) OEButton *pauseButton;

- (void)setupControls;
@end

@interface OEGameControlsBar () <CAAnimationDelegate>
@property (strong) id eventMonitor;
@property (strong) NSTimer *fadeTimer;
@property (strong) NSArray *filterPlugins;
@property (strong) NSMutableArray *cheats;
@property          NSMutableSet *openMenus;
@property          BOOL cheatsLoaded;

@property (unsafe_unretained) OEGameViewController *gameViewController;
@property (strong) OEHUDControlsBarView *controlsView;
@property (strong, nonatomic) NSDate *lastMouseMovement;
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
        OEGameControlsBarShowsQuickSaveStateKey : @NO,
        OEGameControlsBarShowsAudioOutput : @NO,
     }];
}

- (id)initWithGameViewController:(OEGameViewController *)controller
{
    BOOL hideOptions = [[NSUserDefaults standardUserDefaults] boolForKey:OEGameControlsBarHidesOptionButtonKey];

    self = [super initWithContentRect:NSMakeRect(0, 0, 431 + (hideOptions ? 0 : 50), 45) styleMask:NSBorderlessWindowMask backing:NSBackingStoreBuffered defer:YES];
    if(self != nil)
    {
        [self setMovableByWindowBackground:YES];
        [self setOpaque:NO];
        [self setBackgroundColor:[NSColor clearColor]];
        [self setAlphaValue:0.0];
        [self setCanShow:YES];
        [self setGameViewController:controller];
        [self setAnimationBehavior:NSWindowAnimationBehaviorNone];

        OEHUDControlsBarView *barView = [[OEHUDControlsBarView alloc] initWithFrame:NSMakeRect(0, 0, 431 + (hideOptions ? 0 : 50), 45)];
        [[self contentView] addSubview:barView];
        [barView setupControls];
        
        _eventMonitor = [NSEvent addLocalMonitorForEventsMatchingMask:NSMouseMovedMask handler:^NSEvent*(NSEvent* e)
                         {
                             if([NSApp isActive] && [[self gameWindow] isMainWindow])
                                 [self performSelectorOnMainThread:@selector(mouseMoved:) withObject:e waitUntilDone:NO];
                             return e;
                         }];
        _openMenus = [NSMutableSet set];
        _controlsView = barView;

        [NSCursor setHiddenUntilMouseMoves:YES];

        NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
        // Show HUD when switching back from other applications
        [nc addObserver:self selector:@selector(mouseMoved:) name:NSApplicationDidBecomeActiveNotification object:nil];
        [nc addObserver:self selector:@selector(didMove:) name:NSWindowDidMoveNotification object:self];

        // Setup plugins menu
        NSMutableSet   *filterSet     = [NSMutableSet set];
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

    _gameWindow = nil;
}

- (NSRect)bounds
{
    NSRect bounds = [self frame];
    bounds.origin = NSMakePoint(0, 0);
    return bounds;
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

#pragma mark - Manage Visibility
- (void)show
{
    if([self canShow])
        [[self animator] setAlphaValue:1.0];
}

- (void)hideAnimated:(BOOL)animated
{
    [NSCursor setHiddenUntilMouseMoves:YES];

    // only hide if 'docked' to game window (aka on the same screen)
    if([self parentWindow])
    {
        if(animated)
            [[self animator] setAlphaValue:0.0];
        else
            [self setAlphaValue:0];
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
    NSWindow *gameWindow = [self gameWindow];
    if(gameWindow == nil) return;

    NSView *gameView = [[self gameViewController] view];
    NSRect viewFrame = [gameView frame];
    NSPoint mouseLoc = [NSEvent mouseLocation];

    NSRect viewFrameOnScreen = [gameWindow convertRectToScreen:viewFrame];
    if(!NSPointInRect(mouseLoc, viewFrameOnScreen)) return;

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

            [self hideAnimated:YES];
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

- (BOOL)canFadeOut
{
    return [_openMenus count]==0 && !NSPointInRect([self mouseLocationOutsideOfEventStream], [self bounds]);
}

- (void)repositionOnGameWindow
{
    if(!_gameWindow || ![self parentWindow]) return;

    static const CGFloat _OEControlsMargin = 19;

    NSView *gameView = [[self gameViewController] view];
    NSRect gameViewFrame = [gameView frame];
    NSRect gameViewFrameInWindow = [gameView convertRect:gameViewFrame toView:nil];
    NSPoint origin = [_gameWindow convertRectToScreen:gameViewFrameInWindow].origin;

    origin.x += (NSWidth(gameViewFrame) - NSWidth([self frame])) / 2;

    // If the controls bar fits, it sits over the window
    if(NSWidth(gameViewFrame) >= NSWidth([self frame]))
        origin.y += _OEControlsMargin;
    else
    {
        // Otherwise, it sits below the window
        origin.y -= (NSHeight([self frame]) + _OEControlsMargin);

        // Unless below the window means it being off-screen, in which case it sits above the window
        if(origin.y < NSMinY([[_gameWindow screen] visibleFrame]))
            origin.y = NSMaxY([_gameWindow frame]) + _OEControlsMargin;
    }

    [self setFrameOrigin:origin];
}
#pragma mark -
- (void)gameWindowDidChangeScreen:(NSNotification*)notification
{
    [self adjustWindowAttachment:YES];
}

- (void)didMove:(NSNotification*)notification
{
    [self adjustWindowAttachment:NO];
}

- (void)adjustWindowAttachment:(BOOL)userMovesGameWindow;
{
    NSWindow *gameWindow = [self gameWindow];
    NSScreen *barScreen  = [self screen];
    NSScreen *gameScreen = [gameWindow screen];

    BOOL screensDiffer = barScreen != gameScreen;

    if(!userMovesGameWindow && screensDiffer && [self parentWindow] != nil && barScreen != nil)
    {
        NSRect f = [self frame];
        [self orderOut:nil];
        [self setFrame:NSZeroRect display:NO];
        [self setFrame:f display:NO];
        [self orderFront:self];
    }
    else if(!screensDiffer && [self parentWindow] == nil)
    {
        // attach to window and center the controls bar
        [[self gameWindow] addChildWindow:self ordered:NSWindowAbove];
        [self repositionOnGameWindow];
    }
}

- (void)mouseUp:(NSEvent *)theEvent
{
    [super mouseUp:theEvent];
    [self adjustWindowAttachment:NO];
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
            return [[obj1 displayName] localizedCaseInsensitiveCompare:[obj2 displayName]];
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

    // Setup Disc selection Menu
    if([[self gameViewController] supportsMultipleDiscs])
    {
        NSUInteger maxDiscs = [[self gameViewController] discCount];

        NSMenu *discsMenu = [[NSMenu alloc] init];
        [discsMenu setTitle:NSLocalizedString(@"Select Disc", @"")];
        item = [[NSMenuItem alloc] init];
        [item setTitle:NSLocalizedString(@"Select Disc", @"")];
        [menu addItem:item];
        [item setSubmenu:discsMenu];
        [item setEnabled:maxDiscs > 1 ? YES : NO];

        for(unsigned int disc = 1; disc <= maxDiscs; disc++)
        {
            NSString *discTitle  = [NSString stringWithFormat:NSLocalizedString(@"Disc %u", @"Disc selection menu item title"), disc];
            NSMenuItem *discsMenuItem = [[NSMenuItem alloc] initWithTitle:discTitle action:@selector(setDisc:) keyEquivalent:@""];
            [discsMenuItem setRepresentedObject:@(disc)];

            [discsMenu addItem:discsMenuItem];
        }
    }

    // Setup Video Filter Menu
    NSMenu *filterMenu = [[NSMenu alloc] init];
    [filterMenu setTitle:NSLocalizedString(@"Select Filter", @"")];

    NSString *selectedFilter = ([[NSUserDefaults standardUserDefaults] objectForKey:[NSString stringWithFormat:OEGameSystemVideoFilterKeyFormat, systemIdentifier]]
                                ? : [[NSUserDefaults standardUserDefaults] objectForKey:OEGameDefaultVideoFilterKey]);

    // Select the Default Filter if the current is not available (ie. deleted)
    if(![_filterPlugins containsObject:selectedFilter])
        selectedFilter = [[NSUserDefaults standardUserDefaults] objectForKey:OEGameDefaultVideoFilterKey];

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
        unsigned int currentScale = [integralScalingDelegate currentIntegralScale];

        for(unsigned int scale = 1; scale <= maxScale; scale++)
        {
            NSString *scaleTitle  = [NSString stringWithFormat:NSLocalizedString(@"%ux", @"Integral scale menu item title"), scale];
            NSMenuItem *scaleItem = [[NSMenuItem alloc] initWithTitle:scaleTitle action:@selector(changeIntegralScale:) keyEquivalent:@""];
            [scaleItem setRepresentedObject:@(scale)];
            [scaleItem setState:(scale == currentScale ? NSOnState : NSOffState)];
            [scaleMenu addItem:scaleItem];
        }
    }
    else
        [item setEnabled:NO];

    if([[NSUserDefaults standardUserDefaults] boolForKey:OEGameControlsBarShowsAudioOutput])
    {
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
    }

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

    NSMenuItem *newSaveItem = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Save Current Game…", @"") action:@selector(saveState:) keyEquivalent:@""];
    [newSaveItem setEnabled:[[self gameViewController] supportsSaveStates]];
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
        BOOL canDeleteSaveStates = [[NSUserDefaults standardUserDefaults] boolForKey:OEGameControlsBarCanDeleteSaveStatesKey];

        if(includeQuickSaveState && !useQuickSaveSlots && [rom quickSaveStateInSlot:0] != nil)
            saveStates = [@[[rom quickSaveStateInSlot:0]] arrayByAddingObjectsFromArray:saveStates];

        if(includeAutoSaveState && [rom autosaveState] != nil)
            saveStates = [@[[rom autosaveState]] arrayByAddingObjectsFromArray:saveStates];

        if([saveStates count] != 0 || (includeQuickSaveState && useQuickSaveSlots))
        {
            [menu addItem:[NSMenuItem separatorItem]];
            
            NSMenuItem *item = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Load", @"") action:NULL keyEquivalent:@""];
            [item setEnabled:NO];
            [menu addItem:item];

            if(canDeleteSaveStates)
            {
                item = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Delete", @"") action:NULL keyEquivalent:@""];
                [item setAlternate:YES];
                [item setEnabled:NO];
                [item setKeyEquivalentModifierMask:NSAlternateKeyMask];
                [menu addItem:item];
            }

            // Build Quck Load item with submenu
            if(includeQuickSaveState && useQuickSaveSlots)
            {
                NSString *loadTitle   = NSLocalizedString(@"Quick Load", @"Quick load menu title");
                NSMenuItem *loadItem  = [[NSMenuItem alloc] initWithTitle:loadTitle action:NULL keyEquivalent:@""];

                NSMenu *loadSubmenu = [[NSMenu alloc] initWithTitle:loadTitle];
                [loadItem setIndentationLevel:1];

                for(NSInteger i = 1; i <= 9; i++)
                {
                    OEDBSaveState *state = [rom quickSaveStateInSlot:i];

                    loadTitle = [NSString stringWithFormat:NSLocalizedString(@"Slot %d", @"Quick load menu item title"), i];
                    NSMenuItem *loadItem = [[NSMenuItem alloc] initWithTitle:loadTitle action:@selector(quickLoad:) keyEquivalent:@""];
                    [loadItem setEnabled:state != nil];
                    [loadItem setRepresentedObject:@(i)];
                    [loadSubmenu addItem:loadItem];
                }

                [loadItem setSubmenu:loadSubmenu];
                [menu addItem:loadItem];
            }

            // Add 'normal' save states
            for(OEDBSaveState *saveState in saveStates)
            {
                NSString *itemTitle = [saveState displayName];

                if(!itemTitle || [itemTitle isEqualToString:@""])
                    itemTitle = [NSString stringWithFormat:@"%@", [saveState timestamp]];

                NSMenuItem *item = [[NSMenuItem alloc] initWithTitle:itemTitle action:@selector(loadState:) keyEquivalent:@""];
                [item setIndentationLevel:1];

                [item setRepresentedObject:saveState];
                [menu addItem:item];

                if(canDeleteSaveStates)
                {
                    NSMenuItem *deleteStateItem = [[NSMenuItem alloc] initWithTitle:itemTitle action:@selector(deleteSaveState:) keyEquivalent:@""];
                    [deleteStateItem setAlternate:YES];
                    [deleteStateItem setKeyEquivalentModifierMask:NSAlternateKeyMask];
                    [deleteStateItem setRepresentedObject:saveState];
                    [deleteStateItem setIndentationLevel:1];

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
    [_openMenus addObject:menu];
}

- (void)menuDidClose:(NSMenu *)menu
{
    [_openMenus removeObject:menu];
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

- (void)gameWindowDidEnterFullScreen:(NSNotification *)notification;
{
    OEHUDControlsBarView *view = [[[self contentView] subviews] lastObject];
    [[view fullScreenButton] setState:NSOnState];
    [self _performMouseMoved:nil];  // Show HUD because fullscreen animation makes the cursor appear
}

- (void)gameWindowWillExitFullScreen:(NSNotification *)notification;
{
    OEHUDControlsBarView *view = [[[self contentView] subviews] lastObject];
    [[view fullScreenButton] setState:NSOffState];
}

- (void)setGameWindow:(NSWindow *)gameWindow
{
    NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];

    // un-register notifications for parent window
    if([self parentWindow] != nil)
    {
        [nc removeObserver:self name:NSWindowDidEnterFullScreenNotification object:_gameWindow];
        [nc removeObserver:self name:NSWindowWillExitFullScreenNotification object:_gameWindow];
        [nc removeObserver:self name:NSWindowDidChangeScreenNotification    object:_gameWindow];
    }

    // remove from parent window if there was one, and attach to to the new game window
    if((!_gameWindow || [self parentWindow]) && gameWindow != [self parentWindow])
    {
        [[self parentWindow] removeChildWindow:self];
        [gameWindow addChildWindow:self ordered:NSWindowAbove];
    }

    // if there is no new window we should close all menus
    if(gameWindow == nil)
    {
        id openOEMenus = [_openMenus valueForKey:@"oeMenu"];
        [openOEMenus makeObjectsPerformSelector:@selector(cancelTrackingWithoutAnimation)];
    }

    _gameWindow = gameWindow;

    // register notifications and update state of the fullscreen button
    if(gameWindow != nil)
    {
        [nc addObserver:self selector:@selector(gameWindowDidEnterFullScreen:) name:NSWindowDidEnterFullScreenNotification object:gameWindow];
        [nc addObserver:self selector:@selector(gameWindowWillExitFullScreen:) name:NSWindowWillExitFullScreenNotification object:gameWindow];
        [nc addObserver:self selector:@selector(gameWindowDidChangeScreen:) name:NSWindowDidChangeScreenNotification object:gameWindow];

        OEHUDControlsBarView *view = [[[self contentView] subviews] lastObject];
        [[view fullScreenButton] setState:[gameWindow isFullScreen] ? NSOnState : NSOffState];
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

- (void)stopEmulation:(id)sender
{
    [[[self window] parentWindow] performClose:self];
}

#pragma mark -
- (void)drawRect:(NSRect)dirtyRect
{
    NSImage *barBackground = [[OETheme sharedTheme] imageForKey:@"hud_bar" forState:OEThemeStateDefault];
    [barBackground drawInRect:[self bounds] fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil];
}

- (void)setupControls
{
    OEButton *stopButton = [[OEButton alloc] init];
    [stopButton setThemeKey:@"hud_button_power"];
    [stopButton setTitle:@""];
    [stopButton setTarget:self];
    [stopButton setAction:@selector(stopEmulation:)];
    [stopButton setFrame:NSMakeRect(10, 13, 51, 23)];
    [stopButton setAutoresizingMask:NSViewMaxXMargin | NSViewMinYMargin];
    [stopButton setToolTip:NSLocalizedString(@"Stop Emulation", @"Tooltip")];
    [stopButton setToolTipStyle:OEToolTipStyleHUD];
    [self addSubview:stopButton];

    _pauseButton = [[OEButton alloc] init];
    [_pauseButton setButtonType:NSToggleButton];
    [_pauseButton setThemeKey:@"hud_button_toggle_pause"];
    [_pauseButton setTitle:@""];
    [_pauseButton setAction:@selector(toggleEmulationPaused:)];
    [_pauseButton setFrame:NSMakeRect(82, 9, 32, 32)];
    [_pauseButton setAutoresizingMask:NSViewMaxXMargin | NSViewMinYMargin];
    [_pauseButton setToolTip:NSLocalizedString(@"Pause Gameplay", @"Tooltip")];
    [_pauseButton setToolTipStyle:OEToolTipStyleHUD];
    [self addSubview:_pauseButton];

    OEButton *restartButton = [[OEButton alloc] init];
    [restartButton setThemeKey:@"hud_button_restart"];
    [restartButton setTitle:@""];
    [restartButton setAction:@selector(resetEmulation:)];
    [restartButton setFrame:NSMakeRect(111, 9, 32, 32)];
    [restartButton setAutoresizingMask:NSViewMaxXMargin | NSViewMinYMargin];
    [restartButton setToolTip:NSLocalizedString(@"Restart System", @"Tooltip")];
    [restartButton setToolTipStyle:OEToolTipStyleHUD];
    [self addSubview:restartButton];

    OEButton *saveButton = [[OEButton alloc] init];
    [saveButton setThemeKey:@"hud_button_save"];
    [saveButton setTitle:@""];
    [saveButton setTarget:[self window]];
    [saveButton setAction:@selector(showSaveMenu:)];
    [saveButton setFrame:NSMakeRect(162, 6, 32, 32)];
    [saveButton setAutoresizingMask:NSViewMaxXMargin | NSViewMinYMargin];
    [saveButton setToolTip:NSLocalizedString(@"Create or Load Save State", @"Tooltip")];
    [saveButton setToolTipStyle:OEToolTipStyleHUD];
    [self addSubview:saveButton];

    BOOL hideOptions = [[NSUserDefaults standardUserDefaults] boolForKey:OEGameControlsBarHidesOptionButtonKey];
    if(!hideOptions)
    {
        OEButton *optionsButton = [[OEButton alloc] init];
        [optionsButton setThemeKey:@"hud_button_options"];
        [optionsButton setTitle:@""];
        [optionsButton setTarget:[self window]];
        [optionsButton setAction:@selector(showOptionsMenu:)];
        [optionsButton setFrame:NSMakeRect(212, 6, 32, 32)];
        [optionsButton setAutoresizingMask:NSViewMaxXMargin | NSViewMinYMargin];
        [optionsButton setToolTip:NSLocalizedString(@"Options", @"Tooltip")];
        [optionsButton setToolTipStyle:OEToolTipStyleHUD];
        [self addSubview:optionsButton];
    }

    OEButton *volumeDownButton = [[OEButton alloc] initWithFrame:NSMakeRect(223 + (hideOptions ? 0 : 50), 17, 13, 14)];
    [volumeDownButton setTitle:@""];
    [volumeDownButton setThemeKey:@"hud_button_volume_down"];
    [volumeDownButton setAction:@selector(mute:)];
    [volumeDownButton setToolTip:NSLocalizedString(@"Mute Audio", @"Tooltip")];
    [volumeDownButton setToolTipStyle:OEToolTipStyleHUD];
    [self addSubview:volumeDownButton];

    OEButton *volumeUpButton = [[OEButton alloc] initWithFrame:NSMakeRect(320 + (hideOptions? 0 : 50), 17, 15, 14)];
    [volumeUpButton setTitle:@""];
    [volumeUpButton setThemeKey:@"hud_button_volume_up"];
    [volumeUpButton setAction:@selector(unmute:)];
    [volumeUpButton setToolTip:NSLocalizedString(@"Unmute Audio", @"Tooltip")];
    [volumeUpButton setToolTipStyle:OEToolTipStyleHUD];
    [self addSubview:volumeUpButton];

    _slider = [[OESlider alloc] initWithFrame:NSMakeRect(238 + (hideOptions ? 0 : 50), 13, 76, 23)];

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
    [_fullScreenButton setTitle:@""];
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
