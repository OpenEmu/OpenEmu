/*
 Copyright (c) 2009, OpenEmu Team
 All rights reserved.
 
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

#import "GameDocument.h"
#import "OECorePlugin.h"
#import "GameDocumentController.h"
#import "OEGameLayer.h"
#import "OEGameView.h"
#import "GameCore.h"
#import "NSApplication+OEHIDAdditions.h"
#import "OEHIDEvent.h"
#import "OEGameCoreController.h"
#import "GameQTRecorder.h"

#import "OEGameCoreHelper.h"

#import "NSString+UUID.h"

NSString *const OEGameDocumentErrorDomain = @"OEGameDocumentErrorDomain";

@implementation GameDocument

@synthesize emulatorName, view, gameWindow, playPauseToolbarItem;

- (id)init
{
    self = [super init];
    if(self != nil)
    {
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(applicationWillTerminate:)
                                                     name:NSApplicationWillTerminateNotification
                                                   object:NSApp];
    }
    return self;
}

- (void)dealloc
{
    [playPauseToolbarItem release];
    [view release];
    [gameWindow release];
    [taskUUIDForDOServer release];
    
    [[NSNotificationCenter defaultCenter] removeObserver:self name:NSApplicationWillTerminateNotification object:NSApp];
    [super dealloc];
}

- (void)applicationWillTerminate:(NSNotification *)aNotification
{
    [self endHelperProcess];
}

- (NSString *)windowNibName
{
    // Override returning the nib file name of the document
    // If you need to use a subclass of NSWindowController or if your document supports multiple NSWindowControllers, you should remove this method and override -makeWindowControllers instead.
    return @"GameDocument";
}

- (void)windowControllerDidLoadNib:(NSWindowController *) aController
{
    [view setRootProxy:rootProxy];
    [gameWindow setAcceptsMouseMovedEvents:YES];
    
    NSSize aspect = NSMakeSize([rootProxy screenWidth], [rootProxy screenHeight]);
    
    NSLog(@"Aspect IS: %@", NSStringFromSize(aspect));
    
    CGFloat scaleFactor = [view preferredWindowScale];
    [gameWindow setContentSize:NSMakeSize(aspect.width * scaleFactor, aspect.height * scaleFactor)];
    [gameWindow setContentAspectRatio:aspect];
    [gameWindow center];
    //[gameWindow setContentResizeIncrements:aspect];
    
    //[recorder startRecording];
    [gameWindow makeKeyAndOrderFront:self];
    
    if([self defaultsToFullScreenMode])
        [self toggleFullScreen:self];
}

- (NSData *)dataOfType:(NSString *)typeName error:(NSError **)outError
{
    if(outError != NULL)
        *outError = [NSError errorWithDomain:NSOSStatusErrorDomain code:unimpErr userInfo:NULL];
    return nil;
}

- (BOOL)readFromURL:(NSURL *)absoluteURL ofType:(NSString *)typeName error:(NSError **)outError
{
    NSString *romPath = [absoluteURL path];
    if([[NSFileManager defaultManager] fileExistsAtPath:romPath])
    {
        DLog(@"%@", self);
        
        GameDocumentController *docControl = [GameDocumentController sharedDocumentController];
        OECorePlugin *plugin = nil;
        
        for(OEPlugin *aPlugin in [docControl plugins])
            if([[aPlugin displayName] isEqualToString:typeName])
            {
                plugin = (OECorePlugin *)aPlugin;
                break;
            }
        
        if(plugin != nil)
        {
            gameController = [[plugin controller] retain];
            emulatorName = [[plugin displayName] retain];
            
            if([self startHelperProcessError:outError])
            {
                GameCore *gameCore = nil;
                if([rootProxy loadRomAtPath:romPath withCorePluginAtPath:[[plugin bundle] bundlePath] gameCore:&gameCore])
                {
                    [gameCore setOwner:gameController];
                    [gameController addSettingObserver:gameCore];
                    
                    [rootProxy setupEmulation];
                    
                    return YES;
                }
            }
        }
        else if(outError != NULL)
        {
            *outError = [NSError errorWithDomain:OEGameDocumentErrorDomain
                                            code:OEIncorrectFileError
                                        userInfo:
                         [NSDictionary dictionaryWithObjectsAndKeys:
                          NSLocalizedString(@"The launched file isn't handled by OpenEmu", @"Incorrect file error reason."),
                          NSLocalizedFailureReasonErrorKey,
                          NSLocalizedString(@"Choose a file with a supported file format or download an appropriate OpenEmu plugin.", @"Incorrect file error recovery suggestion."),
                          NSLocalizedRecoverySuggestionErrorKey,
                          nil]];
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
    
    return NO;
}

#pragma mark -
#pragma mark Background process construction and destruction

- (BOOL)startHelperProcessError:(NSError **)outError
{
    // run our background task. Get our IOSurface ids from its standard out.
    NSString *cliPath = [[NSBundle bundleForClass:[self class]] pathForResource: @"OpenEmuHelperApp" ofType: @""];
    
    // generate a UUID string so we can have multiple screen capture background tasks running.
    taskUUIDForDOServer = [[NSString stringWithUUID] retain];
    // NSLog(@"helper tool UUID should be %@", taskUUIDForDOServer);
    
    NSArray *args = [NSArray arrayWithObjects: cliPath, taskUUIDForDOServer, nil];
    
    helper = [[TaskWrapper alloc] initWithController:self arguments:args userInfo:nil];
    [helper startProcess];
    
    if(![helper isRunning])
    {
        [helper release];
        if(outError != NULL)
        {
            *outError = [NSError errorWithDomain:OEGameDocumentErrorDomain
                                            code:OEHelperAppNotRunningError
                                        userInfo:[NSDictionary dictionaryWithObject:NSLocalizedString(@"The background process couldn't be launched", @"Not running background process error") forKey:NSLocalizedFailureReasonErrorKey]];
        }
        return NO;
    }
    
    // now that we launched the helper, start up our NSConnection for DO object vending and configure it
    // this is however a race condition if our helper process is not fully launched yet. 
    // we hack it out here. Normally this while loop is not noticable, its very fast
    
    NSDate *start = [NSDate date];
    
    taskConnection = nil;
    while(taskConnection == nil)
    {
        taskConnection = [NSConnection connectionWithRegisteredName:[NSString stringWithFormat:@"com.openemu.OpenEmuHelper-%@", taskUUIDForDOServer, nil] host:nil];
        
        if(-[start timeIntervalSinceNow] > 3.0)
        {
            [self endHelperProcess];
            if(outError != NULL)
            {
                *outError = [NSError errorWithDomain:OEGameDocumentErrorDomain
                                                code:OEConnectionTimedOutError
                                            userInfo:[NSDictionary dictionaryWithObject:NSLocalizedString(@"Couldn't connect to the background process.", @"Timed out error reason.") forKey:NSLocalizedFailureReasonErrorKey]];
            }
            return NO;
        }
    }
    
    [taskConnection retain];
    
    if(![taskConnection isValid])
    {
        [self endHelperProcess];
        if(outError != NULL)
        {
            *outError = [NSError errorWithDomain:OEGameDocumentErrorDomain
                                            code:OEInvalidHelperConnectionError
                                        userInfo:[NSDictionary dictionaryWithObject:NSLocalizedString(@"The background process connection couldn't be established", @"Invalid helper connection error reason.") forKey:NSLocalizedFailureReasonErrorKey]];
        }
        return NO;
    }
    
    // now that we have a valid connection...
    rootProxy = [[taskConnection rootProxy] retain];
    if(rootProxy == nil)
    {
        NSLog(@"nil root proxy object?");
        [self endHelperProcess];
        if(outError != NULL)
        {
            *outError = [NSError errorWithDomain:OEGameDocumentErrorDomain
                                            code:OENilRootProxyObjectError
                                        userInfo:[NSDictionary dictionaryWithObject:NSLocalizedString(@"The root proxy object is nil.", @"Nil root proxy object error reason.") forKey:NSLocalizedFailureReasonErrorKey]];
        }
        return NO;
    }
    
    [(NSDistantObject *)rootProxy setProtocolForProxy:@protocol(OEGameCoreHelper)];
    
    return YES;
}

- (void)endHelperProcess
{
    [view setRootProxy:nil];
    
    [gameController removeSettingObserver:[rootProxy gameCore]];
    [gameWindow makeFirstResponder:nil];
    
    // kill our background friend
    [helper stopProcess];
    helper = nil;
    
    [rootProxy release];
    rootProxy = nil;
    
    [taskConnection release];
    taskConnection = nil;
    
    [gameController release];
    gameController = nil;
    
    [emulatorName release];
    emulatorName = nil;
}

#pragma mark -
#pragma mark Emulation utilities

- (void)refresh
{
}

- (BOOL)backgroundPauses
{
    return [[[NSUserDefaultsController sharedUserDefaultsController] valueForKeyPath:@"values.backgroundPause"] boolValue];
}

- (BOOL)defaultsToFullScreenMode
{
    return [[[NSUserDefaultsController sharedUserDefaultsController] valueForKeyPath:@"values.fullScreen"] boolValue];
}

- (BOOL)isEmulationPaused
{
    return [rootProxy isEmulationPaused];
}

- (void)setPauseEmulation:(BOOL)flag
{    
    [rootProxy setPauseEmulation:flag];

    if (flag)
    {
        [playPauseToolbarItem setImage:[NSImage imageNamed:NSImageNameRightFacingTriangleTemplate]];
        [playPauseToolbarItem setLabel:@"Play"];
    }
    else
    {
        [playPauseToolbarItem setImage:[NSImage imageNamed:NSImageNameStopProgressTemplate]];
        [playPauseToolbarItem setLabel:@"Pause"];
    }
}

- (BOOL)isFullScreen
{
    return [view isInFullScreenMode];
}

- (IBAction)toggleFullScreen:(id)sender
{
    [self setPauseEmulation:YES];
    if(![view isInFullScreenMode])
    {
        [view enterFullScreenMode:[[view window] screen]
                      withOptions:[NSDictionary dictionaryWithObjectsAndKeys:
                                   [NSNumber numberWithBool:NO], NSFullScreenModeAllScreens, nil]];
        [NSCursor hide];
    }
    else
    {
        [view exitFullScreenModeWithOptions:nil];           
        [NSCursor unhide];
    }
    [self setPauseEmulation:NO];
}

- (IBAction)saveState:(id)sender
{
    NSSavePanel *panel = [NSSavePanel savePanel];
    
    [panel beginSheetModalForWindow:gameWindow
                  completionHandler:
     ^(NSInteger result)
     {
         if(result == NSOKButton) [self saveStateToFile:[panel filename]];
     }];
}

- (IBAction)loadState:(id)sender
{
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    
    [panel beginSheetModalForWindow:gameWindow
                  completionHandler:
     ^(NSInteger result)
     {
         if(result == NSOKButton) [self loadStateFromFile:[panel filename]];
     }];
}

- (void)saveStateToFile:(NSString *)fileName
{
    //GameCore *gameCore = [rootProxy gameCore];
    //if([gameCore respondsToSelector:@selector(saveStateToFileAtPath:)])
    //    [gameCore saveStateToFileAtPath: fileName];
}

- (void)loadStateFromFile:(NSString *)fileName
{
    //GameCore *gameCore = [rootProxy gameCore];
    //if([gameCore respondsToSelector:@selector(loadStateFromFileAtPath:)])
    //    [gameCore loadStateFromFileAtPath: fileName];
}

- (IBAction)scrambleRam:(id)sender
{
    [self scrambleBytesInRam:100];
}

- (void)scrambleBytesInRam:(NSUInteger)bytes
{
    //for(NSUInteger i = 0; i < bytes; i++)
    //    [gameCore setRandomByte];
}

- (IBAction)resetGame:(id)sender
{
    [[rootProxy gameCore] resetEmulation];
}

- (IBAction)playPauseGame:(id)sender
{
    [self setPauseEmulation:![self isEmulationPaused]];
}

- (NSImage *)screenShot
{
    return [view imageForCurrentFrame];
}

#pragma mark -
#pragma mark Window management utilities

- (void)windowDidBecomeKey:(NSNotification *)notification
{
    [[GameDocumentController sharedDocumentController] setGameLoaded:YES];
    if([self backgroundPauses]) [self setPauseEmulation:NO];
}

- (void)windowDidResignKey:(NSNotification *)notification
{
    if([self backgroundPauses])
    {
        if(![self isFullScreen])
        {
            @try {
                [self setPauseEmulation:YES];
            }
            @catch (NSException * e) {
                NSLog(@"Failed to pause");
            }
        }
    }
}

/*- (NSSize)windowWillResize:(NSWindow *)sender toSize:(NSSize)proposedFrameSize
{
    // We want to force aspect ratio with resize increments
    int scale;
    if(proposedFrameSize.width < proposedFrameSize.height)
        scale = proposedFrameSize.width / [gameCore screenWidth];
    else
        scale = proposedFrameSize.height / [gameCore screenHeight];
    scale = MAX(scale, 1);
    
    NSRect newContentRect = NSMakeRect(0,0, [gameCore screenWidth] * scale, [gameCore screenHeight] * scale);
    return [sender frameRectForContentRect:newContentRect].size;
}*/

- (void)windowDidResize:(NSNotification *)notification
{
    //adjust the window to zoom from the center
    if([gameWindow isZoomed]) [gameWindow center];
}

- (void)windowWillClose:(NSNotification *)notification
{
    if([view isInFullScreenMode]) [self toggleFullScreen:self];

    [self endHelperProcess];
    
    //[recorder finishRecording];
    [[GameDocumentController sharedDocumentController] setGameLoaded:NO];
}

- (void)performClose:(id)sender
{
    [gameWindow performClose:sender];
}

#pragma mark TaskWrapper delegate methods

- (void)appendOutput:(NSString *)output fromProcess:(TaskWrapper *)aTask
{
    printf("%s", [output UTF8String]);
}    

- (void)processStarted:(TaskWrapper *)aTask
{
}

- (void)processFinished:(TaskWrapper *)aTask withStatus:(NSInteger)statusCode
{
}

@end
