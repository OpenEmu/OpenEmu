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
#import "OEDBSystem.h"
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
#import "OEGameControlsBar.h"

#import "OECoreUpdater.h"

#import "OEGameCore.h"
#import "OEGameDocument.h"

#import "OEHUDAlert+DefaultAlertsAdditions.h"

#import "NSString+UUID.h"
#import "NSURL+OELibraryAdditions.h"

#import "OEPreferencesController.h"

NSString *const OEGameVolumeKey = @"volume";
NSString *const OEGameVideoFilterKey = @"videoFilter";
NSString *const OEGameCoresInBackgroundKey = @"gameCoreInBackgroundThread";
NSString *const OEDontShowGameTitleInWindowKey = @"dontShowGameTitleInWindow";
NSString *const OEAutoSwitchCoreAlertSuppressionKey = @"changeCoreWhenLoadingStateWitoutConfirmation";
NSString *const OEBackgroundPauseKey = @"backgroundPause";
NSString *const OEForceCorePicker = @"forceCorePicker";
NSString *const OEGameViewControllerEmulationWillFinishNotification = @"OEGameViewControllerEmulationWillFinishNotification";
NSString *const OEGameViewControllerEmulationDidFinishNotification = @"OEGameViewControllerEmulationDidFinishNotification";
NSString *const OEGameViewControllerROMKey = @"OEROM";

NSString *const OEDefaultWindowTitle = @"OpenEmu";

typedef enum : NSUInteger
{
    OEGameViewControllerEmulationStatusNotStarted  = 0,
    OEGameViewControllerEmulationStatusPlaying     = 1,
    OEGameViewControllerEmulationStatusPaused      = 2,
    OEGameViewControllerEmulationStatusTerminating = 3,
} OEGameViewControllerEmulationStatus;


#define UDDefaultCoreMappingKeyPrefix   @"defaultCore"
#define UDSystemCoreMappingKeyForSystemIdentifier(_SYSTEM_IDENTIFIER_) [NSString stringWithFormat:@"%@.%@", UDDefaultCoreMappingKeyPrefix, _SYSTEM_IDENTIFIER_]
@interface OEGameViewController ()
{
    NSTimer                             *_systemSleepTimer;
    OECorePlugin                        *_corePlugin;
    OEGameViewControllerEmulationStatus  _emulationStatus;
    OEDBSaveState                       *_saveStateForGameStart;
    NSDate                              *_lastPlayStartDate;
    BOOL                                 _pausedByGoingToBackground;
}

+ (OEDBRom *)OE_chooseRomFromGame:(OEDBGame *)game;

- (BOOL)OE_loadFromURL:(NSURL *)aurl core:(OECorePlugin *)core error:(NSError **)outError;
- (OECorePlugin *)OE_coreForSystem:(OESystemPlugin *)system error:(NSError **)outError;
- (BOOL)OE_loadStateFromFile:(NSString *)fileName;

- (void)OE_restartUsingCore:(OECorePlugin *)core;

- (void)OE_repositionControlsWindow;
- (void)OE_terminateEmulationWithoutNotification;

@end

@implementation OEGameViewController
@synthesize delegate;
@synthesize rom = _rom, document;
@synthesize controlsWindow;
@synthesize gameView;

// We cannot use +initialize since it is only sent when the class is first used
// by the runtime. If the user opens the Preferences window before running any games,
// i.e., before OEGameViewController is used and +initialize is sent, defaults aren’t
// registered, NSUserDefaultsController doesn’t know about them and the Preferences
// window doesn’t show them.
+ (void)load
{
    [[NSUserDefaults standardUserDefaults] registerDefaults:(@{
                                                             OEBackgroundPauseKey : @YES,
                                                             })];
}

- (id)initWithRom:(OEDBRom *)rom
{
    return [self initWithRom:rom core:nil error:nil];
}
- (id)initWithRom:(OEDBRom *)rom core:(OECorePlugin *)core
{
    return [self initWithRom:rom core:core error:nil];
}
- (id)initWithRom:(OEDBRom *)rom error:(NSError **)outError
{
    return [self initWithRom:rom core:nil error:outError];
}

