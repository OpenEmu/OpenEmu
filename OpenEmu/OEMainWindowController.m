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
#import "NSImage+OEDrawingAdditions.h"
#import "OEMainWindow.h"
#import "OESetupAssistant.h"
#import "OELibraryController.h"
#import "OEGameViewController.h"
#import "NSViewController+OEAdditions.h"
#import "OEGameDocument.h"

#import "OEHUDAlert+DefaultAlertsAdditions.h"
#import "OEDBGame.h"
@interface OEMainWindowController () <OELibraryControllerDelegate>
- (void)OE_replaceCurrentContentController:(NSViewController *)oldController withViewController:(NSViewController *)newController;
@end

@implementation OEMainWindowController
@synthesize currentContentController;
@synthesize defaultContentController;
@synthesize allowWindowResizing;
@synthesize libraryController;
@synthesize placeholderView;
@synthesize deviceHandlers, coreList;

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
    currentContentController = nil;
    [self setDefaultContentController:nil];
    [self setLibraryController:nil];
    [self setPlaceholderView:nil];    
}

- (void)windowDidLoad
{
    [super windowDidLoad];
    
    [[self libraryController] setDelegate:self];
    [[self libraryController] setSidebarChangesWindowSize:YES];
    
    [self setAllowWindowResizing:YES];
    [[self window] setWindowController:self];
    [[self window] setDelegate:self];
    
    // Setup Window behavior
    [[self window] setRestorable:NO];
    [[self window] setExcludedFromWindowsMenu:YES];
    
    if(![[NSUserDefaults standardUserDefaults] boolForKey:UDSetupAssistantHasRun])
    {
        OESetupAssistant *setupAssistant = [[OESetupAssistant alloc] init];
        [setupAssistant setDeviceHandlers:[self deviceHandlers]];
        
        [setupAssistant setCompletionBlock:
         ^(BOOL discoverRoms, NSArray* volumes)
         {
             if(discoverRoms) 
                 [[self libraryController] discoverRoms:volumes];
             
             [self setCurrentContentController:[self libraryController]];
         }];
        
        [self setCurrentContentController:setupAssistant];
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

- (void)openGameDocument:(OEGameDocument *)aDocument;
{
    NSUserDefaults *standardDefaults = [NSUserDefaults standardUserDefaults];
    BOOL allowPopout = [standardDefaults boolForKey:UDAllowPopoutKey];
    BOOL forcePopout = [standardDefaults boolForKey:UDForcePopoutKey];
    
    BOOL usePopout = forcePopout || allowPopout;
    
    if(usePopout) [aDocument showInSeparateWindow:self];
    else          [self setCurrentContentController:[aDocument gameViewController]];
}

- (IBAction)terminateEmulation:(id)sender;
{
    OEGameViewController *current = (OEGameViewController *)[self currentContentController];
    
    if(![current isKindOfClass:[OEGameViewController class]]) return;
    
    [self setCurrentContentController:[self libraryController]];
}

- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
    if([menuItem action] == @selector(terminateEmulation:))
        return [[self currentContentController] isKindOfClass:[OEGameViewController class]];
    
    return YES;
}

- (void)windowWillEnterFullScreen:(NSNotification *)notification
{
    [[self libraryController] setSidebarChangesWindowSize:NO];
}

- (void)windowDidExitFullScreen:(NSNotification *)notification
{
    [[self libraryController] setSidebarChangesWindowSize:YES];
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

- (void)setCurrentContentController:(NSViewController *)controller
{
    if(controller == nil) controller = [self libraryController];
    
    if(controller == [self currentContentController]) return;
    
    [currentContentController viewWillDisappear];
    [controller               viewWillAppear];
    
    [self OE_replaceCurrentContentController:currentContentController withViewController:controller];
    
    [[self window] makeFirstResponder:[controller view]];
    
    [currentContentController viewDidDisappear];
    [controller               viewDidAppear];
    
    currentContentController = controller;
}

#pragma mark -
#pragma mark OELibraryControllerDelegate protocol conformance

- (void)libraryController:(OELibraryController *)sender didSelectGame:(OEDBGame *)aGame
{
    NSError         *error          = nil;
    OEDBSaveState   *state          = [aGame autosaveForLastPlayedRom];
    OEGameDocument  *gameDocument;

    if(state && [[OEHUDAlert loadAutoSaveGameAlert] runModal] == NSAlertDefaultReturn)
         gameDocument = [[OEGameDocument alloc] initWithSaveState:state error:&error];
    else
        gameDocument = [[OEGameDocument alloc] initWithGame:aGame error:&error];
    
    if(gameDocument == nil)
    {
        if(error!=nil)
            [NSApp presentError:error];
        return;
    }
    
    [[NSDocumentController sharedDocumentController] addDocument:gameDocument];
    [self openGameDocument:gameDocument];
}


- (void)libraryController:(OELibraryController *)sender didSelectSaveState:(OEDBSaveState *)aSaveState
{
    NSError        *error = nil;
    OEGameDocument *gameDocument = [[OEGameDocument alloc] initWithSaveState:aSaveState error:&error];
    
    if(gameDocument == nil)
    {
        if(error!=nil)
            [NSApp presentError:error];
        return;
    }
    
    [[NSDocumentController sharedDocumentController] addDocument:gameDocument];
    [self openGameDocument:gameDocument];
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

- (void)setupMenuItems
{
    NSMenu *mainMenu = [NSApp mainMenu];
    
    // Window Menu
    NSMenu *windowMenu = [[mainMenu itemAtIndex:5] submenu];
    NSMenuItem *item = [windowMenu itemWithTag:MainMenu_Window_OpenEmuTag];
    [item bind:@"state" toObject:[self window] withKeyPath:@"visible" options:nil];
}

@end
