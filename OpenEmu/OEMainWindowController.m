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
#import "OEGameDocument.h"

#import "OEHUDAlert+DefaultAlertsAdditions.h"
#import "OEDBGame.h"

#import "NSView+FadeImage.h"
#import "OEFadeView.h"

NSString *const OEAllowPopoutGameWindowKey = @"allowPopout";
NSString *const OEForcePopoutGameWindowKey = @"forcePopout";

@interface OEMainWindowController () <OELibraryControllerDelegate>
- (void)OE_replaceCurrentContentController:(NSViewController *)oldController withViewController:(NSViewController *)newController;
@end

@implementation OEMainWindowController
@synthesize currentContentController;
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

- (id)init
{
    self = [super init];
    if (self) {
        [self setAllowWindowResizing:YES];
    }
    return self;
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
    
    if(![[NSUserDefaults standardUserDefaults] boolForKey:OESetupAssistantHasFinishedKey])
    {
        OESetupAssistant *setupAssistant = [[OESetupAssistant alloc] init];
        [setupAssistant setCompletionBlock:
         ^(BOOL discoverRoms, NSArray* volumes)
         {
             if(discoverRoms)
                 [[[OELibraryDatabase defaultDatabase] importer] discoverRoms:volumes];
             [self setCurrentContentController:[self libraryController]];
         }];

        [[self window] center];

        [self setCurrentContentController:setupAssistant];
    }
    else
    {
        [self setCurrentContentController:[self libraryController]];
    }
}

- (NSString *)windowNibName
{
    return @"MainWindow";
}

- (void)openGameDocument:(OEGameDocument *)aDocument;
{
    NSUserDefaults *standardDefaults = [NSUserDefaults standardUserDefaults];
    BOOL allowPopout = [standardDefaults boolForKey:OEAllowPopoutGameWindowKey];
    BOOL forcePopout = [standardDefaults boolForKey:OEForcePopoutGameWindowKey];
    
    BOOL usePopout = forcePopout || allowPopout;
    
    if(usePopout) 
        [aDocument showInSeparateWindow:self];
    else
        [self setCurrentContentController:[aDocument viewController]];
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

    // final target
    [[newController view] setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    [[newController view] setFrame:[contentView frame]];
    
    if(oldController != nil)
       [contentView replaceSubview:[oldController view] with:[newController view]];
    else
        [contentView addSubview:[newController view]];
        
    [[self window] makeFirstResponder:[newController view]];
}

- (void)setCurrentContentController:(NSViewController *)controller
{
    if(controller == nil) controller = [self libraryController];
    
    if(controller == [self currentContentController]) return;

    NSBitmapImageRep *currentState = [[self placeholderView] fadeImage], *newState = nil;
    if([currentContentController respondsToSelector:@selector(setCachedSnapshot:)])
        [(id <OEMainWindowContentController>)currentContentController setCachedSnapshot:currentState];
    
    [currentContentController viewWillDisappear];
    [controller                     viewWillAppear];
    
    NSView *placeHolderView = [self placeholderView];
    OEFadeView *fadeView = [[OEFadeView alloc] initWithFrame:[placeHolderView bounds]];
    
    if(currentContentController)
        [placeholderView replaceSubview:[currentContentController view] with:fadeView];
    else 
        [placeholderView addSubview:fadeView];
    
    if([controller respondsToSelector:@selector(cachedSnapshot)])
        newState = [(id <OEMainWindowContentController>)controller cachedSnapshot];
    
    [fadeView fadeFromImage:currentState toImage:newState callback:
     ^{
         [[controller view] setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
         [[controller view] setFrame:[placeHolderView frame]];
         
         [placeHolderView replaceSubview:fadeView with:[controller view]];
         
         [[self window] makeFirstResponder:[controller view]];
         
         [currentContentController viewDidDisappear];
         [controller                     viewDidAppear];
         currentContentController = controller;
         
         [fadeView removeFromSuperview];
     }];
}

#pragma mark -
#pragma mark OELibraryControllerDelegate protocol conformance
- (void)libraryController:(OELibraryController *)sender didSelectGame:(OEDBGame *)aGame
{
    NSError         *error = nil;
    OEDBSaveState   *state = [aGame autosaveForLastPlayedRom];
    OEGameDocument  *gameDocument = nil;

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
    
    if(gameDocument != nil)
    {
        [[NSDocumentController sharedDocumentController] addDocument:gameDocument];
        [self openGameDocument:gameDocument];
    }
    else if(error != nil) [NSApp presentError:error];
}

#pragma mark - OEGameViewControllerDelegate protocol conformance
- (void)emulationDidFinishForGameViewController:(id)sender
{

}
- (void)emulationWillFinishForGameViewController:(OEGameViewController *)sender{
        [self setCurrentContentController:nil];
}
#pragma mark -
#pragma mark NSWindow delegate
- (NSSize)windowWillResize:(NSWindow *)sender toSize:(NSSize)frameSize
{
    return [self allowWindowResizing] ? frameSize : [sender frame].size;
}

- (void)windowWillClose:(NSNotification *)notification
{
    [self setCurrentContentController:nil];
}


- (void)windowDidBecomeMain:(NSNotification *)notification
{
    NSMenu *mainMenu = [NSApp mainMenu];

    NSMenu *windowMenu = [[mainMenu itemAtIndex:5] submenu];
    NSMenuItem *item = [windowMenu itemWithTag:MainMenu_Window_OpenEmuTag];
    [item setState:NSOnState];
}

- (void)windowDidResignMain:(NSNotification *)notification
{
    NSMenu *mainMenu = [NSApp mainMenu];
    
    NSMenu *windowMenu = [[mainMenu itemAtIndex:5] submenu];
    NSMenuItem *item = [windowMenu itemWithTag:MainMenu_Window_OpenEmuTag];
    [item setState:NSOffState];
}
#pragma mark -
#pragma mark Menu Items
- (IBAction)showOpenEmuWindow:(id)sender;
{
    [self close];
}

- (IBAction)launchLastPlayedROM:(id)sender
{
    OEDBRom *rom = [sender representedObject];
    if(!rom) return;
    
    NSError        *error = nil;
    OEGameDocument *gameDocument = [[OEGameDocument alloc] initWithRom:rom error:&error];
    
    if(gameDocument != nil)
    {
        [[NSDocumentController sharedDocumentController] addDocument:gameDocument];
        [self openGameDocument:gameDocument];
    }
    else if(error != nil) [NSApp presentError:error];
}

@end