- (id)initWithRom:(OEDBRom *)aRom core:(OECorePlugin *)core error:(NSError **)outError
{
    if((self = [super init]))
    {
        controlsWindow = [[OEGameControlsBar alloc] initWithGameViewController:self];
        [controlsWindow setReleasedWhenClosed:YES];
        
        [self setRom:aRom];
        NSURL *url = [[self rom] URL];

        if(![[aRom game] filesAvailable])
        {
            // TODO: Implement user info
            if(outError != NULL)
                *outError = [NSError errorWithDomain:OEGameDocumentErrorDomain code:OEFileDoesNotExistError userInfo:@{ }];
            return nil;
        }

        NSView *view = [[NSView alloc] initWithFrame:(NSRect){ .size = { 1.0, 1.0 }}];
        [self setView:view];

        NSError *error = nil;
        
        if(![self OE_loadFromURL:url core:core error:&error])
        {
            if(error != nil)
            {
                if(outError != NULL)
                    *outError = error;
                else 
                    [NSApp presentError:error];
            }
            
            return nil;
        }
    }
    
    return self;
}

- (id)initWithGame:(OEDBGame *)game
{
    return [self initWithGame:game core:nil error:nil];
}

- (id)initWithGame:(OEDBGame *)game core:(OECorePlugin *)core
{
    return [self initWithGame:game core:core error:nil];
}

- (id)initWithGame:(OEDBGame *)game error:(NSError **)outError
{
    return [self initWithGame:game core:nil error:outError];
}

- (id)initWithGame:(OEDBGame *)game core:(OECorePlugin *)core error:(NSError **)outError
{
    return [self initWithRom:[OEGameViewController OE_chooseRomFromGame:game] core:core error:outError];
}

- (id)initWithSaveState:(OEDBSaveState *)state
{
    return [self initWithSaveState:state error:nil];
}

- (id)initWithSaveState:(OEDBSaveState *)state error:(NSError **)outError
{
    OEDBRom      *rom            = [state rom];
    NSString     *coreIdentifier = [state coreIdentifier];
    OECorePlugin *core           = [OECorePlugin corePluginWithBundleIdentifier:coreIdentifier];
    
    if(![[rom game] filesAvailable])
    {
        // TODO: Implement user info
        if(outError != NULL)
            *outError = [NSError errorWithDomain:OEGameDocumentErrorDomain code:OEFileDoesNotExistError userInfo:@{ }];
        return nil;
    }
    
    if(core == nil)
    {
        NSError *error = [NSError errorWithDomain:OEGameDocumentErrorDomain
                                             code:OENoCoreForSaveStateError
                                         userInfo:
                          [NSDictionary dictionaryWithObjectsAndKeys:
                           NSLocalizedString(@"No suitable core found.", @"Core not installed error reason."),
                           NSLocalizedFailureReasonErrorKey,
                           NSLocalizedString(@"Install a core for this save state.", @"Core not installed error recovery suggestion."),
                           NSLocalizedRecoverySuggestionErrorKey,
                           nil]];

        if(outError != NULL)
            *outError = error;
        else
            [NSApp presentError:error];
        
        return nil;
    }
    
    if((self = [self initWithRom:rom core:core error:outError])) _saveStateForGameStart = state;

    return self;
}

- (void)dealloc
{
    NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
    [nc removeObserver:self name:NSViewFrameDidChangeNotification object:gameView];
    
    [controlsWindow close];
    controlsWindow = nil;
    gameView = nil;
}

#pragma mark -

- (void)viewDidAppear
{
    [super viewDidAppear];
    
    if([controlsWindow parentWindow] != nil) [[controlsWindow parentWindow] removeChildWindow:controlsWindow];
    
    NSWindow *window = [self OE_rootWindow];
    if(window == nil) return;

    NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
    [nc addObserver:self selector:@selector(windowDidBecomeKey:)    name:NSWindowDidBecomeKeyNotification    object:window];
    [nc addObserver:self selector:@selector(windowDidResignKey:)    name:NSWindowDidResignKeyNotification    object:window];
    [nc addObserver:self selector:@selector(windowDidMove:)         name:NSWindowDidMoveNotification         object:window];
    [nc addObserver:self selector:@selector(windowDidChangeScreen:) name:NSWindowDidChangeScreenNotification object:window];

    [window addChildWindow:controlsWindow ordered:NSWindowAbove];
    [self OE_repositionControlsWindow];
    [controlsWindow orderFront:self];

    [window makeFirstResponder:gameView];
    
    if(![[NSUserDefaults standardUserDefaults] boolForKey:OEDontShowGameTitleInWindowKey])
    {
        [window setTitle:[[[self rom] game] name]];
        gameView.gameTitle = [[[self rom] game] name];
    }
}

