//
//  OENewGameDocument.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 05.09.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OENewGameDocument.h"

#import "OESystemPlugin.h"
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
#import "OESystemResponder.h"
#import "OESystemController.h"

#import "OEGameCoreHelper.h"

#import "NSString+UUID.h"
#import "OEHUDControls.h"
#import "OEHUDGameWindow.h"

#import "OEDBSaveState.h"
@interface OENewGameDocument (Private)
- (void)_setup;
- (BOOL)loadFromURL:(NSURL*)url error:(NSError**)outError;
- (OECorePlugin *)OE_pluginForFileExtension:(NSString *)ext error:(NSError **)outError;

- (void)_setURL:(NSURL*)aURL;
- (void)_setROM:(id)aRom;
@end
@implementation OENewGameDocument
@synthesize url, rom;
+ (id)newDocumentWithROM:(id)aRom error:(NSError**)error{
	id result = [[self alloc] init];
	
	[result _setROM:aRom];
	NSString* path = [aRom valueForKey:@"path"];
	
	if(!path){
		[result release];
		return nil;
	}
	
	[result _setup];
	NSURL* url = [NSURL fileURLWithPath:path];
	if(![result loadFromURL:url error:error]){
		[result release];
		return nil;
	}
	
	return result;
}

+ (id)newDocumentWithRomAtURL:(NSURL*)aUrl error:(NSError**)error{
	id result = [[self alloc] init];
	
	[result _setURL:aUrl];
	[result _setup];
	if(![result loadFromURL:aUrl error:error]){
		[result release];
		return nil;
	}
	
	return result;
}

- (void)dealloc {
	NSNotificationCenter* nc = [NSNotificationCenter defaultCenter];
	[nc removeObserver:self name:NSApplicationWillTerminateNotification object:NSApp];
    [nc removeObserver:self name:NSViewFrameDidChangeNotification object:gameView];
	[nc removeObserver:self name:@"OEGameViewDidMoveToWindow" object:nil];
	
	[controlsWindow release], controlsWindow = nil;
	[gameView release], gameView = nil;
	
	[rom release], rom = nil;
	[url release], url = nil;
	 
	[super dealloc];
}
#pragma mark -
#pragma mark Displaying
- (void)openWindow:(NSRect)contentFrame{
	OEHUDGameWindow* gameWindow = [[OEHUDGameWindow alloc] initWithContentRect:contentFrame andGameDocument:self];
	[gameWindow makeKeyAndOrderFront:self];
    [gameWindow center];
	// check if we default to fullscreen, then go fullscreen
}

- (OEGameView*)gameView{
	return gameView;
}
#pragma mark -
#pragma mark Emulation State
- (void)resetGame{
    [[rootProxy gameCore] resetEmulation];
	// TODO: draw one frame to reflect reset
}

