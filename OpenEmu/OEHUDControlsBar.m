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

#import "OEHUDControlsBar.h"
#import "NSImage+OEDrawingAdditions.h"

#import "OEImageButton.h"
#import "OEHUDButtonCell.h"
#import "OEHUDSlider.h"

#import "OEMenu.h"
#import "OEDBRom.h"

#import "OECompositionPlugin.h"
#import "OEGameViewController.h"

@interface OEHUDControlsBarWindow ()
@property(strong) OEHUDControlsBarView *controlsView;
@end

@implementation OEHUDControlsBarWindow
@synthesize lastMouseMovement;
@synthesize gameViewController;
@synthesize controlsView;

- (id)initWithGameViewController:(OEGameViewController *)controller
{
    BOOL hideOptions = [[NSUserDefaults standardUserDefaults] boolForKey:UDHUDHideOptionsKey];
    
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
    }
    
    NSLog(@"OEHUDControlsBarWindow init");
    return self;
}

- (void)dealloc
{
    NSLog(@"OEHUDControlsBarWindow dealloc");
    
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
        NSTimeInterval interval = [[NSUserDefaults standardUserDefaults] doubleForKey:UDHUDFadeOutDelayKey];
        fadeTimer = [NSTimer scheduledTimerWithTimeInterval:interval target:self selector:@selector(timerDidFire:) userInfo:nil repeats:YES];
    }
    
    
    lastMouseMovement = lastMouseMovementDate;
}

