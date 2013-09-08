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
#import "NSWindow+OEFullScreenAdditions.h"
#import "OESetupAssistant.h"
#import "OECoreUpdater.h"
#import "OELibraryController.h"

#import "NSDocumentController+OEAdditions.h"
#import "NSViewController+OEAdditions.h"
#import "OEGameDocument.h"
#import "OEGameCoreManager.h"
#import "OEGameViewController.h"

#import "OEHUDAlert+DefaultAlertsAdditions.h"
#import "OEDBGame.h"

#import "NSView+FadeImage.h"
#import "OEFadeView.h"

NSString *const OEForcePopoutGameWindowKey = @"forcePopout";
NSString *const OEFullScreenGameWindowKey  = @"fullScreen";
NSString *const OEMainWindowFullscreenKey  = @"mainWindowFullScreen";

#define MainMenu_Window_OpenEmuTag 501

@interface OEMainWindowController () <OELibraryControllerDelegate>
{
    OEGameDocument *_gameDocument;
    BOOL            _shouldExitFullScreenWhenGameFinishes;
    BOOL            _shouldUndockGameWindowOnFullScreenExit;
    BOOL            _resumePlayingAfterFullScreenTransition;
}

@end

@implementation OEMainWindowController

- (void)dealloc
{
    _currentContentController = nil;
    [self setDefaultContentController:nil];
    [self setLibraryController:nil];
    [self setPlaceholderView:nil];
}

- (id)initWithWindow:(NSWindow *)window
{
    if((self = [super initWithWindow:window]) == nil)
        return nil;
    
    // Since restoration from autosave happens before NSWindowController
    // receives -windowDidLoad and we are autosaving the window size, we
    // need to set allowWindowResizing to YES before -windowDidLoad
    _allowWindowResizing = YES;
    
    return self;
}

- (void)windowDidLoad
{
    NSWindow *window = [self window];
    
    [super windowDidLoad];
    
    [[self libraryController] setDelegate:self];
    
    [window setWindowController:self];
    [window setDelegate:self];
    
    // Setup Window behavior
    [window setRestorable:NO];
    [window setExcludedFromWindowsMenu:YES];
    
    if(![[NSUserDefaults standardUserDefaults] boolForKey:OESetupAssistantHasFinishedKey])
    {
        OESetupAssistant *setupAssistant = [[OESetupAssistant alloc] init];
        [setupAssistant setCompletionBlock:
         ^(BOOL discoverRoms, NSArray *volumes)
         {
             if(discoverRoms)
                 [[[OELibraryDatabase defaultDatabase] importer] discoverRoms:volumes];
             [self setCurrentContentController:[self libraryController] animate:NO];
         }];
        
        [window center];
        
        [self setCurrentContentController:setupAssistant];
    }
    else
    {
        [self setCurrentContentController:[self libraryController] animate:NO];
    }
    
#if DEBUG_PRINT
    [window setTitle:[[window title] stringByAppendingString:@" (DEBUG BUILD)"]];
#endif
}

- (NSString *)windowNibName
{
    return @"MainWindow";
}

#pragma mark -

