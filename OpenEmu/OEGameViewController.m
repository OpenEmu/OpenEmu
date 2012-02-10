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

#import "OEGameViewController.h"
#import "NSViewController+OEAdditions.h"

#import "OEDBRom.h"
#import "OEDBGame.h"

#import "OEGameView.h"
#import "OEGameCoreHelper.h"
#import "OEGameCoreManager.h"
#import "OECorePickerController.h"

#import "OESystemPlugin.h"
#import "OESystemResponder.h"
#import "OESystemController.h"
#import "OECorePlugin.h"

#import "OEDBSaveState.h"
#import "OEHUDControlsBar.h"

#import "OEGameCore.h"
#import "OEGameDocument.h"

#import "OEHUDAlert.h"

@interface OEGameViewController ()

+ (OEDBRom *)OE_choseRomFromGame:(OEDBGame *)game;

- (NSString *)OE_saveStatePath;
- (void)OE_repositionControlsWindow;
- (NSString *)OE_convertToValidFileName:(NSString *)fileName;
- (BOOL)OE_loadFromURL:(NSURL *)aURL error:(NSError **)outError;
- (OECorePlugin *)OE_pluginForFileExtension:(NSString *)ext error:(NSError **)outError;

@end

@implementation OEGameViewController

- (id)initWithWindowController:(OEMainWindowController*)aWindowController andRom:(OEDBRom*)rom
{
    return [self initWithWindowController:aWindowController andRom:rom error:nil];    return self;
}