- (void)timerDidFire:(NSTimer *)timer
{
    NSTimeInterval interval = [[NSUserDefaults standardUserDefaults] doubleForKey:UDHUDFadeOutDelayKey];
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
            NSTimeInterval interval = [[NSUserDefaults standardUserDefaults] doubleForKey:UDHUDFadeOutDelayKey];
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

#pragma mark -

- (void)muteAction:(id)sender
{
    NSLog(@"%@ %@", self, [NSApp keyWindow]);
    
    id slider = [(OEHUDControlsBarView *)[[[self contentView] subviews] lastObject] slider];
    [slider setFloatValue:0.0];
    [[self gameViewController] setVolume:0.0];
}

- (void)unmuteAction:(id)sender
{
    id slider = [(OEHUDControlsBarView *)[[[self contentView] subviews] lastObject] slider];
    [slider setFloatValue:1.0];
    [[self gameViewController] setVolume:1.0];
}

- (void)resetAction:(id)sender
{
    [[self gameViewController] resetGame];
}

- (void)playPauseAction:(id)sender
{
    [[self gameViewController] setPauseEmulation:![[self gameViewController] isEmulationPaused]];
}

- (void)stopAction:(id)sender
{
    [[self gameViewController] terminateEmulation];
}

- (void)fullscreenAction:(id)sender
{
    [[self gameViewController] toggleFullscreen];
}

- (void)volumeAction:(id)sender
{
    [[self gameViewController] setVolume:[sender floatValue]];
}

- (void)optionsAction:(id)sender
{
    NSMenu *menu = [[NSMenu alloc] init];
    
    NSMenuItem *item;
    item = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Edit Game Controls", @"") action:@selector(optionsActionEditControls:) keyEquivalent:@""];
    [item setTarget:self];
    [menu addItem:item];
    
    NSMenu *filterMenu = [[NSMenu alloc] init];
    [filterMenu setTitle:NSLocalizedString(@"Select Filter", @"")];
    // Setup plugins menu
    NSArray *filterPlugins = [[OECompositionPlugin allPluginNames] sortedArrayUsingSelector:@selector(caseInsensitiveCompare:)];
    // These filters are loaded and run by GL, and do not rely on QTZs
    NSArray *filterNames = [filterPlugins arrayByAddingObjectsFromArray:OEOpenGLFilterNameArray];
    
    NSString *selectedFilter = [[NSUserDefaults standardUserDefaults] objectForKey:UDVideoFilterKey];
    for(NSString *aName in filterNames)
    {
        NSMenuItem *filterItem = [[NSMenuItem alloc] initWithTitle:aName action:@selector(optionsActionSelectFilter:) keyEquivalent:@""];
        [filterItem setTarget:self];
        
        if([aName isEqualToString:selectedFilter]) [filterItem setState:NSOnState];
        
        [filterMenu addItem:filterItem];
    }
    
    item = [[NSMenuItem alloc] init];
    item.title = NSLocalizedString(@"Select Filter", @"");
    [menu addItem:item];
    [item setSubmenu:filterMenu];
    
    item = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Edit Core Settings", @"") action:@selector(optionsActionCorePreferences:) keyEquivalent:@""];
    [item setTarget:self];
    [menu addItem:item];
    
    OEMenu *oemenu = [menu convertToOEMenu];
    [oemenu setStyle:OEMenuStyleLight];
    [oemenu setDelegate:self];
    oemenu.itemsAboveScroller = 2;
    oemenu.maxSize = NSMakeSize(500, 256);
    NSRect targetRect = (NSRect){{[sender frame].origin.x,0},{[sender frame].size.width -6, NSHeight([self frame])}};
    targetRect = NSInsetRect(targetRect, 0, 17);
    [oemenu openOnEdge:OEMaxYEdge ofRect:targetRect ofWindow:self];
}

- (void)optionsActionEditControls:(id)sender{}

- (void)optionsActionSelectFilter:(id)sender
{
    NSString *selectedFilter = [sender title];
    [[NSUserDefaults standardUserDefaults] setObject:selectedFilter forKey:UDVideoFilterKey];
}

- (void)optionsActionCorePreferences:(id)sender {}

#pragma mark -
#pragma mark Save States

- (void)saveAction:(id)sender
{
    NSMenu *menu = [[NSMenu alloc] init];
    
    NSMenuItem *item;
    item = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Save Current Game", @"") action:@selector(doSaveState:) keyEquivalent:@""];
    [item setTarget:self];
    [menu addItem:item];
    
    NSArray *saveStates = nil;
    if([[self gameViewController] rom] != nil && (saveStates = [[[self gameViewController] rom] saveStatesByTimestampAscending:YES]) && [saveStates count] != 0)
    {
        [menu addItem:[NSMenuItem separatorItem]];
        for(id saveState in saveStates)
        {
            NSString *itemTitle = [saveState valueForKey:@"userDescription"];
            if(!itemTitle || [itemTitle isEqualToString:@""])
                itemTitle = [NSString stringWithFormat:@"%@", [saveState valueForKey:@"timestamp"]];
            
            if([[NSUserDefaults standardUserDefaults] boolForKey:OEHUDCanDeleteStateKey])
            {
                OEMenuItem *oeitem = [[OEMenuItem alloc] initWithTitle:itemTitle action:@selector(doLoadState:) keyEquivalent:@""];
                [oeitem setTarget:self];
                
                [oeitem setHasAlternate:YES];
                [oeitem setAlternateTarget:self];
                [oeitem setAlternateAction:@selector(doDeleteState:)];
                
                [oeitem setRepresentedObject:saveState];
                [menu addItem:oeitem];
            }
            else
            {
                item = [[NSMenuItem alloc] initWithTitle:itemTitle action:@selector(doLoadState:) keyEquivalent:@""];
                [item setTarget:self];
                [item setRepresentedObject:saveState];
                [menu addItem:item];
            }
        }
    }
    
    OEMenu *oemenu = [menu convertToOEMenu];
    [oemenu setDelegate:self];
    [oemenu setStyle:OEMenuStyleLight];
    oemenu.itemsAboveScroller = 2;
    oemenu.maxSize = NSMakeSize(5000, 256);

    NSRect targetRect = (NSRect){{[sender frame].origin.x,0},{[sender frame].size.width -6, NSHeight([self frame])}};
    targetRect = NSInsetRect(targetRect, 0, 17);
    [oemenu openOnEdge:OEMaxYEdge ofRect:targetRect ofWindow:self];
}

- (void)doLoadState:(id)stateItem
{
    [[self gameViewController] loadSaveState:[stateItem representedObject]];
    
    [self hide];
}

- (void)doDeleteState:(id)stateItem
{
    [[self gameViewController] deleteSaveState:[stateItem representedObject]];
}

- (void)doSaveState:(id)sender
{
    [[self gameViewController] saveStateAskingUserForName:nil];
}

#pragma mark -
#pragma mark OEMenuDelegate Implementation

- (void)menuDidShow:(OEMenu *)men
{
    openMenus++;
}

- (void)menuDidHide:(OEMenu *)men
{
    openMenus--;
}

- (void)parentWindowDidEnterFullScreen:(NSNotification *)notification;
{
}

- (void)parentWindowWillExitFullScreen:(NSNotification *)notification;
{
    
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
    }
}

