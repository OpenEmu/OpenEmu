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
#import <Quartz/Quartz.h>

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


NSString *const OEDebugModeKey = @"debug";
NSString *const OESelectedPreferencesTabKey = @"selectedPreferencesTab";

NSString *const OEPreferencesOpenPaneNotificationName  = @"OEPrefOpenPane";
NSString *const OEPreferencesSetupPaneNotificationName = @"OEPrefSetupPane";
NSString *const OEPreferencesUserInfoPanelNameKey        = @"panelName";
NSString *const OEPreferencesUserInfoSystemIdentifierKey = @"systemIdentifier";

#define AnimationDuration 0.3

@interface OEPreferencesController () <NSWindowDelegate>
{
	OEToolbarView *toolbar;
	IBOutlet OEBackgroundGradientView *coreGradientOverlayView;
}

- (void)OE_showView:(NSView *)view atSize:(NSSize)size animate:(BOOL)animateFlag;
- (void)OE_reloadPreferencePanes;
- (void)OE_rebuildToolbar;
- (void)OE_openPreferencePane:(NSNotification *)notification;

@property OEAppStoreWindow *window;
@property id konamiCodeMonitor;
@property unsigned short konamiCodeIndex;
@end

@implementation OEPreferencesController
static const unsigned short konamiCode[] = {0x7e,0x7e,0x7d,0x7d,0x7b,0x7c,0x7b,0x7c,0x0b,0x00};
static const unsigned short konamiCodeSize = 10;

@synthesize preferencePanes;
@synthesize visiblePaneIndex = _visiblePaneIndex;
@dynamic window;

- (id)initWithWindow:(NSWindow *)window
{
    self = [super initWithWindow:window];
    if (self) 
    {
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(OE_openPreferencePane:) name:OEPreferencesOpenPaneNotificationName object:nil];
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
    // Preload window to prevent flickering when it's first shown
    [self window];
}

- (void)windowDidLoad
{
    [super windowDidLoad];

    OEAppStoreWindow *win = (OEAppStoreWindow *)[self window];
    [win close]; // Make sure window doesn't show up in window menu until it's actual visible

    NSColor *windowBackgroundColor = [NSColor colorWithDeviceRed:0.149 green:0.149 blue:0.149 alpha:1.0];
    [win setBackgroundColor:windowBackgroundColor];
    
    [self OE_reloadPreferencePanes];
    
    [win setTitleBarView:toolbar];
    [win setCenterTrafficLightButtons:NO];
    [win setTitleBarHeight:83.0];
    [win setMovableByWindowBackground:NO];
   
    NSUserDefaults *standardDefaults = [NSUserDefaults standardUserDefaults];
    NSInteger selectedTab = [standardDefaults integerForKey:OESelectedPreferencesTabKey];
    
    [self setVisiblePaneIndex:-1];
    
    // Make sure that value from User Defaults is valid
    if(selectedTab < 0 || selectedTab >= [[toolbar items] count])
        selectedTab = 0;
    
    OEToolbarItem *selectedItem = [[toolbar items] objectAtIndex:selectedTab];
    [toolbar markItemAsSelected:selectedItem];
    [self switchView:selectedItem animate:NO];
    
    [[[self window] contentView] setWantsLayer:YES];
    
    CATransition *paneTransition = [CATransition animation];
    paneTransition.type = kCATransitionFade;
    paneTransition.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionDefault];
    paneTransition.duration = AnimationDuration;
    
    [[[self window] contentView] setAnimations:[NSDictionary dictionaryWithObject:paneTransition  forKey:@"subviews"]];
}

#pragma mark - NSWindowDelegate

- (NSRect)window:(NSWindow *)window willPositionSheet:(NSWindow *)sheet usingRect:(NSRect)rect
{
    if([window isKindOfClass:[OEAppStoreWindow class]])
        rect.origin.y -= [(OEAppStoreWindow*)window titleBarHeight]-22.0;
    
    return rect;
}

- (void)windowWillClose:(NSNotification *)notification
{
    [[self selectedPreferencePane] viewWillDisappear];
}

- (void)windowDidBecomeKey:(NSNotification *)notification
{
    _konamiCodeIndex = 0;
    _konamiCodeMonitor = [NSEvent addLocalMonitorForEventsMatchingMask:NSKeyDownMask handler:^NSEvent *(NSEvent *e) {
        if([e keyCode] == konamiCode[_konamiCodeIndex])
        {
            _konamiCodeIndex ++;
            if(_konamiCodeIndex == konamiCodeSize)
            {
                NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
                [defaults setBool:![defaults boolForKey:OEDebugModeKey] forKey:OEDebugModeKey];
                [[NSSound soundNamed:@"secret"] play];
                [self OE_rebuildToolbar];
                _konamiCodeIndex = 0;
                return nil;
            }
        } else _konamiCodeIndex = 0;
        return _konamiCodeIndex ? nil : e;
    }];
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
    NSInteger selected = [self visiblePaneIndex];
    return selected >= 0 && selected < [preferencePanes count] ? [[self preferencePanes] objectAtIndex:selected] : nil;
}

