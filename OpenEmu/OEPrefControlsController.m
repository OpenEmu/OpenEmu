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
#import "OEDBSystem.h"
#import "OESystemPlugin.h"
#import "OESystemController.h"

#import "OEControlsViewController.h"
#import "OEControllerImageView.h"
@interface OEPrefControlsController (Private)
- (void)_rebuildSystemsMenu;
- (void)_rebuildInputMenu;
@end

@implementation OEPrefControlsController
#pragma mark Properties
@synthesize controllerView;

@synthesize consolesPopupButton, playerPopupButton, inputPopupButton;

@synthesize gradientOverlay;
@synthesize controlsContainer;
#pragma mark -
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {}
    
    return self;
}

- (void)dealloc{
    self.controllerView = nil;
    
    self.consolesPopupButton = nil;
    self.playerPopupButton = nil;
    self.inputPopupButton = nil;
    
    self.gradientOverlay = nil;
    self.controlsContainer = nil;
    
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    
    [super dealloc];
}
#pragma mark -
#pragma mark ViewController Overrides
- (void)awakeFromNib
{
    NSUserDefaults* sud = [NSUserDefaults standardUserDefaults];
    
    NSImage* controlsBackgroundImage = [NSImage imageNamed:@"controls_background"];
    [(OEBackgroundImageView*)[self view] setImage:controlsBackgroundImage];
    
    /** ** ** ** ** ** ** ** **/
    // Setup controls popup console list
    [self _rebuildSystemsMenu];
    
    // restore previous state
    NSInteger binding = [sud integerForKey:UDControlsDeviceTypeKey];
    [[self inputPopupButton] selectItemWithTag:binding];
    [self changeInputDevice:self];
    
    NSString* pluginName = [sud stringForKey:UDControlsPluginIdentifierKey];
    [[self consolesPopupButton] selectItemAtIndex:0];
    for(NSMenuItem* anItem in [[self consolesPopupButton] itemArray])
    {
        if([[anItem representedObject] isEqualTo:pluginName])
        {
            [[self consolesPopupButton] selectItem:anItem];
            break;
        }
    }
    
    [CATransaction setDisableActions:YES];
    [self changeSystem:[self consolesPopupButton]];
    [CATransaction commit];
    
    [self gradientOverlay].topColor = [NSColor colorWithDeviceWhite:0.0 alpha:0.3];
    [self gradientOverlay].bottomColor = [NSColor colorWithDeviceWhite:0.0 alpha:0.0];
    
    [[self controllerView] setWantsLayer:YES];    
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(bindingTypeChanged:) name:@"OEControlsViewControllerChangedBindingType" object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(systemsChanged) name:OEDBSystemsChangedNotificationName object:nil];
}

- (void)animationDidStart:(CAAnimation *)theAnimation
{
    [[self controllerView].layer setValue:[NSNumber numberWithFloat:1.0] forKeyPath:@"filters.pixellate.inputScale"];
}

- (void)animationDidStop:(CAAnimation *)theAnimation finished:(BOOL)flag
{
    if(flag)
    {          
        [[self controllerView].layer setValue:[NSNumber numberWithInt:10.0] forKeyPath:@"filters.pixellate.inputScale"];
    }
}

- (NSString*)nibName
{
    return @"OEPrefControlsController";
}

