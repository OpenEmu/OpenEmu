//
//  PreferencesController.m
//  OEPreferencesMockup
//
//  Created by Christoph Leimbrock on 17.05.11.
//  Copyright 2011 none. All rights reserved.
//

#import "PreferencesController.h"
#import "OEBackgroundImageView.h"
#import "OEBackgroundGradientView.h"

#import "OEToolbarView.h"
#import "INAppStoreWindow.h"

#import "NSImage+OEDrawingAdditions.h"
@interface PreferencesController (priavte)
- (void)_showView:(NSView*)view atSize:(NSSize)size;
@end
@implementation PreferencesController

- (id)initWithWindow:(NSWindow *)window{
    self = [super initWithWindow:window];
    if (self) {
    }
    
    return self;
}

- (void)dealloc{
	[toolbar release];
	toolbar = nil;
	
	
    [super dealloc];
}

- (NSString*)windowNibName{
	return @"PreferencesNew";
}

- (void)windowDidLoad{
    [super windowDidLoad];
	
	INAppStoreWindow* win = (INAppStoreWindow*)[self window];	
	
	NSColor* windowBackgroundColor = [NSColor colorWithDeviceRed:0.149 green:0.149 blue:0.149 alpha:1.0];
	[win setBackgroundColor:windowBackgroundColor];
	
	NSImage* controlsBackgroundImage = [NSImage imageNamed:@"controls_background"];
	[(OEBackgroundImageView*)controls setImage:controlsBackgroundImage];
	
	NSColor* controlsTopLineColor = [NSColor colorWithDeviceWhite:0.32 alpha:1.0];
	[(OEBackgroundImageView*)controls setTopLineColor:controlsTopLineColor];
	
	toolbar = [[OEToolbarView alloc] initWithFrame:NSMakeRect(0, 0, win.frame.size.width-10, 58)];
	
	coreGradientOverlayView.topColor = [NSColor colorWithDeviceWhite:0.0 alpha:0.3];
	coreGradientOverlayView.bottomColor = [NSColor colorWithDeviceWhite:0.0 alpha:0.0];
	
	// Setup Toolbar
	OEToolbarItem* libraryItem = [[[OEToolbarItem alloc] init] autorelease];
	[libraryItem setTitle:@"Library"];
	[libraryItem setIcon:[NSImage imageNamed:@"tb_pref_library"]];
	[libraryItem setTarget:self];
	[libraryItem setAction:@selector(showLibrary:)];
	[toolbar addItem:libraryItem];
	
	
	OEToolbarItem* gameplayItem = [[[OEToolbarItem alloc] init] autorelease];
	[gameplayItem setTitle:@"Gameplay"];
	[gameplayItem setIcon:[NSImage imageNamed:@"tb_pref_gameplay"]];
	[gameplayItem setTarget:self];
	[gameplayItem setAction:@selector(showGameplay:)];
	[toolbar addItem:gameplayItem];
	
	
	OEToolbarItem* controlsItem = [[[OEToolbarItem alloc] init] autorelease];
	[controlsItem setTitle:@"Controls"];
	[controlsItem setIcon:[NSImage imageNamed:@"tb_pref_controls"]];
	[controlsItem setTarget:self];
	[controlsItem setAction:@selector(showControls:)];
	[toolbar addItem:controlsItem];
	
	
	OEToolbarItem* coresItem = [[[OEToolbarItem alloc] init] autorelease];
	[coresItem setTitle:@"Cores"];
	[coresItem setIcon:[NSImage imageNamed:@"tb_pref_cores"]];
	[coresItem setTarget:self];
	[coresItem setAction:@selector(showCores:)];
	[toolbar addItem:coresItem];
		
	win.titleBarString = @"Preferences";
	win.titleBarHeight = 83;
	win.titleBarView = toolbar;
	win.trafficLightAlignment = 0;

	/** ** ** ** ** ** ** ** **/
	// Setup controls popup console list
	NSMenu* consolesMenu = [[NSMenu alloc] init];
	
	NSMenuItem* item = [[[NSMenuItem alloc] initWithTitle:@"Nintendo (NES)" action:NULL keyEquivalent:@""] autorelease];
	[item setImage:[NSImage imageNamed:[item title]]];
	[consolesMenu addItem:item];
	
	item = [[[NSMenuItem alloc] initWithTitle:@"Super Nintendo (SNES)" action:NULL keyEquivalent:@""] autorelease];
	[item setImage:[NSImage imageNamed:[item title]]];
	[consolesMenu addItem:item];
	
	item = [[[NSMenuItem alloc] initWithTitle:@"GameBoy" action:NULL keyEquivalent:@""] autorelease];
	[item setImage:[NSImage imageNamed:[item title]]];
	[consolesMenu addItem:item];
	
	item = [[[NSMenuItem alloc] initWithTitle:@"GameBoy Advance" action:NULL keyEquivalent:@""] autorelease];
	[item setImage:[NSImage imageNamed:[item title]]];
	[consolesMenu addItem:item];
	
	item = [[[NSMenuItem alloc] initWithTitle:@"Sega SG-1000" action:NULL keyEquivalent:@""] autorelease];
	[item setImage:[NSImage imageNamed:[item title]]];
	[consolesMenu addItem:item];
	
	item = [[[NSMenuItem alloc] initWithTitle:@"Sega Master System" action:NULL keyEquivalent:@""] autorelease];
	[item setImage:[NSImage imageNamed:[item title]]];
	[consolesMenu addItem:item];
	
	item = [[[NSMenuItem alloc] initWithTitle:@"Sega Genesis" action:NULL keyEquivalent:@""] autorelease];
	[item setImage:[NSImage imageNamed:[item title]]];
	[consolesMenu addItem:item];
	
	item = [[[NSMenuItem alloc] initWithTitle:@"Game Gear" action:NULL keyEquivalent:@""] autorelease];
	[item setImage:[NSImage imageNamed:[item title]]];
	[consolesMenu addItem:item];
	
	[consolesPopupButton setMenu:consolesMenu];

	[consolesMenu release];
	/** ** ** ** ** ** ** ** **/
	[self showLibrary:nil];
}
#pragma mark -
- (IBAction)showLibrary:(id)sender{
	toolbar.contentSeperatorColor = [NSColor blackColor];
	
	NSSize viewSize = NSMakeSize(423, 480);
	[self _showView:library atSize:viewSize];
}

- (IBAction)showGameplay:(id)sender{
	toolbar.contentSeperatorColor = [NSColor blackColor];
	
	NSSize viewSize = NSMakeSize(423, 347);
	[self _showView:gameplay atSize:viewSize];
}

- (IBAction)showControls:(id)sender{
	toolbar.contentSeperatorColor = [NSColor colorWithDeviceRed:0.318 green:0.318 blue:0.318 alpha:1.0];

	NSSize viewSize = NSMakeSize(561, 536);
	[self _showView:controls atSize:viewSize];
}

- (IBAction)showCores:(id)sender{
	toolbar.contentSeperatorColor = [NSColor blackColor];
	
	NSSize viewSize = NSMakeSize(423, 480);
	
	[self _showView:cores atSize:viewSize];
}

- (void)_showView:(NSView*)view atSize:(NSSize)size{
	NSWindow* win = [self window];
	
	[view setFrameSize:size];

	[win setContentSize:size];
	[win setContentView:view];
}

@end
