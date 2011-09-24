//
//  OEOSDControls.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 12.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OEHUDControls.h"
#import "NSImage+OEDrawingAdditions.h"

#import "OEImageButton.h"
#import "OEHUDButtonCell.h"
#import "OEHUDSlider.h"

#import "OENewGameDocument.h"
#import "OEMenu.h"
@implementation OEHUDControlsWindow
- (id)initWithGameDocument:(OENewGameDocument*)doc{
    self = [super initWithContentRect:NSMakeRect(0, 0, 431, 45) styleMask:NSBorderlessWindowMask backing:NSWindowBackingLocationDefault defer:YES];
    if (self) {
		self.gameDocument = doc;
		
		[self setOpaque:NO];
		[self setBackgroundColor:[NSColor clearColor]];
		
		OEHUDControlsView* controlsView = [[OEHUDControlsView alloc] initWithFrame:NSMakeRect(0, 0, 431, 45)];
		[[self contentView] addSubview:controlsView];
		[controlsView setupControls];
		[controlsView release];
	}
    return self;
}

- (void)resetAction:(id)sender{
	[self.gameDocument resetGame];
}

- (void)playPauseAction:(id)sender{
	[self.gameDocument setPauseEmulation:![self.gameDocument isEmulationPaused]];
}

- (void)saveAction:(id)sender{
	NSMenu* menu = [[NSMenu alloc] init];
	
	NSMenuItem* item;
	item = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Save Current Game", @"") action:NULL keyEquivalent:@""];
	[menu addItem:item];
	[item release];
	
	
	NSArray* saveStates;
	if(self.gameDocument.rom && (saveStates=[self.gameDocument.rom valueForKey:@"saveStates"]) && [saveStates count]){
		[menu addItem:[NSMenuItem separatorItem]];
		for(id saveState in saveStates){
			
			NSString* itemTitle = [saveState valueForKey:@"userDescription"];
			if(!itemTitle || [itemTitle isEqualToString:@""]){
				itemTitle = [NSString stringWithFormat:@"%@", [saveState valueForKey:@"timeStamp"]];
			}
			
			item = [[NSMenuItem alloc] initWithTitle:itemTitle action:NULL keyEquivalent:@""];
			[menu addItem:item];
			[item release];
		}
	}
	
	
	OEMenu* oemenu = [menu convertToOEMenu];
	NSRect buttonRect = [sender frame];
	NSPoint menuPoint = NSMakePoint(NSMaxX(buttonRect)+self.frame.origin.x, NSMinY(buttonRect)+self.frame.origin.y);
	[oemenu openAtPoint:menuPoint ofWindow:self];
	[menu release];
}

- (void)doneAction:(id)sender{
	[[self animator] setAlphaValue:0.0];
}

- (void)fullscreenAction:(id)sender{
	[[self parentWindow] toggleFullScreen:nil];
}

@synthesize gameDocument;
@end

@implementation OEHUDControlsView

+ (void)initialize{
	NSImage* spriteSheet = [NSImage imageNamed:@"hud_glyphs"];

	[spriteSheet setName:@"hud_playpause" forSubimageInRect:NSMakeRect(0, 56, spriteSheet.size.width, 56)];
	[spriteSheet setName:@"hud_restart" forSubimageInRect:NSMakeRect(0, 28, spriteSheet.size.width, 28)];
	[spriteSheet setName:@"hud_save" forSubimageInRect:NSMakeRect(0, 0, spriteSheet.size.width, 28)];
	
	NSImage* fsImage = [NSImage imageNamed:@"hud_fullscreen_glyph"];
	[fsImage setName:@"hud_fullscreen_glyph_normal" forSubimageInRect:NSMakeRect(fsImage.size.width/2, 0, fsImage.size.width/2, fsImage.size.height)];
	[fsImage setName:@"hud_fullscreen_glyph_pressed" forSubimageInRect:NSMakeRect(0, 0, fsImage.size.width/2, fsImage.size.height)];
	
	NSImage* volume = [NSImage imageNamed:@"hud_volume"];
	[volume setName:@"hud_volume_down" forSubimageInRect:NSMakeRect(0, 0, volume.size.width/2, volume.size.height)];
	[volume setName:@"hud_volume_up" forSubimageInRect:NSMakeRect(volume.size.width/2, 0, volume.size.width/2, volume.size.height)];
}

