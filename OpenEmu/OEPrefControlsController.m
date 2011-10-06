//
//  OEPrefControlsController.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 07.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import <Quartz/Quartz.h>

#import "OEPrefControlsController.h"
#import "OEBackgroundGradientView.h"
#import "OEBackgroundImageView.h"
#import "OELibraryDatabase.h"

#import "OEPlugin.h"
#import "OESystemPlugin.h"
#import "OESystemController.h"

#import "OEControlsViewController.h"
#import "OEControlerImageView.h"
@interface OEPrefControlsController (Private)
- (void)_rebuildSystemsMenu;
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
    NSUserDefaults* sud = [NSUserDefaults standardUserDefaults];
    
	NSImage* controlsBackgroundImage = [NSImage imageNamed:@"controls_background"];
	[(OEBackgroundImageView*)[self view] setImage:controlsBackgroundImage];
	
	NSColor* controlsTopLineColor = [NSColor colorWithDeviceWhite:0.32 alpha:1.0];
	[(OEBackgroundImageView*)[self view] setTopLineColor:controlsTopLineColor];
	
	/** ** ** ** ** ** ** ** **/
	// Setup controls popup console list
	[self _rebuildSystemsMenu];
    
    // restore previous state
    NSInteger binding = [sud integerForKey:UDControlsDeviceTypeKey];
    [inputPopupButton selectItemWithTag:binding];
    [self changeInputDevice:self];
    
    NSString* pluginName = [sud stringForKey:UDControlsPluginNameKey];
    if(pluginName && [consolesPopupButton itemWithTitle:pluginName]){
        [consolesPopupButton selectItemWithTitle:pluginName];
    } else {
        [consolesPopupButton selectItemAtIndex:0];
    }
    [self changeSystem:consolesPopupButton];
   	
	gradientOverlay.topColor = [NSColor colorWithDeviceWhite:0.0 alpha:0.3];
	gradientOverlay.bottomColor = [NSColor colorWithDeviceWhite:0.0 alpha:0.0];

    // Animation for controller image swapping
    CATransition *controllerTransition = [CATransition animation];
    controllerTransition.type = kCATransitionPush;
    controllerTransition.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionDefault];
    controllerTransition.duration = 1.0;
    
    [controllerView setWantsLayer:YES];    
    [controllerView setAnimations: [NSDictionary dictionaryWithObject:controllerTransition forKey:@"subviews"]];
       
    // The following enables the pixelation animation, if you want it.
//    CIFilter *pixellate = [CIFilter filterWithName:@"CIPixellate"];
//    [pixellate setDefaults];
//    [pixellate setValue:[NSNumber numberWithInt:1.0] forKeyPath:@"inputScale"];
//    pixellate.name = @"pixellate";
//
//    controllerView.layer.filters = [NSArray arrayWithObject:pixellate];
//    transition.delegate = self;

    CATransition *controlsTransition = [CATransition animation];
    controlsTransition.type = kCATransitionFade;
    controlsTransition.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionDefault];
    controlsTransition.duration = 1.0;

    [controlsContainer setWantsLayer:YES];
    [controlsContainer setAnimations: [NSDictionary dictionaryWithObject:controlsTransition forKey:@"subviews"]];

    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(bindingTypeChanged:) name:@"OEControlsViewControllerChangedBindingType" object:nil];
}

- (void)animationDidStart:(CAAnimation *)theAnimation
{
    [controllerView.layer setValue:[NSNumber numberWithFloat:1.0] forKeyPath:@"filters.pixellate.inputScale"];
}

- (void)animationDidStop:(CAAnimation *)theAnimation finished:(BOOL)flag
{
    if(flag)
    {          
        [controllerView.layer setValue:[NSNumber numberWithInt:10.0] forKeyPath:@"filters.pixellate.inputScale"];
    }
}

- (NSString*)nibName{
	return @"OEPrefControlsController";
}

