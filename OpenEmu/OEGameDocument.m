/*
 Copyright (c) 2009, OpenEmu Team
 
 
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

#import "OEGameDocument.h"
#import "OELibraryDatabase.h"
#import "OEDBRom.h"
#import "OEDBGame.h"

#import "OEApplicationDelegate.h"
#import "OEGameViewController.h"
#import "OEGameCoreManager.h"

#import "NSData+HashingAdditions.h"
#import "OEROMImporter.h"
#import "OEHUDGameWindow.h"
@interface OEGameDocument (Private)
- (BOOL)loadRom:(OEDBRom*)rom withError:(NSError**)outError;
- (BOOL)loadGame:(OEDBGame*)game withError:(NSError**)outError;
- (BOOL)_setupGameViewController:(OEGameViewController*)aGameViewController;
@end
@implementation OEGameDocument
@synthesize gameViewController;
#pragma mark -
- (id)init
{
    self = [super init];
    if(self != nil)
    {
        NSLog(@"OEGameDocument init");
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(applicationWillTerminate:)
                                                     name:NSApplicationWillTerminateNotification
                                                   object:NSApp];
    }
    return self;
}

- (id)initWithRom:(OEDBRom*)rom {
    self = [self init];
    if (self) {
        if(![self loadRom:rom withError:nil])
        {
            [self close];
            return nil;
        }
    }
    return self;
}

- (id)initWithGame:(OEDBGame*)game
{
    self = [self init];
    if (self) {
        if(![self loadGame:game withError:nil])
        {
            [self close];
            return nil;
        }
    }
    return self;
}

- (void)dealloc
{
    NSLog(@"OEGameDocument dealloc");
    [self setGameViewController:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:self name:NSApplicationWillTerminateNotification object:NSApp];
    
    [super dealloc];
}

- (void)applicationWillTerminate:(NSNotification *)aNotification
{
}
#pragma mark -
- (BOOL)_setupGameViewController:(OEGameViewController*)aGameViewController
{
    [aGameViewController setDocument:self];
    OEMainWindowController *winController = (OEMainWindowController*)[(OEApplicationDelegate*)[NSApp delegate] mainWindowController];
    
    NSUserDefaults *standardDefaults = [NSUserDefaults standardUserDefaults];
    BOOL allowPopout = [standardDefaults boolForKey:UDAllowPopoutKey];
    BOOL forcePopout = [standardDefaults boolForKey:UDForcePopoutKey];
    
    BOOL usePopout = forcePopout || (allowPopout && ([winController currentContentController] != [winController defaultContentController]));
    if(usePopout)
    {
        // Create a window, set gameviewcontroller.view as view, open it
        DLog(@"use popout");
        BOOL useScreenSize = [standardDefaults boolForKey:UDPopoutHasScreenSizeKey] || ![standardDefaults valueForKey:UDLastPopoutFrameKey];
        NSRect windowRect;
        if(useScreenSize)
        {
            windowRect.size = [aGameViewController defaultScreenSize];
            windowRect.origin = NSZeroPoint;
        }
        else
        {
            windowRect = NSRectFromString([standardDefaults stringForKey:UDLastPopoutFrameKey]);
        }
        
        OEHUDGameWindow *window = [[OEHUDGameWindow alloc] initWithContentRect:windowRect andGameViewController:aGameViewController];
        
        if(useScreenSize)
            [window center];
        [window makeKeyAndOrderFront:self];
        
        [aGameViewController setWindowController:nil];
    }
    else
    {
        DLog(@"do not use popout");
        [winController setCurrentContentController:aGameViewController];
    }
    [self setGameViewController:aGameViewController];
    
    return YES;
    
}
#pragma mark -
- (BOOL)loadGame:(OEDBGame*)game withError:(NSError**)outError
{
    OEMainWindowController *winController = (OEMainWindowController*)[(OEApplicationDelegate*)[NSApp delegate] mainWindowController];
    OEGameViewController *aGameViewController = [[OEGameViewController alloc] initWithWindowController:winController andGame:game error:outError];
    if(!aGameViewController) return NO;
    
    BOOL res = [self _setupGameViewController:aGameViewController];
    [aGameViewController release];
    return res;
}

- (BOOL)loadRom:(OEDBRom*)rom withError:(NSError**)outError
{
    OEMainWindowController *winController = (OEMainWindowController*)[(OEApplicationDelegate*)[NSApp delegate] mainWindowController];
    OEGameViewController *aGameViewController = [[OEGameViewController alloc] initWithWindowController:winController andRom:rom error:outError];
    if(!aGameViewController)
    {
        DLog(@"no game view controller");
        return NO;
    }
    BOOL res = [self _setupGameViewController:aGameViewController];
    if(!res) DLog(@"_setupGameViewController failed");
    [aGameViewController release];
    return res;
}
#pragma mark -
#pragma mark NSDocument Stuff
- (NSData *)dataOfType:(NSString *)typeName error:(NSError **)outError
{
    DLog(@"%@", typeName);
    
    if(outError != NULL)
        *outError = [NSError errorWithDomain:NSOSStatusErrorDomain code:unimpErr userInfo:NULL];
    return nil;
}

- (BOOL)readFromURL:(NSURL *)absoluteURL ofType:(NSString *)typeName error:(NSError **)outError
{
    DLog(@"%@", absoluteURL);    
    
    NSString *romPath = [absoluteURL path];
    if(![[NSFileManager defaultManager] fileExistsAtPath:romPath])
    {
        if(outError != NULL)
        {
            *outError = [NSError errorWithDomain:OEGameDocumentErrorDomain
                                            code:OEFileDoesNotExistError
                                        userInfo:
                         [NSDictionary dictionaryWithObjectsAndKeys:
                          NSLocalizedString(@"The file you selected doesn't exist", @"Inexistent file error reason."),
                          NSLocalizedFailureReasonErrorKey,
                          NSLocalizedString(@"Choose a valid file.", @"Inexistent file error recovery suggestion."),
                          NSLocalizedRecoverySuggestionErrorKey,
                          nil]];
        }
        DLog(@"File does not exist");    

        return NO;
    }
    
    BOOL isArchive = [typeName isEqualToString:@"Archived Game"];
    if(isArchive)
    {
        // TODO: Handle archived games
        DLog(@"Archived Games are not supported right now!");
        return NO;
    }
    
    // get rom by path
    NSString *filePath = nil;
    if(![absoluteURL isFileURL])
    {
        DLog(@"URLs that are not file urls are currently not supported!");
        // TODO: Handle URLS, by downloading to temp folder
    }
    else
    {
        filePath = [absoluteURL path];
    }
    
    
    OEDBGame *game = [OEDBGame gameWithFilePath:filePath createIfNecessary:YES error:outError];
    if(!game) 
    {
        DLog(@"game could not be created");
        return NO;
    }
    
    // TODO: Load rom that was just imported instead of the default one
    return [self loadRom:[game defaultROM] withError:outError];
}

#pragma mark -
#pragma mark Window management utilities
- (void)windowDidBecomeKey:(NSNotification *)notification
{
    // if([self backgroundPauses]) [self setPauseEmulation:NO];
}

- (void)windowDidResignKey:(NSNotification *)notification
{
    /* if([self backgroundPauses])
     {
     if(![self isFullScreen])
     {
     @try
     {
     [self setPauseEmulation:YES];
     }
     @catch (NSException *e)
     {
     NSLog(@"Failed to pause");
     }
     }
     }*/
}

- (void)windowDidResize:(NSNotification *)notification
{
    
}

- (void)windowWillClose:(NSNotification *)notification
{
    /*if([view isInFullScreenMode]) [self toggleFullScreenMode:self];
     [self terminateEmulation];
     
     //[recorder finishRecording];
     */
}

- (void)performClose:(id)sender
{
    // [gameWindow performClose:sender];
}
@end
