//
//  OEGameViewController.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 25.10.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OEGameViewController.h"

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
#import "OEHUDControls.h"

#import "OEGameCore.h"
#import "OEGameDocument.h"
@interface OEGameViewController (Private)
- (BOOL)_setupGameDocument;
+ (OEDBRom*)_choseRomFromGame:(OEDBGame*)game;
- (NSString *)_convertToValidFileName:(NSString *)fileName;
- (BOOL)loadFromURL:(NSURL*)aurl error:(NSError**)outError;
- (OECorePlugin *)OE_pluginForFileExtension:(NSString *)ext error:(NSError **)outError;
@end
#define OESaveStatePath (NSString*)^{\
NSArray *paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);\
NSString *basePath = ([paths count] > 0) ? [paths objectAtIndex:0] : NSTemporaryDirectory();\
NSString* saveStateFolderName = [[NSUserDefaults standardUserDefaults] valueForKey:UDSaveStateFolderNameKey];\
return [[basePath stringByAppendingPathComponent:@"OpenEmu"] stringByAppendingPathComponent:saveStateFolderName];\
}()
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
        
        [[self rom] setValue:[NSDate date] forKey:@"lastPlayed"];
        NSString* path = [[self rom] valueForKey:@"path"];
        
        if(!path){
            [self release];
            if(outError != NULL)
                // TODO: Implement proper error
                *outError = [NSError errorWithDomain:@"OESomeErrorDomain" code:0 userInfo:[NSDictionary dictionary]];
            self = nil;
            return nil;
        }
        
        gameView = [[OEGameView alloc] initWithFrame:(NSRect){{0,0},{1,1}}];
        controlsWindow = [[OEHUDControlsWindow alloc] initWithGameViewController:self];
        
        NSNotificationCenter* nc = [NSNotificationCenter defaultCenter];
        [nc addObserver:self selector:@selector(viewDidMoveToWindow:) name:@"OEGameViewDidMoveToWindow" object:nil];
        [nc addObserver:self selector:@selector(viewDidChangeFrame:) name:NSViewFrameDidChangeNotification object:gameView];
        
        NSURL* url = [NSURL fileURLWithPath:path];
        NSError* error = nil;
        if(![self loadFromURL:url error:&error])
        {
            [self release];
            self = nil;
            
            if(outError != NULL)
                *outError = error;
            else 
                [NSApp presentError:error];
            return nil;
        }
        
    }
    return self;
}

- (id)initWithWindowController:(OEMainWindowController*)aWindowController andGame:(OEDBGame*)game
{
    return [self initWithWindowController:aWindowController andGame:game error:nil];
}

- (id)initWithWindowController:(OEMainWindowController*)aWindowController andGame:(OEDBGame*)game error:(NSError**)outError
{
    OEDBRom* rom = [OEGameViewController _choseRomFromGame:game];
    [self setRom:rom];
    return [self initWithWindowController:aWindowController andRom:rom error:outError];
}

- (BOOL)_setupGameDocument
{
    return YES;
}

- (void)dealloc {
    NSLog(@"OEGameViewController dealloc start");
    [self setRom:nil];
    
    NSNotificationCenter* nc = [NSNotificationCenter defaultCenter];
    [nc removeObserver:self name:NSApplicationWillTerminateNotification object:NSApp];
    [nc removeObserver:self name:NSViewFrameDidChangeNotification object:gameView];
    [nc removeObserver:self name:@"OEGameViewDidMoveToWindow" object:nil];
    
    [controlsWindow close];
    [controlsWindow release], controlsWindow = nil;
    [gameView release], gameView = nil;
    
    NSLog(@"OEGameViewController dealloc end");
    
    [super dealloc];
}
#pragma mark -
- (void)contentWillShow
{
    
}
- (void)contentWillHide
{
    [self terminateEmulation];
}

