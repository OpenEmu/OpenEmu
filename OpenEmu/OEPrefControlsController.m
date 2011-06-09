//
//  OEPrefControlsController.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 07.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OEPrefControlsController.h"
#import "OEBackgroundGradientView.h"
#import "OEBackgroundImageView.h"
#import "LibraryDatabase.h"

@implementation OEPrefControlsController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {}
    
    return self;
}

- (void)dealloc
{
    [super dealloc];
}
#pragma mark ViewController Overrides
- (void)awakeFromNib{
	NSImage* controlsBackgroundImage = [NSImage imageNamed:@"controls_background"];
	[(OEBackgroundImageView*)[self view] setImage:controlsBackgroundImage];
	
	NSColor* controlsTopLineColor = [NSColor colorWithDeviceWhite:0.32 alpha:1.0];
	[(OEBackgroundImageView*)[self view] setTopLineColor:controlsTopLineColor];
	
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
	
	gradientOverlay.topColor = [NSColor colorWithDeviceWhite:0.0 alpha:0.3];
	gradientOverlay.bottomColor = [NSColor colorWithDeviceWhite:0.0 alpha:0.0];
}
- (NSString*)nibName{
	return @"OEPrefControlsController";
}

#pragma mark OEPreferencePane Protocol
- (NSImage*)icon{
	return [NSImage imageNamed:@"tb_pref_controls"];
}

- (NSString*)title{
	return @"Controls";
}

- (NSSize)viewSize{
	return NSMakeSize(561, 536);
}

- (NSColor*)toolbarSeparationColor{
	return [NSColor colorWithDeviceWhite:0.32 alpha:1.0];
}
@end