#pragma mark -
#pragma mark UI Methods
- (IBAction)changeSystem:(id)sender{
    NSUserDefaults* sud = [NSUserDefaults standardUserDefaults];

	NSMenuItem* menuItem = [consolesPopupButton selectedItem];
	NSString* systemIdentifier = [menuItem representedObject];
	
	OESystemPlugin* nextPlugin = [OESystemPlugin gameSystemPluginForIdentifier:systemIdentifier];
	if(selectedPlugin!=nil && nextPlugin==selectedPlugin) return;
	selectedPlugin = nextPlugin;
	
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
    [playerPopupButton selectItemWithTag:[sud integerForKey:UDControlsPlayerKey]];
    
	OEControlsViewController* preferenceViewController = (OEControlsViewController*)[systemController preferenceViewControllerForKey:OEControlsPreferenceKey];
    
	NSView* preferenceView = [preferenceViewController view];
	[preferenceView setFrame:[controlsContainer bounds]];
	if([[controlsContainer subviews] count])
        [[controlsContainer animator] replaceSubview:[[controlsContainer subviews] objectAtIndex:0] with:preferenceView];

	[[controlsContainer animator] addSubview:preferenceView];
    
    [sud setObject:systemIdentifier forKey:UDControlsPluginNameKey];
    
    [self changePlayer:playerPopupButton];
    [self changeInputDevice:inputPopupButton];
    
    OEControlerImageView* newControllerView = [[OEControlerImageView alloc] initWithFrame:[controllerView bounds]];
	[newControllerView setImage:[preferenceViewController controllerImage]];
	
    if([[controllerView subviews] count])
    {
        [[controllerView animator] replaceSubview:[[controllerView subviews] objectAtIndex:0] with:newControllerView];
    }
    else
        [[controllerView animator] addSubview:newControllerView];
    
    [newControllerView release];

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
    
    NSUserDefaults* sud = [NSUserDefaults standardUserDefaults];
    [sud setInteger:player forKey:UDControlsPlayerKey];
}

- (IBAction)changeInputDevice:(id)sender{
    NSInteger bindingType = 0;
	if(sender && [sender respondsToSelector:@selector(selectedTag)]){
		bindingType = [sender selectedTag];
	} else if(sender && [sender respondsToSelector:@selector(tag)]){
		bindingType = [sender tag];
	}
    
    if(selectedPlugin){
		OESystemController* systemController = [selectedPlugin controller];
		OEControlsViewController* preferenceViewController = (OEControlsViewController*)[systemController preferenceViewControllerForKey:OEControlsPreferenceKey];
		[preferenceViewController selectBindingType:bindingType];		
	}
    NSUserDefaults* sud = [NSUserDefaults standardUserDefaults];
    [sud setInteger:bindingType forKey:UDControlsDeviceTypeKey];
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
	NSArray* plugins = [OEPlugin pluginsForType:[OESystemPlugin class]];
    NSArray* sortedPlugins = [plugins sortedArrayUsingComparator:^NSComparisonResult(id obj1, id obj2) {
        return [[obj1 systemName] compare:[obj2 systemName]];
    }];
    
	for(OESystemPlugin* plugin in sortedPlugins){
		NSMenuItem* item = [[NSMenuItem alloc] initWithTitle:[plugin systemName] action:@selector(changeSystem:) keyEquivalent:@""];
		[item setTarget:self];
		[item setRepresentedObject:[plugin systemIdentifier]];
		
		if([plugin icon]) [item setImage:[plugin icon]];
		else [item setImage:[NSImage imageNamed:[item title]]]; // TODO: remove if/else, only keep [plugin icon]
		
		[consolesMenu addItem:item];
		[item release];
	}

	[consolesPopupButton setMenu:consolesMenu];	
	[consolesMenu release];
}

- (void)_rebuildInputMenu{
	
}
#pragma mark -
- (void)bindingTypeChanged:(id)sender{
    id object = [sender object];
    [inputPopupButton selectItemWithTag:[object selectedBindingType]];
}
@end