@end

@implementation OEHUDControlsBarView
@synthesize slider, fullScreenButton;

+ (void)initialize
{
    // Make sure not to reinitialize for subclassed objects
    if(self != [OEHUDControlsBarView class])
        return;

    NSImage *spriteSheet = [NSImage imageNamed:@"hud_glyphs"];
    
    float itemHeight = (spriteSheet.size.height/5);
    [spriteSheet setName:@"hud_playpause" forSubimageInRect:NSMakeRect(0, 3*itemHeight, spriteSheet.size.width, itemHeight*2)];
    
    [spriteSheet setName:@"hud_restart" forSubimageInRect:NSMakeRect(0, 2*itemHeight, spriteSheet.size.width, itemHeight)];
    [spriteSheet setName:@"hud_save" forSubimageInRect:NSMakeRect(0, 1*itemHeight, spriteSheet.size.width, itemHeight)];
    [spriteSheet setName:@"hud_options" forSubimageInRect:NSMakeRect(0, 0, spriteSheet.size.width, itemHeight)];
    
    NSImage *fsImage = [NSImage imageNamed:@"hud_fullscreen_glyph"];
    [fsImage setName:@"hud_fullscreen_glyph_normal" forSubimageInRect:NSMakeRect(fsImage.size.width/2, 0, fsImage.size.width/2, fsImage.size.height)];
    [fsImage setName:@"hud_fullscreen_glyph_pressed" forSubimageInRect:NSMakeRect(0, 0, fsImage.size.width/2, fsImage.size.height)];
    
    NSImage *volume = [NSImage imageNamed:@"hud_volume"];
    [volume setName:@"hud_volume_down" forSubimageInRect:NSMakeRect(0, 0, 13, volume.size.height)];
    [volume setName:@"hud_volume_up" forSubimageInRect:NSMakeRect(13, 0, 15, volume.size.height)];
    
    NSImage *power = [NSImage imageNamed:@"hud_power_glyph"];
    [power setName:@"hud_power_glyph_normal" forSubimageInRect:(NSRect){{power.size.width/2,0}, {power.size.width/2, power.size.height}}];
    [power setName:@"hud_power_glyph_pressed" forSubimageInRect:(NSRect){{0,0}, {power.size.width/2, power.size.height}}];
}

- (id)initWithFrame:(NSRect)frame
{
    if((self = [super initWithFrame:frame]))
    {
        [self setWantsLayer:YES];
    }
    
    return self;
}

