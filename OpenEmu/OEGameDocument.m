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
#import "OEDBSaveState.h"

#import "OEApplicationDelegate.h"
#import "OEGameViewController.h"
#import "OEGameView.h"
#import "OEGameCoreManager.h"

#import "OEBackgroundColorView.h"

#import "OEDistantViewController.h"
#import "NSViewController+OEAdditions.h"
#import "NSView+FadeImage.h"
#import "OEHUDWindow.h"
#import "NSWindow+OEFullScreenAdditions.h"

NSString *const OEPopoutHasScreenSizeKey = @"forceDefaultScreenSize";
NSString *const UDLastPopoutFrameKey     = @"lastPopoutFrame";

NSString *const OEGameDocumentErrorDomain = @"OEGameDocumentErrorDomain";

@interface OEGameDocument ()
- (BOOL)OE_loadRom:(OEDBRom *)rom core:(OECorePlugin*)core withError:(NSError**)outError;
- (BOOL)OE_loadGame:(OEDBGame *)game core:(OECorePlugin*)core withError:(NSError**)outError;

@property (strong) OEGameViewController *gameViewController;
@property (strong) NSViewController *viewController;
- (OEDistantViewController*)distantViewController;

@property (strong) NSWindow *popoutWindow;
@end
#pragma mark -

@implementation OEGameDocument
@synthesize gameViewController, viewController, popoutWindow;

- (id)init
{
    self = [super init];
    if(self != nil)
    {
        DLog(@"init OEGameDocument");
        [[NSNotificationCenter defaultCenter] addObserver:self 
                                                 selector:@selector(applicationWillTerminate:) 
                                                     name:NSApplicationWillTerminateNotification
                                                   object:NSApp];
    
    
        OEDistantViewController *distantViewController = [[OEDistantViewController alloc] init];
        [self setViewController:distantViewController];
    }
    return self;
}

- (id)initWithRom:(OEDBRom *)rom
{
    return [self initWithRom:rom core:nil error:nil];
}
- (id)initWithRom:(OEDBRom *)rom core:(OECorePlugin*)core
{
    return [self initWithRom:rom core:core error:nil];
}
- (id)initWithRom:(OEDBRom *)rom error:(NSError **)outError
{
    return [self initWithRom:rom core:nil error:outError];
}

- (id)initWithRom:(OEDBRom *)rom core:(OECorePlugin*)core error:(NSError **)outError
{
    if((self = [self init]))
    {
        if(![self OE_loadRom:rom core:core withError:outError])
        {
            [self close];
            return nil;
        }
    }
    return self;
}
- (id)initWithGame:(OEDBGame *)game
{
    return [self initWithGame:game core:nil error:nil];
}

- (id)initWithGame:(OEDBGame *)game core:(OECorePlugin*)core
{
    return [self initWithGame:game core:core error:nil];
}

- (id)initWithGame:(OEDBGame *)game error:(NSError **)outError
{
    return [self initWithGame:game core:nil error:outError];
}

- (id)initWithGame:(OEDBGame *)game core:(OECorePlugin*)core error:(NSError **)outError
{
    if((self = [self init]))
    {
        if(![self OE_loadGame:game core:core withError:outError])
        {
            [self close];
            return nil;
        }
    }
    return self;
}

- (id)initWithSaveState:(OEDBSaveState *)state
{
    return [self initWithSaveState:state error:nil];
}

- (id)initWithSaveState:(OEDBSaveState *)state error:(NSError **)outError
{
    if((self = [self init]))
    {
        gameViewController = [[OEGameViewController alloc] initWithSaveState:state error:outError];
        if(gameViewController == nil)
        {
            [self close];
            return nil;
        }
        
        [gameViewController setDocument:self];
        [[self distantViewController] setRealViewController:gameViewController];
    }
    return self;
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self name:NSApplicationWillTerminateNotification object:NSApp];
}

#pragma mark -

- (void)applicationWillTerminate:(NSNotification *)aNotification
{
    [[self gameViewController] terminateEmulation];

    // Since the library database is also saved when OELibraryDatabase receives the same notification, it's possible that
    // the database is saved _before_ we terminate emulation. In order to prevent loss of changes that have happened
    // after the database was saved, we save it again.
    [[OELibraryDatabase defaultDatabase] save:NULL];
}