- (void)OE_replaceCurrentContentController:(NSViewController *)oldController withViewController:(NSViewController *)newController
{
    NSView *contentView = [self placeholderView];
    
    // final target
    [[newController view] setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    [[newController view] setFrame:[contentView frame]];
    
    [oldController viewWillDisappear];
    [newController viewWillAppear];
    
    if(oldController != nil)
    {
        if(newController != nil)
            [contentView replaceSubview:[oldController view] with:[newController view]];
        else
            [[oldController view] removeFromSuperview];
    }
    else if(newController != nil)
        [contentView addSubview:[newController view]];
    
    [oldController viewDidDisappear];
    [newController viewDidAppear];
    
    if(newController)
        [[self window] makeFirstResponder:[newController view]];
    
    _currentContentController = newController;
}

- (void)setCurrentContentController:(NSViewController *)controller animate:(BOOL)shouldAnimate
{
    if(controller == nil) controller = [self libraryController];
    
    if(controller == [self currentContentController]) return;

    NSView *placeHolderView = [self placeholderView];

    // We use Objective-C blocks to factor out common code used in both animated and non-animated controller switching
    void (^sendViewWillDisappear)(void) =
    ^{
        [_currentContentController viewWillDisappear];
        [controller viewWillAppear];
    };

    void (^replaceController)(NSView *) =
    ^(NSView *viewToReplace)
    {
        [[controller view] setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
        [[controller view] setFrame:[placeHolderView bounds]];

        if (viewToReplace)
            [placeHolderView replaceSubview:viewToReplace with:[controller view]];
        else
            [placeHolderView addSubview:[controller view]];
        
        [[self window] makeFirstResponder:[controller view]];

        [_currentContentController viewDidDisappear];
        [controller viewDidAppear];
        _currentContentController = controller;

        [viewToReplace removeFromSuperview];

        if(controller == [_gameDocument viewController])
        {
            [_gameDocument setGameWindowController:self];
            [_gameDocument setEmulationPaused:NO];
        }
    };

    if(shouldAnimate)
    {
        NSImage *currentState = [[self placeholderView] fadeImage];
        NSImage *newState = nil;
        if([_currentContentController respondsToSelector:@selector(setCachedSnapshot:)])
            [(id<OEMainWindowContentController>)_currentContentController setCachedSnapshot:currentState];

        sendViewWillDisappear();

        OEFadeView *fadeView = [[OEFadeView alloc] initWithFrame:[placeHolderView bounds]];

        if(_currentContentController)
            [_placeholderView replaceSubview:[_currentContentController view] with:fadeView];
        else
            [_placeholderView addSubview:fadeView];

        if([controller respondsToSelector:@selector(cachedSnapshot)])
            newState = [(id <OEMainWindowContentController>)controller cachedSnapshot];

        [fadeView fadeFromImage:currentState toImage:newState callback:
         ^{
             replaceController(fadeView);
         }];
    }
    else
    {
        sendViewWillDisappear();
        replaceController([_currentContentController view]);
    }
}

- (void)setCurrentContentController:(NSViewController *)newCurrentContentController
{
    [self setCurrentContentController:newCurrentContentController animate:YES];
}

- (IBAction)undockGameWindow:(id)sender
{
    _mainWindowRunsGame = NO;

    if(_shouldExitFullScreenWhenGameFinishes && [[self window] isFullScreen])
    {
        [[self window] toggleFullScreen:self];
        _shouldExitFullScreenWhenGameFinishes = NO;
        _shouldUndockGameWindowOnFullScreenExit = YES;
    }
    else
    {
        [_gameDocument setGameWindowController:nil];
        [self OE_replaceCurrentContentController:[self currentContentController] withViewController:nil];
        [self setCurrentContentController:nil];

        [_gameDocument showInSeparateWindowInFullScreen:NO];
        _gameDocument = nil;
    }
}

#pragma mark - OELibraryControllerDelegate protocol conformance

- (void)OE_openGameDocumentWithGame:(OEDBGame *)game saveState:(OEDBSaveState *)state
{
    NSUserDefaults *standardDefaults = [NSUserDefaults standardUserDefaults];
    BOOL openInSeparateWindow = _mainWindowRunsGame || [standardDefaults boolForKey:OEForcePopoutGameWindowKey];
    BOOL fullScreen = [standardDefaults boolForKey:OEFullScreenGameWindowKey];

    _shouldExitFullScreenWhenGameFinishes = NO;
    void (^openDocument)(OEGameDocument *, NSError *) =
    ^(OEGameDocument *document, NSError *error)
    {
        if(document == nil)
        {
            if([[error domain] isEqualToString:OEGameDocumentErrorDomain] && [error code] == OEFileDoesNotExistError)
            {
                NSString *messageText = [NSString stringWithFormat:NSLocalizedString(@"The game '%@' could not be started because a rom file could not be found. Do you want to locate it?", @""), [game name]];
                if([[OEHUDAlert alertWithMessageText:messageText
                                       defaultButton:NSLocalizedString(@"Locate", @"")
                                     alternateButton:NSLocalizedString(@"Cancel", @"")] runModal] == NSAlertDefaultReturn)
                {
                    OEDBRom  *missingRom = [[game roms] anyObject];
                    NSURL   *originalURL = [missingRom URL];
                    NSString  *extension = [originalURL pathExtension];

                    NSString *panelTitle = [NSString stringWithFormat:NSLocalizedString(@"Locate '%@'", @"Locate panel title"), [[originalURL pathComponents] lastObject]];
                    NSOpenPanel  *panel = [NSOpenPanel openPanel];
                    [panel setTitle:panelTitle];
                    [panel setCanChooseDirectories:NO];
                    [panel setCanChooseFiles:YES];
                    [panel setDirectoryURL:[originalURL URLByDeletingLastPathComponent]];
                    [panel setAllowsOtherFileTypes:NO];
                    [panel setAllowedFileTypes:@[extension]];

                    if([panel runModal])
                    {
                        [missingRom setURL:[panel URL]];
                        [self OE_openGameDocumentWithGame:game saveState:[game autosaveForLastPlayedRom]];
                    }
                }
            } else [NSApp presentError:error];
            return;
        }

        if(openInSeparateWindow) return;

        _shouldExitFullScreenWhenGameFinishes = ![[self window] isFullScreen];
        _gameDocument = document;
        _mainWindowRunsGame = YES;

        if(fullScreen && ![[self window] isFullScreen])
        {
            [NSApp activateIgnoringOtherApps:YES];

            [self OE_replaceCurrentContentController:[self currentContentController] withViewController:nil];
            [[self window] toggleFullScreen:self];
        }
        else
        {
            [self setCurrentContentController:[document viewController]];
            [document setGameWindowController:self];
        }
    };

    if(state != nil && [[OEHUDAlert loadAutoSaveGameAlert] runModal] == NSAlertDefaultReturn)
        [[NSDocumentController sharedDocumentController] openGameDocumentWithSaveState:state display:openInSeparateWindow fullScreen:fullScreen completionHandler:openDocument];
    else
        [[NSDocumentController sharedDocumentController] openGameDocumentWithGame:game display:openInSeparateWindow fullScreen:fullScreen completionHandler:openDocument];
}

- (void)libraryController:(OELibraryController *)sender didSelectGame:(OEDBGame *)game
{
    [self OE_openGameDocumentWithGame:game saveState:[game autosaveForLastPlayedRom]];
}

- (void)libraryController:(OELibraryController *)sender didSelectSaveState:(OEDBSaveState *)saveState
{
    [self OE_openGameDocumentWithGame:nil saveState:saveState];
}

- (void)stopEmulation:(id)sender
{
    // If we are in full screen mode and terminating the emulation will exit full screen,
    // the controller switching animation interferes with the exiting full screen animation.
    // We therefore only animate controller switching in case there won't be a concurrent
    // exit full screen animation. See issue #245.
    [_gameDocument setGameWindowController:nil];
    [_gameDocument stopEmulation:self];
    _gameDocument = nil;
    _mainWindowRunsGame = NO;

    BOOL exitFullScreen = (_shouldExitFullScreenWhenGameFinishes && [[self window] isFullScreen]);
    if(exitFullScreen)
    {
        [[self window] toggleFullScreen:self];
        _shouldExitFullScreenWhenGameFinishes = NO;
    }

    [self setCurrentContentController:nil animate:exitFullScreen];
}

#pragma mark - NSWindow delegate

- (NSSize)windowWillResize:(NSWindow *)sender toSize:(NSSize)frameSize
{
    return [self allowWindowResizing] ? frameSize : [sender frame].size;
}

- (BOOL)windowShouldClose:(id)sender
{
    if([self currentContentController] == [self libraryController])
        return YES;
    else
    {
        [self stopEmulation:self];
        return NO;
    }
}

- (void)windowWillClose:(NSNotification *)notification
{
    // Make sure the current content controller gets viewWillDisappear / viewDidAppear so it has a chance to store its state
    if([self currentContentController] == [self libraryController])
    {
        [[self libraryController] viewWillDisappear];
        [[self libraryController] viewDidDisappear];
        [[self libraryController] viewWillAppear];
        [[self libraryController] viewDidAppear];
    }
    
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

- (void)windowWillEnterFullScreen:(NSNotification *)notification
{
    if(_gameDocument && [_gameDocument gameWindowController] == self)
    {
        _resumePlayingAfterFullScreenTransition = ![_gameDocument isEmulationPaused];
        [_gameDocument setEmulationPaused:YES];
    }
}

- (void)windowDidEnterFullScreen:(NSNotification *)notification
{
    [[NSUserDefaults standardUserDefaults] setBool:YES forKey:OEMainWindowFullscreenKey];
    if(_gameDocument == nil) return;

    if(_shouldExitFullScreenWhenGameFinishes)
    {
        [self setCurrentContentController:[_gameDocument viewController]];
        [_gameDocument setGameWindowController:self];
    }
    
    if(_resumePlayingAfterFullScreenTransition)
        [_gameDocument setEmulationPaused:NO];
}

- (void)windowWillExitFullScreen:(NSNotification *)notification
{
    if(_gameDocument)
    {
        _resumePlayingAfterFullScreenTransition = ![_gameDocument isEmulationPaused];
        [_gameDocument setEmulationPaused:YES];
    }
}

- (void)windowDidExitFullScreen:(NSNotification *)notification
{
    [[NSUserDefaults standardUserDefaults] setBool:NO forKey:OEMainWindowFullscreenKey];

    if(_shouldUndockGameWindowOnFullScreenExit)
    {
        _shouldUndockGameWindowOnFullScreenExit = NO;
        
        [self OE_replaceCurrentContentController:[self currentContentController] withViewController:nil];
        [self setCurrentContentController:nil];

        [_gameDocument showInSeparateWindowInFullScreen:NO];

        if(_resumePlayingAfterFullScreenTransition)
            [_gameDocument setEmulationPaused:NO];

        _gameDocument = nil;
    }
    else if(_gameDocument && _resumePlayingAfterFullScreenTransition)
        [_gameDocument setEmulationPaused:NO];
}

#pragma mark - Menu Items

- (IBAction)launchLastPlayedROM:(id)sender
{
    OEDBGame *game = [[sender representedObject] game];

    [self libraryController:nil didSelectGame:game];
}

@end