- (void)awakeFromNib
{
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
    NSButton *stopButton = [[NSButton alloc] init];
    OEHUDButtonCell *pcell = [[OEHUDButtonCell alloc] init];
    pcell.buttonColor = OEHUDButtonColorRed;
    [stopButton setCell:pcell];
    [stopButton setImage:[NSImage imageNamed:@"hud_power_glyph_normal"]];
    [stopButton setAlternateImage:[NSImage imageNamed:@"hud_power_glyph_pressed"]];
    [stopButton setAction:@selector(terminateEmulation:)];
    [stopButton setFrame:NSMakeRect(10, 13, 51, 23)];
    [stopButton setAutoresizingMask:NSViewMaxXMargin | NSViewMinYMargin];
    [self addSubview:stopButton];
    
    OEImageButton *pauseButton = [[OEImageButton alloc] init];
    OEImageButtonHoverSelectable *cell = [[OEImageButtonHoverSelectable alloc] init];
    [pauseButton setCell:cell];
    [cell setImage:[NSImage imageNamed:@"hud_playpause"]];
    [pauseButton setTarget:[self window]];
    [pauseButton setAction:@selector(playPauseAction:)];
    [pauseButton setFrame:NSMakeRect(82, 9, 32, 32)];
    [pauseButton setAutoresizingMask:NSViewMaxXMargin | NSViewMinYMargin];
    [self addSubview:pauseButton];
    
    
    OEImageButton *restartButton = [[OEImageButton alloc] init];
    OEImageButtonHoverPressed *hcell = [[OEImageButtonHoverPressed alloc] init];
    [restartButton setCell:hcell];
    [hcell setImage:[NSImage imageNamed:@"hud_restart"]];
    [restartButton setTarget:[self window]];
    [restartButton setAction:@selector(resetAction:)];
    [restartButton setFrame:NSMakeRect(111, 9, 32, 32)];
    [restartButton setAutoresizingMask:NSViewMaxXMargin | NSViewMinYMargin];
    [self addSubview:restartButton];
    
    OEImageButton *saveButton = [[OEImageButton alloc] init];
    hcell = [[OEImageButtonHoverPressed alloc] init];
    [saveButton setCell:hcell];
    [hcell setImage:[NSImage imageNamed:@"hud_save"]];
    [saveButton setTarget:[self window]];
    [saveButton setAction:@selector(saveAction:)];
    [saveButton setFrame:NSMakeRect(162, 6, 32, 32)];
    [saveButton setAutoresizingMask:NSViewMaxXMargin | NSViewMinYMargin];
    [self addSubview:saveButton];
    
    BOOL hideOptions = [[NSUserDefaults standardUserDefaults] boolForKey:UDHUDHideOptionsKey];
    if(!hideOptions)
    {
        OEImageButton *optionsButton = [[OEImageButton alloc] init];
        hcell = [[OEImageButtonHoverPressed alloc] init];
        [optionsButton setCell:hcell];
        [hcell setImage:[NSImage imageNamed:@"hud_options"]];
        [optionsButton setTarget:[self window]];
        [optionsButton setAction:@selector(optionsAction:)];
        [optionsButton setFrame:NSMakeRect(212, 6, 32, 32)];
        [optionsButton setAutoresizingMask:NSViewMaxXMargin | NSViewMinYMargin];
        [self addSubview:optionsButton];
    }
    
    NSButton *volumeDownView = [[NSButton alloc] initWithFrame:NSMakeRect(223 + (hideOptions ? 0 : 50), 17, 13, 14)];
    [volumeDownView setBordered:NO];
    [[volumeDownView cell] setHighlightsBy:NSNoCellMask];
    [volumeDownView setImage:[NSImage imageNamed:@"hud_volume_down"]];
    [volumeDownView setTarget:[self window]];
    [volumeDownView setAction:@selector(muteAction:)];    
    [self addSubview:volumeDownView];
    
    NSButton *volumeUpView = [[NSButton alloc] initWithFrame:NSMakeRect(320 + (hideOptions? 0 : 50), 17, 15, 14)];
    [volumeUpView setBordered:NO];
    [[volumeUpView cell] setHighlightsBy:NSNoCellMask];
    [volumeUpView setImage:[NSImage imageNamed:@"hud_volume_up"]];
    [volumeUpView setTarget:[self window]];
    [volumeUpView setAction:@selector(unmuteAction:)];
    [self addSubview:volumeUpView];
    
    slider = [[OEHUDSlider alloc] initWithFrame:NSMakeRect(240 + (hideOptions ? 0 : 50), 13, 80, 23)];
    
    OEHUDSliderCell *sliderCell = [[OEHUDSliderCell alloc] init];
    [slider setCell:sliderCell];
    [slider setContinuous:YES];
    [slider setMaxValue:1.0];
    [slider setMinValue:0.0];
    [slider setFloatValue:[[NSUserDefaults standardUserDefaults] floatForKey:UDVolumeKey]];
    [slider setTarget:[self window]];
    [slider setAction:@selector(volumeAction:)];
    
    [self addSubview:slider];
    
    
    fullScreenButton = [[NSButton alloc] init];
    pcell = [[OEHUDButtonCell alloc] init];
    [fullScreenButton setCell:pcell];
    
    [fullScreenButton setImage:[NSImage imageNamed:@"hud_fullscreen_glyph_normal"]];
    [fullScreenButton setAlternateImage:[NSImage imageNamed:@"hud_fullscreen_glyph_pressed"]];
    
    [fullScreenButton setAction:@selector(toggleFullScreen:)];
    
    [fullScreenButton setFrame:NSMakeRect(370 + (hideOptions ? 0 : 50), 13, 51, 23)];
    [fullScreenButton setAutoresizingMask:NSViewMaxXMargin | NSViewMinYMargin];
    [self addSubview:fullScreenButton];
    [fullScreenButton setTitle:@""];
}

@end