- (void)systemsChanged
{
    NSLog(@"OEDBSystemsChangedNotificationName");
    NSMenuItem* menuItem = [[self consolesPopupButton] selectedItem];
    NSString* selectedSystemIdentifier = [menuItem representedObject];

    [self _rebuildSystemsMenu];
    
    [[self consolesPopupButton] selectItemAtIndex:0];
    for(NSMenuItem* anItem in [[self consolesPopupButton] itemArray])
    {
        if([[anItem representedObject] isEqualTo:selectedSystemIdentifier])
        {
            [[self consolesPopupButton] selectItem:anItem];
            break;
        }
    }
    
    [CATransaction setDisableActions:YES];
    [self changeSystem:[self consolesPopupButton]];
    [CATransaction commit];
}
#pragma mark -
#pragma mark UI Methods
- (IBAction)changeSystem:(id)sender
{
    NSUserDefaults* sud = [NSUserDefaults standardUserDefaults];
    
    NSMenuItem *menuItem = [[self consolesPopupButton] selectedItem];
    NSString *systemIdentifier = [menuItem representedObject];
    
    NSString *oldPluginName = [selectedPlugin systemName];
    
    OESystemPlugin *nextPlugin = [OESystemPlugin gameSystemPluginForIdentifier:systemIdentifier];
    if(selectedPlugin!=nil && nextPlugin==selectedPlugin) return;
    selectedPlugin = nextPlugin;
    
    OESystemController *systemController = [selectedPlugin controller];
    
    // Rebuild player menu
    NSUInteger numberOfPlayers = [systemController numberOfPlayers];
    NSMenu *playerMenu = [[NSMenu alloc] init];
    for(NSUInteger player=0; player<numberOfPlayers; player++)
    {
        NSString *playerTitle = [NSString stringWithFormat:NSLocalizedString(@"Player %ld", @""), player+1];
        NSMenuItem *playerItem = [[NSMenuItem alloc] initWithTitle:playerTitle action:NULL keyEquivalent:@""];
        [playerItem setTag:player+1];
        [playerMenu addItem:playerItem];
        [playerItem release];
    }
    [[self playerPopupButton] setMenu:playerMenu];
    [playerMenu release];
    
    // Hide player PopupButton if there is only one player
    [[self playerPopupButton] setHidden:(numberOfPlayers==1)];
    [[self playerPopupButton] selectItemWithTag:[sud integerForKey:UDControlsPlayerKey]];
    
    OEControlsViewController* preferenceViewController = (OEControlsViewController*)[systemController preferenceViewControllerForKey:OEControlsPreferenceKey];
    
    NSView* preferenceView = [preferenceViewController view];
    [preferenceView setAutoresizingMask:NSViewMaxXMargin|NSViewMaxYMargin];
    NSRect rect = (NSRect){{0,0}, {[self controlsContainer].bounds.size.width, preferenceView.frame.size.height}};
    [preferenceView setFrame:rect];
    [[self controlsContainer] setFrame:rect];
    if([[[self controlsContainer] subviews] count])
        [[[self controlsContainer] animator] replaceSubview:[[[self controlsContainer] subviews] objectAtIndex:0] with:preferenceView];
    else
        [[[self controlsContainer] animator] addSubview:preferenceView];
    
    NSScrollView* scrollView = [[self controlsContainer] enclosingScrollView];
    [[self controlsContainer] setFrameOrigin:(NSPoint){0,scrollView.frame.size.height-rect.size.height}];
    if([self controlsContainer].frame.size.height<=scrollView.frame.size.height)
    {
        [scrollView setVerticalScrollElasticity:NSScrollElasticityNone];
    }
    else
    {
        [scrollView setVerticalScrollElasticity:NSScrollElasticityAutomatic];
        [scrollView flashScrollers];
    }
    
    [sud setObject:systemIdentifier forKey:UDControlsPluginIdentifierKey];
    
    [self changePlayer:[self playerPopupButton]];
    [self changeInputDevice:[self inputPopupButton]];
    
    OEControllerImageView* newControllerView = [[OEControllerImageView alloc] initWithFrame:[[self controllerView] bounds]];
    [newControllerView setImage:[preferenceViewController controllerImage]];
    [newControllerView setControlsViewController:preferenceViewController];
    
    // Animation for controller image swapping
    NSComparisonResult order = [oldPluginName compare:[selectedPlugin systemName]];
    CATransition *controllerTransition = [CATransition animation];
    controllerTransition.type = kCATransitionPush;
    controllerTransition.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionDefault];
    controllerTransition.duration = 1.0;
    controllerTransition.subtype = (order == NSOrderedDescending ? kCATransitionFromLeft : kCATransitionFromRight);
    
    [[self controllerView] setAnimations:[NSDictionary dictionaryWithObject:controllerTransition forKey:@"subviews"]];
    
    if([[[self controllerView] subviews] count])
    {
        [[[self controllerView] animator] replaceSubview:[[[self controllerView] subviews] objectAtIndex:0] with:newControllerView];
    }
    else
        [[[self controllerView] animator] addSubview:newControllerView];
    
    [newControllerView release];
    
    [[self controllerView] setAnimations:[NSDictionary dictionary]];
}

