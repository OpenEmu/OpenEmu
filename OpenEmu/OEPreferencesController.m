//
//  PreferencesController.m
//  OEPreferencesMockup
//
//  Created by Christoph Leimbrock on 17.05.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OEPreferencesController.h"
#import <Quartz/Quartz.h>

#import "OEBackgroundImageView.h"
#import "OEBackgroundGradientView.h"

#import "OEToolbarView.h"
#import "INAppStoreWindow.h"

#import "NSImage+OEDrawingAdditions.h"

#import "OEPreferencePane.h"

#import "OEPrefLibraryController.h"
#import "OEPrefGameplayController.h"
#import "OEPrefControlsController.h"
#import "OEPrefCoresController.h"

#define AnimationDuration 0.3
@interface OEPreferencesController (priavte)
- (void)_showView:(NSView*)view atSize:(NSSize)size animate:(BOOL)animateFlag;
- (void)_reloadPreferencePanes;
- (void)_rebuildToolbar;
@end
@implementation OEPreferencesController
@synthesize preferencePanes;
- (id)initWithWindow:(NSWindow *)window
{
    self = [super initWithWindow:window];
    if (self) 
    {
    }
    
    return self;
}

- (void)dealloc
{
    [toolbar release];
    toolbar = nil;
    
    [self setPreferencePanes:nil];
    
    [super dealloc];
}

- (NSString*)windowNibName
{
    return @"Preferences";
}

- (void)awakeFromNib
{
    // Prelaod window to prevent flickering when it's first shown
    [self window];
}

- (void)windowDidLoad
{
    [super windowDidLoad];
    
    INAppStoreWindow *win = (INAppStoreWindow*)[self window];
    
    NSColor *windowBackgroundColor = [NSColor colorWithDeviceRed:0.149 green:0.149 blue:0.149 alpha:1.0];
    [win setBackgroundColor:windowBackgroundColor];
    
    [self _reloadPreferencePanes];
    
    win.titleBarString = @"Preferences";
    win.titleBarHeight = 83;
    win.titleBarView = toolbar;
    win.trafficLightAlignment = 0;
    
    NSUserDefaults *standardDefaults = [NSUserDefaults standardUserDefaults];
    NSInteger selectedTab = [standardDefaults integerForKey:UDSelectedPreferencesTab];
    
    // Make sure that value from User Defaults is valid
    if(selectedTab < 0 || selectedTab >= [[toolbar items] count])
    {
        selectedTab = 0;
    }
    
    OEToolbarItem *selectedItem = [[toolbar items] objectAtIndex:selectedTab];
    [toolbar markItemAsSelected:selectedItem];
    [self switchView:selectedItem animate:NO];
    
    [self.window.contentView setWantsLayer:YES];    
    
    CATransition *paneTransition = [CATransition animation];
    paneTransition.type = kCATransitionFade;
    paneTransition.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionDefault];
    paneTransition.duration = AnimationDuration;
    
    [self.window.contentView setAnimations:[NSDictionary dictionaryWithObject:paneTransition  forKey:@"subviews"]];
}
#pragma mark -
- (void)_reloadPreferencePanes
{
    NSMutableArray *array = [NSMutableArray array];
    
    NSViewController <OEPreferencePane>  *controller;
    
    controller = [[[OEPrefLibraryController alloc] init] autorelease];
    [array addObject:controller];
    
    controller = [[[OEPrefGameplayController alloc] init] autorelease];
    [array addObject:controller];
    
    controller = [[[OEPrefControlsController alloc] init] autorelease];
    [array addObject:controller];
    
    controller = [[[OEPrefCoresController alloc] init] autorelease];
    [array addObject:controller];
    
    [self setPreferencePanes:array];    
    [self _rebuildToolbar];
}

- (void)_rebuildToolbar
{
    if(toolbar)
    {
        [toolbar removeFromSuperview];
        [toolbar release];
        toolbar = nil;
    }
    
    INAppStoreWindow *win = (INAppStoreWindow*)[self window];
    toolbar = [[OEToolbarView alloc] initWithFrame:NSMakeRect(0, 0, win.frame.size.width-10, 58)];
    
    for(id <OEPreferencePane> aPreferencePane in self.preferencePanes)
    {
        OEToolbarItem *toolbarItem = [[[OEToolbarItem alloc] init] autorelease];
        [toolbarItem setTitle:[aPreferencePane title]];
        [toolbarItem setIcon:[aPreferencePane icon]];
        [toolbarItem setTarget:self];
        [toolbarItem setAction:@selector(switchView:)];
        [toolbar addItem:toolbarItem];
    }
}
#pragma mark -
- (void)switchView:(id)sender
{
    [self switchView:sender animate:YES];
}

- (void)switchView:(id)sender animate:(BOOL)animateFlag
{
    NSInteger selectedTab = [[toolbar items] indexOfObject:sender];
    NSViewController <OEPreferencePane>  *pane = [self.preferencePanes objectAtIndex:selectedTab];
    
    NSSize viewSize = [pane viewSize];
    NSView *view = [pane view];
    
    toolbar.contentseparatorColor = [NSColor blackColor];
    
    [self _showView:view atSize:viewSize animate:animateFlag];
    
    BOOL viewHasCustomColor = [pane respondsToSelector:@selector(toolbarSeparationColor)];
    if(viewHasCustomColor) toolbar.contentseparatorColor = [pane toolbarSeparationColor];
    
    NSUserDefaults *standardDefaults = [NSUserDefaults standardUserDefaults];
    [standardDefaults setInteger:selectedTab forKey:UDSelectedPreferencesTab];
}

- (void)_showView:(NSView*)view atSize:(NSSize)size animate:(BOOL)animateFlag
{
    NSWindow *win = [self window];
    
    if(view==[win contentView]) return;
    
    NSRect contentRect = [win contentRectForFrameRect:[win frame]];
    contentRect.size = size;
    NSRect frameRect = [win frameRectForContentRect:contentRect];
    frameRect.origin.y += win.frame.size.height-frameRect.size.height;
    
    nextView = view;
    [view setFrameSize:size];
    
    CAAnimation *anim = [win animationForKey:@"frame"];
    anim.duration = AnimationDuration;
    [win setAnimations:[NSDictionary dictionaryWithObject:anim forKey:@"frame"]];
    
    [CATransaction begin];
    if([win.contentView subviews].count)
        [animateFlag?[win.contentView animator]:win.contentView replaceSubview:[win.contentView subviews].lastObject with:view];
    else {
        [animateFlag?[win.contentView animator]:win.contentView addSubview:view];
    }
    [animateFlag?[win animator]:win setFrame:frameRect display:YES];
    [CATransaction commit];
}
@end
