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

#import "OEPreferencesController.h"

#import "OEBackgroundGradientView.h"
#import "OEBackgroundColorView.h"
#import "OEPreferencePane.h"

#import "OEPrefLibraryController.h"
#import "OEPrefGameplayController.h"
#import "OEPrefControlsController.h"
#import "OEPrefCoresController.h"
#import "OEPrefDebugController.h"
#import "OEPrefBiosController.h"

#import <AppKit/NSToolbarItem.h>

@import QuartzCore;

#import "OpenEmu-Swift.h"

NSString *const OEDebugModeKey = @"debug";
NSString *const OESelectedPreferencesTabKey = @"selectedPreferencesTab";

NSString *const OEPreferencesOpenPaneNotificationName  = @"OEPrefOpenPane";
NSString *const OEPreferencesSetupPaneNotificationName = @"OEPrefSetupPane";
NSString *const OEPreferencesUserInfoPanelNameKey        = @"panelName";
NSString *const OEPreferencesUserInfoSystemIdentifierKey = @"systemIdentifier";
NSString *const OEPreferencePaneDidChangeVisibilityNotificationName = @"OEPrefVisibilityChanged";

NSString *const OEToolbarItemIdentifierDebug = @"OEToolBarItemIdentifierDebug";
NSString *const OEToolbarItemIdentifierSeparator = @"OEToolbarItemIdentifierSeparator";

const CGFloat OEPreferencesPaneTransitionDuration = 0.3;

@interface OEPreferencesController () <NSWindowDelegate>

- (void)OE_showView:(NSView *)view atSize:(NSSize)size animate:(BOOL)animateFlag;
- (void)OE_createPreferencePanes;
- (void)OE_openPreferencePane:(NSNotification *)notification;
- (void)OE_updateSecretPreferencesVisiblity;
@property (nonatomic) NSInteger visibleItemIndex;

@property (assign) OEBackgroundColorView *separatorLineView;
@property NSView *subviewContainer;
@property id konamiCodeMonitor;
@property unsigned short konamiCodeIndex;
@end

@implementation OEPreferencesController
static const unichar konamiCode[] = { NSUpArrowFunctionKey, NSUpArrowFunctionKey, NSDownArrowFunctionKey, NSDownArrowFunctionKey, NSLeftArrowFunctionKey, NSRightArrowFunctionKey, NSLeftArrowFunctionKey, NSRightArrowFunctionKey, 'b', 'a' };
static const unsigned short konamiCodeSize = 10;

@synthesize preferencePanes;
@synthesize visibleItemIndex = _visibleItemIndex;
@dynamic window;

- (id)initWithWindow:(NSWindow *)window
{
    self = [super initWithWindow:window];
    if (self) 
    {
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(OE_openPreferencePane:) name:OEPreferencesOpenPaneNotificationName object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(OE_rebuildToolbarWithNotification:) name:OEPreferencePaneDidChangeVisibilityNotificationName object:nil];

        [self setWindowFrameAutosaveName:@"Preferences"];
    }
    
    return self;
}

- (NSString*)windowNibName
{
    return @"Preferences";
}

