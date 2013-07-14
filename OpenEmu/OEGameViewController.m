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
#import "OECorePlugin.h"

#import "OEDBSaveState.h"
#import "OEGameControlsBar.h"

#import "OECoreUpdater.h"

#import "OEGameDocument.h"
#import "OEAudioDeviceManager.h"

#import "OEHUDAlert+DefaultAlertsAdditions.h"

#import "NSURL+OELibraryAdditions.h"

#import "OEPreferencesController.h"
#import "OELibraryDatabase.h"

#import <OpenEmuSystem/OpenEmuSystem.h>

NSString *const OEGameVolumeKey = @"volume";
NSString *const OEGameDefaultVideoFilterKey = @"videoFilter";
NSString *const OEGameSystemVideoFilterKeyFormat = @"videoFilter.%@";
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

@interface OEGameViewControllerSaveStateCallback : NSObject <OEGameCoreHelperSaveStateDelegate>
+ (instancetype)saveStateCallbackWithBlock:(void (^)(BOOL))block;
@property(copy) void (^callback)(BOOL success);
@end

#define UDDefaultCoreMappingKeyPrefix   @"defaultCore"
#define UDSystemCoreMappingKeyForSystemIdentifier(_SYSTEM_IDENTIFIER_) [NSString stringWithFormat:@"%@.%@", UDDefaultCoreMappingKeyPrefix, _SYSTEM_IDENTIFIER_]

@interface OEGameViewController ()
{
    // IPC from our OEHelper
    id<OEGameCoreHelper>                 _rootProxy;
    OEGameCoreManager                   *_gameCoreManager;

    // Standard game document stuff
    OEGameView                          *_gameView;
    OESystemController                  *_gameSystemController;

    OESystemResponder                   *_gameSystemResponder;

    NSTimer                             *_systemSleepTimer;
    OECorePlugin                        *_corePlugin;
    OEGameViewControllerEmulationStatus  _emulationStatus;
    OEDBSaveState                       *_saveStateForGameStart;
    NSDate                              *_lastPlayStartDate;
    BOOL                                 _pausedByGoingToBackground;
}

@end

@implementation OEGameViewController

// We cannot use +initialize since it is only sent when the class is first used
// by the runtime. If the user opens the Preferences window before running any games,
// i.e., before OEGameViewController is used and +initialize is sent, defaults aren’t
// registered, NSUserDefaultsController doesn’t know about them and the Preferences
// window doesn’t show them.
+ (void)load
{
    [[NSUserDefaults standardUserDefaults] registerDefaults:@{ OEBackgroundPauseKey : @YES }];
}

