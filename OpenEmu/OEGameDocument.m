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

#import "OEApplicationDelegate.h"
#import "OEGameViewController.h"
#import "OEGameCoreManager.h"

#import "NSData+HashingAdditions.h"
#import "OEROMImporter.h"
@interface OEGameDocument (Private)
- (OEDBRom*)_romFromURL:(NSURL*)url;
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
    NSLog(@"OEGameDocument dealloc start");
    self.gameViewController = nil;
    [[NSNotificationCenter defaultCenter] removeObserver:self name:NSApplicationWillTerminateNotification object:NSApp];
    
    NSLog(@"OEGameDocument dealloc end");
    [super dealloc];
}

- (void)applicationWillTerminate:(NSNotification *)aNotification
{
    
}
#pragma mark -
- (BOOL)_setupGameViewController:(OEGameViewController*)aGameViewController
{
    [aGameViewController setDocument:self];
    OEMainWindowController* winController = (OEMainWindowController*)[(OEApplicationDelegate*)[NSApp delegate] mainWindowController];
    
    NSUserDefaults* standardDefaults = [NSUserDefaults standardUserDefaults];
    BOOL allowPopout = [standardDefaults boolForKey:UDAllowPopoutKey];
    BOOL forcePopout = [standardDefaults boolForKey:UDForcePopoutKey];
    
    BOOL usePopout = forcePopout || (allowPopout && ([winController currentContentController] != [winController defaultContentController]));
    if(usePopout)
    {
        // Create a window, set gameviewcontroller.view as view, open it
        NSLog(@"use popout");
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
        
        NSWindow* window = [[NSWindow alloc] initWithContentRect:windowRect styleMask:NSTitledWindowMask|NSClosableWindowMask|NSResizableWindowMask|NSMiniaturizableWindowMask backing:NSWindowBackingLocationDefault defer:NO];
        [window setHasShadow:YES];
        
        [aGameViewController.view setFrame:[[window contentView] bounds]];
        [aGameViewController.view setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];
        [[window contentView] addSubview:aGameViewController.view];
        
        if(useScreenSize)
            [window center];
        [window makeKeyAndOrderFront:self];
        [window setReleasedWhenClosed:YES];
    }
    else
    {
        NSLog(@"do not use popout");
        [winController setCurrentContentController:aGameViewController];
    }
    [self setGameViewController:aGameViewController];
    
    return YES;
    
}
#pragma mark -
- (BOOL)loadGame:(OEDBGame*)game withError:(NSError**)outError
{
    OEMainWindowController* winController = (OEMainWindowController*)[(OEApplicationDelegate*)[NSApp delegate] mainWindowController];
    OEGameViewController* aGameViewController = [[OEGameViewController alloc] initWithWindowController:winController andGame:game error:outError];
    if(!aGameViewController) return NO;
    
    BOOL res = [self _setupGameViewController:aGameViewController];
    [aGameViewController release];
    return res;
}
- (BOOL)loadRom:(OEDBRom*)rom withError:(NSError**)outError
{
    OEMainWindowController* winController = (OEMainWindowController*)[(OEApplicationDelegate*)[NSApp delegate] mainWindowController];
    OEGameViewController* aGameViewController = [[OEGameViewController alloc] initWithWindowController:winController andRom:rom error:outError];
    if(!aGameViewController) return NO;
    
    BOOL res = [self _setupGameViewController:aGameViewController];
    [aGameViewController release];
    return res;
}
#pragma mark -
#pragma mark NSDocument Stuff

- (NSData *)dataOfType:(NSString *)typeName error:(NSError **)outError
{
    if(outError != NULL)
        *outError = [NSError errorWithDomain:NSOSStatusErrorDomain code:unimpErr userInfo:NULL];
    return nil;
}