- (id)initWithWindowController:(OEMainWindowController*)aWindowController andRom:(OEDBRom*)rom error:(NSError**)outError
{
    self = [super initWithWindowController:aWindowController];
    if(self)
    {
        [self setRom:rom];
        
        [[self rom] markAsPlayedNow];
        NSString *path = [[self rom] valueForKey:@"path"];
        
        if(!path){
            if(outError != NULL)
                // TODO: Implement proper error
                *outError = [NSError errorWithDomain:@"OESomeErrorDomain" code:0 userInfo:[NSDictionary dictionary]];
            [self release];
            return nil;
        }
        
        NSView *view = [[NSView alloc] initWithFrame:(NSRect){{ 0.0, 0.0 }, { 1.0, 1.0 }}];
        
        gameView = [[OEGameView alloc] initWithFrame:(NSRect){{ 0.0, 0.0 }, { 1.0, 1.0 }}];
        [gameView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
        [view addSubview:gameView];
        
        [self setView:view];
        [view release];
        
        controlsWindow = [[OEHUDControlsBarWindow alloc] initWithGameViewController:self];
        [controlsWindow setReleasedWhenClosed:YES];
        
        NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
        [nc addObserver:self selector:@selector(viewDidChangeFrame:)  name:NSViewFrameDidChangeNotification object:gameView];
        
        NSURL *url = [NSURL fileURLWithPath:path];
        NSError *error = nil;
        if(![self OE_loadFromURL:url error:&error])
        {
            if(outError != NULL)
                *outError = error;
            else 
                [NSApp presentError:error];
            
            [self release];
            return nil;
        }
        
    }
    NSLog(@"OEGameViewController init");
    return self;
}

- (id)initWithWindowController:(OEMainWindowController*)aWindowController andGame:(OEDBGame*)game
{
    return [self initWithWindowController:aWindowController andGame:game error:nil];
}

- (id)initWithWindowController:(OEMainWindowController*)aWindowController andGame:(OEDBGame*)game error:(NSError**)outError
{
    OEDBRom *rom = [OEGameViewController OE_choseRomFromGame:game];
    [self setRom:rom];
    return [self initWithWindowController:aWindowController andRom:rom error:outError];
}

- (void)dealloc
{
    NSLog(@"OEGameViewController dealloc");
    [self setRom:nil];
    
    NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
    [nc removeObserver:self name:NSApplicationWillTerminateNotification object:NSApp];
    [nc removeObserver:self name:NSViewFrameDidChangeNotification object:gameView];
    
    [controlsWindow close];
    [controlsWindow release], controlsWindow = nil;
    [gameView release], gameView = nil;
    
    [super dealloc];
}

- (NSString *)OE_saveStatePath;
{
    NSArray  *paths               = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
    NSString *basePath            = [paths count] > 0 ? [paths objectAtIndex:0] : NSTemporaryDirectory();
    NSString *saveStateFolderName = [[NSUserDefaults standardUserDefaults] valueForKey:UDSaveStateFolderNameKey];
    
    return [[basePath stringByAppendingPathComponent:@"OpenEmu"] stringByAppendingPathComponent:saveStateFolderName];
}

#pragma mark -

- (void)viewDidAppear
{
    [super viewDidAppear];
    
    if([controlsWindow parentWindow] != nil) [[controlsWindow parentWindow] removeChildWindow:controlsWindow];
    
    NSWindow *window = [gameView window];
    if(window == nil) return;
    
    [window addChildWindow:controlsWindow ordered:NSWindowAbove];
    
    [self OE_repositionControlsWindow];
    
    [controlsWindow orderFront:self];
}

- (void)viewWillDisappear
{
    [super viewWillDisappear];
    
    [self terminateEmulation];
}

#pragma mark -
#pragma mark Controlling Emulation

- (void)resetGame
{
    [[rootProxy gameCore] resetEmulation];
    // TODO: draw one frame to reflect reset
}

- (void)terminateEmulation
{
    if(!emulationRunning) return;
    NSLog(@"terminateEmulation");
    
    emulationRunning = NO;
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
    
    // if windowcontroller is set
    if([self windowController])
        // tell it to show its default controller
        [[self windowController] setCurrentContentController:nil];
    else
        [[[self view] window] close];
    
    [[self document] close];
}

- (void)pauseGame
{
    [self setPauseEmulation:YES];
}
- (void)playGame
{
    [self setPauseEmulation:NO];
}
- (BOOL)isEmulationPaused
{
    return [rootProxy isEmulationPaused];
}

- (void)setPauseEmulation:(BOOL)flag
{
    [rootProxy setPauseEmulation:flag];
    // TODO: Update HUD Button state
    
}
- (void)setVolume:(float)volume
{
    [rootProxy setVolume:volume];
    [[NSUserDefaults standardUserDefaults] setValue:[NSNumber numberWithFloat:volume] forKey:UDVolumeKey];
}
- (void)toggleFullscreen
{
    if([self windowController])
        [[[self windowController] window] toggleFullScreen:self];
    else 
        [[[self view] window] toggleFullScreen:self];
}
#pragma mark -
- (void)loadSaveState:(OEDBSaveState *)state
{
    NSString *path = [state valueForKey:@"path"];
    [self loadStateFromFile:path error:nil];
}

- (void)deleteSaveState:(id)state
{
    // TODO: use OEAlert once it's been written
    // TODO: localize and rephrase text
    
    NSString *stateName = [state valueForKey:@"userDescription"];
    OEHUDAlert *alert = [OEHUDAlert deleteGameAlertWithStateName:stateName];
    
    NSUInteger result = [alert runModal];
    
    if(result)
    {        
        // TODO: does this also remove the screenshot from the database?
        NSString *path = [state valueForKey:@"path"];
        
        NSError *err = nil;
        if(![[NSFileManager defaultManager] removeItemAtPath:path error:&err])
        {
            NSLog(@"Error deleting save file!");
            NSLog(@"%@", err);
            return;
        }
        
        NSManagedObjectContext *moc = [state managedObjectContext];
        [moc deleteObject:state];
        [moc save:nil];
    }
}
- (void)saveStateAskingUserForName:(NSString*)proposedName
{
    [self pauseGame];
    
    if(!proposedName)
    {
        // TODO: properly format date
        NSInteger saveGameNo = [[self rom] saveStateCount]+1;
        proposedName = [NSString stringWithFormat:@"%@%ld %@", NSLocalizedString(@"Save-Game-", @""), saveGameNo, [NSDate date]];
    }
    
    OEHUDAlert *alert = [OEHUDAlert saveGameAlertWithProposedName:proposedName];
    [alert setWindow:[[self view] window]];
    [alert setCallbackHandler:
     ^(OEHUDAlert *alert, NSUInteger result)
     {
         [self saveStateWithName:[alert stringValue]];
         
         [self playGame];
     }];
    
    [alert runModal];
}

- (void)saveStateWithName:(NSString *)stateName
{
    if([self rom] == nil)
    {
        NSLog(@"Error: Can not save states without rom");
        return;
    }
    
    [self pauseGame];
    
    NSString *systemIdentifier= [gameSystemController systemIdentifier];
    NSURL *systemSaveDirectoryURL= [NSURL fileURLWithPath:[[self OE_saveStatePath] stringByAppendingPathComponent:systemIdentifier]];
    
    NSError *err = nil;
    BOOL success = [[NSFileManager defaultManager] createDirectoryAtURL:systemSaveDirectoryURL withIntermediateDirectories:YES attributes:nil error:&err];
    
    if(!success)
    {
        // TODO: inform user
        NSLog(@"could not create save state directory");
        NSLog(@"%@", err);
        return;
    }
    
    NSString *fileName = stateName;
    if(!fileName) fileName = [NSString stringWithFormat:@"%@", [NSDate date]];
    fileName = [self OE_convertToValidFileName:fileName];
    
    NSURL *saveStateURL = [[systemSaveDirectoryURL URLByAppendingPathComponent:fileName] URLByAppendingPathExtension:@"oesavestate"];
    int count = 0;
    
    while([[NSFileManager defaultManager] fileExistsAtPath:[saveStateURL path] isDirectory:NULL])
    {
        count++;
        
        NSString *countedFileName = [NSString stringWithFormat:@"%@ %d.oesavestate", fileName, count];
        saveStateURL = [systemSaveDirectoryURL URLByAppendingPathComponent:countedFileName];
    }
    
    success = [[rootProxy gameCore] saveStateToFileAtPath:[saveStateURL path]];
    if(!success)
    {
        // TODO: inform user
        NSLog(@"could not write file");
        NSLog(@"%@", err);
        return;
    }
    
    // we need to make sure that we are on the same thread where self.rom was created!!
    OEDBSaveState *saveState = [OEDBSaveState newSaveStateInContext:[[self rom] managedObjectContext]];
    
    [saveState setValue:[saveStateURL path] forKey:@"path"];
    [saveState setValue:[NSDate date] forKey:@"timestamp"];
    [saveState setValue:[[rootProxy gameCore] pluginName] forKey:@"emulatorID"];
    [saveState setValue:[self rom] forKey:@"rom"];
    
    if(stateName != nil) [saveState setValue:stateName forKey:@"userDescription"];
    
    [self captureScreenshotUsingBlock:
     ^(NSImage *img)
     {
         [saveState setValue:[img TIFFRepresentation] forKey:@"screenshot"];
     }];
}

- (BOOL)saveStateToToFile:(NSString*)fileName error:(NSError**)error
{
    // TODO: implement if we want this
    return YES;
}
- (BOOL)loadStateFromFile:(NSString*)fileName error:(NSError**)error
{
    if(error!=NULL) *error = nil;
    return [[rootProxy gameCore] loadStateFromFileAtPath:fileName];
}

#pragma mark -

- (void)captureScreenshotUsingBlock:(void(^)(NSImage *img))block
{
    [gameView captureScreenshotUsingBlock:block];
}

#pragma mark -
#pragma mark Menu Items

- (IBAction)volumeUp:(id)sender;
{
    
}

- (IBAction)volumeDown:(id)sender;
{
    
}

- (IBAction)saveState:(id)sender;
{
    
}

- (IBAction)loadState:(id)sender;
{
    
}

- (IBAction)pauseEmulation:(id)sender;
{
    
}

- (IBAction)resumeEmulation:(id)sender;
{
    
}

- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
    return YES;
    /*
    NSInteger tag = [menuItem tag];
    
    return (tag >= MainMenu_Controls_VolumeUp && tag <= MainMenu_Controls_ResumeEmulation);
     */
}
- (void)menuItemAction:(id)sender
{
}

