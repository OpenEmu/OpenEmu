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
#import "OEAppStoreWindow.h"

#import "NSImage+OEDrawingAdditions.h"
#import "NSViewController+OEAdditions.h"

#import "OEPreferencePane.h"

#import "OEPrefLibraryController.h"
#import "OEPrefGameplayController.h"
#import "OEPrefControlsController.h"
#import "OEPrefCoresController.h"
#import "OEPrefDebugController.h"

#define AnimationDuration 0.3
@interface OEPreferencesController (priavte)
- (void)_showView:(NSView*)view atSize:(NSSize)size animate:(BOOL)animateFlag;
- (void)_reloadPreferencePanes;
- (void)_rebuildToolbar;
- (void)_openPreferencePane:(NSNotification*)notification;
@end
@implementation OEPreferencesController
@synthesize preferencePanes;
- (id)initWithWindow:(NSWindow *)window
{
    self = [super initWithWindow:window];
    if (self) 
    {
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(_openPreferencePane:) name:OEPreferencesOpenPaneNotificationName object:nil];
    }
    
    return self;
}

- (void)dealloc
{
    toolbar = nil;
 
    [[NSNotificationCenter defaultCenter] removeObserver:self];
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
    
    [win setTitleBarView:toolbar];
    [win setCenterTrafficLightButtons:NO];
    [win setTitleBarHeight:83.0];
    [win setMovableByWindowBackground:NO];
   
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
#pragma mark NSWindow Delegte
- (NSRect)window:(NSWindow *)window willPositionSheet:(NSWindow *)sheet usingRect:(NSRect)rect
{
    if([window isKindOfClass:[INAppStoreWindow class]])
        rect.origin.y -= [(INAppStoreWindow*)window titleBarHeight]-22.0;
    
    return rect;
}
#pragma mark -
- (void)_reloadPreferencePanes
{
    NSMutableArray *array = [NSMutableArray array];
    
    NSViewController <OEPreferencePane>  *controller;
    
    controller = [[OEPrefLibraryController alloc] init];
    [array addObject:controller];
    
    controller = [[OEPrefGameplayController alloc] init];
    [array addObject:controller];
    
    controller = [[OEPrefControlsController alloc] init];
    [array addObject:controller];
    
    controller = [[OEPrefCoresController alloc] init];
    [array addObject:controller];
    
    if([[NSUserDefaults standardUserDefaults] boolForKey:UDDebugModeKey])
    {
        controller = [[OEPrefDebugController alloc] init];
        [array addObject:controller];
    }
    
    [self setPreferencePanes:array];    
    [self _rebuildToolbar];
}

- (void)_rebuildToolbar
{
    if(toolbar)
    {
        [toolbar removeFromSuperview];
        toolbar = nil;
    }
    
    INAppStoreWindow *win = (INAppStoreWindow*)[self window];
    toolbar = [[OEToolbarView alloc] initWithFrame:NSMakeRect(0, 0, win.frame.size.width-10, 58)];
    
    for(id <OEPreferencePane> aPreferencePane in self.preferencePanes)
    {
        OEToolbarItem *toolbarItem = [[OEToolbarItem alloc] init];
        [toolbarItem setTitle:[aPreferencePane localizedTitle]];
        [toolbarItem setIcon:[aPreferencePane icon]];
        [toolbarItem setTarget:self];
        [toolbarItem setAction:@selector(switchView:)];
        [toolbar addItem:toolbarItem];
    }
}

- (void)_openPreferencePane:(NSNotification*)notification
{
    NSDictionary* userInfo = [notification userInfo];
    NSString* paneName = [userInfo valueForKey:OEPreferencesOpenPanelUserInfoPanelNameKey];
    
    NSInteger index = 0;
    for(id <OEPreferencePane> aPreferencePane in self.preferencePanes)
    {
        if([[aPreferencePane title] isEqualToString:paneName])
            break;
        index++;
    }

    [self switchView:[NSNumber numberWithInteger:index] animate:[[self window] isVisible]];
    [[self window] makeKeyAndOrderFront:self];
}
#pragma mark -
- (void)switchView:(id)sender
{
    [self switchView:sender animate:YES];
}

- (void)switchView:(id)sender animate:(BOOL)animateFlag
{
    NSInteger selectedTab;
    if([sender isKindOfClass:[OEToolbarItem class]])
    {
        selectedTab = [[toolbar items] indexOfObject:sender];
    }
    else if([sender isKindOfClass:[NSNumber class]])
    {
        selectedTab = [sender integerValue];
    } else {
        return;
    }
    
    NSViewController <OEPreferencePane>  *currentPane = [self.preferencePanes objectAtIndex:[toolbar selectedItemIndex]];
    NSViewController <OEPreferencePane>  *nextPane = [self.preferencePanes objectAtIndex:selectedTab];
    [nextPane viewWillAppear];
    [currentPane viewWillDisappear];
    
    NSSize viewSize = [nextPane viewSize];
    NSView *view = [nextPane view];
    
    toolbar.contentseparatorColor = [NSColor blackColor];
    
    [self _showView:view atSize:viewSize animate:animateFlag];
    [nextPane viewDidAppear];
    [currentPane viewDidDisappear];
    
    BOOL viewHasCustomColor = [nextPane respondsToSelector:@selector(toolbarSeparationColor)];
    if(viewHasCustomColor) toolbar.contentseparatorColor = [nextPane toolbarSeparationColor];
    
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