- (void)viewWillDisappear
{
    [super viewWillDisappear];
    
    NSWindow *window = [self OE_rootWindow];

    NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
    [nc removeObserver:self name:NSWindowDidBecomeKeyNotification    object:window];
    [nc removeObserver:self name:NSWindowDidResignKeyNotification    object:window];
    [nc removeObserver:self name:NSWindowDidMoveNotification         object:window];
    [nc removeObserver:self name:NSWindowDidChangeScreenNotification object:window];

    if(![[NSUserDefaults standardUserDefaults] boolForKey:OEDontShowGameTitleInWindowKey])
        [window setTitle:OEDefaultWindowTitle];
    
    [[self controlsWindow] hide];
    [self terminateEmulation];
}

- (void)windowDidResignKey:(NSNotification *)notification
{
    BOOL backgroundPause = [[NSUserDefaults standardUserDefaults] boolForKey:OEBackgroundPauseKey];

    if(_emulationStatus == OEGameViewControllerEmulationStatusPlaying && backgroundPause)
    {
        [self pauseGame:self];
        _pausedByGoingToBackground = YES;
    }
}

- (void)windowDidBecomeKey:(NSNotification *)notification
{
    if(_pausedByGoingToBackground)
    {
        [self playGame:self];
        _pausedByGoingToBackground = NO;
    }
}

#pragma mark - OS Sleep Handling

- (void)preventSystemSleepTimer:(NSTimer *)aTimer;
{
    UpdateSystemActivity(OverallAct);
}

- (void)enableOSSleep
{
    if(_systemSleepTimer == nil) return;
    
    [_systemSleepTimer invalidate];
    _systemSleepTimer = nil;
}

- (void)disableOSSleep
{
    if(_systemSleepTimer != nil) return;
    
    _systemSleepTimer = [NSTimer scheduledTimerWithTimeInterval:30.0 target:self selector:@selector(preventSystemSleepTimer:) userInfo:nil repeats:YES];
}

#pragma mark - Controlling Emulation

- (void)resetGame
{
    if([[OEHUDAlert resetSystemAlert] runModal] == NSAlertDefaultReturn)
    {
        [[rootProxy gameCore] resetEmulation];
        [self playGame:self];
    }
}

- (void)terminateEmulation
{
    if(_emulationStatus == OEGameViewControllerEmulationStatusNotStarted ||
       _emulationStatus == OEGameViewControllerEmulationStatusTerminating)
        return;

    [self enableOSSleep];
    [self pauseGame:self];
    
    [[self controlsWindow] setCanShow:NO];
    
    if([[OEHUDAlert saveAutoSaveGameAlert] runModal])
        [self saveStateWithName:OESaveStateAutosaveName];

    _emulationStatus = OEGameViewControllerEmulationStatusTerminating;

    NSDictionary *userInfo = @{OEGameViewControllerROMKey : [self rom]};
    // TODO: why isn't this being sent to the delegate directly?
    [NSApp sendAction:@selector(emulationWillFinishForGameViewController:) to:nil from:self];
    [[NSNotificationCenter defaultCenter] postNotificationName:OEGameViewControllerEmulationWillFinishNotification object:self userInfo:userInfo];

    [self OE_terminateEmulationWithoutNotification];
    
    [NSApp sendAction:@selector(emulationDidFinishForGameViewController:) to:nil from:self];
    [[NSNotificationCenter defaultCenter] postNotificationName:OEGameViewControllerEmulationDidFinishNotification object:self userInfo:userInfo];

    [[self document] close];
}

- (void)OE_terminateEmulationWithoutNotification
{
    [[NSNotificationCenter defaultCenter] removeObserver:self name:NSViewFrameDidChangeNotification object:gameView];

    _emulationStatus = OEGameViewControllerEmulationStatusNotStarted;

    [gameView setRootProxy:nil];
    [gameView setGameResponder:nil];

    gameView = nil;
    
    gameSystemController = nil;
    gameSystemResponder  = nil;
    
    // kill our background friend
    [gameCoreManager stop];
    
    gameCoreManager = nil;
    rootProxy = nil;

    [[self rom] addTimeIntervalToPlayTime:ABS([_lastPlayStartDate timeIntervalSinceNow])];
    _lastPlayStartDate = nil;

}