- (void)terminateEmulation{
	[gameView setRootProxy:nil];
    [gameView setGameResponder:nil];
    
    [gameController removeSettingObserver:[rootProxy gameCore]];
//    [gameWindow makeFirstResponder:nil];
    
    [gameSystemController release];
    gameSystemController = nil;
    [gameSystemResponder release];
    gameSystemResponder  = nil;
    
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

- (void)pauseGame{
	[self setPauseEmulation:YES];
}

- (void)playGame{
	[self setPauseEmulation:NO];
}

- (BOOL)isEmulationPaused{
	return [rootProxy isEmulationPaused];
}

- (void)setPauseEmulation:(BOOL)flag{
	[rootProxy setPauseEmulation:flag];
	// TODO: Update HUD Button state
}

- (void)setVolume:(float)volume{
	[rootProxy setVolume:volume];
	[[NSUserDefaults standardUserDefaults] setValue:[NSNumber numberWithFloat:volume] forKey:UDVolumeKey];
}
#pragma mark -
#pragma mark Save States
- (void)loadState:(id)state{}
- (void)saveStateAskingUser:(NSString *)proposedName{
	return;
	if(!proposedName){
		// TODO: properly format date
		proposedName = [NSString stringWithFormat:@"%@", [NSDate date]];
	}
	
	NSString* name = nil;
	[self saveState:name];

}

#define OESaveStatePath @""
- (void)saveState:(NSString*)stateName{
	return;
	if(!self.rom){
		NSLog(@"Error: Can not save states without rom");
		return;
	}
	
	NSString* systemIdentifier		= [[rootProxy gameCore] systemIdentifier];
	NSString* systemSaveDirectory	= [OESaveStatePath stringByAppendingPathComponent:systemIdentifier];
	
	/*
	NSString* fileName				= 
	
	
	NSString* tmpFileName = [NSString stringWithUUID];
	NSString* tmpSaveDirectory = @"tmp"];
	NSString* tmpSavePath = [[tmpSaveDirectory stringByAppendingPathComponent:tmpFileName] stringByAppendingPathExtension:@"oesavestate"];
	BOOL success = [[rootProxy gameCore] saveStateToFileAtPath:tmpSavePath];
	if(!success){
		NSLog(@"Error could not save state!");
		// TODO: writer proper error, decide what to do with it!
		return;
	}	
	
	OEDBSaveState* saveState = [OEDBSaveState newSaveStateInContext:[self.rom managedObjectContext]];
	if(stateName)
		[saveState setValue:stateName forKey:@"userDescription"];
	 */
}

- (BOOL)saveStateToToFile:(NSString*)fileName error:(NSError**)error{
	return YES;
}
- (BOOL)loadStateFromFile:(NSString*)fileName error:(NSError**)error{
	return YES;
}

#pragma mark -
#pragma mark Recording
- (void)captureScreenshotUsingBlock:(void(^)(NSImage* img))block{}

#pragma mark -
#pragma mark Private Methods
#pragma mark -

- (void)_setURL:(NSURL*)aURL{
	url = [aURL retain];
}
- (void)_setROM:(id)aRom{
	rom = [aRom retain];
}

- (void)_setup{
	NSNotificationCenter* nc = [NSNotificationCenter defaultCenter];
	[nc addObserver:self selector:@selector(applicationWillTerminate:)
			   name:NSApplicationWillTerminateNotification
			 object:NSApp];
	
	gameView = [[OEGameView alloc] initWithFrame:NSZeroRect];
				
	controlsWindow = [[OEHUDControlsWindow alloc] initWithGameDocument:self];
	
	[nc addObserver:self selector:@selector(viewDidMoveToWindow:) name:@"OEGameViewDidMoveToWindow" object:nil];
	[nc addObserver:self selector:@selector(viewDidChangeFrame:) name:NSViewFrameDidChangeNotification object:gameView];
}

- (BOOL)loadFromURL:(NSURL*)aurl error:(NSError**)outError{
    NSString *romPath = [aurl path];
    
    if([[NSFileManager defaultManager] fileExistsAtPath:romPath]){
        OECorePlugin *plugin = [self OE_pluginForFileExtension:[aurl pathExtension] error:outError];
		
        if(plugin == nil) return NO;
        
        gameController = [[plugin controller] retain];
        emulatorName = [[plugin displayName] retain];
        
        Class managerClass = ([[[NSUserDefaultsController sharedUserDefaultsController] valueForKeyPath:@"values.gameCoreInBackgroundThread"] boolValue]
                              ? [OEGameCoreThreadManager  class]
                              : [OEGameCoreProcessManager class]);
        
        gameCoreManager = [[managerClass alloc] initWithROMAtPath:romPath corePlugin:plugin owner:gameController error:outError];
        
        if(gameCoreManager != nil)
        {	
            rootProxy = [[gameCoreManager rootProxy] retain];
            
            [rootProxy setupEmulation];
			
			// set initial volume
			[self setVolume:[[NSUserDefaults standardUserDefaults] floatForKey:UDVolumeKey]];
            
            OEGameCore *gameCore = [rootProxy gameCore];
            NSLog(@"-----------------------------------");
			NSLog(@"%@", gameCore);
            gameSystemController = [[[OESystemPlugin gameSystemPluginForIdentifier:[gameCore systemIdentifier]] controller] retain];
			NSLog(@"[gameCore systemIdentifier]: %@", [gameCore systemIdentifier]);
			NSLog(@"gameSystemController: %@",gameSystemController);
            gameSystemResponder  = [gameSystemController newGameSystemResponder];
			NSLog(@"%@", gameSystemController);
            [gameSystemResponder setClient:gameCore];

			
			if(gameView){
				NSLog(@"setting root proxy + game system responder");
				NSLog(@"%@", rootProxy);
				NSLog(@"%@", gameSystemResponder);
				[gameView setRootProxy:rootProxy];
				[gameView setGameResponder:gameSystemResponder];
			}
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

- (void)_repositionControlsWindow{
	NSPoint origin;
	
	NSWindow *gameWindow = [[self gameView] window];
	if(!gameWindow) return;
	
	origin = [gameWindow convertBaseToScreen:[gameView frame].origin];
	origin.x += (gameView.frame.size.width-controlsWindow.frame.size.width)/2;
	origin.y += 19;
	
	[controlsWindow setFrameOrigin:origin];
}
#pragma mark -
#pragma mark Notifications
- (void)viewDidChangeFrame:(NSNotification*)notification{
	[self _repositionControlsWindow];
}
- (void)viewDidMoveToWindow:(NSNotification*)notification{
	if([controlsWindow parentWindow]){
		[[controlsWindow parentWindow] removeChildWindow:controlsWindow];
	}
	
	NSWindow* window = [gameView window];	
	if(window==nil){
		return;
	}
	
	[self _repositionControlsWindow];
	[window addChildWindow:controlsWindow ordered:NSWindowAbove];
	
	[controlsWindow orderFront:self];
}

- (void)applicationWillTerminate:(NSNotification*)notification{
	[self terminateEmulation];
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
