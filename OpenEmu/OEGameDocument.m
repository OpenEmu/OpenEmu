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
#import "OEHUDAlert+DefaultAlertsAdditions.h"
#import "OEPopoutGameWindowController.h"

NSString *const OEGameDocumentErrorDomain = @"OEGameDocumentErrorDomain";

@interface OEGameDocument ()
{
    BOOL _isTerminatingEmulation;
}

- (BOOL)OE_loadRom:(OEDBRom *)rom core:(OECorePlugin *)core withError:(NSError **)outError;
- (BOOL)OE_loadGame:(OEDBGame *)game core:(OECorePlugin *)core withError:(NSError **)outError;

@property OEGameViewController *gameViewController;
@property NSViewController *viewController;

- (OEDistantViewController *)distantViewController;

@end

#pragma mark -

@implementation OEGameDocument

- (id)init
{
    if((self = [super init]) != nil)
    {
        DLog(@"init OEGameDocument");
        [self setViewController:[[OEDistantViewController alloc] init]];
    }

    return self;
}

- (id)initWithRom:(OEDBRom *)rom core:(OECorePlugin *)core error:(NSError **)outError
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

- (id)initWithGame:(OEDBGame *)game core:(OECorePlugin *)core error:(NSError **)outError
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

- (id)initWithSaveState:(OEDBSaveState *)state error:(NSError **)outError
{
    if((self = [self init]))
    {
        _gameViewController = [[OEGameViewController alloc] initWithSaveState:state error:outError];
        if(_gameViewController == nil)
        {
            [self close];
            return nil;
        }

        [_gameViewController setDocument:self];
        [[self distantViewController] setRealViewController:_gameViewController];
    }
    return self;
}

- (void)close
{
    if(_isTerminatingEmulation) return;

    _isTerminatingEmulation = YES;
    [[self gameViewController] terminateEmulation];
    [super close];
    _isTerminatingEmulation = NO;
}

#pragma mark -

- (void)showInSeparateWindow:(id)sender fullScreen:(BOOL)fullScreen;
{
    OEHUDWindow *window                            = [[OEHUDWindow alloc] initWithContentRect:NSZeroRect];
    OEPopoutGameWindowController *windowController = [[OEPopoutGameWindowController alloc] initWithWindow:window];

    [windowController setWindowFullScreen:fullScreen];
    [self addWindowController:windowController];
    [self showWindows];
}

- (NSString *)displayName
{
    // If we do not have a title yet, return an empty string instead of [super displayName].
    // The latter uses Cocoa document architecture and relies on documents having URLs,
    // including untitled (new) documents.
    return ([[[self gameViewController] gameView] gameTitle] ? : @"");
}

#pragma mark -

- (BOOL)OE_loadGame:(OEDBGame *)game core:(OECorePlugin*)core withError:(NSError **)outError
{
    _gameViewController = [[OEGameViewController alloc] initWithGame:game core:core error:outError];
    if(_gameViewController == nil) return NO;

    [[self gameViewController] setDocument:self];
    [[self distantViewController] setRealViewController:_gameViewController];

    return YES;
}

- (BOOL)OE_loadRom:(OEDBRom *)rom core:(OECorePlugin*)core withError:(NSError **)outError
{
    _gameViewController = [[OEGameViewController alloc] initWithRom:rom core:core error:outError];
    if(_gameViewController == nil)
    {
        DLog(@"no game view controller");
        return NO;
    }

    [[self gameViewController] setDocument:self];
    [[self distantViewController] setRealViewController:_gameViewController];

    return YES;
}

- (BOOL)OE_loadSaveState:(OEDBSaveState *)state withError:(NSError **)outError
{
    _gameViewController = [[OEGameViewController alloc] initWithSaveState:state error:outError];
    if(_gameViewController == nil)
    {
        DLog(@"no game view controller");
        return NO;
    }

    [[self gameViewController] setDocument:self];
    [[self distantViewController] setRealViewController:_gameViewController];

    return YES;
}

#pragma mark -

- (OEDistantViewController *)distantViewController
{
    return (OEDistantViewController *)[self viewController];
}

#pragma mark - NSDocument Stuff

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

    if([typeName isEqualToString:@"OpenEmu Save State"])
    {
        OEDBSaveState *state = [OEDBSaveState saveStateWithURL:absoluteURL];
        _gameViewController = [[OEGameViewController alloc] initWithSaveState:state error:nil];

        if(_gameViewController == nil)
        {
            DLog(@"no game view controller");
            return NO;
        }

        [_gameViewController setDocument:self];
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
    else filePath = [absoluteURL path];

    OEDBGame *game = [OEDBGame gameWithURL:absoluteURL inDatabase:[OELibraryDatabase defaultDatabase] error:outError];
    if(game == nil)
    {
        // Could not find game in database. Try to import the file
        OEROMImporter *importer = [[OELibraryDatabase defaultDatabase] importer];
        OEImportItemCompletionBlock completion =
        ^{
            OEHUDAlert *alert = [[OEHUDAlert alloc] init];

            NSString *fileName    = [[absoluteURL lastPathComponent] stringByDeletingPathExtension];
            NSString *messageText = [NSString stringWithFormat:NSLocalizedString(@"The game '%@' was imported.", ""), fileName];
            
            alert.headlineText = NSLocalizedString(@"Your game finished importing, do you want to play it now?", @"");
            alert.messageText = messageText;
            alert.defaultButtonTitle = NSLocalizedString(@"Yes", @"");
            alert.alternateButtonTitle = NSLocalizedString(@"No", @"");

            if([alert runModal] == NSAlertDefaultReturn)
                [[NSDocumentController sharedDocumentController] openDocumentWithContentsOfURL:absoluteURL display:YES completionHandler:nil];
        };

        if([importer importItemAtURL:absoluteURL withCompletionHandler:completion])
        {
            if(outError != NULL)
                *outError = [NSError errorWithDomain:OEGameDocumentErrorDomain code:OEImportRequiredError userInfo:nil];
        }

        return NO;
    }

    // TODO: Load rom that was just imported instead of the default one
    OEDBSaveState   *state = [game autosaveForLastPlayedRom];
    if(state && [[OEHUDAlert loadAutoSaveGameAlert] runModal] == NSAlertDefaultReturn)
        return [self OE_loadSaveState:state withError:outError];
    else
        return [self OE_loadRom:[game defaultROM] core:nil withError:outError];
}

@end