- (void)OE_startEmulation
{
    if(_emulationStatus != OEGameViewControllerEmulationStatusNotStarted) return;


    NSError *error;
    Class managerClass = ([[NSUserDefaults standardUserDefaults] boolForKey:OEGameCoresInBackgroundKey]
                          ? [OEGameCoreThreadManager  class]
                          : [OEGameCoreProcessManager class]);
    gameCoreManager = [[managerClass alloc] initWithROMAtPath:[[[self rom] URL] path] corePlugin:_corePlugin error:&error];

    if(!gameCoreManager)
    {
        [[self document] presentError:error];

        if([[self delegate] respondsToSelector:@selector(emulationDidFinishForGameViewController:)])
            [[self delegate] emulationDidFinishForGameViewController:self];

        [[self document] close];
    }

    rootProxy = [gameCoreManager rootProxy];
    
    OEGameCore *gameCore = [rootProxy gameCore];
    gameSystemController = [[[[[self rom] game] system] plugin] controller];
    gameSystemResponder  = [gameSystemController newGameSystemResponder];
    [gameSystemResponder setClient:gameCore];

    gameView = [[OEGameView alloc] initWithFrame:[[self view] bounds]];
    [gameView setRootProxy:rootProxy];
    [gameView setGameResponder:gameSystemResponder];
    [gameView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    [[self view] addSubview:gameView];
    
    [rootProxy setupEmulation];
    _emulationStatus = OEGameViewControllerEmulationStatusPlaying;

    NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
    [nc addObserver:self selector:@selector(viewDidChangeFrame:) name:NSViewFrameDidChangeNotification object:gameView];

    NSWindow *window = [self OE_rootWindow];
    [window makeFirstResponder:gameView];

    [self disableOSSleep];
    [[self rom] incrementPlayCount];
    [[self rom] markAsPlayedNow];
    _lastPlayStartDate = [NSDate date];

    [[self controlsWindow] reflectEmulationRunning:YES];

    if(_saveStateForGameStart)
    {
        [self loadState:_saveStateForGameStart];
        _saveStateForGameStart = nil;
    }
    
    // set initial volume
    [self setVolume:[[NSUserDefaults standardUserDefaults] floatForKey:OEGameVolumeKey] asDefault:NO];
}

- (IBAction)pauseGame:(id)sender
{
    if(_emulationStatus == OEGameViewControllerEmulationStatusPlaying) [self setPauseEmulation:YES];
}

- (IBAction)playGame:(id)sender
{
    if(_emulationStatus == OEGameViewControllerEmulationStatusNotStarted) [self OE_startEmulation];
    else if(_emulationStatus == OEGameViewControllerEmulationStatusPaused) [self setPauseEmulation:NO];
}

- (void)toggleEmulationPaused
{
    [self setPauseEmulation:(_emulationStatus == OEGameViewControllerEmulationStatusPlaying)];
}

- (void)setPauseEmulation:(BOOL)pauseEmulation
{
    if(pauseEmulation)
    {
        [self enableOSSleep];
        _emulationStatus = OEGameViewControllerEmulationStatusPaused;
        [[self rom] addTimeIntervalToPlayTime:ABS([_lastPlayStartDate timeIntervalSinceNow])];
        _lastPlayStartDate = nil;
    }
    else
    {
        [self disableOSSleep];
        [[self rom] markAsPlayedNow];
        _lastPlayStartDate = [NSDate date];
        _emulationStatus = OEGameViewControllerEmulationStatusPlaying;
    }

    [rootProxy setPauseEmulation:pauseEmulation];
    [[self controlsWindow] reflectEmulationRunning:!pauseEmulation];
}

- (BOOL)isEmulationRunning
{
    return _emulationStatus == OEGameViewControllerEmulationStatusPlaying;
}

#pragma mark - HUD Bar Actions

// switchCore:: expects sender or [sender representedObject] to be an OECorePlugin object and prompts the user for confirmation
- (void)switchCore:(id)sender
{
    OECorePlugin *plugin;
    if([sender isKindOfClass:[OECorePlugin class]])
        plugin = sender;
    else if([sender respondsToSelector:@selector(representedObject)] && [[sender representedObject] isKindOfClass:[OECorePlugin class]])
        plugin = [sender representedObject];
    else
    {
        DLog(@"Invalid argument passed: %@", sender);
        return;
    }
    
    if([[plugin bundleIdentifier] isEqualTo:[self coreIdentifier]]) return;
    
    OEHUDAlert *alert = [OEHUDAlert alertWithMessageText:@"If you change the core you current progress will be lost and save states will not work anymore." defaultButton:@"Change Core" alternateButton:@"Cancel"];
    [alert showSuppressionButtonForUDKey:OEAutoSwitchCoreAlertSuppressionKey];
    
    [alert setCallbackHandler:
     ^(OEHUDAlert *alert, NSUInteger result)
     {
         if(result == NSAlertDefaultReturn)
         {
             NSUserDefaults* standardUserDefaults = [NSUserDefaults standardUserDefaults];
             NSString* systemIdentifier = [self systemIdentifier];
             [standardUserDefaults setValue:[plugin bundleIdentifier] forKey:UDSystemCoreMappingKeyForSystemIdentifier(systemIdentifier)];
             
             [self OE_restartUsingCore:plugin];
         }
     }];
    
    [alert runModal];
}

- (void)OE_restartUsingCore:(OECorePlugin *)core
{
    if(core == [gameCoreManager plugin])
    {
        [[rootProxy gameCore] resetEmulation];
        return;
    }

    [self OE_terminateEmulationWithoutNotification];

    NSURL   *url   = [[self rom] URL];
    NSError *error = nil;
    if(![self OE_loadFromURL:url core:core error:&error])
    {
        [NSApp presentError:error];

        if([[self delegate] respondsToSelector:@selector(emulationDidFinishForGameViewController:)])
            [[self delegate] emulationDidFinishForGameViewController:self];

        [[self document] close];
        return;
    }

    [self OE_startEmulation];
}

- (void)editControls:(id)sender
{
    NSString *systemIdentifier = [self systemIdentifier];
    NSDictionary *userInfo = @{
        OEPreferencesOpenPanelUserInfoPanelNameKey : @"Controls",
        OEPreferencesOpenPanelUserInfoSystemIdentifierKey : systemIdentifier,
    };
    
    [[NSNotificationCenter defaultCenter] postNotificationName:OEPreferencesOpenPaneNotificationName object:nil userInfo:userInfo];
}

- (void)selectFilter:(id)sender
{
    NSString *filterName;
    if([sender isKindOfClass:[NSString class]])
        filterName = sender;
    else if([sender respondsToSelector:@selector(representedObject)] && [[sender representedObject] isKindOfClass:[NSString class]])
        filterName = [sender representedObject];
    else if([sender respondsToSelector:@selector(title)] && [[sender title] isKindOfClass:[NSString class]])
        filterName = [sender title];
    else
        DLog(@"Invalid argument passed: %@", sender);
    
    [[NSUserDefaults standardUserDefaults] setObject:filterName forKey:OEGameVideoFilterKey];
}

#pragma mark - Volume

- (void)setVolume:(float)volume asDefault:(BOOL)defaultFlag
{
    [rootProxy setVolume:volume];
    [[self controlsWindow] reflectVolume:volume];
    
    if(defaultFlag)
        [[NSUserDefaults standardUserDefaults] setValue:[NSNumber numberWithFloat:volume] forKey:OEGameVolumeKey];
}

- (void)changeVolume:(id)sender
{
    if([sender respondsToSelector:@selector(floatValue)])
        [self setVolume:[sender floatValue] asDefault:YES];
    else if([sender respondsToSelector:@selector(representedObject)] && [[sender representedObject] respondsToSelector:@selector(floatValue)])
        [self setVolume:[[sender representedObject] floatValue] asDefault:YES];
    else
       DLog(@"Invalid argument passed: %@", sender);
}

- (IBAction)volumeUp:(id)sender
{
}

- (IBAction)volumeDown:(id)sender
{
}

- (void)mute:(id)sender
{
    [self setVolume:0.0 asDefault:NO];
}

- (void)unmute:(id)sender
{
    [self setVolume:1.0 asDefault:YES];
}

#pragma mark - Saving States

- (IBAction)saveState:(id)sender
{
    NSInteger   saveGameNo    = [[self rom] saveStateCount]+1;
    // TODO: properly format date
    NSString    *proposedName = [NSString stringWithFormat:@"%@%ld %@", NSLocalizedString(@"Save-Game-", @""), saveGameNo, [NSDate date]];
    OEHUDAlert  *alert        = [OEHUDAlert saveGameAlertWithProposedName:proposedName];

    [alert setWindow:[[self view] window]];
    [alert setCallbackHandler:
     ^(OEHUDAlert *alert, NSUInteger result)
     {
         if(result == NSAlertDefaultReturn)
             [self saveStateWithName:[alert stringValue]];
     }];
    
    [alert runModal];
}

- (IBAction)quickSave:(id)sender;
{
    [self saveStateWithName:OESaveStateQuicksaveName];
}

- (void)saveStateWithName:(NSString *)stateName
{
    NSAssert(_emulationStatus != OEGameViewControllerEmulationStatusNotStarted, @"Cannot save state if emulation has not been set up");

    // calling pauseGame here because it might need some time to execute
    BOOL wasPreviouslyPlaying = (_emulationStatus == OEGameViewControllerEmulationStatusPlaying);
    if(wasPreviouslyPlaying) [self pauseGame:self];

    @try
    {
        if([self rom] == nil)
        {
            NSLog(@"Error: Can not save states without rom");
            return;
        }

        BOOL      success                = NO;
        NSString *temporaryDirectoryPath = NSTemporaryDirectory();
        NSURL    *temporaryDirectoryURL  = [NSURL fileURLWithPath:temporaryDirectoryPath];
        NSURL    *temporaryStateFileURL  = [NSURL URLWithString:[NSString stringWithUUID] relativeToURL:temporaryDirectoryURL];

        temporaryStateFileURL = [temporaryStateFileURL uniqueURLUsingBlock:
                                 ^ NSURL *(NSInteger triesCount)
                                 {
                                     return [NSURL URLWithString:[NSString stringWithUUID] relativeToURL:temporaryDirectoryURL];
                                 }];

        success = [rootProxy saveStateToFileAtPath:[temporaryStateFileURL path]];
        if(!success)
        {
            NSLog(@"Could not create save state file at url: %@", temporaryStateFileURL);
            return;
        }

        BOOL isSpecialSaveState = [stateName hasPrefix:OESaveStateSpecialNamePrefix];
        OEDBSaveState *state;
        if(isSpecialSaveState)
        {
            state = [[self rom] saveStateWithName:stateName];
        }

        if(state == nil)
            state = [OEDBSaveState createSaveStateNamed:stateName forRom:[self rom] core:[gameCoreManager plugin] withFile:temporaryStateFileURL];
        else
        {
            [state replaceStateFileWithFile:temporaryStateFileURL];
            [state setTimestamp:[NSDate date]];
            [state writeInfoPlist];
        }

        NSImage *screenshotImage = [gameView nativeScreenshot];
        NSData *TIFFData = [screenshotImage TIFFRepresentation];
        NSBitmapImageRep *bitmapImageRep = [NSBitmapImageRep imageRepWithData:TIFFData];
        NSData *PNGData = [bitmapImageRep representationUsingType:NSPNGFileType properties:nil];
        success = [PNGData writeToURL:[state screenshotURL] atomically: YES];

        if(!success) NSLog(@"Could not create screenshot at url: %@", [state screenshotURL]);
    }
    @finally
    {
        if(wasPreviouslyPlaying) [self playGame:self];
    }
}

#pragma mark - Loading States

- (IBAction)loadState:(id)sender
{
    // calling pauseGame here because it might need some time to execute
    [self pauseGame:self];
    
    
    OEDBSaveState *state;
    if([sender isKindOfClass:[OEDBSaveState class]])
        state = sender;
    else if([sender respondsToSelector:@selector(representedObject)] && [[sender representedObject] isKindOfClass:[OEDBSaveState class]])
        state = [sender representedObject];
    else
    {
        DLog(@"Invalid argument passed: %@", sender);
        return;
    }
    
    if([state rom] != [self rom])
    {
        NSLog(@"Invalid save state for current rom");
        return;
    }
    
    if([[self coreIdentifier] isNotEqualTo:[state coreIdentifier]])
    {
        OEHUDAlert *alert = [OEHUDAlert alertWithMessageText:@"This save state was created with a different core. Do you want to switch to that core now?" defaultButton:@"OK" alternateButton:@"Cancel"];
        [alert showSuppressionButtonForUDKey:OEAutoSwitchCoreAlertSuppressionKey];
        if([alert runModal])
        {
            OECorePlugin *core = [OECorePlugin corePluginWithBundleIdentifier:[state coreIdentifier]];
            if(core != nil)
                [self OE_restartUsingCore:core];
            else
            {
                [[OECoreUpdater sharedUpdater] installCoreForSaveState:state withCompletionHandler:^(NSError *error) {
                    if(error == nil)
                    {
                        OECorePlugin *core = [OECorePlugin corePluginWithBundleIdentifier:[state coreIdentifier]];
                        [self OE_restartUsingCore:core];
                        [self OE_loadStateFromFile:[[state stateFileURL] path]];
                        [self playGame:self];
                    }
                }];
                return;
            }
        }
        else
        {
            [self playGame:self];
            return;
        }
    }
    [self OE_loadStateFromFile:[[state stateFileURL] path]];
    [self playGame:self];
}

- (IBAction)quickLoad:(id)sender;
{
    OEDBSaveState *quicksaveState = [[self rom] quickSaveStateInSlot:0];
    if(quicksaveState)
        [self loadState:quicksaveState];
}

- (BOOL)OE_loadStateFromFile:(NSString*)fileName
{
    return [rootProxy loadStateFromFileAtPath:fileName];
}

#pragma mark

// delete save state expects sender or [sender representedObject] to be an OEDBSaveState object and prompts the user for confirmation
- (void)deleteSaveState:(id)sender
{
    OEDBSaveState *state;
    if([sender isKindOfClass:[OEDBSaveState class]])
        state = sender;
    else if([sender respondsToSelector:@selector(representedObject)] && [[sender representedObject] isKindOfClass:[OEDBSaveState class]])
        state = [sender representedObject];
    else
    {
        DLog(@"Invalid argument passed: %@", sender);
        return;        
    }

    NSString *stateName = [state name];
    OEHUDAlert *alert = [OEHUDAlert deleteStateAlertWithStateName:stateName];
    
    if([alert runModal]) [state remove];
}

#pragma mark - Menu Items

- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
    SEL action = [menuItem action];
    
    if(action == @selector(quickLoad:))
        return [[self rom] quickSaveStateInSlot:0]!=nil;
    else if(action == @selector(pauseGame:))
        return _emulationStatus == OEGameViewControllerEmulationStatusPlaying;
    else if(action == @selector(playGame:))
        return _emulationStatus == OEGameViewControllerEmulationStatusPaused;
    
    return YES;
}

