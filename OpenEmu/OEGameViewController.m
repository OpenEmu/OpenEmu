
//
//  OEGameViewController.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 25.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OEGameViewController.h"

#import "OECorePlugin.h"
#import "OEGameLayer.h"
#import "OEGameView.h"
#import "OEGameCore.h"

#import "NSApplication+OEHIDAdditions.h"
#import "OEHIDEvent.h"
#import "OEGameCoreController.h"
#import "OEGameQTRecorder.h"
#import "OECorePickerController.h"
#import "OEGameCoreManager.h"

#import "OEGameCoreHelper.h"

#import "NSString+UUID.h"
@interface OEGameViewController (Private)
- (OECorePlugin *)OE_pluginForFileExtension:(NSString *)ext error:(NSError **)outError;
@end
@implementation OEGameViewController

- (NSString*)nibName{
    return @"OEGameViewController";
}
#pragma mark -
- (void)applicationWillTerminate:(NSNotification *)aNotification
{
    [self terminateEmulation];
}

- (void)terminateEmulation{}

- (void)awakeFromNib{
    [self view];
    
    [gameView setRootProxy:rootProxy];
    [gameView.window setAcceptsMouseMovedEvents:YES];
    
    OEIntSize maxScreenSize = rootProxy.screenSize;
    NSSize aspect = NSMakeSize(maxScreenSize.width, maxScreenSize.height);
    
    NSLog(@"Aspect IS: %@", NSStringFromSize(aspect));
  
    /*
    CGFloat scaleFactor = [view preferredWindowScale];
    [gameWindow setContentSize:NSMakeSize(aspect.width * scaleFactor, aspect.height * scaleFactor)];
    [gameWindow setContentAspectRatio:aspect];
    [gameWindow center];
    //[gameWindow setContentResizeIncrements:aspect];
    */
    //[recorder startRecording];
    
    if([self defaultsToFullScreenMode])
        [self toggleFullScreen:self];
}

- (BOOL)loadFromURL:(NSURL*)url error:(NSError**)outError{
    NSString *romPath = [url path];
    
	
	NSLog(@"%@", [OECorePlugin supportedTypeExtensions]);
    if([[NSFileManager defaultManager] fileExistsAtPath:romPath]){
        OECorePlugin *plugin = [self OE_pluginForFileExtension:[url pathExtension] error:outError];
		
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

#pragma mark -
#pragma mark Emulation utilities
- (void)refresh{
}

- (BOOL)backgroundPauses{
    return [[[NSUserDefaultsController sharedUserDefaultsController] valueForKeyPath:@"values.backgroundPause"] boolValue];
}

- (BOOL)defaultsToFullScreenMode{
    return [[[NSUserDefaultsController sharedUserDefaultsController] valueForKeyPath:@"values.fullScreen"] boolValue];
}

- (BOOL)isEmulationPaused{
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

- (BOOL)isFullScreen{
    return [self.view isInFullScreenMode];
}

- (IBAction)toggleFullScreen:(id)sender
{
    [self setPauseEmulation:YES];
    if(![self.view isInFullScreenMode])
    {
        [self.view enterFullScreenMode:[[self.view window] screen]
                      withOptions:[NSDictionary dictionaryWithObjectsAndKeys:
                                   [NSNumber numberWithBool:NO], NSFullScreenModeAllScreens, nil]];
        [[self.view window] makeFirstResponder:self.view];
        [NSCursor hide];
    }
    else
    {
        [self.view exitFullScreenModeWithOptions:nil];           
        [[self.view window] makeFirstResponder:self.view];
        [NSCursor unhide];
    }
    [self setPauseEmulation:NO];
}

- (IBAction)saveState:(id)sender
{
    NSSavePanel *panel = [NSSavePanel savePanel];
    
    [panel beginSheetModalForWindow:gameView.window
                  completionHandler:
     ^(NSInteger result)
     {
         if(result == NSOKButton) [self saveStateToFile:[[panel URL] path]];
     }];
}

- (IBAction)loadState:(id)sender
{
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    
    [panel beginSheetModalForWindow:gameView.window
                  completionHandler:
     ^(NSInteger result)
     {
         if(result == NSOKButton) [self loadStateFromFile:[[panel URL] path]];
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
    [gameView captureScreenshotUsingBlock:block];
}
#pragma mark -
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

@synthesize emulatorName, gameView;

#pragma mark -
#pragma mark Property Getters / Setters
- (void)setGame:(OEDBGame *)_game{
    [self terminateEmulation];
    
    [_game retain];
    [game release];    
    
    game = _game;
}

- (OEDBGame*)game{
    return game;
}

@end