- (void)OE_reloadPreferencePanes
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

    controller = [[OEPrefDebugController alloc] init];
    [array addObject:controller];
    
    [self setPreferencePanes:array];    
    [self OE_rebuildToolbar];
}

- (void)OE_rebuildToolbar
{
    NSUInteger lastSelection = 0;
    if(toolbar)
    {
        lastSelection = [toolbar selectedItemIndex];
        [toolbar removeFromSuperview];
        toolbar = nil;
    }
    
    OEAppStoreWindow *win = (OEAppStoreWindow*)[self window];
    toolbar = [[OEToolbarView alloc] initWithFrame:NSMakeRect(0, 0, win.frame.size.width-10.0, 58.0)];
    
    for(id <OEPreferencePane> aPreferencePane in self.preferencePanes)
    {
        if([aPreferencePane isKindOfClass:[OEPrefDebugController class]] && ![[NSUserDefaults standardUserDefaults] boolForKey:OEDebugModeKey])
            continue;
        
        OEToolbarItem *toolbarItem = [[OEToolbarItem alloc] init];
        [toolbarItem setTitle:[aPreferencePane localizedTitle]];
        [toolbarItem setIcon:[aPreferencePane icon]];
        [toolbarItem setTarget:self];
        [toolbarItem setAction:@selector(switchView:)];
        [toolbar addItem:toolbarItem];
    }
    if(lastSelection >= [[toolbar items] count]) lastSelection = 0;
    [self switchView:[[toolbar items] objectAtIndex:lastSelection] animate:YES];
    [self setVisiblePaneIndex:lastSelection];

    [win setTitleBarView:toolbar];
}

- (void)OE_openPreferencePane:(NSNotification *)notification
{
    NSDictionary *userInfo = [notification userInfo];
    NSString     *paneName = [userInfo valueForKey:OEPreferencesUserInfoPanelNameKey];
    
    NSInteger index = 0;
    for(id<OEPreferencePane> aPreferencePane in [self preferencePanes])
    {
        if([[aPreferencePane title] isEqualToString:paneName]) break;
        
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
        selectedTab = [[toolbar items] indexOfObject:sender];
    else if([sender isKindOfClass:[NSNumber class]])
        selectedTab = [sender integerValue];
    else return;
    
    NSViewController<OEPreferencePane> *currentPane = [self selectedPreferencePane];
    NSViewController<OEPreferencePane> *nextPane    = [[self preferencePanes] objectAtIndex:selectedTab];
    
    if(currentPane == nextPane) return;
    
    [nextPane viewWillAppear];
    [currentPane viewWillDisappear];

    NSSize viewSize = [nextPane viewSize];
    NSView *view = [nextPane view];
    
    [[self window] setBaselineSeparatorColor:[NSColor blackColor]];
    
    [self OE_showView:view atSize:viewSize animate:animateFlag];
    [nextPane viewDidAppear];
    [currentPane viewDidDisappear];
    
    BOOL viewHasCustomColor = [nextPane respondsToSelector:@selector(toolbarSeparationColor)];
    if(viewHasCustomColor) [[self window] setBaselineSeparatorColor:[nextPane toolbarSeparationColor]];
    else [[self window] setBaselineSeparatorColor:[NSColor blackColor]];
    
    NSUserDefaults *standardDefaults = [NSUserDefaults standardUserDefaults];
    [standardDefaults setInteger:selectedTab forKey:OESelectedPreferencesTabKey];
    [self setVisiblePaneIndex:selectedTab];

    [[self window] makeFirstResponder:[nextPane view]];
}

- (void)OE_showView:(NSView *)view atSize:(NSSize)size animate:(BOOL)animateFlag
{
    NSWindow *win = [self window];
    
    if(view == [win contentView]) return;

    NSRect contentRect = [win contentRectForFrameRect:[win frame]];
    contentRect.size = size;
    NSRect frameRect = [win frameRectForContentRect:contentRect];
    frameRect.origin.y += win.frame.size.height - frameRect.size.height;
    
    [view setFrameSize:size];
    
    CAAnimation *anim = [win animationForKey:@"frame"];
    anim.duration = AnimationDuration;
    [win setAnimations:[NSDictionary dictionaryWithObject:anim forKey:@"frame"]];
    
    [CATransaction begin];
    
    id target = [win contentView];
    if(animateFlag) target = [target animator];
    
    if([[[win contentView] subviews] count] >= 1)
        [target replaceSubview:[[[win contentView] subviews] lastObject] with:view];
    else
        [target addSubview:view];
    
    [animateFlag ? [win animator] : win setFrame:frameRect display:YES];
    
    [CATransaction commit];
}

#pragma mark - Properties

- (void)setVisiblePaneIndex:(NSInteger)visiblePaneIndex
{
    _visiblePaneIndex = visiblePaneIndex;
    [toolbar markItemIndexAsSelected:visiblePaneIndex];
}

@end