#pragma mark - Info

- (NSSize)defaultScreenSize
{
    NSAssert(rootProxy, @"Default screen size requires a running rootProxy");

    OEGameCore *gameCore = [rootProxy gameCore];
    OEIntRect screenRect = [gameCore screenRect];
    
    float wr = (float) rootProxy.aspectSize.width / screenRect.size.width;
    float hr = (float) rootProxy.aspectSize.height / screenRect.size.height;
    float ratio = MAX(hr, wr);
    NSSize scaled = NSMakeSize((wr / ratio), (hr / ratio));
    
    float halfw = scaled.width;
    float halfh = scaled.height;
    
    return NSMakeSize(screenRect.size.width / halfh, screenRect.size.height / halfw);
}

- (NSString *)systemIdentifier
{
    return [gameSystemController systemIdentifier];
}

- (NSString *)coreIdentifier
{
    return [[gameCoreManager plugin] bundleIdentifier];
}

#pragma mark - Private Methods

+ (OEDBRom *)OE_chooseRomFromGame:(OEDBGame *)game
{
    // TODO: we could display a list of roms here if we wanted to, do we?
    return [game defaultROM];
}

- (BOOL)OE_loadFromURL:(NSURL *)aurl core:(OECorePlugin *)core error:(NSError **)outError
{
    NSString *romPath = [aurl path];
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

    if(!core) core = [self OE_coreForSystem:[[[[self rom] game] system] plugin] error:outError];
    if(!core)
    {
        if(outError != NULL)
            *outError = [NSError errorWithDomain:OEGameDocumentErrorDomain
                                            code:OENoCoreForSystemError
                                        userInfo:
                         @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(@"No suitable core found.", @"Core not installed error reason."),
                            NSLocalizedRecoverySuggestionErrorKey : NSLocalizedString(@"Install a core for this system.", @"Core not installed error recovery suggestion.") }];
        return NO;
    }
    
    _corePlugin = core;

    return YES;
}