- (id)initWithRom:(OEDBRom *)aRom core:(OECorePlugin *)core error:(NSError **)outError
{
    if((self = [super init]))
    {
        _controlsWindow = [[OEGameControlsBar alloc] initWithGameViewController:self];
        [_controlsWindow setReleasedWhenClosed:YES];
        
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

- (id)initWithGame:(OEDBGame *)game core:(OECorePlugin *)core error:(NSError **)outError
{
    return [self initWithRom:[OEGameViewController OE_chooseRomFromGame:game] core:core error:outError];
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
    [nc removeObserver:self name:NSViewFrameDidChangeNotification object:_gameView];
    [nc removeObserver:self name:OEDeviceHandlerDidReceiveLowBatteryWarningNotification object:nil];
    [nc removeObserver:self name:OEDeviceManagerDidRemoveDeviceHandlerNotification object:nil];

    [_controlsWindow
     close];
    _controlsWindow = nil;
    _gameView = nil;
}

#pragma mark -

- (void)viewDidAppear
{
    [super viewDidAppear];

    if([_controlsWindow parentWindow] != nil) [[_controlsWindow parentWindow] removeChildWindow:_controlsWindow];
    
    NSWindow *window = [self OE_rootWindow];
    if(window == nil) return;

    NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
    [nc addObserver:self selector:@selector(windowDidBecomeKey:)    name:NSWindowDidBecomeKeyNotification    object:window];
    [nc addObserver:self selector:@selector(windowDidResignKey:)    name:NSWindowDidResignKeyNotification    object:window];
    [nc addObserver:self selector:@selector(windowDidMove:)         name:NSWindowDidMoveNotification         object:window];
    [nc addObserver:self selector:@selector(windowDidChangeScreen:) name:NSWindowDidChangeScreenNotification object:window];

    [window addChildWindow:_controlsWindow ordered:NSWindowAbove];
    [self OE_repositionControlsWindow];
    [_controlsWindow orderFront:self];

    [window makeFirstResponder:_gameView];
    
    if(![[NSUserDefaults standardUserDefaults] boolForKey:OEDontShowGameTitleInWindowKey])
    {
        [window setTitle:[[[self rom] game] displayName]];
        _gameView.gameTitle = [[[self rom] game] displayName];
    }
#if DEBUG_PRINT
    [window setTitle:[[window title] stringByAppendingString:@" (DEBUG BUILD)"]];
#endif
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
    
#if DEBUG_PRINT
    [window setTitle:[[window title] stringByAppendingString:@" (DEBUG BUILD)"]];
#endif
    [[self controlsWindow] hide];
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

- (void)resetEmulation:(id)sender
{
    if([[OEHUDAlert resetSystemAlert] runModal] == NSAlertDefaultReturn)
    {
        [[_rootProxy gameCore] resetEmulation];
        [self playGame:self];
    }
}

- (BOOL)shouldTerminateEmulation
{
    [self enableOSSleep];
    [self pauseGame:self];

    [[self controlsWindow] setCanShow:NO];

    if(![[OEHUDAlert stopEmulationAlert] runModal] == NSAlertDefaultReturn)
    {
        [[self controlsWindow] setCanShow:YES];
        [self disableOSSleep];
        [self playGame:self];
        return NO;
    }

    return YES;
}

- (void)terminateEmulation
{
    if(_emulationStatus == OEGameViewControllerEmulationStatusNotStarted ||
       _emulationStatus == OEGameViewControllerEmulationStatusTerminating)
        return;

    [self OE_pauseEmulationIfNeeded];
    [self saveStateWithName:OESaveStateAutosaveName synchronously:YES resumeGame:NO];

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
    NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
    [nc removeObserver:self name:NSViewFrameDidChangeNotification object:_gameView];
    [nc removeObserver:self name:OEDeviceHandlerDidReceiveLowBatteryWarningNotification object:nil];
    [nc removeObserver:self name:OEDeviceManagerDidRemoveDeviceHandlerNotification object:nil];

    _emulationStatus = OEGameViewControllerEmulationStatusNotStarted;

    [_gameView setRootProxy:nil];
    [_gameView setGameResponder:nil];

    _gameView = nil;
    
    _gameSystemController = nil;
    _gameSystemResponder  = nil;
    
    // kill our background friend
    [_gameCoreManager stop];
    
    _gameCoreManager = nil;
    _rootProxy = nil;

    [[self rom] addTimeIntervalToPlayTime:ABS([_lastPlayStartDate timeIntervalSinceNow])];
    _lastPlayStartDate = nil;

}

- (IBAction)performClose:(id)sender
{
    if([self shouldTerminateEmulation])
        [self terminateEmulation];
}

- (void)OE_startEmulation
{
    if(_emulationStatus != OEGameViewControllerEmulationStatusNotStarted) return;

    NSString *systemIdentifier = [[[[self rom] game] system] systemIdentifier];
    NSError *error;
    Class managerClass = ([[NSUserDefaults standardUserDefaults] boolForKey:OEGameCoresInBackgroundKey]
                          ? [OEGameCoreThreadManager  class]
                          : [OEGameCoreProcessManager class]);
    _gameCoreManager = [[managerClass alloc] initWithROMAtPath:[[[self rom] URL] path] corePlugin:_corePlugin systemIdentifier:systemIdentifier error:&error];

    if(_gameCoreManager == nil)
    {
        [[self document] presentError:error];

        if([[self delegate] respondsToSelector:@selector(emulationDidFinishForGameViewController:)])
            [[self delegate] emulationDidFinishForGameViewController:self];

        [[self document] close];
    }

    _rootProxy = [_gameCoreManager rootProxy];
    
    OEGameCore *gameCore = [_rootProxy gameCore];
    _gameSystemController = [[[[[self rom] game] system] plugin] controller];
    _gameSystemResponder  = [_gameSystemController newGameSystemResponder];
    [_gameSystemResponder setClient:gameCore];

    if(_gameView == nil)
    {
        _gameView = [[OEGameView alloc] initWithFrame:[[self view] bounds]];
        [_gameView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
        [[self view] addSubview:_gameView];
    }
    [_gameView setGameFrameInterval:[gameCore frameInterval]];
    [_gameView setRootProxy:_rootProxy];
    [_gameView setGameResponder:_gameSystemResponder];
    
    [_rootProxy setupEmulation];
    _emulationStatus = OEGameViewControllerEmulationStatusPlaying;

    NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
    [nc addObserver:self selector:@selector(viewDidChangeFrame:) name:NSViewFrameDidChangeNotification object:_gameView];
    [nc addObserver:self selector:@selector(OE_lowDeviceBattery:) name:OEDeviceHandlerDidReceiveLowBatteryWarningNotification object:nil];
    [nc addObserver:self selector:@selector(OE_deviceDidDisconnect:) name:OEDeviceManagerDidRemoveDeviceHandlerNotification object:nil];

    NSWindow *window = [self OE_rootWindow];
    [window makeFirstResponder:_gameView];

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

- (BOOL)OE_pauseEmulationIfNeeded
{
    BOOL pauseNeeded = _emulationStatus == OEGameViewControllerEmulationStatusPlaying;

    if(pauseNeeded) [self setPauseEmulation:YES];

    return pauseNeeded;
}

- (IBAction)pauseGame:(id)sender
{
    [self OE_pauseEmulationIfNeeded];
}

- (IBAction)playGame:(id)sender
{
    if(_emulationStatus == OEGameViewControllerEmulationStatusNotStarted) [self OE_startEmulation];
    else if(_emulationStatus == OEGameViewControllerEmulationStatusPaused) [self setPauseEmulation:NO];
}

- (void)toggleEmulationPause:(id)sender
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

    [_rootProxy setPauseEmulation:pauseEmulation];
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
    
    OEHUDAlert *alert = [OEHUDAlert alertWithMessageText:NSLocalizedString(@"If you change the core you current progress will be lost and save states will not work anymore.", @"")
                                           defaultButton:NSLocalizedString(@"Change Core", @"")
                                         alternateButton:NSLocalizedString(@"Cancel", @"")];
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
    if(core == [_gameCoreManager plugin])
    {
        [[_rootProxy gameCore] resetEmulation];
        return;
    }

    _emulationStatus = OEGameViewControllerEmulationStatusNotStarted;

    // kill our background friend
    [_gameCoreManager stop];

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
        OEPreferencesUserInfoPanelNameKey : @"Controls",
        OEPreferencesUserInfoSystemIdentifierKey : systemIdentifier,
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

    [_gameView setFilterName:filterName];
    [[NSUserDefaults standardUserDefaults] setObject:filterName forKey:[NSString stringWithFormat:OEGameSystemVideoFilterKeyFormat, [self systemIdentifier]]];
}

- (void)changeAudioOutputDevice:(id)sender
{
    OEAudioDevice *device = nil;

    if([sender isKindOfClass:[OEAudioDevice class]])
        device = sender;
    else if ([sender respondsToSelector:@selector(representedObject)] && [[sender representedObject] isKindOfClass:[OEAudioDevice class]])
        device = [sender representedObject];

    if(device == nil)
    {
        DLog(@"Invalid argument: %@", sender);
        return;
    }

    [_rootProxy setAudioOutputDeviceID:[device deviceID]];
}

#pragma mark - Volume

- (void)setVolume:(float)volume asDefault:(BOOL)defaultFlag
{
    [_rootProxy setVolume:volume];
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
    [_rootProxy volumeUp];
    float volume = [[self controlsWindow] reflectVolumeUp];

    [[NSUserDefaults standardUserDefaults] setValue:[NSNumber numberWithFloat:volume] forKey:OEGameVolumeKey];
}

- (IBAction)volumeDown:(id)sender
{
    [_rootProxy volumeDown];
    float volume = [[self controlsWindow] reflectVolumeDown];

    [[NSUserDefaults standardUserDefaults] setValue:[NSNumber numberWithFloat:volume] forKey:OEGameVolumeKey];
}

- (void)mute:(id)sender
{
    [self setVolume:0.0 asDefault:NO];
}

- (void)unmute:(id)sender
{
    [self setVolume:1.0 asDefault:YES];
}

#pragma mark - Cheats

- (IBAction)addCheat:(id)sender
{
    OEHUDAlert *alert = [[OEHUDAlert alloc] init];
    
    [alert setOtherInputLabelText:NSLocalizedString(@"Title:", @"")];
    [alert setShowsOtherInputField:YES];
    [alert setOtherStringValue:NSLocalizedString(@"Cheat Description", @"")];
    
    [alert setInputLabelText:NSLocalizedString(@"Code:", @"")];
    [alert setShowsInputField:YES];
    [alert setStringValue:@"000-000+000-000;01HHHHHH"];
    
    [alert setDefaultButtonTitle:NSLocalizedString(@"Add Cheat", @"")];
    [alert setAlternateButtonTitle:NSLocalizedString(@"Cancel", @"")];
    
    [alert setInputLimit:1000];
    
    if([alert runModal])
    {
        // TODO: decide how to handle setting a cheat type from the modal and save added cheats to file
        NSMutableDictionary *cheatsDictionary = [[NSMutableDictionary alloc] init];
        [cheatsDictionary setObject:[alert stringValue] forKey:@"code"];
        [cheatsDictionary setObject:@"Unknown" forKey:@"type"];
        [cheatsDictionary setObject:[alert otherStringValue] forKey:@"description"];
        [cheatsDictionary setObject:[NSNumber numberWithBool:NO] forKey:@"enabled"];
    
        [[sender representedObject] addObject:cheatsDictionary];
    }
}

- (IBAction)setCheat:(id)sender
{
    NSString *code, *type;
    BOOL enabled;
    code = [[sender representedObject] objectForKey:@"code"];
    type = [[sender representedObject] objectForKey:@"type"];
    enabled = [[[sender representedObject] objectForKey:@"enabled"] boolValue];
    
    if (enabled) {
        [[sender representedObject] setObject:[NSNumber numberWithBool:NO] forKey:@"enabled"];
        enabled = NO;
    }
    else {
        [[sender representedObject] setObject:[NSNumber numberWithBool:YES] forKey:@"enabled"];
        enabled = YES;
    }
    
    [self setCheatWithCodeAndType:code setType:type setEnabled:enabled];
}

- (BOOL)cheatSupport
{
    OEGameCore *gameCore = [_rootProxy gameCore];
    return [gameCore canCheat];
}

- (void)setCheatWithCodeAndType:(NSString *)code setType:(NSString *)type setEnabled:(BOOL)enabled
{
    [_rootProxy setCheat:code setType:type setEnabled:enabled];
}

#pragma mark - Saving States

- (IBAction)saveState:(id)sender
{
    BOOL didPauseEmulation = [self OE_pauseEmulationIfNeeded];

    NSInteger   saveGameNo    = [[self rom] saveStateCount] + 1;
    // TODO: properly format date
    NSString    *proposedName = [NSString stringWithFormat:@"%@%ld %@", NSLocalizedString(@"Save-Game-", @""), saveGameNo, [NSDate date]];
    OEHUDAlert  *alert        = [OEHUDAlert saveGameAlertWithProposedName:proposedName];

    [alert setWindow:[[self view] window]];
    [alert setCallbackHandler:
     ^(OEHUDAlert *alert, NSUInteger result)
     {
         if(result == NSAlertDefaultReturn)
             [self saveStateWithName:[alert stringValue] synchronously:NO resumeGame:didPauseEmulation];
     }];
    
    [alert runModal];
}

- (IBAction)quickSave:(id)sender;
{
    int slot = 0;
    if([sender respondsToSelector:@selector(representedObject)] && [[sender representedObject] respondsToSelector:@selector(intValue)])
        slot = [[sender representedObject] intValue];
    else if([sender respondsToSelector:@selector(tag)])
        slot = [sender tag];
    
    NSString *name = [OEDBSaveState nameOfQuickSaveInSlot:slot];
    [self saveStateWithName:name synchronously:NO resumeGame:[self OE_pauseEmulationIfNeeded]];
}

- (void)saveStateWithName:(NSString *)stateName synchronously:(BOOL)synchronously resumeGame:(BOOL)resumeGame
{
    NSAssert(_emulationStatus != OEGameViewControllerEmulationStatusNotStarted, @"Cannot save state if emulation has not been set up");

    @try
    {
        if([self rom] == nil)
        {
            NSLog(@"Error: Can not save states without rom");
            return;
        }

        NSString *temporaryDirectoryPath = NSTemporaryDirectory();
        NSURL    *temporaryDirectoryURL  = [NSURL fileURLWithPath:temporaryDirectoryPath];
        NSURL    *temporaryStateFileURL  = [NSURL URLWithString:[NSString stringWithUUID] relativeToURL:temporaryDirectoryURL];
        OECorePlugin *core = [_gameCoreManager plugin];

        temporaryStateFileURL = [temporaryStateFileURL uniqueURLUsingBlock:
                                 ^ NSURL *(NSInteger triesCount)
                                 {
                                     return [NSURL URLWithString:[NSString stringWithUUID] relativeToURL:temporaryDirectoryURL];
                                 }];

        void (^completionBlock)(BOOL success) =
        ^(BOOL success)
        {
            if(!success)
            {
                NSLog(@"Could not create save state file at url: %@", temporaryStateFileURL);
                
                if(resumeGame) [self playGame:self];
                return;
            }

            BOOL isSpecialSaveState = [stateName hasPrefix:OESaveStateSpecialNamePrefix];
            OEDBSaveState *state;
            if(isSpecialSaveState)
            {
                state = [[self rom] saveStateWithName:stateName];
                [state setCoreIdentifier:[core bundleIdentifier]];
                [state setCoreVersion:[core version]];
            }

            if(state == nil)
                state = [OEDBSaveState createSaveStateNamed:stateName forRom:[self rom] core:core withFile:temporaryStateFileURL];
            else
            {
                [state replaceStateFileWithFile:temporaryStateFileURL];
                [state setTimestamp:[NSDate date]];
                [state writeInfoPlist];
            }

            NSImage *screenshotImage = [_gameView nativeScreenshot];
            NSData *TIFFData = [screenshotImage TIFFRepresentation];
            NSBitmapImageRep *bitmapImageRep = [NSBitmapImageRep imageRepWithData:TIFFData];
            NSData *PNGData = [bitmapImageRep representationUsingType:NSPNGFileType properties:nil];
            success = [PNGData writeToURL:[state screenshotURL] atomically: YES];

            if(!success) NSLog(@"Could not create screenshot at url: %@", [state screenshotURL]);
            if(resumeGame) [self playGame:self];
        };

        if(synchronously)
            completionBlock([_rootProxy saveStateToFileAtPath:[temporaryStateFileURL path]]);
        else
            [_rootProxy saveStateToFileAtPath:[temporaryStateFileURL path] delegate:[OEGameViewControllerSaveStateCallback saveStateCallbackWithBlock:completionBlock]];
    }
    @finally
    {
    }
}

- (void)OE_saveStateDidCompleteForFileAtURL:(NSURL *)fileURL withSuccess:(BOOL)successful resumeGame:(BOOL)resume
{
    
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
        OEHUDAlert *alert = [OEHUDAlert alertWithMessageText:NSLocalizedString(@"This save state was created with a different core. Do you want to switch to that core now?", @"")
                                               defaultButton:NSLocalizedString(@"OK", @"")
                                             alternateButton:NSLocalizedString(@"Cancel", @"")];
        [alert showSuppressionButtonForUDKey:OEAutoSwitchCoreAlertSuppressionKey];

        if([alert runModal])
        {
            OECorePlugin *core = [OECorePlugin corePluginWithBundleIdentifier:[state coreIdentifier]];
            if(core != nil)
                [self OE_restartUsingCore:core];
            else
            {
                [[OECoreUpdater sharedUpdater] installCoreForSaveState:state withCompletionHandler:
                 ^(NSError *error)
                 {
                    if(error == nil)
                    {
                        OECorePlugin *core = [OECorePlugin corePluginWithBundleIdentifier:[state coreIdentifier]];
                        [self OE_restartUsingCore:core];
                        [_rootProxy loadStateFromFileAtPath:[[state stateFileURL] path] delegate:nil];
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

    [_rootProxy loadStateFromFileAtPath:[[state stateFileURL] path] delegate:nil];
    [self playGame:self];
}

- (IBAction)quickLoad:(id)sender;
{
    int slot = 0;
    if([sender respondsToSelector:@selector(representedObject)] && [[sender representedObject] respondsToSelector:@selector(intValue)])
        slot = [[sender representedObject] intValue];
    else if([sender respondsToSelector:@selector(tag)])
        slot = [sender tag];

    OEDBSaveState *quicksaveState = [[self rom] quickSaveStateInSlot:slot];
    if(quicksaveState!= nil) [self loadState:quicksaveState];
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

#pragma mark - Taking Screenshots

- (void)takeScreenshot:(id)sender
{
    NSImage *screenshotImage = [_gameView screenshot];
    NSData *TIFFData = [screenshotImage TIFFRepresentation];
    NSBitmapImageRep *bitmapImageRep = [NSBitmapImageRep imageRepWithData:TIFFData];
    NSData *PNGData = [bitmapImageRep representationUsingType:NSPNGFileType properties:nil];

    NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
    [dateFormatter setDateFormat:@"yyyy-MM-dd kk.mm.ss"];
    NSString *timeStamp = [dateFormatter stringFromDate:[NSDate date]];
    
    NSURL *screenshotFolderURL = [[OELibraryDatabase defaultDatabase] screenshotFolderURL];
    NSURL *screenshotURL = [screenshotFolderURL URLByAppendingPathComponent:[NSString stringWithFormat:@"%@ %@.png", [[[self rom] game] displayName], timeStamp]];
    
    [PNGData writeToURL:screenshotURL atomically: YES];
}

#pragma mark - Menu Items

- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
    SEL action = [menuItem action];
    
    if(action == @selector(quickLoad:))
    {
        int slot = [menuItem representedObject] ? [[menuItem representedObject] intValue] : [menuItem tag];
        return [[self rom] quickSaveStateInSlot:slot]!=nil;
    }
    else if(action == @selector(pauseGame:))
        return _emulationStatus == OEGameViewControllerEmulationStatusPlaying;
    else if(action == @selector(playGame:))
        return _emulationStatus == OEGameViewControllerEmulationStatusPaused;
    
    return YES;
}

#pragma mark - Info

- (NSSize)defaultScreenSize
{
    NSAssert(_rootProxy, @"Default screen size requires a running _rootProxy");

    OEGameCore *gameCore = [_rootProxy gameCore];
    OEIntRect screenRect = [gameCore screenRect];
    
    float wr = (float) _rootProxy.aspectSize.width / screenRect.size.width;
    float hr = (float) _rootProxy.aspectSize.height / screenRect.size.height;
    float ratio = MAX(hr, wr);
    NSSize scaled = NSMakeSize((wr / ratio), (hr / ratio));
    
    float halfw = scaled.width;
    float halfh = scaled.height;
    
    return NSMakeSize(screenRect.size.width / halfh, screenRect.size.height / halfw);
}

- (NSString *)systemIdentifier
{
    return [_gameSystemController systemIdentifier];
}

- (NSString *)coreIdentifier
{
    return [[_gameCoreManager plugin] bundleIdentifier];
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

    if(core == nil) core = [self OE_coreForSystem:[[[[self rom] game] system] plugin] error:outError];

    if(core == nil)
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

    const NSRect         gameViewFrameInWindow = [_gameView convertRect:[_gameView frame] toView:nil];
    NSPoint              origin                = [gameWindow convertRectToScreen:gameViewFrameInWindow].origin;
    static const CGFloat _OEControlsMargin     = 19;

    origin.x += ([_gameView frame].size.width - [_controlsWindow frame].size.width) / 2;

    // If the controls bar fits, it sits over the window
    if([_gameView frame].size.width >= [_controlsWindow frame].size.width)
    {
        origin.y += _OEControlsMargin;
    }
    // Otherwise, it sits below the window
    else
    {
        origin.y -= ([_controlsWindow frame].size.height + _OEControlsMargin);

        // Unless below the window means it being off-screen, in which case it sits above the window
        if(origin.y < NSMinY([[gameWindow screen] visibleFrame]))
            origin.y = NSMaxY([gameWindow frame]) + _OEControlsMargin;
    }

    [_controlsWindow setFrameOrigin:origin];
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

- (void)OE_lowDeviceBattery:(NSNotification *)notification
{
    BOOL isRunning = [self isEmulationRunning];
    [self pauseGame:self];
    
    OEDeviceHandler *devHandler = [notification object];
    NSString *lowBatteryString = [NSString stringWithFormat:NSLocalizedString(@"The battery in device number %lu, %@, is low. Please charge or replace the battery.", @"Low battery alert detail message."), [devHandler deviceNumber], [[devHandler deviceDescription] name]];
    OEHUDAlert *alert = [OEHUDAlert alertWithMessageText:lowBatteryString
                                           defaultButton:NSLocalizedString(@"Resume", nil)
                                         alternateButton:nil];
    [alert setHeadlineText:[NSString stringWithFormat:NSLocalizedString(@"Low Controller Battery", @"Device battery level is low.")]];
    [alert runModal];
    
    if(isRunning)
        [self playGame:self];
}

- (void)OE_deviceDidDisconnect:(NSNotification *)notification
{
    BOOL isRunning = [self isEmulationRunning];
    [self pauseGame:self];
    
    OEDeviceHandler *devHandler = [[notification userInfo] objectForKey:OEDeviceManagerDeviceHandlerUserInfoKey];
    NSString *lowBatteryString = [NSString stringWithFormat:NSLocalizedString(@"Device number %lu, %@, has disconnected.", @"Device disconnection detail message."), [devHandler deviceNumber], [[devHandler deviceDescription] name]];
    OEHUDAlert *alert = [OEHUDAlert alertWithMessageText:lowBatteryString
                                           defaultButton:NSLocalizedString(@"Resume", nil)
                                         alternateButton:nil];
    [alert setHeadlineText:[NSString stringWithFormat:NSLocalizedString(@"Device Disconnected", @"A controller device has disconnected.")]];
    [alert runModal];

    if(isRunning)
        [self playGame:self];
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

@implementation OEGameViewControllerSaveStateCallback

+ (instancetype)saveStateCallbackWithBlock:(void (^)(BOOL))block;
{
    OEGameViewControllerSaveStateCallback *newInstance = [[OEGameViewControllerSaveStateCallback alloc] init];
    [newInstance setCallback:block];
    return newInstance;
}

- (void)gameCoreHelperDidSaveState:(BOOL)success
{
    [self callback](success);
}

- (void)gameCoreHelperDidLoadState:(BOOL)success
{
    [self callback](success);
}

@end