- (void)awakeFromNib
{
    NSMenu  *menu  = [NSApp mainMenu];
    NSMenu *oemenu = [[menu itemAtIndex:0] submenu];
    NSMenuItem *preferencesItem = [oemenu itemWithTag:121];
    [preferencesItem setTarget:self];
    [preferencesItem setAction:@selector(showWindow:)];
    [preferencesItem setEnabled:YES];

    NSWindow *win = [self window];
    [win setTitlebarAppearsTransparent:YES];
    [win close]; // Make sure window doesn't show up in window menu until it's actual visible

    [self setSubviewContainer:[[[win contentView] subviews] lastObject]];

    NSColor *windowBackgroundColor = [NSColor colorWithDeviceRed:0.149 green:0.149 blue:0.149 alpha:1.0];
    [win setBackgroundColor:windowBackgroundColor];

    [self OE_createPreferencePanes];

    // setup toolbar
    [self OE_updateSecretPreferencesVisiblity];

    NSTitlebarAccessoryViewController *separatorViewController = [[NSTitlebarAccessoryViewController alloc] init];
    [separatorViewController setLayoutAttribute:NSLayoutAttributeBottom];
    [separatorViewController setFullScreenMinHeight:1.0];
    OEBackgroundColorView *view = [[OEBackgroundColorView alloc] initWithFrame:NSMakeRect(0, 0, 1, 1)];
    [view setBackgroundColor:[NSColor greenColor]];
    [view setAutoresizingMask:NSViewWidthSizable];
    [separatorViewController setView:view];
    [win addTitlebarAccessoryViewController:separatorViewController];
    self.separatorLineView = view;

    // setup transition
    [_subviewContainer setWantsLayer:YES];
    
    CATransition *paneTransition = [CATransition animation];
    paneTransition.type = kCATransitionFade;
    paneTransition.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionDefault];
    paneTransition.duration = OEPreferencesPaneTransitionDuration;

    [_subviewContainer setAnimations:[NSDictionary dictionaryWithObject:paneTransition  forKey:@"subviews"]];

    // restore previous selectin
    NSUserDefaults *standardDefaults = [NSUserDefaults standardUserDefaults];
    NSInteger selectedTab = [standardDefaults integerForKey:OESelectedPreferencesTabKey];
    [self setVisibleItemIndex:-1];

    // Make sure that value from User Defaults is valid
    if(selectedTab < 0 || selectedTab >= self.preferencePanes.count)
        selectedTab = 0;

    [self showPane:self.preferencePanes[selectedTab] animate:NO];
}

#pragma mark - NSWindowDelegate
- (void)windowWillClose:(NSNotification *)notification
{
    [[self selectedPreferencePane] viewWillDisappear];
    [[self selectedPreferencePane] viewDidDisappear];
}

- (void)windowDidBecomeKey:(NSNotification *)notification
{
    [[self selectedPreferencePane] viewWillAppear];
    [[self selectedPreferencePane] viewDidAppear];

    _konamiCodeIndex = 0;
    _konamiCodeMonitor =
    [NSEvent addLocalMonitorForEventsMatchingMask:NSKeyDownMask handler:
     ^ NSEvent * (NSEvent *e)
     {
         if([[e characters] characterAtIndex:0] == konamiCode[_konamiCodeIndex])
         {
             _konamiCodeIndex++;
             if(_konamiCodeIndex == konamiCodeSize)
             {
                 NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
                 BOOL debugModeActivated = ![defaults boolForKey:OEDebugModeKey];
                 [defaults setBool:debugModeActivated forKey:OEDebugModeKey];
                 [[NSSound soundNamed:@"secret"] play];

                 [self OE_updateSecretPreferencesVisiblity];

                 NSViewController<OEPreferencePane> *pane = self.preferencePanes[0];
                 if(debugModeActivated) {
                     pane = [preferencePanes lastObject];
                 }

                 [self showPane:pane];

                 _konamiCodeIndex = 0;
             }

             return nil;
         }

         _konamiCodeIndex = 0;
         return e;
    }];
}

- (void)OE_updateSecretPreferencesVisiblity
{
    NSInteger index = [self.toolbar.items indexOfObjectPassingTest:^BOOL(__kindof NSToolbarItem * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
        return [[obj itemIdentifier] isEqualToString:OEToolbarItemIdentifierDebug];
    }];

    BOOL debugModeActive = [[NSUserDefaults standardUserDefaults] boolForKey:OEDebugModeKey];

    if(debugModeActive == (index != NSNotFound))
        return;

    if(debugModeActive) {
        NSInteger lastIndex = self.toolbar.items.count-1;
        [self.toolbar insertItemWithItemIdentifier:OEToolbarItemIdentifierDebug atIndex:lastIndex];
        [self.toolbar insertItemWithItemIdentifier:OEToolbarItemIdentifierSeparator atIndex:lastIndex];
    } else {
        [self.toolbar removeItemAtIndex:index-1];
        [self.toolbar removeItemAtIndex:index-1];
    }
}

- (void)windowDidResignKey:(NSNotification *)notification
{
    [NSEvent removeMonitor:_konamiCodeMonitor];
    _konamiCodeIndex   = 0;
    _konamiCodeMonitor = nil;
}