- (void)OE_repositionControlsWindow
{
    NSWindow *gameWindow = [self OE_rootWindow];
    if(gameWindow == nil) return;

    const NSRect         gameViewFrameInWindow = [gameView convertRect:[gameView frame] toView:nil];
    NSPoint              origin                = [gameWindow convertRectToScreen:gameViewFrameInWindow].origin;
    static const CGFloat _OEControlsMargin     = 19;

    origin.x += ([gameView frame].size.width - [controlsWindow frame].size.width) / 2;

    // If the controls bar fits, it sits over the window
    if([gameView frame].size.width >= [controlsWindow frame].size.width)
    {
        origin.y += _OEControlsMargin;
    }
    // Otherwise, it sits below the window
    else
    {
        origin.y -= ([controlsWindow frame].size.height + _OEControlsMargin);

        // Unless below the window means it being off-screen, in which case it sits above the window
        if(origin.y < NSMinY([[gameWindow screen] visibleFrame]))
            origin.y = NSMaxY([gameWindow frame]) + _OEControlsMargin;
    }

    [controlsWindow setFrameOrigin:origin];
}

- (NSWindow *)OE_rootWindow
{
    NSWindow *window = [[self gameView] window];
    while([window parentWindow])
        window = [window parentWindow];
    return window;
}

