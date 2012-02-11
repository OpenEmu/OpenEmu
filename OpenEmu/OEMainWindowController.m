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

#import "OEMainWindowController.h"
#import "OEMainWindowContentController.h"
#import "NSImage+OEDrawingAdditions.h"
#import "OEMainWindow.h"
#import "OESetupAssistant.h"
#import "OELibraryController.h"
#import "NSViewController+OEAdditions.h"

@interface OEMainWindowController ()
- (void)OE_replaceCurrentContentController:(NSViewController *)oldController withViewController:(NSViewController *)newController;
@end

@implementation OEMainWindowController
@synthesize currentContentController;
@synthesize toolbarFlowViewButton, toolbarGridViewButton, toolbarListViewButton;
@synthesize toolbarSearchField, toolbarSidebarButton, toolbarAddToSidebarButton, toolbarSlider;
@synthesize defaultContentController;
@synthesize allowWindowResizing;
@synthesize libraryController;
@synthesize placeholderView;

+ (void)initialize
{
    if(self == [OEMainWindowController class])
    {
        // toolbar sidebar button image
        NSImage *image = [NSImage imageNamed:@"toolbar_sidebar_button"];
        [image setName:@"toolbar_sidebar_button_close" forSubimageInRect:NSMakeRect(0, 23, 84, 23)];
        [image setName:@"toolbar_sidebar_button_open" forSubimageInRect:NSMakeRect(0, 0, 84, 23)];
        
        // toolbar view button images
        image = [NSImage imageNamed:@"toolbar_view_buttons"];
        [image setName:@"toolbar_view_button_grid" forSubimageInRect:NSMakeRect(0, 0, 27, 115)];
        [image setName:@"toolbar_view_button_flow" forSubimageInRect:NSMakeRect(27, 0, 27, 115)];
        [image setName:@"toolbar_view_button_list" forSubimageInRect:NSMakeRect(54, 0, 27, 115)];
    }
}

- (void)dealloc 
{
    [currentContentController release], currentContentController = nil;
    
    [self setDefaultContentController:nil];
    [self setLibraryController:nil];
    [self setPlaceholderView:nil];
    
    [super dealloc];
}

- (void)windowDidLoad
{
    [super windowDidLoad];
    
    [self setAllowWindowResizing:YES];
    [[self window] setWindowController:self];
    [[self window] setDelegate:self];
    
    // Setup Window behavior
    [[self window] setRestorable:NO];
    [[self window] setExcludedFromWindowsMenu:YES];
    
    [[self libraryController] setWindowController:self];
    
    if(![[NSUserDefaults standardUserDefaults] boolForKey:UDSetupAssistantHasRun])
    {
        OESetupAssistant *setupAssistant = [[OESetupAssistant alloc] init];
        [setupAssistant setCompletionBlock:
         ^(BOOL discoverRoms)
         {
             if(discoverRoms) [[self libraryController] discoverRoms];
             [self setCurrentContentController:[self libraryController]];
         }];
        
        [self setCurrentContentController:setupAssistant];
        [setupAssistant release];
    }
    else
    {
        [self setCurrentContentController:[self libraryController]];
    }
    
    [self setupMenuItems];
}

- (NSString *)windowNibName
{
    return @"MainWindow";
}

#pragma mark -

- (void)OE_replaceCurrentContentController:(NSViewController *)oldController withViewController:(NSViewController *)newController
{
    NSView *contentView = [self placeholderView];
    NSView *view        = [newController view];
    
    [view setFrame:[contentView bounds]];
    [view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    
    if(oldController != nil)
        [[contentView animator] replaceSubview:[oldController view] with:view];
    else
        [[contentView animator] addSubview:view];
}

- (void)setCurrentContentController:(OEMainWindowContentController *)controller
{
    if(controller == nil) controller = [self libraryController];
    
    if(controller == [self currentContentController]) return;
    
    [controller setWindowController:self];
    
    [currentContentController viewWillDisappear];
    [controller               viewWillAppear];
    
    [self OE_replaceCurrentContentController:currentContentController withViewController:controller];
    
    [[self window] makeFirstResponder:[controller view]];
    
    [currentContentController viewDidDisappear];
    [controller               viewDidAppear];
    
    [controller retain];
    [currentContentController release];
    currentContentController = controller;
    
    [[self currentContentController] setupMenuItems];
}

#pragma mark -
#pragma mark NSWindow delegate

- (NSSize)windowWillResize:(NSWindow *)sender toSize:(NSSize)frameSize
{
    return [self allowWindowResizing] ? frameSize : [sender frame].size;
}

#pragma mark -
#pragma mark Menu Items

- (IBAction)showOpenEmuWindow:(id)sender;
{
    [self close];
}


- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
#if 0
    if([menuItem tag] == MainMenu_Window_OpenEmuTag)
    {
        NSLog(@"Item: %@, %ld enabled from OEMainWindowController", [menuItem title], [menuItem tag]);
        return YES;
    }
    
    return [[self currentContentController] validateMenuItem:menuItem]/*|| ([self currentContentController]!=[self defaultContentController] && [[self currentContentController] validateMenuItem:menuItem])*/; 
#endif
    return YES;
}

- (void)menuItemAction:(NSMenuItem *)sender
{
#if 0
    if([sender tag] == MainMenu_Window_OpenEmuTag)
    {
        if([(NSMenuItem*)sender state])
            [[self window] orderOut:self];
        else
            [[self window] makeKeyAndOrderFront:self];
        return;
    }
    
    [[self currentContentController] menuItemAction:sender];
#endif
}

- (void)setupMenuItems
{
    NSMenu *mainMenu = [NSApp mainMenu];
    
    // Window Menu
    NSMenu *windowMenu = [[mainMenu itemAtIndex:5] submenu];
    NSMenuItem *item = [windowMenu itemWithTag:MainMenu_Window_OpenEmuTag];
    [item bind:@"state" toObject:[self window] withKeyPath:@"visible" options:nil];
    
    [[self currentContentController] setupMenuItems];
}

@end