#pragma mark - Toolbar
- (NSViewController<OEPreferencePane> *)selectedPreferencePane
{
    NSInteger selected = [self visibleItemIndex];
    return selected >= 0 && selected < [preferencePanes count] ? [[self preferencePanes] objectAtIndex:selected] : nil;
}

- (void)OE_createPreferencePanes
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

    controller = [[OEPrefBiosController alloc] init];
    [array addObject:controller];

    controller = [[OEPrefDebugController alloc] init];
    [array addObject:controller];
    
    [self setPreferencePanes:array];    
}

- (void)OE_rebuildToolbarWithNotification:(NSNotification*)notification
{
    [self OE_updateSecretPreferencesVisiblity];
}

- (void)OE_openPreferencePane:(NSNotification *)notification
{
    NSDictionary *userInfo = [notification userInfo];
    NSString     *paneName = [userInfo valueForKey:OEPreferencesUserInfoPanelNameKey];

    NSInteger index = [[self preferencePanes] indexOfObjectPassingTest:^BOOL(id <OEPreferencePane>obj, NSUInteger idx, BOOL *stop) {
        return [[obj title] isEqualToString:paneName] && (*stop=YES);
    }];

    if(index != NSNotFound)
    {
        BOOL windowVisible = [[self window] isVisible];

        [self showPane:self.preferencePanes[index] animate:windowVisible];
        [self setVisibleItemIndex:index];
        [[self window] makeKeyAndOrderFront:self];
    }
}

#pragma mark -
- (IBAction)switchPaneByToolbarItem:(id)sender
{
    [self showPane:[[self preferencePanes] objectAtIndex:[sender tag]]];
}

- (void)showPane:(NSViewController<OEPreferencePane>*)sender
{
    [self showPane:sender animate:YES];
}

- (void)showPane:(NSViewController<OEPreferencePane>*)nextPane animate:(BOOL)animateFlag
{
    NSInteger selectedTab = [[self preferencePanes] indexOfObject:nextPane];
    NSViewController<OEPreferencePane> *currentPane = [self selectedPreferencePane];

    if(currentPane == nextPane) return;
    
    [nextPane viewWillAppear];
    [currentPane viewWillDisappear];

    NSSize viewSize = [nextPane viewSize];
    NSView *view = [nextPane view];

    [self OE_showView:view atSize:viewSize animate:animateFlag];
    [nextPane viewDidAppear];
    [currentPane viewDidDisappear];

    NSColor *separatorColor = [NSColor blackColor];
    BOOL viewHasCustomColor = [nextPane respondsToSelector:@selector(toolbarSeparationColor)];
    if(viewHasCustomColor) separatorColor = [nextPane toolbarSeparationColor];
    self.separatorLineView.backgroundColor = separatorColor;

    NSUserDefaults *standardDefaults = [NSUserDefaults standardUserDefaults];
    [standardDefaults setInteger:selectedTab forKey:OESelectedPreferencesTabKey];
    [self setVisibleItemIndex:selectedTab];

    [[self window] makeFirstResponder:[nextPane view]];
}

- (void)OE_showView:(NSView *)view atSize:(NSSize)size animate:(BOOL)animateFlag
{
    NSWindow *win = [self window];
    
    if(view == [[_subviewContainer subviews] lastObject]) return;

    NSRect contentRect = [win contentRectForFrameRect:[win frame]];
    contentRect.size = size;
    NSRect frameRect = [win frameRectForContentRect:contentRect];
    frameRect.origin.y += win.frame.size.height - frameRect.size.height;
    
    [view setFrameSize:size];

    [CATransaction begin];
    
    id target = _subviewContainer;
    if(animateFlag) target = [target animator];
    
    if([[_subviewContainer subviews] count] >= 1)
        [target replaceSubview:[[_subviewContainer subviews] lastObject] with:view];
    else
        [target addSubview:view];
    
    [animateFlag ? [win animator] : win setFrame:frameRect display:YES];
    
    [CATransaction commit];
}

- (void)setVisibleItemIndex:(NSInteger)visiblePaneIndex
{
    _visibleItemIndex = visiblePaneIndex;
    if(self.toolbar.items.count <= 2*visiblePaneIndex+1) return;
    self.toolbar.selectedItemIdentifier = self.toolbar.items[2*visiblePaneIndex+1].itemIdentifier;
}

@end
