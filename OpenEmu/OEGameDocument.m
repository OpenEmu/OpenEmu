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
#import "OEHUDControlsBar.h"

#import "OEROMImporter.h"
#import "OEGameWindowController.h"

@interface OEGameDocument ()

@property (strong) NSTimer * mouseIdleTimer;

- (BOOL)OE_loadRom:(OEDBRom *)rom core:(OECorePlugin*)core withError:(NSError**)outError;
- (BOOL)OE_loadGame:(OEDBGame *)game core:(OECorePlugin*)core withError:(NSError**)outError;

- (void)checkMouseIdleTime:(NSTimer*)aNotification;

- (void)windowDidEnterFullScreen:(NSNotification*)aNotification;
- (void)windowDidExitFullScreen:(NSNotification*)aNotification;

@end

@implementation OEGameDocument
@synthesize gameViewController,mouseIdleTimer;

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
        
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowDidEnterFullScreen:) name:NSWindowDidEnterFullScreenNotification object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowDidExitFullScreen:) name:NSWindowDidExitFullScreenNotification object:nil];
        
    }
    return self;
}

- (void)windowDidEnterFullScreen:(NSNotification *)aNotification
{
    mouseIdleTimer = [NSTimer scheduledTimerWithTimeInterval:3 target:self selector:@selector(checkMouseIdleTime:) userInfo:nil repeats:YES];
    [mouseIdleTimer fire];
}

- (void)windowDidExitFullScreen:(NSNotification *)aNotification
{
    [mouseIdleTimer invalidate];
    [NSCursor setHiddenUntilMouseMoves:NO];
}

- (void)checkMouseIdleTime:(NSTimer*)aNotification
{
    CFTimeInterval mouseIdleTime = CGEventSourceSecondsSinceLastEventType(kCGEventSourceStateCombinedSessionState, kCGEventMouseMoved);
    if (mouseIdleTime >= 3)
    {
        [[gameViewController controlsWindow] hide];
        [NSCursor setHiddenUntilMouseMoves:YES];
    }
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
}

- (void)showInSeparateWindow:(id)sender;
{
    // Create a window, set gameviewcontroller.view as view, open it
    NSUserDefaults *standardDefaults = [NSUserDefaults standardUserDefaults];
    
    BOOL useScreenSize = [standardDefaults boolForKey:UDPopoutHasScreenSizeKey] || ![standardDefaults valueForKey:UDLastPopoutFrameKey];
    NSRect windowRect;
    if(useScreenSize)
    {
        windowRect.size = [[self gameViewController] defaultScreenSize];
        windowRect.origin = NSZeroPoint;
    }
    else
    {
        windowRect = NSRectFromString([standardDefaults stringForKey:UDLastPopoutFrameKey]);
    }
    
    OEGameWindowController *windowController = [[OEGameWindowController alloc] initWithGameViewController:gameViewController contentRect:windowRect];
    
    [self addWindowController:windowController];
    
    [windowController showWindow:self];
    [[windowController window] center];
}

#pragma mark -

- (BOOL)OE_loadGame:(OEDBGame *)game core:(OECorePlugin*)core withError:(NSError **)outError
{
    gameViewController = [[OEGameViewController alloc] initWithGame:game core:core error:outError];
    if(gameViewController == nil) return NO;
    
    [gameViewController setDocument:self];
    
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
    
    [gameViewController setDocument:self];
    
    return YES;
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
    
    OEDBGame *game = [OEDBGame gameWithURL:absoluteURL createIfNecessary:YES error:outError];
    if(!game) 
    {
        DLog(@"game could not be created");
        return NO;
    }
    
    // TODO: Load rom that was just imported instead of the default one
    return [self OE_loadRom:[game defaultROM] core:nil withError:outError];
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