- (id)initWithFrame:(NSRect)frame{
    self = [super initWithFrame:frame];
    if (self) {
		[self setWantsLayer:TRUE];
	}
    return self;
}

- (void)dealloc{
    [super dealloc];
}

- (void)awakeFromNib{
}

- (BOOL)isOpaque{
	return NO;
}

#pragma mark -
- (void)drawRect:(NSRect)dirtyRect{
	NSImage* barBackground = [NSImage imageNamed:@"hud_bar"];
	[barBackground drawInRect:[self bounds] fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:TRUE hints:nil leftBorder:15 rightBorder:15 topBorder:0 bottomBorder:0];
}

- (void)setupControls{	
	OEImageButton* pauseButton = [[OEImageButton alloc] init];
	OEImageButtonHoverSelectable* cell = [[OEImageButtonHoverSelectable alloc] init];
	[pauseButton setCell:cell];
	[cell setImage:[NSImage imageNamed:@"hud_playpause"]];
	[cell release];
		
	[pauseButton setTarget:[self window]];
	[pauseButton setAction:@selector(playPauseAction:)];
	[pauseButton setFrame:NSMakeRect(83, 9, 28, 28)];	
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
	[restartButton setFrame:NSMakeRect(112, 11, 28, 28)];	
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
	[saveButton setFrame:NSMakeRect(164, 9, 28, 28)];	
	[saveButton setAutoresizingMask:NSViewMaxXMargin | NSViewMinYMargin];
	[self addSubview:saveButton];
	[saveButton release];
		
	
	NSButton* doneButton = [[NSButton alloc] init];
	OEHUDButtonCell* pcell = [[OEHUDButtonCell alloc] init];
	[pcell setBlue:YES];
	[doneButton setCell:pcell];
	[pcell release];
	[doneButton setTitle:@"Done"];
	
	[doneButton setTarget:[self window]];
	[doneButton setAction:@selector(doneAction:)];
	[doneButton setFrame:NSMakeRect(9, 13, 51, 23)];	
	[doneButton setAutoresizingMask:NSViewMaxXMargin | NSViewMinYMargin];
	[self addSubview:doneButton];
	[doneButton release];

	
	NSButton* fsButton = [[NSButton alloc] init];
	pcell = [[OEHUDButtonCell alloc] init];
	[fsButton setCell:pcell];
	[pcell release];
	
	[fsButton setImage:[NSImage imageNamed:@"hud_fullscreen_glyph_normal"]];
	[fsButton setAlternateImage:[NSImage imageNamed:@"hud_fullscreen_glyph_pressed"]];

	[fsButton setTarget:[self window]];
	[fsButton setAction:@selector(fullscreenAction:)];

	[fsButton setFrame:NSMakeRect(370, 13, 51, 23)];	
	[fsButton setAutoresizingMask:NSViewMaxXMargin | NSViewMinYMargin];
	[self addSubview:fsButton];
	[fsButton release];
	[fsButton setTitle:@""];
	
	NSImageView* volumeDownView = [[NSImageView alloc] initWithFrame:NSMakeRect(222, 17, 14, 14)];
	[volumeDownView setImage:[NSImage imageNamed:@"hud_volume_down"]];
	[self addSubview:volumeDownView];
	[volumeDownView release];
	
	NSImageView* volumeUpView = [[NSImageView alloc] initWithFrame:NSMakeRect(320, 17, 14, 14)];
	[volumeUpView setImage:[NSImage imageNamed:@"hud_volume_up"]];
	[self addSubview:volumeUpView];
	[volumeUpView release];
	
	OEHUDSlider* slider = [[OEHUDSlider alloc] initWithFrame:NSMakeRect(240, 13, 80, 23)];
	
	OEHUDSliderCell* sliderCell = [[OEHUDSliderCell alloc] init];
	[slider setCell:sliderCell];
	[sliderCell release];
	[slider setContinuous:YES];
	[slider setMaxValue:1.0];
	[slider setMinValue:0.0];
	[slider bind:@"value" toObject:[NSUserDefaultsController sharedUserDefaultsController] withKeyPath:@"values.volume" options:nil];

	[self addSubview:slider];
	[slider release];
}
@end
