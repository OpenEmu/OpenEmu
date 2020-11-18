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
#import "OEGameViewController.h"
#import "OEPopoutGameWindowController.h"

#import "OEDBSaveState.h"
#import "OEHUDBar.h"

#import "OEAudioDeviceManager.h"

@import QuartzCore;

#import "OpenEmu-Swift.h"

#pragma mark - Public variables

NSString *const OEGameControlsBarCanDeleteSaveStatesKey = @"HUDBarCanDeleteState";
NSString *const OEGameControlsBarShowsAutoSaveStateKey  = @"HUDBarShowAutosaveState";
NSString *const OEGameControlsBarShowsQuickSaveStateKey = @"HUDBarShowQuicksaveState";
NSString *const OEGameControlsBarHidesOptionButtonKey   = @"HUDBarWithoutOptions";
NSString *const OEGameControlsBarFadeOutDelayKey        = @"fadeoutdelay";
NSString *const OEGameControlsBarShowsAudioOutput       = @"HUDBarShowAudioOutput";

@interface OEGameControlsBar () <CAAnimationDelegate>
@property (strong) id eventMonitor;
@property (strong) NSTimer *fadeTimer;
@property (strong) NSMutableArray *cheats;
@property          NSMutableSet *openMenus;
@property          BOOL cheatsLoaded;

@property (unsafe_unretained) OEGameViewController *gameViewController;
@property (strong) NSView<OEHUDBarView> *controlsView;
@property (strong, nonatomic) NSDate *lastMouseMovement;
@end

@implementation OEGameControlsBar
{
    NSArray<NSString *> *_sortedSystemShaders;
    NSArray<NSString *> *_sortedCustomShaders;
    NSRect _lastGameWindowFrame;
}

- (BOOL)canBecomeKeyWindow {
    return NO;
}