- (void)showInSeparateWindow:(id)sender fullScreen:(BOOL)fullScreen;
{
    // Create a window, set gameviewcontroller.view as view, open it
    NSUserDefaults *standardDefaults = [NSUserDefaults standardUserDefaults];
    
    BOOL useScreenSize = [standardDefaults boolForKey:OEPopoutHasScreenSizeKey] || ![standardDefaults valueForKey:UDLastPopoutFrameKey];
    NSRect windowRect;
    if(useScreenSize)
    {
        windowRect.size = [[self gameViewController] defaultScreenSize];
        windowRect.origin = NSZeroPoint;
        windowRect.size.height *= 3;
        windowRect.size.height += 21;
        windowRect.size.width *= 3;
    }
    else
    {
        windowRect = NSRectFromString([standardDefaults stringForKey:UDLastPopoutFrameKey]);
    }
    
    [[self gameViewController] viewWillAppear];
    
    OEHUDWindow *window = [[OEHUDWindow alloc] initWithContentRect:windowRect];
    [window setCollectionBehavior:([window collectionBehavior] | NSWindowCollectionBehaviorFullScreenPrimary)];
    [window center];
    [window setContentView:[[self gameViewController] view]];
    [window makeKeyAndOrderFront:self];
    [self setPopoutWindow:window];
    [[self gameViewController] viewDidAppear];
    [window display];
    
    if(fullScreen) [window toggleFullScreen:self];
    
    [self addWindowController:[[NSWindowController alloc] initWithWindow:window]];
}

- (NSString *)displayName
{
    return ([[[self gameViewController] gameView] gameTitle] ? : [super displayName]);
}

#pragma mark -

- (BOOL)OE_loadGame:(OEDBGame *)game core:(OECorePlugin*)core withError:(NSError **)outError
{
    gameViewController = [[OEGameViewController alloc] initWithGame:game core:core error:outError];
    if(gameViewController == nil) return NO;
    
    [[self gameViewController] setDocument:self];
    [[self distantViewController] setRealViewController:gameViewController];
    
    return YES;
}

- (BOOL)OE_loadRom:(OEDBRom *)rom core:(OECorePlugin*)core withError:(NSError **)outError
{
    gameViewController = [[OEGameViewController alloc] initWithRom:rom core:core error:outError];
    if(gameViewController == nil)
    {
        DLog(@"no game view controller");
        return NO;
    }

    [[self gameViewController] setDocument:self];
    [[self distantViewController] setRealViewController:gameViewController];
    
    return YES;
}
#pragma mark -

- (OEDistantViewController *)distantViewController
{
    return (OEDistantViewController *)[self viewController];
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
    
    if ([typeName isEqualToString:@"OpenEmu Save State"])
    {
        OEDBSaveState *state = [OEDBSaveState saveStateWithURL:absoluteURL];
        gameViewController = [[OEGameViewController alloc] initWithSaveState:state error:nil];
        if(gameViewController == nil)
        {
            DLog(@"no game view controller");
            return NO;
        }
        
        [gameViewController setDocument:self];
        return YES;
    }
    
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
    
    OEDBGame *game =  [OEDBGame gameWithURL:absoluteURL inDatabase:[OELibraryDatabase defaultDatabase] error:outError];
    if(game == nil)
    {
        // Could not find game in database. Try to import the file
        OEROMImporter *importer = [[OELibraryDatabase defaultDatabase] importer];
        OEImportItemCompletionBlock completion = ^{
            NSString *fileName    = [[absoluteURL lastPathComponent] stringByDeletingPathExtension];
            NSString *messageText = [NSString stringWithFormat:NSLocalizedString(@"The game '%@' was imported.", ""), fileName];
            NSAlert *alert = [NSAlert alertWithMessageText:messageText defaultButton:@"Yes" alternateButton:@"No" otherButton:nil informativeTextWithFormat:@"Your game finished importing, do you want to play it now?"];
            if([alert runModal] == NSAlertDefaultReturn)
            {
                [[NSDocumentController sharedDocumentController] openDocumentWithContentsOfURL:absoluteURL display:YES completionHandler:nil];
            }
        };
        
        if([importer importItemAtURL:absoluteURL withCompletionHandler:completion])
        {
            if(outError != NULL)
                *outError = [NSError errorWithDomain:OEGameDocumentErrorDomain code:OEImportRequiredError userInfo:nil];
        }
        return NO;
    }
    
    // TODO: Load rom that was just imported instead of the default one
    return [self OE_loadRom:[game defaultROM] core:nil withError:outError];
}

@end