- (BOOL)readFromURL:(NSURL *)absoluteURL ofType:(NSString *)typeName error:(NSError **)outError
{
    NSLog(@"readFromURL:ofType:error: %d", [NSThread isMainThread]);
    
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
        return NO;
    }
    
    BOOL isArchive = [typeName isEqualToString:@"Archived Game"];
    if(isArchive)
    {
        // TODO: Handle archived games
        NSLog(@"Archived Games are not supported right now!");
        return NO;
    }
    
    // get rom by path
    OEDBRom* rom = [self _romFromURL:absoluteURL];
    if(rom == nil)
    {
        NSLog(@"Could not import file as new rom, should not happen!");
        return NO;
    }
    return [self loadRom:rom withError:outError];
    
    /*
     NSString *romPath = [absoluteURL path];
     if([[NSFileManager defaultManager] fileExistsAtPath:romPath])
     {
     DLog(@"%@", self);
     
     OECorePlugin *plugin = [self OE_pluginForFileExtension:[absoluteURL pathExtension] error:outError];
     
     if(plugin == nil) return NO;
     
     gameController = [[plugin controller]  retain];
     emulatorName   = [[plugin displayName] retain];
     
     [gameSystemController registerGameSystemResponder:gameSystemResponder];
     
     Class managerClass = ([[[NSUserDefaultsController sharedUserDefaultsController] valueForKeyPath:@"values.gameCoreInBackgroundThread"] boolValue]
     ? [OEGameCoreThreadManager  class]
     : [OEGameCoreProcessManager class]);
     
     NSLog(@"managerClass = %@", managerClass);
     gameCoreManager = [[managerClass alloc] initWithROMAtPath:romPath corePlugin:plugin owner:gameController error:outError];
     
     if(gameCoreManager != nil)
     {
     rootProxy = [[gameCoreManager rootProxy] retain];
     
     [rootProxy setupEmulation];
     
     OEGameCore *gameCore = [rootProxy gameCore];
     
     gameSystemController = [[[OESystemPlugin gameSystemPluginForIdentifier:[gameCore systemIdentifier]] controller] retain];
     gameSystemResponder  = [gameSystemController newGameSystemResponder];
     
     [gameSystemResponder setClient:gameCore];
     
     return YES;
     }
     }
     else if(outError != NULL)
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
     */
    return NO;
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
#pragma mark -
#pragma mark Private
- (OEDBRom*)_romFromURL:(NSURL*)url
{
    OEDBRom* rom = nil;
    BOOL isMD5Hash = [[NSUserDefaults standardUserDefaults] boolForKey:UDUseMD5HashingKey];    
    NSString* hash = nil;
    
    OELibraryDatabase* database = [OELibraryDatabase defaultDatabase];
    
    rom = [database romForWithPath:[url path]];
    if(rom == nil)
    {
        NSData* file = [NSData dataWithContentsOfURL:url options:NSDataReadingUncached error:nil];
        if(!file) 
        {
            NSLog(@"can not load file into data");
            return nil;
        }
        
        if(isMD5Hash)
        {
            hash = [file MD5HashString];
            rom = [database romForMD5Hash:hash];
        }
        else
        {
            hash = [file CRC32HashString];
            rom = [database romForCRC32Hash:hash];
        }
    }
    
    if(rom == nil)
    {
        NSLog(@"no rom after hashing: %@ %@", isMD5Hash?@"MD5":@"CRC32", hash);
        OEROMImporter* importer = [[OEROMImporter alloc] initWithDatabase:database];
        [importer setErrorBehaviour:OEImportErrorAskUser]; // TODO: set proper error behaviour
        BOOL success = [importer importROMsAtURL:url inBackground:NO error:nil];
        if(!success)
        {
            [importer release];
            NSLog(@"importing was not sucessfull");
            return nil;
        }
        
        rom = [[importer importedRoms] lastObject];
        [importer release];
        
        NSLog(@"rom after hashing and import: %@", rom);
    }
    
    return rom;
}
@end
