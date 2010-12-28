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
#import "OECorePickerController.h"
#import "OEGameCoreManager.h"
#import "OEROMFile.h"

#import "OEGameCoreHelper.h"

#import "NSString+UUID.h"

@interface GameDocument ()
- (OECorePlugin *)OE_pluginForFileExtension:(NSString *)ext error:(NSError **)outError;
- (OECorePlugin *)pluginWithName:(NSString *)name;
@end


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
    
    [[NSNotificationCenter defaultCenter] removeObserver:self name:NSApplicationWillTerminateNotification object:NSApp];
    [super dealloc];
}

- (void)applicationWillTerminate:(NSNotification *)aNotification
{
    [self terminateEmulation];
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
    
    OEIntSize maxScreenSize = rootProxy.screenSize;
    NSSize aspect = NSMakeSize(maxScreenSize.width, maxScreenSize.height);
    
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

        OEROMFile *rom = [OEROMFile fileWithPath:romPath createIfNecessary:NO inManagedObjectContext:[[NSApp delegate] managedObjectContext]];
        OECorePlugin *plugin = nil;
        if ((rom != nil) && ([rom preferredEmulator] != nil))
            plugin = [self pluginWithName:[rom preferredEmulator]];
        if (plugin == nil)
            plugin = [self OE_pluginForFileExtension:[absoluteURL pathExtension] error:outError];
        
        if(plugin == nil) return NO;
        
        gameController = [[plugin controller] retain];
        emulatorName = [[plugin displayName] retain];
        
        Class managerClass = ([[[NSUserDefaultsController sharedUserDefaultsController] valueForKeyPath:@"values.gameCoreInBackgroundThread"] boolValue]
                              ? [OEGameCoreThreadManager  class]
                              : [OEGameCoreProcessManager class]);
        
        NSLog(@"managerClass = %@", managerClass);
        gameCoreManager = [[managerClass alloc] initWithROMAtPath:romPath corePlugin:plugin owner:gameController error:outError];
        
        if(gameCoreManager != nil)
        {
            rootProxy = [[gameCoreManager rootProxy] retain];
            
            [gameController addSettingObserver:[rootProxy gameCore]];
            
            [rootProxy setupEmulation];
            
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
    
    return NO;
}

#pragma mark -
#pragma mark Plugin discovery
- (OECorePlugin *)OE_pluginForFileExtension:(NSString *)ext error:(NSError **)outError
{
    OECorePlugin *ret = nil;
    
    
    NSArray *validPlugins = [OECorePlugin pluginsForFileExtension:ext];
    
    if([validPlugins count] <= 1) ret = [validPlugins lastObject];
    else
    {
        OECorePickerController *c = [[[OECorePickerController alloc] initWithCoreList:validPlugins] autorelease];
        
        if([[NSApplication sharedApplication] runModalForWindow:[c window]] == 1)
            ret = [c selectedCore];
    }
    
    if(ret == nil && outError != NULL)
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
    
    return ret;
}

- (OECorePlugin *)pluginWithName:(NSString *)name {
    OECorePlugin *ret = nil;

    for (OECorePlugin *plugin in [OECorePlugin allPlugins]) {
        if ([[plugin displayName] isEqual:name]) {
            ret = plugin;
            break;
        }
    }

    return ret;
}

#pragma mark -
#pragma mark Background process construction and destruction

- (void)terminateEmulation
{
    [view setRootProxy:nil];
    
    [gameController removeSettingObserver:[rootProxy gameCore]];
    [gameWindow makeFirstResponder:nil];
    
    // kill our background friend
    [gameCoreManager stop];
    [gameCoreManager release];
    gameCoreManager = nil;
    
    [rootProxy release];
    rootProxy = nil;
    
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
        [[view window] makeFirstResponder:view];
        [NSCursor hide];
    }
    else
    {
        [view exitFullScreenModeWithOptions:nil];           
        [[view window] makeFirstResponder:view];
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

// FIXME: Need to upgrade state method
- (void)saveStateToFile:(NSString *)fileName
{
    [[rootProxy gameCore] saveStateToFileAtPath: fileName];
}

- (void)loadStateFromFile:(NSString *)fileName
{
    [[rootProxy gameCore] loadStateFromFileAtPath: fileName];
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
    return nil;
}

- (void)captureScreenshotUsingBlock:(void(^)(NSImage *img))block
{
    [view captureScreenshotUsingBlock:block];
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

- (void)windowDidResize:(NSNotification *)notification
{
    //adjust the window to zoom from the center
    if([gameWindow isZoomed]) [gameWindow center];
}

- (void)windowWillClose:(NSNotification *)notification
{
    if([view isInFullScreenMode]) [self toggleFullScreen:self];

    [self terminateEmulation];
    
    //[recorder finishRecording];
    [[GameDocumentController sharedDocumentController] setGameLoaded:NO];
}

- (void)performClose:(id)sender
{
    [gameWindow performClose:sender];
}

#pragma mark TaskWrapper delegate methods

- (void)appendOutput:(NSString *)output fromProcess:(OETaskWrapper *)aTask
{
    printf("%s", [output UTF8String]);
}    

- (void)processStarted:(OETaskWrapper *)aTask
{
}

- (void)processFinished:(OETaskWrapper *)aTask withStatus:(NSInteger)statusCode
{
}

@end