- (BOOL)canBecomeMainWindow {
    return NO;
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
    BOOL hideOptions = [NSUserDefaults.standardUserDefaults boolForKey:OEGameControlsBarHidesOptionButtonKey];
    BOOL useNew = [NSUserDefaults.standardUserDefaults integerForKey:OEHUDBarAppearancePreferenceKey] == OEHUDBarAppearancePreferenceValueVibrant;
    
    if(useNew)
        self = [super initWithContentRect:NSMakeRect(0, 0, 442, 42) styleMask:NSWindowStyleMaskTitled backing:NSBackingStoreBuffered defer:YES];
    else
        self = [super initWithContentRect:NSMakeRect(0, 0, 431 + (hideOptions ? 0 : 50), 45) styleMask:NSWindowStyleMaskBorderless backing:NSBackingStoreBuffered defer:YES];
    
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
        
        NSView<OEHUDBarView> *barView;
        
        if(useNew)
            barView = [[OEGameControlsBarView alloc] initWithFrame:NSMakeRect(0, 0, 442, 42)];
        else
            barView = [[OEHUDControlsBarView alloc] initWithFrame:NSMakeRect(0, 0, 431 + (hideOptions ? 0 : 50), 45)];
        
        [self.contentView addSubview:barView];
        [barView setupControls];
        
        _eventMonitor = [NSEvent addLocalMonitorForEventsMatchingMask:NSEventMaskMouseMoved handler:^NSEvent*(NSEvent* e)
                         {
                             if(NSApp.isActive && self.gameWindow.isMainWindow)
                                 [self performSelectorOnMainThread:@selector(mouseMoved:) withObject:e waitUntilDone:NO];
                             return e;
                         }];
        _openMenus = [NSMutableSet set];
        _controlsView = barView;

        [NSCursor setHiddenUntilMouseMoves:YES];

        NSNotificationCenter *nc = NSNotificationCenter.defaultCenter;
        // Show HUD when switching back from other applications
        [nc addObserver:self selector:@selector(mouseMoved:) name:NSApplicationDidBecomeActiveNotification object:nil];
        [nc addObserver:self selector:@selector(willMove:) name:NSWindowWillMoveNotification object:self];
        [nc addObserver:self selector:@selector(didMove:) name:NSWindowDidMoveNotification object:self];

        _sortedSystemShaders = [OEShadersModel.shared.systemShaderNames sortedArrayUsingSelector:@selector(caseInsensitiveCompare:)];
        _sortedCustomShaders = [OEShadersModel.shared.customShaderNames sortedArrayUsingSelector:@selector(caseInsensitiveCompare:)];
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

#pragma mark - Cheats
- (void)OE_loadCheats
{
    // In order to load cheats, we need the game core to be running and, consequently, the ROM to be set.
    // We use -reflectEmulationRunning:, which we receive from OEGameViewController when the emulation
    // starts or resumes
    if(self.gameViewController.supportsCheats)
    {
        NSString *md5Hash = self.gameViewController.document.rom.md5Hash;
        if(md5Hash)
        {
            OECheats *cheatsXML = [[OECheats alloc] initWithMd5Hash:md5Hash];
            _cheats             = [cheatsXML.allCheats mutableCopy];
            _cheatsLoaded       = YES;
        }
    }
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
    return _openMenus.count==0 && !NSPointInRect(self.mouseLocationOutsideOfEventStream, [self bounds]);
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

#pragma mark - Menus
- (void)showOptionsMenu:(id)sender
{
    NSMenu *menu = [NSMenu new];

    NSMenuItem *item;
    item = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Edit Game Controls…", @"") action:@selector(editControls:) keyEquivalent:@""];
    [menu addItem:item];

    // Insert Cart/Disk/Tape Menu
    if(self.gameViewController.supportsFileInsertion)
    {
        item = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Insert Cart/Disk/Tape…", @"") action:@selector(insertFile:) keyEquivalent:@""];
        [menu addItem:item];
    }

    // Setup Cheats Menu
    if(self.gameViewController.supportsCheats)
    {
        NSMenu *cheatsMenu = [NSMenu new];
        cheatsMenu.title = NSLocalizedString(@"Select Cheat", @"");
        item = [NSMenuItem new];
        item.title = NSLocalizedString(@"Select Cheat", @"");
        [menu addItem:item];
        item.submenu = cheatsMenu;

        NSMenuItem *addCheatMenuItem = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Add Cheat…", @"")
                                                                  action:@selector(addCheat:)
                                                           keyEquivalent:@""];
        addCheatMenuItem.representedObject = _cheats;
        [cheatsMenu addItem:addCheatMenuItem];

        if(_cheats.count != 0)
            [cheatsMenu addItem:[NSMenuItem separatorItem]];

        for(NSDictionary *cheatObject in _cheats)
        {
            NSString *description = [cheatObject objectForKey:@"description"];
            BOOL enabled          = [[cheatObject objectForKey:@"enabled"] boolValue];

            NSMenuItem *cheatsMenuItem = [[NSMenuItem alloc] initWithTitle:description action:@selector(setCheat:) keyEquivalent:@""];
            cheatsMenuItem.representedObject = cheatObject;
            cheatsMenuItem.state = enabled ? NSControlStateValueOn : NSControlStateValueOff;

            [cheatsMenu addItem:cheatsMenuItem];
        }
    }

    // Setup Core selection menu
    NSMenu *coresMenu = [NSMenu new];
    coresMenu.title = NSLocalizedString(@"Select Core", @"");

    NSString *systemIdentifier = self.gameViewController.systemIdentifier;
    NSArray *corePlugins = [OECorePlugin corePluginsForSystemIdentifier:systemIdentifier];
    if(corePlugins.count > 1)
    {
        corePlugins = [corePlugins sortedArrayUsingComparator:^NSComparisonResult(id obj1, id obj2) {
            return [[obj1 displayName] localizedCaseInsensitiveCompare:[obj2 displayName]];
        }];

        for(OECorePlugin *aPlugin in corePlugins)
        {
            NSMenuItem *coreItem = [[NSMenuItem alloc] initWithTitle:aPlugin.displayName action:@selector(switchCore:) keyEquivalent:@""];
            coreItem.representedObject = aPlugin;

            if([aPlugin.bundleIdentifier isEqual:self.gameViewController.coreIdentifier])
                coreItem.state = NSControlStateValueOn;

            [coresMenu addItem:coreItem];
        }

        item = [NSMenuItem new];
        item.title = NSLocalizedString(@"Select Core", @"");
        item.submenu = coresMenu;
        if(coresMenu.itemArray.count > 1)
            [menu addItem:item];
    }

    // Setup Disc selection Menu
    if(self.gameViewController.supportsMultipleDiscs)
    {
        NSUInteger maxDiscs = self.gameViewController.discCount;

        NSMenu *discsMenu = [NSMenu new];
        discsMenu.title = NSLocalizedString(@"Select Disc", @"");
        item = [NSMenuItem new];
        item.title = NSLocalizedString(@"Select Disc", @"");
        [menu addItem:item];
        item.submenu = discsMenu;
        item.enabled = maxDiscs > 1 ? YES : NO;

        for(unsigned int disc = 1; disc <= maxDiscs; disc++)
        {
            NSString *discTitle  = [NSString stringWithFormat:NSLocalizedString(@"Disc %u", @"Disc selection menu item title"), disc];
            NSMenuItem *discsMenuItem = [[NSMenuItem alloc] initWithTitle:discTitle action:@selector(setDisc:) keyEquivalent:@""];
            [discsMenuItem setRepresentedObject:@(disc)];

            [discsMenu addItem:discsMenuItem];
        }
    }

    // Setup Change Display Mode Menu
    if(self.gameViewController.supportsDisplayModeChange && self.gameViewController.displayModes.count > 0)
    {
        NSMenu *displayMenu = [NSMenu new];
        displayMenu.autoenablesItems = NO;
        displayMenu.title = NSLocalizedString(@"Select Display Mode", @"");
        item = [NSMenuItem new];
        item.title = displayMenu.title;
        [menu addItem:item];
        item.submenu = displayMenu;

        NSString *mode;
        BOOL selected, enabled;
        NSInteger indentationLevel;

        for (NSDictionary *modeDict in self.gameViewController.displayModes)
        {
            if (modeDict[OEGameCoreDisplayModeSeparatorItemKey])
            {
                [displayMenu addItem:[NSMenuItem separatorItem]];
                continue;
            }

            mode             =  modeDict[OEGameCoreDisplayModeNameKey] ?:
                                modeDict[OEGameCoreDisplayModeLabelKey];
            selected         = [modeDict[OEGameCoreDisplayModeStateKey] boolValue];
            enabled          =  modeDict[OEGameCoreDisplayModeLabelKey] ? NO : YES;
            indentationLevel = [modeDict[OEGameCoreDisplayModeIndentationLevelKey] integerValue] ?: 0;

            // Submenu group
            if (modeDict[OEGameCoreDisplayModeGroupNameKey])
            {
                // Setup Submenu
                NSMenu *displaySubmenu = [NSMenu new];
                displaySubmenu.autoenablesItems = NO;
                displaySubmenu.title = modeDict[OEGameCoreDisplayModeGroupNameKey];
                item = [NSMenuItem new];
                item.title = displaySubmenu.title;
                [displayMenu addItem:item];
                item.submenu = displaySubmenu;

                // Submenu items
                for (NSDictionary *subModeDict in modeDict[OEGameCoreDisplayModeGroupItemsKey])
                {
                    // Disallow deeper submenus
                    if (subModeDict[OEGameCoreDisplayModeGroupNameKey])
                        continue;

                    if (subModeDict[OEGameCoreDisplayModeSeparatorItemKey])
                    {
                        [displaySubmenu addItem:[NSMenuItem separatorItem]];
                        continue;
                    }

                    mode             =  subModeDict[OEGameCoreDisplayModeNameKey] ?:
                                        subModeDict[OEGameCoreDisplayModeLabelKey];
                    selected         = [subModeDict[OEGameCoreDisplayModeStateKey] boolValue];
                    enabled          =  subModeDict[OEGameCoreDisplayModeLabelKey] ? NO : YES;
                    indentationLevel = [subModeDict[OEGameCoreDisplayModeIndentationLevelKey] integerValue] ?: 0;

                    NSMenuItem *displaySubmenuItem = [[NSMenuItem alloc] initWithTitle:mode action:@selector(changeDisplayMode:) keyEquivalent:@""];
                    displaySubmenuItem.representedObject = subModeDict;
                    displaySubmenuItem.state = selected ? NSControlStateValueOn : NSControlStateValueOff;
                    displaySubmenuItem.enabled = enabled;
                    displaySubmenuItem.indentationLevel = indentationLevel;
                    [displaySubmenu addItem:displaySubmenuItem];
                }

                continue;
            }

            NSMenuItem *displayMenuItem = [[NSMenuItem alloc] initWithTitle:mode action:@selector(changeDisplayMode:) keyEquivalent:@""];
            displayMenuItem.representedObject = modeDict;
            displayMenuItem.state = selected ? NSControlStateValueOn : NSControlStateValueOff;
            displayMenuItem.enabled = enabled;
            displayMenuItem.indentationLevel = indentationLevel;
            [displayMenu addItem:displayMenuItem];
        }
    }

    // Setup Video Shader Menu
    NSMenu *shaderMenu = [NSMenu new];

    // Configure shader
    item = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Configure Shader…", @"")
                                      action:@selector(configureShader:)
                               keyEquivalent:@""];
    [shaderMenu addItem:item];
    [shaderMenu addItem:[NSMenuItem separatorItem]];

    NSUserDefaults *defaults = NSUserDefaults.standardUserDefaults;
    NSString *selectedShader = ([defaults objectForKey:[NSString stringWithFormat:OEGameSystemVideoShaderKeyFormat, systemIdentifier]]
                                ? : [defaults objectForKey:OEGameDefaultVideoShaderKey]);

    // Select the Default Shader if the current is not available (ie. deleted)
    if(![_sortedSystemShaders containsObject:selectedShader] && ![_sortedCustomShaders containsObject:selectedShader])
        selectedShader = [defaults objectForKey:OEGameDefaultVideoShaderKey];

    // add system shaders first
    for(NSString *shaderName in _sortedSystemShaders)
    {
        NSMenuItem *shaderItem = [[NSMenuItem alloc] initWithTitle:shaderName action:@selector(selectShader:) keyEquivalent:@""];

        if([shaderName isEqualToString:selectedShader]) shaderItem.state = NSControlStateValueOn;

        [shaderMenu addItem:shaderItem];
    }
    
    if (_sortedCustomShaders.count > 0) {
        [shaderMenu addItem:[NSMenuItem separatorItem]];
    
        for(NSString *shaderName in _sortedCustomShaders)
        {
            NSMenuItem *shaderItem = [[NSMenuItem alloc] initWithTitle:shaderName action:@selector(selectShader:) keyEquivalent:@""];

            if([shaderName isEqualToString:selectedShader]) shaderItem.state = NSControlStateValueOn;

            [shaderMenu addItem:shaderItem];
        }
    }

    item = [NSMenuItem new];
    item.title = NSLocalizedString(@"Select Shader", @"");
    [menu addItem:item];
    item.submenu = shaderMenu;

    // Setup integral scaling
    id<OEGameIntegralScalingDelegate> integralScalingDelegate = self.gameViewController.integralScalingDelegate;
    const BOOL hasSubmenu = integralScalingDelegate.shouldAllowIntegralScaling && [integralScalingDelegate respondsToSelector:@selector(maximumIntegralScale)];

    NSMenu *scaleMenu = [NSMenu new];
    scaleMenu.title = NSLocalizedString(@"Select Scale", @"");
    item = [NSMenuItem new];
    item.title = scaleMenu.title;
    [menu addItem:item];
    item.submenu = scaleMenu;

    if(hasSubmenu)
    {
        unsigned int maxScale = integralScalingDelegate.maximumIntegralScale;
        unsigned int currentScale = integralScalingDelegate.currentIntegralScale;

        for(unsigned int scale = 1; scale <= maxScale; scale++)
        {
            NSString *scaleTitle  = [NSString stringWithFormat:NSLocalizedString(@"%ux", @"Integral scale menu item title"), scale];
            NSMenuItem *scaleItem = [[NSMenuItem alloc] initWithTitle:scaleTitle action:@selector(changeIntegralScale:) keyEquivalent:@""];
            scaleItem.representedObject = @(scale);
            scaleItem.state = (scale == currentScale ? NSControlStateValueOn : NSControlStateValueOff);
            [scaleMenu addItem:scaleItem];
        }
        
        if (self.gameWindow.isFullScreen)
        {
            NSMenuItem *scaleItem = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Fill Screen", @"Integral scale menu item title")
                                                               action:@selector(changeIntegralScale:)
                                                        keyEquivalent:@""];
            scaleItem.representedObject = @0;
            scaleItem.state = (currentScale == 0 ? NSControlStateValueOn : NSControlStateValueOff);
            [scaleMenu addItem:scaleItem];
        }
    }
    else
        item.enabled = NO;

    if([NSUserDefaults.standardUserDefaults boolForKey:OEGameControlsBarShowsAudioOutput])
    {
        // Setup audio output
        NSMenu *audioOutputMenu = [NSMenu new];
        audioOutputMenu.title = NSLocalizedString(@"Select Audio Output Device", @"");
        item = [NSMenuItem new];
        item.title = audioOutputMenu.title;
        [menu addItem:item];
        item.submenu = audioOutputMenu;

        NSPredicate *outputPredicate = [NSPredicate predicateWithBlock:^BOOL(OEAudioDevice *device, NSDictionary *bindings) {
            return device.numberOfOutputChannels > 0;
        }];
        NSArray *audioOutputDevices = [OEAudioDeviceManager.sharedAudioDeviceManager.audioDevices filteredArrayUsingPredicate:outputPredicate];
        if(audioOutputDevices.count == 0) {
            item.enabled = NO;
        } else {
            NSMenuItem *deviceItem = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"System Default", @"Default audio device setting") action:@selector(changeAudioOutputDevice:) keyEquivalent:@""];
            deviceItem.representedObject = nil;
            [audioOutputMenu addItem:deviceItem];
            
            [audioOutputMenu addItem:[NSMenuItem separatorItem]];
            
            for(OEAudioDevice *device in audioOutputDevices)
            {
                deviceItem = [[NSMenuItem alloc] initWithTitle:device.deviceName action:@selector(changeAudioOutputDevice:) keyEquivalent:@""];
                deviceItem.representedObject = device;
                [audioOutputMenu addItem:deviceItem];
            }
        }
    }

    menu.delegate = self;

    // Display menu.
    NSRect targetRect = NSInsetRect([sender bounds], -2.0, 1.0);
    NSPoint menuPosition = NSMakePoint(NSMinX(targetRect), NSMaxY(targetRect));
    [menu popUpMenuPositioningItem:nil atLocation:menuPosition inView:sender];
}