- (IBAction)changePlayer:(id)sender
{
    NSInteger player = 1;
    if(sender && [sender respondsToSelector:@selector(selectedTag)])
    {
        player = [sender selectedTag];
    } 
    else if(sender && [sender respondsToSelector:@selector(tag)])
    {
        player = [sender tag];
    }
    
    if(selectedPlugin)
    {
        OESystemController* systemController = [selectedPlugin controller];
        OEControlsViewController* preferenceViewController = (OEControlsViewController*)[systemController preferenceViewControllerForKey:OEControlsPreferenceKey];
        [preferenceViewController selectPlayer:player];
    }
    
    NSUserDefaults* sud = [NSUserDefaults standardUserDefaults];
    [sud setInteger:player forKey:UDControlsPlayerKey];
}

- (IBAction)changeInputDevice:(id)sender
{
    NSInteger bindingType = 0;
    if(sender && [sender respondsToSelector:@selector(selectedTag)])
    {
        bindingType = [sender selectedTag];
    }
    else if(sender && [sender respondsToSelector:@selector(tag)])
    {
        bindingType = [sender tag];
    }
    
    if(selectedPlugin)
    {
        OESystemController* systemController = [selectedPlugin controller];
        OEControlsViewController* preferenceViewController = (OEControlsViewController*)[systemController preferenceViewControllerForKey:OEControlsPreferenceKey];
        [preferenceViewController selectBindingType:bindingType];
    }
    NSUserDefaults* sud = [NSUserDefaults standardUserDefaults];
    [sud setInteger:bindingType forKey:UDControlsDeviceTypeKey];
}

#pragma mark -
#pragma mark OEPreferencePane Protocol
- (NSImage*)icon
{
    return [NSImage imageNamed:@"controls_tab_icon"];
}

- (NSString*)title
{
    return @"Controls";
}

- (NSSize)viewSize
{
    return NSMakeSize(561, 534);
}

- (NSColor*)toolbarSeparationColor
{
    return [NSColor colorWithDeviceWhite:0.32 alpha:1.0];
}

#pragma mark -
- (void)_rebuildSystemsMenu
{
    NSMenu* consolesMenu = [[NSMenu alloc] init];
    
    NSArray* enabledSystems = [[OELibraryDatabase defaultDatabase] enabledSystems]; 

    for(OEDBSystem* system in enabledSystems)
    {
        OESystemPlugin* plugin = [system plugin];
        
        NSMenuItem* item = [[NSMenuItem alloc] initWithTitle:[plugin systemName] action:@selector(changeSystem:) keyEquivalent:@""];
        [item setTarget:self];
        [item setRepresentedObject:[plugin systemIdentifier]];
        
        [item setImage:[NSImage imageNamed:[item title]]];
        
        [consolesMenu addItem:item];
        [item release];
    }
    
    [[self consolesPopupButton] setMenu:consolesMenu];
    [consolesMenu release];
}

- (void)_rebuildInputMenu
{
    
}
#pragma mark -
- (void)bindingTypeChanged:(id)sender
{
    id object = [sender object];
    [[self inputPopupButton] selectItemWithTag:[object selectedBindingType]];
}
@end
