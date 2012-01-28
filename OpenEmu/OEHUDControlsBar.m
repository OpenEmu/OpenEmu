//
//  OEOSDControls.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 12.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OEHUDControlsBar.h"
#import "NSImage+OEDrawingAdditions.h"

#import "OEImageButton.h"
#import "OEHUDButtonCell.h"
#import "OEHUDSlider.h"

#import "OEMenu.h"
#import "OEDBRom.h"

#import "OECompositionPlugin.h"
#import "OEGameViewController.h"
@implementation OEHUDControlsBarWindow
@synthesize lastMouseMovement;
@synthesize gameViewController;
- (id)initWithGameViewController:(OEGameViewController*)controller
{
    BOOL hideOptions = [[NSUserDefaults standardUserDefaults] boolForKey:UDHUDHideOptionsKey];
    self = [super initWithContentRect:NSMakeRect(0, 0, 431+(hideOptions?0:50), 45) styleMask:NSBorderlessWindowMask backing:NSWindowBackingLocationDefault defer:YES];
    if (self) 
    {
        [self setMovableByWindowBackground:YES];
        [self setOpaque:NO];
        [self setBackgroundColor:[NSColor clearColor]];
        [self setAlphaValue:0.0];
        
        [self setGameViewController:controller];
        
        OEHUDControlsBarView* controlsView = [[OEHUDControlsBarView alloc] initWithFrame:NSMakeRect(0, 0, 431+(hideOptions?0:50), 45)];
        [[self contentView] addSubview:controlsView];
        [controlsView setupControls];
        [controlsView release];
        
        eventMonitor = [NSEvent addGlobalMonitorForEventsMatchingMask:NSMouseMovedMask handler:^(NSEvent*incomingEvent)
                        {
                            if([NSApp isActive] && [[self parentWindow] isMainWindow])
                                [self performSelectorOnMainThread:@selector(mouseMoved:) withObject:incomingEvent waitUntilDone:NO];
                        }];
        [eventMonitor retain];
        
        openMenus = 0;
    }
    NSLog(@"OEHUDControlsBarWindow init");
    return self;
}

- (void)dealloc
{    
    NSLog(@"OEHUDControlsBarWindow dealloc");
    
    [fadeTimer invalidate];
    [fadeTimer release];
    fadeTimer = nil;
    
    [lastMouseMovement release];
    
    [self setGameViewController:nil];
    
    [NSEvent removeMonitor:eventMonitor];
    [eventMonitor release];
    
    [super dealloc];
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
    [fadeTimer release];
    fadeTimer = nil;
}

- (void)mouseMoved:(NSEvent *)theEvent
{
    NSWindow *parentWindow = [self parentWindow];
    NSPoint mouseLoc = [NSEvent mouseLocation];
    if(!NSPointInRect(mouseLoc, [parentWindow convertRectToScreen:[(NSView*)[[self gameViewController] view] frame]])) return;
    
    if(self.alphaValue==0.0)
    {
        [lastMouseMovement release];
        lastMouseMovement = [[NSDate date] retain];       
        [self show];
    }
    
    [self setLastMouseMovement:[NSDate date]];
}

- (void)setLastMouseMovement:(NSDate *)lastMouseMovementDate
{
    if(!fadeTimer)
    {
        NSTimeInterval interval = [[NSUserDefaults standardUserDefaults] doubleForKey:UDHUDFadeOutDelayKey];
        fadeTimer = [[NSTimer scheduledTimerWithTimeInterval:interval target:self selector:@selector(timerDidFire:) userInfo:nil repeats:YES] retain];
    }
    
    [lastMouseMovementDate retain];
    [lastMouseMovement release];
    
    lastMouseMovement = lastMouseMovementDate;    
}