- (void)showSaveMenu:(id)sender
{
    NSMenu *menu = [NSMenu new];
    menu.autoenablesItems = NO;

    NSMenuItem *newSaveItem = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Save Current Game…", @"") action:@selector(saveState:) keyEquivalent:@""];
    newSaveItem.enabled = self.gameViewController.supportsSaveStates;
    menu.delegate = self;
    [menu addItem:newSaveItem];

    OEDBRom *rom = self.gameViewController.document.rom;
    [rom removeMissingStates];

    if(rom != nil)
    {
        BOOL includeAutoSaveState = [NSUserDefaults.standardUserDefaults boolForKey:OEGameControlsBarShowsAutoSaveStateKey];
        BOOL includeQuickSaveState = [NSUserDefaults.standardUserDefaults boolForKey:OEGameControlsBarShowsQuickSaveStateKey];
        BOOL useQuickSaveSlots = [NSUserDefaults.standardUserDefaults boolForKey:OESaveStateUseQuickSaveSlotsKey];
        NSArray *saveStates = [rom normalSaveStatesByTimestampAscending:YES];
        BOOL canDeleteSaveStates = [NSUserDefaults.standardUserDefaults boolForKey:OEGameControlsBarCanDeleteSaveStatesKey];

        if(includeQuickSaveState && !useQuickSaveSlots && [rom quickSaveStateInSlot:0] != nil)
            saveStates = [@[[rom quickSaveStateInSlot:0]] arrayByAddingObjectsFromArray:saveStates];

        if(includeAutoSaveState && [rom autosaveState] != nil)
            saveStates = [@[[rom autosaveState]] arrayByAddingObjectsFromArray:saveStates];

        if(saveStates.count != 0 || (includeQuickSaveState && useQuickSaveSlots))
        {
            [menu addItem:[NSMenuItem separatorItem]];
            
            NSMenuItem *item = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Load", @"") action:NULL keyEquivalent:@""];
            item.enabled = NO;
            [menu addItem:item];

            if(canDeleteSaveStates)
            {
                item = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Delete", @"") action:NULL keyEquivalent:@""];
                item.alternate = YES;
                item.enabled = NO;
                item.keyEquivalentModifierMask = NSEventModifierFlagOption;
                [menu addItem:item];
            }

            // Build Quck Load item with submenu
            if(includeQuickSaveState && useQuickSaveSlots)
            {
                NSString *loadTitle   = NSLocalizedString(@"Quick Load", @"Quick load menu title");
                NSMenuItem *loadItem  = [[NSMenuItem alloc] initWithTitle:loadTitle action:NULL keyEquivalent:@""];

                NSMenu *loadSubmenu = [[NSMenu alloc] initWithTitle:loadTitle];
                loadItem.indentationLevel = 1;

                for(NSInteger i = 1; i <= 9; i++)
                {
                    OEDBSaveState *state = [rom quickSaveStateInSlot:i];

                    loadTitle = [NSString stringWithFormat:NSLocalizedString(@"Slot %ld", @"Quick load menu item title"), (long)i];
                    NSMenuItem *loadItem = [[NSMenuItem alloc] initWithTitle:loadTitle action:@selector(quickLoad:) keyEquivalent:@""];
                    loadItem.enabled = state != nil;
                    loadItem.representedObject = @(i);
                    [loadSubmenu addItem:loadItem];
                }

                loadItem.submenu = loadSubmenu;
                [menu addItem:loadItem];
            }

            // Add 'normal' save states
            for(OEDBSaveState *saveState in saveStates)
            {
                NSString *itemTitle = saveState.displayName;

                if(!itemTitle || [itemTitle isEqualToString:@""])
                    itemTitle = [NSString stringWithFormat:@"%@", saveState.timestamp];

                NSMenuItem *item = [[NSMenuItem alloc] initWithTitle:itemTitle action:@selector(loadState:) keyEquivalent:@""];
                item.indentationLevel = 1;

                item.representedObject = saveState;
                [menu addItem:item];

                if(canDeleteSaveStates)
                {
                    NSMenuItem *deleteStateItem = [[NSMenuItem alloc] initWithTitle:itemTitle action:@selector(deleteSaveState:) keyEquivalent:@""];
                    deleteStateItem.alternate = YES;
                    deleteStateItem.keyEquivalentModifierMask = NSEventModifierFlagOption;
                    deleteStateItem.representedObject = saveState;
                    deleteStateItem.indentationLevel = 1;

                    [menu addItem:deleteStateItem];
                }
            }
        }
    }

    // Display menu.
    NSRect targetRect = NSInsetRect([sender bounds], -2.0, 1.0);
    NSPoint menuPosition = NSMakePoint(NSMinX(targetRect), NSMaxY(targetRect));
    [menu popUpMenuPositioningItem:nil atLocation:menuPosition inView:sender];
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
    NSView<OEHUDBarView> *view   = self.contentView.subviews.lastObject;
    NSSlider             *slider = view.slider;

    [slider animator].doubleValue = volume;
}