#pragma mark - Notifications

- (void)viewDidChangeFrame:(NSNotification*)notification
{
    [self OE_repositionControlsWindow];
}

- (void)windowDidMove:(NSNotification *)notification
{
    [self OE_repositionControlsWindow];
}

- (void)windowDidChangeScreen:(NSNotification *)notification
{
    [self OE_repositionControlsWindow];
}

#pragma mark - Plugin discovery

- (OECorePlugin *)OE_coreForSystem:(OESystemPlugin *)system error:(NSError **)outError
{
    OECorePlugin *chosenCore = nil;
    NSArray *validPlugins = [OECorePlugin corePluginsForSystemIdentifier:[system systemIdentifier]];
    
    if([validPlugins count] == 0 && outError != nil)
    {
        *outError = [NSError errorWithDomain:OEGameDocumentErrorDomain
                                        code:OEIncorrectFileError
                                    userInfo: @{
           NSLocalizedFailureReasonErrorKey : NSLocalizedString(@"The launched file isn't handled by OpenEmu", @"Incorrect file error reason."),
      NSLocalizedRecoverySuggestionErrorKey : NSLocalizedString(@"Choose a file with a supported file format or download an appropriate OpenEmu plugin.", @"Incorrect file error recovery suggestion."),
                     }];
        chosenCore = nil;
    }
    else if([validPlugins count] == 1)
        chosenCore = [validPlugins lastObject];
    else
    {
        NSUserDefaults *standardUserDefaults = [NSUserDefaults standardUserDefaults];
        BOOL forceCorePicker = [standardUserDefaults boolForKey:OEForceCorePicker];
        NSString *coreIdentifier = [standardUserDefaults valueForKey:UDSystemCoreMappingKeyForSystemIdentifier([system systemIdentifier])];
        chosenCore = [OECorePlugin corePluginWithBundleIdentifier:coreIdentifier];
        if(chosenCore == nil && !forceCorePicker)
        {
            validPlugins = [validPlugins sortedArrayUsingComparator:
                            ^ NSComparisonResult (id obj1, id obj2)
                            {
                                return [[obj1 displayName] compare:[obj2 displayName]];
                            }];
            
            chosenCore = [validPlugins objectAtIndex:0];
            [standardUserDefaults setValue:[chosenCore bundleIdentifier] forKey:UDSystemCoreMappingKeyForSystemIdentifier([system systemIdentifier])];
        }
        
        if(forceCorePicker)
        {
            OECorePickerController *c = [[OECorePickerController alloc] initWithCoreList:validPlugins];
            if([[NSApplication sharedApplication] runModalForWindow:[c window]] == 1)
                chosenCore = [c selectedCore];
        }
    }
    
    return chosenCore;
}

#pragma mark - TaskWrapper delegate methods

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

@end