- (void)timerDidFire:(NSTimer*)timer
{
    NSTimeInterval interval = [[NSUserDefaults standardUserDefaults] doubleForKey:UDHUDFadeOutDelayKey];
    NSDate* hideDate = [lastMouseMovement dateByAddingTimeInterval:interval];
    
    if(([NSDate timeIntervalSinceReferenceDate]-[hideDate timeIntervalSinceReferenceDate]) >= 0.0)
    {
        if([self canFadeOut])
        {
            [fadeTimer invalidate];
            [fadeTimer release];
            fadeTimer = nil;
            
            [self hide];
        } 
        else 
        {
            NSTimeInterval interval = [[NSUserDefaults standardUserDefaults] doubleForKey:UDHUDFadeOutDelayKey];
            NSDate* nextTime = [NSDate dateWithTimeIntervalSinceNow:interval];
            
            [fadeTimer setFireDate:nextTime];
        }       
    } 
    else {
        [fadeTimer setFireDate:hideDate];
    }
}
- (NSRect)bounds
{
    NSRect bounds = [self frame];
    bounds.origin = NSMakePoint(0, 0);
    return bounds;
}
- (BOOL)canFadeOut
{
    return openMenus==0 && !NSPointInRect([self mouseLocationOutsideOfEventStream], [self bounds]);    
}
#pragma mark -
- (void)muteAction:(id)sender
{
    id slider = [(OEHUDControlsBarView*)[[[self contentView] subviews] lastObject] slider];
    [slider setFloatValue:0.0];
    [[self gameViewController] setVolume:0.0];
}
- (void)unmuteAction:(id)sender
{
    id slider = [(OEHUDControlsBarView*)[[[self contentView] subviews] lastObject] slider];
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
    if([self gameViewController])
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

- (void)optionsAction:(id)sender{
    NSMenu* menu = [[NSMenu alloc] init];
    
    NSMenuItem* item;
    item = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Edit Game Controls", @"") action:@selector(optionsActionEditControls:) keyEquivalent:@""];
    [item setTarget:self];
    [menu addItem:item];
    [item release];
    
    NSMenu* filterMenu = [[NSMenu alloc] init];
    [filterMenu setTitle:NSLocalizedString(@"Select Filter", @"")];
    // Setup plugins menu
    NSArray *filterPlugins = [[OECompositionPlugin allPluginNames] sortedArrayUsingSelector:@selector(caseInsensitiveCompare:)];
    // These filters are loaded and run by GL, and do not rely on QTZs
    NSArray* filterNames = [filterPlugins arrayByAddingObjectsFromArray:
                            [NSArray arrayWithObjects:
                             @"Linear",
                             @"Nearest Neighbor",
                             @"Scale2xHQ",
                             @"Scale2xPlus",
                             @"Scale4x",
                             @"Scale4xHQ",
                             nil]];
    
    NSString* selectedFilter = [[NSUserDefaults standardUserDefaults] objectForKey:UDVideoFilterKey];
    for(NSString* aName in filterNames){
        NSMenuItem* filterItem = [[NSMenuItem alloc] initWithTitle:aName action:@selector(optionsActionSelectFilter:) keyEquivalent:@""];
        [filterItem setTarget:self];
        if([aName isEqualToString:selectedFilter]){
            [filterItem setState:NSOnState];
        }        
        [filterMenu addItem:filterItem];
        [filterItem release];
    }
    
    
    item = [[NSMenuItem alloc] init];
    item.title = NSLocalizedString(@"Select Filter", @"");
    [menu addItem:item];
    [item setSubmenu:filterMenu];
    [item release];    
    [filterMenu release];
    
    item = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Edit Core Settings", @"") action:@selector(optionsActionCorePreferences:) keyEquivalent:@""];
    [item setTarget:self];
    [menu addItem:item];
    [item release];
    
    OEMenu* oemenu = [menu convertToOEMenu];
    [oemenu setDelegate:self];
    oemenu.itemsAboveScroller = 2;
    oemenu.maxSize = NSMakeSize(192, 256);
    NSRect buttonRect = [sender frame];
    NSPoint menuPoint = NSMakePoint(NSMaxX(buttonRect)+[self frame].origin.x, NSMinY(buttonRect)+[self frame].origin.y);
    [oemenu openAtPoint:menuPoint ofWindow:self];
    [menu release];
}
- (void)optionsActionEditControls:(id)sender{}
- (void)optionsActionSelectFilter:(id)sender{
    NSString* selectedFilter = [sender title];
    [[NSUserDefaults standardUserDefaults] setObject:selectedFilter forKey:UDVideoFilterKey];
}
- (void)optionsActionCorePreferences:(id)sender{}
#pragma mark -
#pragma mark Save States
- (void)saveAction:(id)sender
{
    NSMenu* menu = [[NSMenu alloc] init];
    
    NSMenuItem* item;
    item = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Save Current Game", @"") action:@selector(doSaveState:) keyEquivalent:@""];
    [item setTarget:self];
    [menu addItem:item];
    [item release];
    
    NSArray* saveStates;
    if([[self gameViewController] rom] && (saveStates=[[[self gameViewController] rom] saveStatesByTimestampAscending:YES]) && [saveStates count])
    {
        [menu addItem:[NSMenuItem separatorItem]];
        for(id saveState in saveStates)
        {
            NSString* itemTitle = [saveState valueForKey:@"userDescription"];
            if(!itemTitle || [itemTitle isEqualToString:@""])
            {
                itemTitle = [NSString stringWithFormat:@"%@", [saveState valueForKey:@"timestamp"]];
            }
            if([[NSUserDefaults standardUserDefaults] boolForKey:OEHUDCanDeleteStateKey])
            {
                OEMenuItem* oeitem = [[OEMenuItem alloc] initWithTitle:itemTitle action:@selector(doLoadState:) keyEquivalent:@""];
                [oeitem setTarget:self];
                
                [oeitem setHasAlternate:YES];
                [oeitem setAlternateTarget:self];
                [oeitem setAlternateAction:@selector(doDeleteState:)];
                
                [oeitem setRepresentedObject:saveState];
                [menu addItem:oeitem];
                [oeitem release];
            }
            else 
            {
                item = [[NSMenuItem alloc] initWithTitle:itemTitle action:@selector(doLoadState:) keyEquivalent:@""];
                [item setTarget:self];
                [item setRepresentedObject:saveState];
                [menu addItem:item];
                [item release];                
            }
        }
    }
    
    OEMenu* oemenu = [menu convertToOEMenu];
    [oemenu setDelegate:self];
    oemenu.itemsAboveScroller = 2;
    oemenu.maxSize = NSMakeSize(192, 256);
    NSRect buttonRect = [sender frame];
    NSPoint menuPoint = NSMakePoint(NSMaxX(buttonRect)+[self frame].origin.x, NSMinY(buttonRect)+[self frame].origin.y);
    [oemenu openAtPoint:menuPoint ofWindow:self];
    [menu release];
}
- (void)doLoadState:(id)stateItem
{
    [[self gameViewController] loadState:[stateItem representedObject]];
    
    [self hide];
}

- (void)doDeleteState:(id)stateItem
{
    [[self gameViewController] deleteState:[stateItem representedObject]];
}

- (void)doSaveState:(id)sender
{
    [[self gameViewController] saveStateAskingUserForName:nil];
}
#pragma mark -
#pragma mark OEMenuDelegate Implementation
- (void)menuDidShow:(OEMenu *)men
{
    openMenus ++;
}
- (void)menuDidHide:(OEMenu *)men
{
    openMenus --;
}
@end

@implementation OEHUDControlsBarView
@synthesize slider;

+ (void)initialize
{
    // Make sure not to reinitialize for subclassed objects
    if (self != [OEHUDControlsBarView class])
        return;

    NSImage* spriteSheet = [NSImage imageNamed:@"hud_glyphs"];
    
    float itemHeight = (spriteSheet.size.height/5);
    [spriteSheet setName:@"hud_playpause" forSubimageInRect:NSMakeRect(0, 3*itemHeight, spriteSheet.size.width, itemHeight*2)];
    
    [spriteSheet setName:@"hud_restart" forSubimageInRect:NSMakeRect(0, 2*itemHeight, spriteSheet.size.width, itemHeight)];
    [spriteSheet setName:@"hud_save" forSubimageInRect:NSMakeRect(0, 1*itemHeight, spriteSheet.size.width, itemHeight)];
    [spriteSheet setName:@"hud_options" forSubimageInRect:NSMakeRect(0, 0, spriteSheet.size.width, itemHeight)];
    
    NSImage* fsImage = [NSImage imageNamed:@"hud_fullscreen_glyph"];
    [fsImage setName:@"hud_fullscreen_glyph_normal" forSubimageInRect:NSMakeRect(fsImage.size.width/2, 0, fsImage.size.width/2, fsImage.size.height)];
    [fsImage setName:@"hud_fullscreen_glyph_pressed" forSubimageInRect:NSMakeRect(0, 0, fsImage.size.width/2, fsImage.size.height)];
    
    NSImage* volume = [NSImage imageNamed:@"hud_volume"];
    [volume setName:@"hud_volume_down" forSubimageInRect:NSMakeRect(0, 0, 13, volume.size.height)];
    [volume setName:@"hud_volume_up" forSubimageInRect:NSMakeRect(13, 0, 15, volume.size.height)];
    
    
    NSImage* power = [NSImage imageNamed:@"hud_power_glyph"];
    [power setName:@"hud_power_glyph_normal" forSubimageInRect:(NSRect){{power.size.width/2,0}, {power.size.width/2, power.size.height}}];
    [power setName:@"hud_power_glyph_pressed" forSubimageInRect:(NSRect){{0,0}, {power.size.width/2, power.size.height}}];
}

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self) 
    {
        [self setWantsLayer:YES];
    }
    return self;
}