- (void)reflectEmulationRunning:(BOOL)isEmulationRunning
{
    NSView<OEHUDBarView> *view        = self.contentView.subviews.lastObject;
    NSButton             *pauseButton = view.pauseButton;
    pauseButton.state = !isEmulationRunning;

    if(isEmulationRunning)
        pauseButton.toolTip = NSLocalizedString(@"Pause Game", @"HUD bar tooltip");
    else
        pauseButton.toolTip = NSLocalizedString(@"Resume Game", @"HUD bar tooltip");

    if(isEmulationRunning && !_cheatsLoaded)
        [self OE_loadCheats];
}

- (void)gameWindowDidEnterFullScreen:(NSNotification *)notification;
{
    NSView<OEHUDBarView> *view = self.contentView.subviews.lastObject;
    view.fullScreenButton.state = NSControlStateValueOn;
    [self _performMouseMoved:nil];  // Show HUD because fullscreen animation makes the cursor appear
}

- (void)gameWindowWillExitFullScreen:(NSNotification *)notification;
{
    NSView<OEHUDBarView> *view = self.contentView.subviews.lastObject;
    view.fullScreenButton.state = NSControlStateValueOff;
}

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

        NSView<OEHUDBarView> *view = self.contentView.subviews.lastObject;
        view.fullScreenButton.state = gameWindow.isFullScreen ? NSControlStateValueOn : NSControlStateValueOff;
    }
}

@end
