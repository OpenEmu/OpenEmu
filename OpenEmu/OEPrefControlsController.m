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

#import "OEPlugin.h"
#import "OESystemPlugin.h"
#import "OESystemController.h"

#import "OEControlsViewController.h"
@interface OEPrefControlsController (Private)
- (void)_rebuildSystemsMenu;
- (void)_rebuildPlayerMenu;
- (void)_rebuildInputMenu;
@end

@implementation OEPrefControlsController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {}
    
    return self;
}

- (void)dealloc{
    [super dealloc];
}
#pragma mark -
#pragma mark ViewController Overrides
- (void)awakeFromNib{
	NSImage* controlsBackgroundImage = [NSImage imageNamed:@"controls_background"];
	[(OEBackgroundImageView*)[self view] setImage:controlsBackgroundImage];
	
	NSColor* controlsTopLineColor = [NSColor colorWithDeviceWhite:0.32 alpha:1.0];
	[(OEBackgroundImageView*)[self view] setTopLineColor:controlsTopLineColor];
	
	/** ** ** ** ** ** ** ** **/
	// Setup controls popup console list
	[self _rebuildSystemsMenu];
	
	gradientOverlay.topColor = [NSColor colorWithDeviceWhite:0.0 alpha:0.3];
	gradientOverlay.bottomColor = [NSColor colorWithDeviceWhite:0.0 alpha:0.0];
}

- (NSString*)nibName{
	return @"OEPrefControlsController";
}

#pragma mark -
#pragma mark UI Methods
- (IBAction)changeSystem:(id)sender{	
	NSMenuItem* menuItem = [consolesPopupButton selectedItem];
	NSString* systemName = [menuItem title];
	selectedPlugin = [OESystemPlugin gameSystemPluginForName:systemName];
	
	OESystemController* systemController = [selectedPlugin controller];
	
	// Rebuild player menu
	NSUInteger numberOfPlayers = [systemController numberOfPlayers];
	NSMenu* playerMenu = [[NSMenu alloc] init];
	for(NSUInteger player=0; player<numberOfPlayers; player++){
		NSString* playerTitle = [NSString stringWithFormat:NSLocalizedString(@"Player %ld", @""), player+1];
		NSMenuItem* playerItem = [[NSMenuItem alloc] initWithTitle:playerTitle action:NULL keyEquivalent:@""];
		[playerItem setTag:player+1];
		[playerMenu addItem:playerItem];
		[playerItem release];		
	}
	[playerPopupButton setMenu:playerMenu];
	[playerMenu release];
	
	// Hide player PopupButton if there is only one player
	[playerPopupButton setHidden:(numberOfPlayers==1)];

	OEControlsViewController* preferenceViewController = (OEControlsViewController*)[systemController preferenceViewControllerForKey:OEControlsPreferenceKey];
	[controllerView setImage:[preferenceViewController controllerImage]];
	
	NSView* preferenceView = [preferenceViewController view];
	[preferenceView setFrame:[controlsContainer bounds]];
	if([[controlsContainer subviews] count])
		[[[controlsContainer subviews] lastObject] removeFromSuperview];
	[controlsContainer addSubview:preferenceView];
}

- (IBAction)changePlayer:(id)sender{
	NSInteger player = 1;
	if(sender && [sender respondsToSelector:@selector(selectedTag)]){
		player = [sender selectedTag];
	} else if(sender && [sender respondsToSelector:@selector(tag)]){
		player = [sender tag];
	}
	
	if(selectedPlugin){
		OESystemController* systemController = [selectedPlugin controller];
		OEControlsViewController* preferenceViewController = (OEControlsViewController*)[systemController preferenceViewControllerForKey:OEControlsPreferenceKey];
		[preferenceViewController selectPlayer:player];		
	}
}

- (IBAction)changeInputDevice:(id)sender{
}

#pragma mark -
#pragma mark OEPreferencePane Protocol
- (NSImage*)icon{
	return [NSImage imageNamed:@"controls_tab_icon"];
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

#pragma mark -
- (void)_rebuildSystemsMenu{	
	NSMenu* consolesMenu = [[NSMenu alloc] init];
	
	for(OESystemPlugin* plugin in [OEPlugin pluginsForType:[OESystemPlugin class]]){
		NSMenuItem* item = [[NSMenuItem alloc] initWithTitle:[plugin gameSystemName] action:@selector(changeSystem:) keyEquivalent:@""];
		[item setTarget:self];
		
		if([plugin icon]) [item setImage:[plugin icon]];
		else [item setImage:[NSImage imageNamed:[item title]]]; // TODO: remove if/else, only keep [plugin icon]
		
		[consolesMenu addItem:item];
		[item release];
	}

	[consolesPopupButton setMenu:consolesMenu];	
	[consolesMenu release];
}

- (void)_rebuildPlayerMenu{
	NSMenu* playerMenu = [[NSMenu alloc] init];
	
	int maxPlayerCount = 4; // TODO: get max player count from selected plugin
	for(int i=1; i <= maxPlayerCount; i++){
		NSString* title = [NSString stringWithFormat:@"%@ %d",NSLocalizedString(@"Player", @""), i];
		NSMenuItem* item = [[NSMenuItem alloc] initWithTitle:title action:NULL keyEquivalent:@""];

		[playerMenu addItem:item];
		[item release];
	}
	
	[playerPopupButton setMenu:playerMenu];
	[playerMenu release];
}

- (void)_rebuildInputMenu{
	
}
@end
