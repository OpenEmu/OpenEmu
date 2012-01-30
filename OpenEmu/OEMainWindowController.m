//
//  OEMainWindowController.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 21.10.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OEMainWindowController.h"
#import "OEMainWindowContentController.h"
#import "NSImage+OEDrawingAdditions.h"

#import "OEMainWindow.h"
@implementation OEMainWindowController

- (id)initWithWindow:(NSWindow *)window
{
    self = [super initWithWindow:window];
    if (self)
    {
    }
    return self;
}

+ (void)initialize
{
    if (self != [OEMainWindowController class])
        return;
    // toolbar sidebar button image
    NSImage* image = [NSImage imageNamed:@"toolbar_sidebar_button"];
    [image setName:@"toolbar_sidebar_button_close" forSubimageInRect:NSMakeRect(0, 23, 84, 23)];
    [image setName:@"toolbar_sidebar_button_open" forSubimageInRect:NSMakeRect(0, 0, 84, 23)];
    
    // toolbar view button images
    image = [NSImage imageNamed:@"toolbar_view_buttons"];
    [image setName:@"toolbar_view_button_grid" forSubimageInRect:NSMakeRect(0, 0, 27, 115)];
    [image setName:@"toolbar_view_button_flow" forSubimageInRect:NSMakeRect(27, 0, 27, 115)];
    [image setName:@"toolbar_view_button_list" forSubimageInRect:NSMakeRect(54, 0, 27, 115)];

}

- (void)dealloc 
{
    [currentContentController release]; currentContentController = nil;
    [self setDefaultContentController:nil];
    
    [super dealloc];
}

- (void)windowDidLoad
{
    DLog(@"OEMainWindowController windowDidLoad"); 
    [super windowDidLoad];
    
    [self setAllowWindowResizing:YES];
    [[self window] setWindowController:self];
    [[self window] setDelegate:self];
    
    
    [[self toolbarSidebarButton] setImage:[NSImage imageNamed:@"toolbar_sidebar_button_close"]];
    
     // Setup Toolbar Buttons
    [[self toolbarGridViewButton] setImage:[NSImage imageNamed:@"toolbar_view_button_grid"]];
    [[self toolbarFlowViewButton] setImage:[NSImage imageNamed:@"toolbar_view_button_flow"]];
    [[self toolbarListViewButton] setImage:[NSImage imageNamed:@"toolbar_view_button_list"]];
    
    [[self toolbarAddToSidebarButton] setImage:[NSImage imageNamed:@"toolbar_add_button"]];
    
    // Setup Window behavior
    [[self window] setRestorable:NO];
    [[self window] setExcludedFromWindowsMenu:YES];
}

- (NSString*)windowNibName
{
    return @"MainWindow";
}
#pragma mark -
@synthesize currentContentController;
- (void)setCurrentContentController:(OEMainWindowContentController*)controller
{
    if(controller==nil)
        controller = [self defaultContentController];
    if(controller==[self currentContentController]) return;
    
    [[self toolbarFlowViewButton] setEnabled:NO];
    [[self toolbarFlowViewButton] setAction:NULL];
    
    [[self toolbarGridViewButton] setEnabled:NO];
    [[self toolbarGridViewButton] setAction:NULL];
    
    [[self toolbarListViewButton] setEnabled:NO];
    [[self toolbarListViewButton] setAction:NULL];
    
    [[self toolbarSearchField] setEnabled:NO];
    [[self toolbarSearchField] setAction:NULL];
    
    [[self toolbarSidebarButton] setEnabled:NO];
    [[self toolbarSidebarButton] setAction:NULL];
    
    [[self toolbarAddToSidebarButton] setEnabled:NO];
    [[self toolbarAddToSidebarButton] setAction:NULL];
    
    [[self toolbarSlider] setEnabled:NO];
    [[self toolbarSlider] setAction:NULL];
    
    [currentContentController contentWillHide];
    [controller contentWillShow];
    
    OEMainWindow* win = (OEMainWindow*)[self window];
    [win setMainContentView:[controller view]];
    [win makeFirstResponder:[controller view]];
    
    [controller retain];
    [currentContentController release];
    currentContentController = controller;
    
    [[self currentContentController] setupMenuItems];
}
@synthesize defaultContentController;
#pragma mark -
#pragma mark NSWindow delegate
@synthesize allowWindowResizing;
- (NSSize)windowWillResize:(NSWindow *)sender toSize:(NSSize)frameSize
{
    return [self allowWindowResizing] ? frameSize : [sender frame].size;
}

#pragma mark -
#pragma mark Menu Items
- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
    if([menuItem tag] == MainMenu_Window_OpenEmuTag)
    {
        NSLog(@"Item: %@, %ld enabled from OEMainWindowController", [menuItem title], [menuItem tag]);
        return YES;
    }
    return [[self currentContentController] validateMenuItem:menuItem]/* || ([self currentContentController]!=[self defaultContentController] && [[self currentContentController] validateMenuItem:menuItem])*/; 
}

- (void)menuItemAction:(id)sender
{
    if([sender tag] == MainMenu_Window_OpenEmuTag)
    {
        if([sender state])
            [[self window] orderOut:self];
        else
            [[self window] makeKeyAndOrderFront:self];
        return;
    }
    
    [[self currentContentController] menuItemAction:sender];
}

- (void)setupMenuItems
{
    NSMenu* mainMenu = [[NSApp delegate] mainMenu];
    
    // Window Menu
    NSMenu* windowMenu = [[mainMenu itemAtIndex:5] submenu];
    NSMenuItem* item = [windowMenu itemWithTag:MainMenu_Window_OpenEmuTag];
    [item bind:@"state" toObject:[self window] withKeyPath:@"visible" options:nil];
    
    [[self currentContentController] setupMenuItems];
}
#pragma mark -
#pragma mark Toolbar Elements
@synthesize toolbarFlowViewButton, toolbarGridViewButton, toolbarListViewButton;
@synthesize toolbarSearchField, toolbarSidebarButton, toolbarAddToSidebarButton, toolbarSlider;
@end