#pragma mark -
- (NSView*)view
{
    return (NSView*)gameView;
}
#pragma mark -
#pragma mark Controlling Emulation
#pragma mark -
- (void)resetGame
{
    [[rootProxy gameCore] resetEmulation];
    // TODO: draw one frame to reflect reset
}
- (void)terminateEmulation
{
    if(!emulationRunning) return;
    NSLog(@"terminateEmulation start");
    
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
    
    
    NSDocumentController* sharedDocumentController = [NSDocumentController sharedDocumentController];
    [sharedDocumentController removeDocument:[self document]];
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
- (void)loadState:(id)state
{
    NSString* path = [state valueForKey:@"path"];
    [self loadStateFromFile:path error:nil];
}

- (void)deleteState:(id)state
{
    // TODO: use OEAlert once it's been written
    // TODO: localize and rephrase text
    NSAlert* confirm = [NSAlert alertWithMessageText:@"Do you really want to delete the SaveState?" defaultButton:@"Yes" alternateButton:@"Cancel" otherButton:nil informativeTextWithFormat:@"This will delete the save state permanently and can not be undone."];
    
    confirm.showsSuppressionButton = YES;
    [[confirm suppressionButton] setState:[[NSUserDefaults standardUserDefaults] boolForKey:UDRemoveSaveStateWithoutConfirmation]];
    
    if([[confirm suppressionButton] state] || [confirm runModal]==NSOKButton)
    {
        [[NSUserDefaults standardUserDefaults] setBool:[[confirm suppressionButton] state] forKey:UDRemoveSaveStateWithoutConfirmation];
        
        // TODO: does this also remove the screenshot from the database?
        NSString* path = [state valueForKey:@"path"];
        
        NSError *err = nil;
        if(![[NSFileManager defaultManager] removeItemAtPath:path error:&err])
        {
            NSLog(@"Error deleting save file!");
            NSLog(@"%@", err);
            return;
        }
        
        NSManagedObjectContext* moc = [state managedObjectContext];
        [moc deleteObject:state];
        [moc save:nil];
    }
}
- (void)saveStateAskingUserForName:(NSString*)proposedName
{
    [self pauseGame];
    
    BOOL dateAsName = NO;
    if(!proposedName)
    {
        // TODO: properly format date
        proposedName = [NSString stringWithFormat:@"%@", [NSDate date]];
        dateAsName = YES;
    }
    
    NSString* name = nil;
    if(![[NSUserDefaults standardUserDefaults] boolForKey:UDNameStateByDateKey])
    {        
        NSAlert* alert = [NSAlert alertWithMessageText:@"Save Satate" defaultButton:@"Save" alternateButton:@"Cancel" otherButton:nil informativeTextWithFormat:@"Enter a description for this save state"];
        
        NSTextField* field = [[NSTextField alloc] initWithFrame:NSMakeRect(0, 0, 240, 22)];
        if(dateAsName)
            [[field cell] setPlaceholderString:proposedName];
        
        [field setAutoresizingMask:NSViewWidthSizable];
        [alert setAccessoryView:field];
        [field setStringValue:proposedName];
        [field release];
        
        [alert setShowsSuppressionButton:YES];
        [alert beginSheetModalForWindow:[[self view] window] modalDelegate:self didEndSelector:@selector(_stateNameSheetDidEnd:returnCode:) contextInfo:NULL];
    } 
    else 
    {
        [self saveStateWithName:name];
    }
}
- (void)saveStateWithName:(NSString*)stateName
{
    if(!self.rom)
    {
        NSLog(@"Error: Can not save states without rom");
        [self playGame];
        return;
    }
    [self pauseGame];
    
    NSString* systemIdentifier= [[rootProxy gameCore] systemIdentifier];
    NSURL* systemSaveDirectoryURL= [NSURL fileURLWithPath:[OESaveStatePath stringByAppendingPathComponent:systemIdentifier]];
    
    NSError* err = nil;
    BOOL success = [[NSFileManager defaultManager] createDirectoryAtURL:systemSaveDirectoryURL withIntermediateDirectories:YES attributes:nil error:&err];
    if(!success)
    {
        // TODO: inform user
        NSLog(@"could not create save state directory");
        NSLog(@"%@", err);
        [self playGame];
        return;
    }
    
    NSString* fileName = stateName;
    if(!fileName) fileName = [NSString stringWithFormat:@"%@", [NSDate date]];
    fileName = [self _convertToValidFileName:fileName];
    
    NSURL* saveStateURL = [[systemSaveDirectoryURL URLByAppendingPathComponent:fileName] URLByAppendingPathExtension:@"oesavestate"];
    int count = 0;
    while([[NSFileManager defaultManager] fileExistsAtPath:[saveStateURL path] isDirectory:NULL])
    {
        count ++;
        
        NSString* countedFileName = [NSString stringWithFormat:@"%@ %d.oesavestate", fileName, count];
        saveStateURL = [systemSaveDirectoryURL URLByAppendingPathComponent:countedFileName];
    }
    
    success = [[rootProxy gameCore] saveStateToFileAtPath:[saveStateURL path]];
    if(!success)
    {
        // TODO: inform user
        NSLog(@"could not write file");
        NSLog(@"%@", err);
        [self playGame];
        return;
    }
    
    // we need to make sure that we are on the same thread where self.rom was created!!
    OEDBSaveState* saveState = [OEDBSaveState newSaveStateInContext:[self.rom managedObjectContext]];
    [saveState setValue:[saveStateURL path] forKey:@"path"];
    [saveState setValue:[NSDate date] forKey:@"timestamp"];
    [saveState setValue:[[rootProxy gameCore] pluginName] forKey:@"emulatorID"];
    [saveState setValue:self.rom forKey:@"rom"];
    if(stateName)
        [saveState setValue:stateName forKey:@"userDescription"];
    
    [self captureScreenshotUsingBlock:^(NSImage *img) 
     {
         NSData* imgData = [img TIFFRepresentation];
         [saveState setValue:imgData forKey:@"screenshot"];
     }];
    
    [self playGame];
}

- (BOOL)saveStateToToFile:(NSString*)fileName error:(NSError**)error
{
    // TODO: implement if we want this
    return YES;
}
- (BOOL)loadStateFromFile:(NSString*)fileName error:(NSError**)error
{
    if(error!=NULL) *error = nil;
    return [[rootProxy gameCore] loadStateFromFileAtPath:fileName];;
}
#pragma mark -
- (void)captureScreenshotUsingBlock:(void(^)(NSImage* img))block
{
    [(OEGameView*)[self view] captureScreenshotUsingBlock:block];
}

#pragma mark -
#pragma mark Menu Items
- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
    NSInteger tag = [menuItem tag];
    
    return (tag >= MainMenu_Controls_VolumeUp && tag <= MainMenu_Controls_ResumeEmulation);
}
- (void)menuItemAction:(id)sender
{
    NSInteger tag = [sender tag];
    switch (tag) 
    {
        case MainMenu_Controls_VolumeUp: break;
        case MainMenu_Controls_VolumeDown: break;
        case MainMenu_Controls_SaveState: break;
        case MainMenu_Controls_LoadState: break;
        case MainMenu_Controls_PauseEmulation: break;
        case MainMenu_Controls_ResumeEmulation: break;
        default:
            break;
    }
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
+ (OEDBRom*)_choseRomFromGame:(OEDBGame*)game
{
    NSSet* roms = [game valueForKey:@"roms"];
    NSLog(@"%@", [roms className]);
    return [roms anyObject];
}
- (BOOL)loadFromURL:(NSURL*)aurl error:(NSError**)outError
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
            gameSystemController = [[[OESystemPlugin gameSystemPluginForIdentifier:[gameCore systemIdentifier]] controller] retain];
            gameSystemResponder  = [gameSystemController newGameSystemResponder];
            [gameSystemResponder setClient:gameCore];
            
            
            if(gameView)
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

- (void)_repositionControlsWindow
{
    NSPoint origin;
    
    NSWindow *gameWindow = [[self view] window];
    if(!gameWindow) return;
    
    origin = [gameWindow convertBaseToScreen:[gameView frame].origin];
    origin.x += ([gameView frame].size.width-[controlsWindow frame].size.width)/2;
    origin.y += 19;
    
    [controlsWindow setFrameOrigin:origin];
}


- (NSString *)_convertToValidFileName:(NSString *)fileName
{
    NSCharacterSet* illegalFileNameCharacters = [NSCharacterSet characterSetWithCharactersInString:@"/\\?%*|\":<>"];
    return [[fileName componentsSeparatedByCharactersInSet:illegalFileNameCharacters] componentsJoinedByString:@""];
}
- (void)_stateNameSheetDidEnd:(NSAlert*)alert returnCode:(NSInteger) aReturnCode
{
    if(aReturnCode == NSCancelButton)
    {
        
        [self playGame];
        return;
    }
    
    [[NSUserDefaults standardUserDefaults] setBool:[[alert suppressionButton] state] forKey:UDNameStateByDateKey];
    
    NSTextField* inputField = (NSTextField*)[alert accessoryView];
    NSString* stateName = [inputField stringValue];
    
    // if either statename is nil or the same date as placeholder string (not changed)
    if([stateName isEqualToString:@""] || ([[inputField cell] placeholderString] && [[[inputField cell] placeholderString] isEqualToString:stateName]))
    {
        // we want to use the date as name (default behavior)
        [self saveStateWithName:nil];
    }
    else 
    {
        [self saveStateWithName:stateName];
    }
}
#pragma mark -
#pragma mark Notifications
- (void)viewDidChangeFrame:(NSNotification*)notification
{
    [self _repositionControlsWindow];
}
- (void)viewDidMoveToWindow:(NSNotification*)notification
{
    if([controlsWindow parentWindow])
    {
        [[controlsWindow parentWindow] removeChildWindow:controlsWindow];
    }
    
    NSWindow* window = [gameView window];
    if(window==nil)
    {
        return;
    }
    
    [self _repositionControlsWindow];
    [window addChildWindow:(NSWindow*)controlsWindow ordered:NSWindowAbove];
    
    [controlsWindow orderFront:self];
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