- (void)dealloc
{    
    [super dealloc];
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
    NSImage* barBackground = [NSImage imageNamed:@"hud_bar"];
    [barBackground drawInRect:[self bounds] fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil leftBorder:15 rightBorder:15 topBorder:0 bottomBorder:0];
}

- (void)setupControls
{
    NSButton* stopButton = [[NSButton alloc] init];
    OEHUDButtonCell* pcell = [[OEHUDButtonCell alloc] init];
    pcell.buttonColor = OEHUDButtonColorRed;
    [stopButton setCell:pcell];
    [stopButton setImage:[NSImage imageNamed:@"hud_power_glyph_normal"]];
    [stopButton setAlternateImage:[NSImage imageNamed:@"hud_power_glyph_pressed"]];
    [pcell release];
    [stopButton setTarget:[self window]];
    [stopButton setAction:@selector(stopAction:)];
    [stopButton setFrame:NSMakeRect(10, 13, 51, 23)];
    [stopButton setAutoresizingMask:NSViewMaxXMargin | NSViewMinYMargin];
    [self addSubview:stopButton];
    [stopButton release];
    
    
    OEImageButton* pauseButton = [[OEImageButton alloc] init];
    OEImageButtonHoverSelectable* cell = [[OEImageButtonHoverSelectable alloc] init];
    [pauseButton setCell:cell];
    [cell setImage:[NSImage imageNamed:@"hud_playpause"]];
    [cell release];
    [pauseButton setTarget:[self window]];
    [pauseButton setAction:@selector(playPauseAction:)];
    [pauseButton setFrame:NSMakeRect(82, 9, 32, 32)];
    [pauseButton setAutoresizingMask:NSViewMaxXMargin | NSViewMinYMargin];
    [self addSubview:pauseButton];
    [pauseButton release];
    
    
    OEImageButton* restartButton = [[OEImageButton alloc] init];
    OEImageButtonHoverPressed* hcell = [[OEImageButtonHoverPressed alloc] init];
    [restartButton setCell:hcell];
    [hcell setImage:[NSImage imageNamed:@"hud_restart"]];
    [hcell release];
    [restartButton setTarget:[self window]];
    [restartButton setAction:@selector(resetAction:)];
    [restartButton setFrame:NSMakeRect(111, 9, 32, 32)];
    [restartButton setAutoresizingMask:NSViewMaxXMargin | NSViewMinYMargin];
    [self addSubview:restartButton];
    [restartButton release];
    
    OEImageButton* saveButton = [[OEImageButton alloc] init];
    hcell = [[OEImageButtonHoverPressed alloc] init];
    [saveButton setCell:hcell];
    [hcell setImage:[NSImage imageNamed:@"hud_save"]];
    [hcell release];
    [saveButton setTarget:[self window]];
    [saveButton setAction:@selector(saveAction:)];
    [saveButton setFrame:NSMakeRect(162, 6, 32, 32)];
    [saveButton setAutoresizingMask:NSViewMaxXMargin | NSViewMinYMargin];
    [self addSubview:saveButton];
    [saveButton release];
    
    BOOL hideOptions = [[NSUserDefaults standardUserDefaults] boolForKey:UDHUDHideOptionsKey];
    if(!hideOptions)
    {       
        OEImageButton* optionsButton = [[OEImageButton alloc] init];
        hcell = [[OEImageButtonHoverPressed alloc] init];
        [optionsButton setCell:hcell];
        [hcell setImage:[NSImage imageNamed:@"hud_options"]];
        [hcell release];
        [optionsButton setTarget:[self window]];
        [optionsButton setAction:@selector(optionsAction:)];
        [optionsButton setFrame:NSMakeRect(212, 6, 32, 32)];
        [optionsButton setAutoresizingMask:NSViewMaxXMargin | NSViewMinYMargin];
        [self addSubview:optionsButton];
        [optionsButton release];
    }    
    
    NSButton* volumeDownView = [[NSButton alloc] initWithFrame:NSMakeRect(223+(hideOptions?0:50), 17, 13, 14)];
    [volumeDownView setBordered:NO];
    [[volumeDownView cell] setHighlightsBy:NSNoCellMask];
    [volumeDownView setImage:[NSImage imageNamed:@"hud_volume_down"]];
    [volumeDownView setTarget:[self window]];
    [volumeDownView setAction:@selector(muteAction:)];    
    [self addSubview:volumeDownView];
    [volumeDownView release];
    
    NSButton* volumeUpView = [[NSButton alloc] initWithFrame:NSMakeRect(320+(hideOptions?0:50), 17, 15, 14)];
    [volumeUpView setBordered:NO];
    [[volumeUpView cell] setHighlightsBy:NSNoCellMask];
    [volumeUpView setImage:[NSImage imageNamed:@"hud_volume_up"]];
    [volumeUpView setTarget:[self window]];
    [volumeUpView setAction:@selector(unmuteAction:)];
    [self addSubview:volumeUpView];
    [volumeUpView release];
    
    slider = [[OEHUDSlider alloc] initWithFrame:NSMakeRect(240+(hideOptions?0:50), 13, 80, 23)];
    
    OEHUDSliderCell* sliderCell = [[OEHUDSliderCell alloc] init];
    [slider setCell:sliderCell];
    [sliderCell release];
    [slider setContinuous:YES];
    [slider setMaxValue:1.0];
    [slider setMinValue:0.0];
    [slider setFloatValue:[[NSUserDefaults standardUserDefaults] floatForKey:UDVolumeKey]];
    [slider setTarget:[self window]];
    [slider setAction:@selector(volumeAction:)];
    
    [self addSubview:slider];
    [slider release];
    
    
    NSButton* fsButton = [[NSButton alloc] init];
    pcell = [[OEHUDButtonCell alloc] init];
    [fsButton setCell:pcell];
    [pcell release];
    
    [fsButton setImage:[NSImage imageNamed:@"hud_fullscreen_glyph_normal"]];
    [fsButton setAlternateImage:[NSImage imageNamed:@"hud_fullscreen_glyph_pressed"]];
    
    [fsButton setTarget:[self window]];
    [fsButton setAction:@selector(fullscreenAction:)];
    
    [fsButton setFrame:NSMakeRect(370+(hideOptions?0:50), 13, 51, 23)];
    [fsButton setAutoresizingMask:NSViewMaxXMargin | NSViewMinYMargin];
    [self addSubview:fsButton];
    [fsButton release];
    [fsButton setTitle:@""];
}
@end