- (void)setupMenuItems
{}

#pragma mark -
#pragma mark Info
- (NSSize)defaultScreenSize
{
    OEGameCore *gameCore = [rootProxy gameCore];
    OEIntRect screenRect = [gameCore screenRect];
    
    return NSSizeFromOEIntSize(screenRect.size);
}

#pragma mark -
#pragma mark Private Methods

+ (OEDBRom *)OE_choseRomFromGame:(OEDBGame *)game
{
    // TODO: we could display a list of roms here if we wanted to, do we?
    return [game defaultROM];
}

- (BOOL)OE_loadFromURL:(NSURL *)aurl error:(NSError **)outError
{
    NSString *romPath = [aurl path];
    
    if([[NSFileManager defaultManager] fileExistsAtPath:romPath])
    {
        emulationRunning = YES;
        OECorePlugin *plugin = [self OE_pluginForFileExtension:[aurl pathExtension] error:outError];
        
        if(plugin == nil) return NO;
        
        gameController = [[plugin controller] retain];
        
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
            gameSystemController = [[[OESystemPlugin gameSystemPluginForTypeExtension:[aurl pathExtension]] controller] retain];
            gameSystemResponder  = [gameSystemController newGameSystemResponder];
            [gameSystemResponder setClient:gameCore];
            
            if(gameView != nil)
            {
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

- (void)OE_repositionControlsWindow
{
    NSWindow *gameWindow = [[self view] window];
    if(gameWindow == nil) return;
    
    NSPoint origin = [gameWindow convertBaseToScreen:[gameView frame].origin];
    origin.x += ([gameView frame].size.width - [controlsWindow frame].size.width) / 2;
    origin.y += 19;
    
    [controlsWindow setFrameOrigin:origin];
}


- (NSString *)OE_convertToValidFileName:(NSString *)fileName
{
    NSCharacterSet *illegalFileNameCharacters = [NSCharacterSet characterSetWithCharactersInString:@"/\\?%*|\":<>"];
    return [[fileName componentsSeparatedByCharactersInSet:illegalFileNameCharacters] componentsJoinedByString:@""];
}
#pragma mark -
#pragma mark Notifications

- (void)viewDidChangeFrame:(NSNotification*)notification
{
    [self OE_repositionControlsWindow];
}

#pragma mark -
#pragma mark Plugin discovery
- (OECorePlugin *)OE_pluginForFileExtension:(NSString *)ext error:(NSError **)outError
{
    OECorePlugin *chosenCore = nil;
    OESystemPlugin *system = [OESystemPlugin gameSystemPluginForTypeExtension:ext];
    NSArray *validPlugins = [OECorePlugin corePluginsForSystemIdentifier:[system systemIdentifier]];
    
    if([validPlugins count] <= 1) chosenCore = [validPlugins lastObject];
    else
    {
        OECorePickerController *c = [[[OECorePickerController alloc] initWithCoreList:validPlugins] autorelease];
        
        if([[NSApplication sharedApplication] runModalForWindow:[c window]] == 1)
            chosenCore = [c selectedCore];
    }
    
    if(chosenCore == nil && outError != NULL)
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
    return chosenCore;
}
#pragma mark -
#pragma mark TaskWrapper delegate methods
- (void)appendOutput:(NSString *)output fromProcess:(OETaskWrapper *)aTask
{
    // printf("%s", [output UTF8String]);
}    

- (void)processStarted:(OETaskWrapper *)aTask
{
}

- (void)processFinished:(OETaskWrapper *)aTask withStatus:(NSInteger)statusCode
{
}
@synthesize rom, document;
@end
