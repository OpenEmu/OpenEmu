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

#import "OEApplicationDelegate.h"
#import "OEAudioDeviceManager.h"
#import "OEBackgroundColorView.h"
#import "OECorePickerController.h"
#import "OECorePlugin.h"
#import "OECoreUpdater.h"
#import "OEDBRom.h"
#import "OEDBGame.h"
#import "OEDBSaveState.h"
#import "OEDistantViewController.h"
#import "OEDOGameCoreManager.h"
#import "OEGameCoreManager.h"
#import "OEGameView.h"
#import "OEGameViewController.h"
#import "OEHUDAlert+DefaultAlertsAdditions.h"
#import "OEHUDWindow.h"
#import "OELibraryDatabase.h"
#import "OEPopoutGameWindowController.h"
#import "OEPreferencesController.h"
#import "OESystemPlugin.h"
#import "OEThreadGameCoreManager.h"
#import "OEXPCGameCoreManager.h"
#import "NSURL+OELibraryAdditions.h"
#import "NSView+FadeImage.h"
#import "NSViewController+OEAdditions.h"

#import <objc/message.h>

NSString *const OEGameDocumentErrorDomain = @"OEGameDocumentErrorDomain";

#define UDDefaultCoreMappingKeyPrefix   @"defaultCore"
#define UDSystemCoreMappingKeyForSystemIdentifier(_SYSTEM_IDENTIFIER_) [NSString stringWithFormat:@"%@.%@", UDDefaultCoreMappingKeyPrefix, _SYSTEM_IDENTIFIER_]

// Helper to call a method with this signature:
// - (void)document:(NSDocument *)doc shouldClose:(BOOL)shouldClose  contextInfo:(void  *)contextInfo
#define CAN_CLOSE_REPLY ((void(*)(id, SEL, NSDocument *, BOOL, void *))objc_msgSend)

typedef enum : NSUInteger
{
    OEEmulationStatusNotSetup,
    OEEmulationStatusSetup,
    OEEmulationStatusStarting,
    OEEmulationStatusPlaying,
    OEEmulationStatusPaused,
    OEEmulationStatusTerminating,
} OEEmulationStatus;

@interface OEGameDocument ()
{
    OEGameCoreManager  *_gameCoreManager;
    OESystemController *_gameSystemController;

    NSTimer            *_systemSleepTimer;

    OEEmulationStatus   _emulationStatus;
    OEDBSaveState      *_saveStateForGameStart;
    NSDate             *_lastPlayStartDate;
    BOOL                _isMuted;
    BOOL                _pausedByGoingToBackground;
    BOOL                _isTerminatingEmulation;
}

@property OEGameViewController *gameViewController;
@property NSViewController *viewController;

@end

@implementation OEGameDocument

- (id)init
{
    if((self = [super init]) != nil)
    {
        _viewController = [[OEDistantViewController alloc] init];
        _gameViewController = [[OEGameViewController alloc] init];
        [[self gameViewController] setDocument:self];
        
        [[self distantViewController] setRealViewController:_gameViewController];
    }

    return self;
}

- (id)initWithRom:(OEDBRom *)rom core:(OECorePlugin *)core error:(NSError **)outError
{
    if(!(self = [self init]))
        return nil;

    if(![self OE_setupDocumentWithROM:rom usingCorePlugin:core error:outError])
        return nil;

    return self;
}

- (id)initWithGame:(OEDBGame *)game core:(OECorePlugin *)core error:(NSError **)outError
{
    return [self initWithRom:[game defaultROM] core:core error:outError];
}

- (id)initWithSaveState:(OEDBSaveState *)state error:(NSError **)outError
{
    if(!(self = [self init]))
        return nil;

    if(![self OE_setupDocumentWithSaveState:state error:outError])
        return nil;

    return self;
}

- (OEDistantViewController *)distantViewController
{
    return (OEDistantViewController *)[self viewController];
}

- (NSString *)coreIdentifier;
{
    return [[_gameCoreManager plugin] bundleIdentifier];
}

- (NSString *)systemIdentifier;
{
    return [_gameSystemController systemIdentifier];
}

- (BOOL)OE_setupDocumentWithSaveState:(OEDBSaveState *)saveState error:(NSError **)outError
{
    if(![self OE_setupDocumentWithROM:[saveState rom] usingCorePlugin:[OECorePlugin corePluginWithBundleIdentifier:[saveState coreIdentifier]] error:outError])
        return NO;

    _saveStateForGameStart = saveState;

    return YES;
}

- (BOOL)OE_setupDocumentWithROM:(OEDBRom *)rom usingCorePlugin:(OECorePlugin *)core error:(NSError **)outError
{
    _rom = rom;
    _gameSystemController = [[[[[self rom] game] system] plugin] controller];

    if(core == nil)
        core = [self OE_coreForSystem:[[[rom game] system] plugin] error:outError];

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

    _gameCoreManager = [self _newGameCoreManagerWithCorePlugin:core];

    return YES;
}

- (void)OE_setupGameCoreManagerUsingCorePlugin:(OECorePlugin *)core completionHandler:(void(^)(void))completionHandler
{
    NSAssert(core != [_gameCoreManager plugin], @"Do not attempt to run a new core using the same plug-in as the current one.");

    _emulationStatus = OEEmulationStatusNotSetup;
    [_gameCoreManager stopEmulationWithCompletionHandler:
     ^{
         _gameCoreManager = [self _newGameCoreManagerWithCorePlugin:core];
         [self setupGameWithCompletionHandler:
          ^(BOOL success, NSError *error)
          {
              if(!success)
              {
                  [self presentError:error];
                  return;
              }

              completionHandler();
          }];
     }];
}

- (OEGameCoreManager *)_newGameCoreManagerWithCorePlugin:(OECorePlugin *)corePlugin
{
    Class managerClass = ([[NSUserDefaults standardUserDefaults] boolForKey:OEGameCoresInBackgroundKey]
                          ? [OEThreadGameCoreManager  class]
                          : ([OEXPCGameCoreManager canUseXPCGameCoreManager]
                             ? [OEXPCGameCoreManager class]
                             : [OEDOGameCoreManager class]));

    return [[managerClass alloc] initWithROMPath:[[[self rom] URL] path] corePlugin:corePlugin systemController:_gameSystemController displayHelper:_gameViewController];
}

- (OECorePlugin *)OE_coreForSystem:(OESystemPlugin *)system error:(NSError **)outError
{
    OECorePlugin *chosenCore = nil;
    NSArray *validPlugins = [OECorePlugin corePluginsForSystemIdentifier:[self systemIdentifier]];

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
        NSString *coreIdentifier = [standardUserDefaults valueForKey:UDSystemCoreMappingKeyForSystemIdentifier([self systemIdentifier])];
        chosenCore = [OECorePlugin corePluginWithBundleIdentifier:coreIdentifier];
        if(chosenCore == nil && !forceCorePicker)
        {
            validPlugins = [validPlugins sortedArrayUsingComparator:
                            ^ NSComparisonResult (id obj1, id obj2)
                            {
                                return [[obj1 displayName] compare:[obj2 displayName]];
                            }];

            chosenCore = [validPlugins objectAtIndex:0];
            [standardUserDefaults setValue:[chosenCore bundleIdentifier] forKey:UDSystemCoreMappingKeyForSystemIdentifier([self systemIdentifier])];
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

#pragma mark - Game Window

- (void)setGameWindowController:(NSWindowController *)value
{
    if(_gameWindowController == value)
        return;

    if(_gameWindowController != nil)
    {
        [self OE_removeObserversForWindowController:_gameWindowController];
        [self removeWindowController:_gameWindowController];
    }

    _gameWindowController = value;

    if(_gameWindowController != nil)
    {
        [self addWindowController:_gameWindowController];
        [self OE_addObserversForWindowController:_gameWindowController];
    }
}

- (void)OE_addObserversForWindowController:(NSWindowController *)windowController
{
    NSWindow *window = [windowController window];
    NSNotificationCenter *center = [NSNotificationCenter defaultCenter];

    [center addObserver:self selector:@selector(windowDidBecomeMain:) name:NSWindowDidBecomeMainNotification object:window];
    [center addObserver:self selector:@selector(windowDidResignMain:) name:NSWindowDidResignMainNotification object:window];
}

- (void)OE_removeObserversForWindowController:(NSWindowController *)windowController
{
    NSWindow *window = [windowController window];
    NSNotificationCenter *center = [NSNotificationCenter defaultCenter];

    [center removeObserver:self name:NSWindowDidBecomeMainNotification object:window];
    [center removeObserver:self name:NSWindowDidResignMainNotification object:window];
}

- (void)windowDidBecomeMain:(NSNotification *)notification
{
    BOOL backgroundPause = [[NSUserDefaults standardUserDefaults] boolForKey:OEBackgroundPauseKey];
    if(backgroundPause && _emulationStatus == OEEmulationStatusPlaying)
    {
        [self setEmulationPaused:YES];
        _pausedByGoingToBackground = YES;
    }
}

- (void)windowDidResignMain:(NSNotification *)notification
{
    if(_pausedByGoingToBackground)
    {
        [self setEmulationPaused:NO];
        _pausedByGoingToBackground = NO;
    }
}

#pragma mark - Device Notifications

- (void)OE_addDeviceNotificationObservers
{
    NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
    [nc addObserver:self selector:@selector(OE_didReceiveLowBatteryWarningNotification:) name:OEDeviceHandlerDidReceiveLowBatteryWarningNotification object:nil];
    [nc addObserver:self selector:@selector(OE_deviceDidDisconnectNotification:) name:OEDeviceManagerDidRemoveDeviceHandlerNotification object:nil];
}

- (void)OE_removeDeviceNotificationObservers
{
    NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
    [nc removeObserver:self name:OEDeviceHandlerDidReceiveLowBatteryWarningNotification object:nil];
    [nc removeObserver:self name:OEDeviceManagerDidRemoveDeviceHandlerNotification object:nil];
}

- (void)OE_didReceiveLowBatteryWarningNotification:(NSNotification *)notification
{
    BOOL isRunning = ![self isEmulationPaused];
    [self setEmulationPaused:YES];

    OEDeviceHandler *devHandler = [notification object];
    NSString *lowBatteryString = [NSString stringWithFormat:NSLocalizedString(@"The battery in device number %lu, %@, is low. Please charge or replace the battery.", @"Low battery alert detail message."), [devHandler deviceNumber], [[devHandler deviceDescription] name]];
    OEHUDAlert *alert = [OEHUDAlert alertWithMessageText:lowBatteryString
                                           defaultButton:NSLocalizedString(@"Resume", nil)
                                         alternateButton:nil];
    [alert setHeadlineText:[NSString stringWithFormat:NSLocalizedString(@"Low Controller Battery", @"Device battery level is low.")]];
    [alert runModal];

    if(isRunning) [self setEmulationPaused:NO];
}

- (void)OE_deviceDidDisconnectNotification:(NSNotification *)notification
{
    BOOL isRunning = ![self isEmulationPaused];
    [self setEmulationPaused:YES];

    OEDeviceHandler *devHandler = [[notification userInfo] objectForKey:OEDeviceManagerDeviceHandlerUserInfoKey];
    NSString *lowBatteryString = [NSString stringWithFormat:NSLocalizedString(@"Device number %lu, %@, has disconnected.", @"Device disconnection detail message."), [devHandler deviceNumber], [[devHandler deviceDescription] name]];
    OEHUDAlert *alert = [OEHUDAlert alertWithMessageText:lowBatteryString
                                           defaultButton:NSLocalizedString(@"Resume", nil)
                                         alternateButton:nil];
    [alert setHeadlineText:[NSString stringWithFormat:NSLocalizedString(@"Device Disconnected", @"A controller device has disconnected.")]];
    [alert runModal];

    if(isRunning) [self setEmulationPaused:NO];
}

- (void)showInSeparateWindowInFullScreen:(BOOL)fullScreen;
{
    OEHUDWindow *window = [[OEHUDWindow alloc] initWithContentRect:NSZeroRect];
    OEPopoutGameWindowController *windowController = [[OEPopoutGameWindowController alloc] initWithWindow:window];

    [windowController setWindowFullScreen:fullScreen];
    [self setGameWindowController:windowController];
    [self showWindows];

    [self setEmulationPaused:NO];
}

- (NSString *)displayName
{
    // If we do not have a title yet, return an empty string instead of [super displayName].
    // The latter uses Cocoa document architecture and relies on documents having URLs,
    // including untitled (new) documents.
    NSString *displayName = [[[self rom] game] displayName];
#if DEBUG_PRINT
    displayName = [displayName stringByAppendingString:@" (DEBUG BUILD)"];
#endif

    return displayName ? : @"";
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

#pragma mark - NSDocument Stuff

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

    if([typeName isEqualToString:@"OpenEmu Save State"])
    {
        OEDBSaveState *state = [OEDBSaveState saveStateWithURL:absoluteURL];
        if([self OE_setupDocumentWithSaveState:state error:outError])
            return YES;

        return NO;
    }

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

    // get rom by path
    NSString *filePath = nil;
    if(![absoluteURL isFileURL])
    {
        DLog(@"URLs that are not file urls are currently not supported!");
        // TODO: Handle URLS, by downloading to temp folder
    }
    else filePath = [absoluteURL path];

    OEDBGame *game = [OEDBGame gameWithURL:absoluteURL inDatabase:[OELibraryDatabase defaultDatabase] error:outError];
    if(game == nil)
    {
        // Could not find game in database. Try to import the file
        OEROMImporter *importer = [[OELibraryDatabase defaultDatabase] importer];
        OEImportItemCompletionBlock completion =
        ^{
            OEHUDAlert *alert = [[OEHUDAlert alloc] init];

            NSString *fileName    = [[absoluteURL lastPathComponent] stringByDeletingPathExtension];
            NSString *messageText = [NSString stringWithFormat:NSLocalizedString(@"The game '%@' was imported.", ""), fileName];
            
            alert.headlineText = NSLocalizedString(@"Your game finished importing, do you want to play it now?", @"");
            alert.messageText = messageText;
            alert.defaultButtonTitle = NSLocalizedString(@"Yes", @"");
            alert.alternateButtonTitle = NSLocalizedString(@"No", @"");

            if([alert runModal] == NSAlertDefaultReturn)
                [[NSDocumentController sharedDocumentController] openDocumentWithContentsOfURL:absoluteURL display:YES completionHandler:nil];
        };

        if([importer importItemAtURL:absoluteURL withCompletionHandler:completion])
        {
            if(outError != NULL)
                *outError = [NSError errorWithDomain:OEGameDocumentErrorDomain code:OEImportRequiredError userInfo:nil];
        }

        return NO;
    }

    // TODO: Load rom that was just imported instead of the default one
    OEDBSaveState *state = [game autosaveForLastPlayedRom];
    if(state != nil && [[OEHUDAlert loadAutoSaveGameAlert] runModal] == NSAlertDefaultReturn)
        return [self OE_setupDocumentWithSaveState:state error:outError];
    else
        return [self OE_setupDocumentWithROM:[game defaultROM] usingCorePlugin:nil error:outError];
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
    else if(action == @selector(toggleEmulationPaused:))
    {
        if(_emulationStatus == OEEmulationStatusPaused)
        {
            [menuItem setTitle:NSLocalizedString(@"Play Game", @"")];
            return YES;
        }

        [menuItem setTitle:NSLocalizedString(@"Pause Game", @"")];
        return _emulationStatus == OEEmulationStatusPlaying;
    }

    return YES;
}

#pragma mark - Control Emulation

- (void)setupGameWithCompletionHandler:(void(^)(BOOL success, NSError *error))handler;
{
    if(_emulationStatus != OEEmulationStatusNotSetup) return;

    [_gameCoreManager loadROMWithCompletionHandler:
     ^(id systemClient)
     {
         [_gameCoreManager setupEmulationWithCompletionHandler:
          ^{
              _emulationStatus = OEEmulationStatusSetup;

              [self OE_addDeviceNotificationObservers];

              _gameSystemResponder = [_gameSystemController newGameSystemResponder];
              [_gameSystemResponder setClient:systemClient];

              [self disableOSSleep];
              [[self rom] incrementPlayCount];
              [[self rom] markAsPlayedNow];
              _lastPlayStartDate = [NSDate date];

              if(_saveStateForGameStart)
              {
                  [self OE_loadState:_saveStateForGameStart];
                  _saveStateForGameStart = nil;
              }

              // set initial volume
              [self setVolume:[self volume] asDefault:NO];

              handler(YES, nil);
          }];
     } errorHandler:
     ^(NSError *error)
     {
         [_gameCoreManager stop];
         _gameCoreManager = nil;
         [self close];

         handler(NO, error);
     }];
}

- (void)OE_startEmulation
{
    if(_emulationStatus != OEEmulationStatusSetup)
        return;

    _emulationStatus = OEEmulationStatusStarting;
    [_gameCoreManager startEmulationWithCompletionHandler:
     ^{
         _emulationStatus = OEEmulationStatusPlaying;
     }];
}

- (BOOL)isEmulationPaused
{
    return _emulationStatus != OEEmulationStatusPlaying;
}

- (void)setEmulationPaused:(BOOL)pauseEmulation
{
    if(_emulationStatus == OEEmulationStatusSetup)
    {
        if(!pauseEmulation) [self OE_startEmulation];
        return;
    }

    if(pauseEmulation)
    {
        [self enableOSSleep];
        _emulationStatus = OEEmulationStatusPaused;
        [[self rom] addTimeIntervalToPlayTime:ABS([_lastPlayStartDate timeIntervalSinceNow])];
        _lastPlayStartDate = nil;
    }
    else
    {
        [self disableOSSleep];
        [[self rom] markAsPlayedNow];
        _lastPlayStartDate = [NSDate date];
        _emulationStatus = OEEmulationStatusPlaying;
    }

    [_gameCoreManager setPauseEmulation:pauseEmulation];
    [[self gameViewController] reflectEmulationPaused:pauseEmulation];
}

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

    if([[plugin bundleIdentifier] isEqual:[[_gameCoreManager plugin] bundleIdentifier]]) return;

    OEHUDAlert *alert = [OEHUDAlert alertWithMessageText:NSLocalizedString(@"If you change the core you current progress will be lost and save states will not work anymore.", @"")
                                           defaultButton:NSLocalizedString(@"Change Core", @"")
                                         alternateButton:NSLocalizedString(@"Cancel", @"")];
    [alert showSuppressionButtonForUDKey:OEAutoSwitchCoreAlertSuppressionKey];

    [alert setCallbackHandler:
     ^(OEHUDAlert *alert, NSUInteger result)
     {
         if(result != NSAlertDefaultReturn)
             return;

         NSUserDefaults *standardUserDefaults = [NSUserDefaults standardUserDefaults];
         [standardUserDefaults setValue:[self coreIdentifier] forKey:UDSystemCoreMappingKeyForSystemIdentifier([self systemIdentifier])];

         [self OE_setupGameCoreManagerUsingCorePlugin:plugin completionHandler:
          ^{
              [self OE_startEmulation];
          }];
     }];

    [alert runModal];
}

- (IBAction)editControls:(id)sender
{
    NSDictionary *userInfo = @{
        OEPreferencesUserInfoPanelNameKey : @"Controls",
        OEPreferencesUserInfoSystemIdentifierKey : [self systemIdentifier],
    };

    [[NSNotificationCenter defaultCenter] postNotificationName:OEPreferencesOpenPaneNotificationName object:nil userInfo:userInfo];
}

#pragma mark - Volume

- (IBAction)changeAudioOutputDevice:(id)sender
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

    [_gameCoreManager setAudioOutputDeviceID:[device deviceID]];
}

- (float)volume
{
    return [[NSUserDefaults standardUserDefaults] floatForKey:OEGameVolumeKey];
}

- (void)setVolume:(float)volume asDefault:(BOOL)defaultFlag
{
    [_gameCoreManager setVolume:volume];
    [[self gameViewController] reflectVolume:volume];

    if(defaultFlag)
        [[NSUserDefaults standardUserDefaults] setValue:[NSNumber numberWithFloat:volume] forKey:OEGameVolumeKey];
}

- (IBAction)changeVolume:(id)sender;
{
    if([sender respondsToSelector:@selector(floatValue)])
        [self setVolume:[sender floatValue] asDefault:YES];
    else if([sender respondsToSelector:@selector(representedObject)] && [[sender representedObject] respondsToSelector:@selector(floatValue)])
        [self setVolume:[[sender representedObject] floatValue] asDefault:YES];
    else
        DLog(@"Invalid argument passed: %@", sender);
}

- (IBAction)mute:(id)sender;
{
    [self setVolume:0.0 asDefault:NO];
}

- (IBAction)unmute:(id)sender;
{
    [self setVolume:[self volume] asDefault:NO];
}

- (IBAction)volumeUp:(id)sender;
{
    CGFloat volume = [self volume];
    volume += 0.1;
    if(volume > 1.0) volume = 1.0;
    [self setVolume:volume asDefault:YES];
}

- (IBAction)volumeDown:(id)sender;
{
    CGFloat volume = [self volume];
    volume -= 0.1;
    if(volume < 0.0) volume = 0.0;
    [self setVolume:volume asDefault:YES];
}

#pragma mark - Controlling Emulation

- (IBAction)performClose:(id)sender
{
    [self close];
}

- (IBAction)stopEmulation:(id)sender;
{
    [self close];
}

- (IBAction)toggleEmulationPaused:(id)sender;
{
    [self setEmulationPaused:![self isEmulationPaused]];
}

- (IBAction)resetEmulation:(id)sender;
{
    if([[OEHUDAlert resetSystemAlert] runModal] == NSAlertDefaultReturn)
    {
        [_gameCoreManager resetEmulationWithCompletionHandler:
         ^{
             [self setEmulationPaused:NO];
         }];
    }
}

- (BOOL)shouldTerminateEmulation
{
    [self enableOSSleep];
    [self setEmulationPaused:YES];

    //[[self controlsWindow] setCanShow:NO];

    if(![[OEHUDAlert stopEmulationAlert] runModal] == NSAlertDefaultReturn)
    {
        //[[self controlsWindow] setCanShow:YES];
        [self disableOSSleep];
        [self setEmulationPaused:NO];
        return NO;
    }

    return YES;
}

- (void)shouldCloseWindowController:(NSWindowController *)windowController delegate:(id)delegate shouldCloseSelector:(SEL)shouldCloseSelector contextInfo:(void *)contextInfo
{
    [super shouldCloseWindowController:windowController delegate:delegate shouldCloseSelector:shouldCloseSelector contextInfo:contextInfo];
}

+ (BOOL)autosavesInPlace
{
    return NO;
}

- (BOOL)isDocumentEdited
{
    return _emulationStatus == OEEmulationStatusPlaying || _emulationStatus == OEEmulationStatusPaused;
}

- (void)canCloseDocumentWithDelegate:(id)delegate shouldCloseSelector:(SEL)shouldCloseSelector contextInfo:(void *)contextInfo
{
    if(_emulationStatus == OEEmulationStatusNotSetup || _emulationStatus == OEEmulationStatusTerminating)
    {
        [super canCloseDocumentWithDelegate:delegate shouldCloseSelector:shouldCloseSelector contextInfo:contextInfo];
        return;
    }

    [self OE_pauseEmulationIfNeeded];

    if(![self shouldTerminateEmulation])
    {
        CAN_CLOSE_REPLY(delegate, shouldCloseSelector, self, NO, contextInfo);
        return;
    }

    [self OE_saveStateWithName:OESaveStateAutosaveName completionHandler:
     ^{
         _emulationStatus = OEEmulationStatusTerminating;
         [self OE_removeDeviceNotificationObservers];

         [_gameCoreManager stopEmulationWithCompletionHandler:
          ^{
              DLog(@"Emulation stopped");
              _emulationStatus = OEEmulationStatusNotSetup;

              _gameSystemController = nil;
              _gameSystemResponder  = nil;

              // kill our background friend
              [_gameCoreManager stop];

              _gameCoreManager = nil;

              [[self rom] addTimeIntervalToPlayTime:ABS([_lastPlayStartDate timeIntervalSinceNow])];
              _lastPlayStartDate = nil;

              [super canCloseDocumentWithDelegate:delegate shouldCloseSelector:shouldCloseSelector contextInfo:contextInfo];
          }];
     }];
}

#pragma mark - Cheats

- (BOOL)supportsCheats
{
    return [[[_gameCoreManager plugin] controller] supportsCheatCode];
}

- (IBAction)addCheat:(id)sender;
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
        TODO("decide how to handle setting a cheat type from the modal and save added cheats to file");
        [[sender representedObject] addObject:@{
             @"code" : [alert stringValue],
             @"type" : @"Unknown",
             @"description" : [alert otherStringValue],
             @"enabled" : @NO,
         }];
    }
}

- (IBAction)setCheat:(id)sender;
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

    [self setCheat:code withType:type enabled:enabled];
}

- (IBAction)toggleCheat:(id)sender;
{
    NSString *code = [[sender representedObject] objectForKey:@"code"];
    NSString *type = [[sender representedObject] objectForKey:@"type"];
    BOOL enabled = ![[[sender representedObject] objectForKey:@"enabled"] boolValue];
    [[sender representedObject] setObject:@(enabled) forKey:@"enabled"];
    [self setCheat:code withType:type enabled:enabled];
}

- (void)setCheat:(NSString *)cheatCode withType:(NSString *)type enabled:(BOOL)enabled;
{
    [_gameCoreManager setCheat:cheatCode withType:type enabled:enabled];
}

#pragma mark - Saving States

- (BOOL)OE_pauseEmulationIfNeeded
{
    BOOL pauseNeeded = _emulationStatus == OEEmulationStatusPlaying;

    if(pauseNeeded) [self setEmulationPaused:YES];

    return pauseNeeded;
}

- (IBAction)saveState:(id)sender;
{
    BOOL didPauseEmulation = [self OE_pauseEmulationIfNeeded];

    NSInteger   saveGameNo    = [[self rom] saveStateCount] + 1;
    // TODO: properly format date
    NSString    *proposedName = [NSString stringWithFormat:@"%@%ld %@", NSLocalizedString(@"Save-Game-", @""), saveGameNo, [NSDate date]];
    OEHUDAlert  *alert        = [OEHUDAlert saveGameAlertWithProposedName:proposedName];

    [alert setWindow:[[[self gameViewController] view] window]];
    [alert setCallbackHandler:
     ^(OEHUDAlert *alert, NSUInteger result)
     {
         if(result == NSAlertDefaultReturn)
         {
             [self OE_saveStateWithName:[alert stringValue] completionHandler:
              ^{
                  if(didPauseEmulation) [self setEmulationPaused:NO];
              }];
         }
         else if(didPauseEmulation) [self setEmulationPaused:NO];
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
    BOOL didPauseEmulation = [self OE_pauseEmulationIfNeeded];
    [self OE_saveStateWithName:name completionHandler:
     ^{
         if(didPauseEmulation) [self setEmulationPaused:NO];
     }];
}

- (void)OE_saveStateWithName:(NSString *)stateName completionHandler:(void(^)(void))handler
{
    NSAssert(_emulationStatus > OEEmulationStatusStarting, @"Cannot save state if emulation has not been set up");
    NSAssert([self rom] != nil, @"Cannot save states without a rom.");

    NSString *temporaryDirectoryPath = NSTemporaryDirectory();
    NSURL    *temporaryDirectoryURL  = [NSURL fileURLWithPath:temporaryDirectoryPath];
    NSURL    *temporaryStateFileURL  = [NSURL URLWithString:[NSString stringWithUUID] relativeToURL:temporaryDirectoryURL];
    OECorePlugin *core = [_gameCoreManager plugin];

    temporaryStateFileURL =
    [temporaryStateFileURL uniqueURLUsingBlock:
     ^ NSURL *(NSInteger triesCount)
     {
         return [NSURL URLWithString:[NSString stringWithUUID] relativeToURL:temporaryDirectoryURL];
     }];

    [_gameCoreManager saveStateToFileAtPath:[temporaryStateFileURL path] completionHandler:
     ^(BOOL success, NSError *error)
     {
         if(!success)
         {
             NSLog(@"Could not create save state file at url: %@", temporaryStateFileURL);

             if(handler != nil) handler();
             return;
         }

         OEDBSaveState *state;
         if([stateName hasPrefix:OESaveStateSpecialNamePrefix])
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

         NSData *TIFFData = [[[self gameViewController] takeNativeScreenshot] TIFFRepresentation];
         NSBitmapImageRep *bitmapImageRep = [NSBitmapImageRep imageRepWithData:TIFFData];
         NSData *PNGData = [bitmapImageRep representationUsingType:NSPNGFileType properties:nil];

         if(![PNGData writeToURL:[state screenshotURL] atomically: YES])
             NSLog(@"Could not create screenshot at url: %@", [state screenshotURL]);

         if(handler != nil) handler();
     }];
}

#pragma mark - Loading States

- (IBAction)loadState:(id)sender;
{
    // calling pauseGame here because it might need some time to execute
    [self OE_pauseEmulationIfNeeded];

    OEDBSaveState *state = nil;
    if([sender isKindOfClass:[OEDBSaveState class]])
        state = sender;
    else if([sender respondsToSelector:@selector(representedObject)] && [[sender representedObject] isKindOfClass:[OEDBSaveState class]])
        state = [sender representedObject];
    else
    {
        DLog(@"Invalid argument passed: %@", sender);
        return;
    }

    [self OE_loadState:state];
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

- (void)OE_loadState:(OEDBSaveState *)state
{
    if([state rom] != [self rom])
    {
        DLog(@"Invalid save state for current rom");
        return;
    }

    void (^loadState)(void) =
    ^{
        [_gameCoreManager loadStateFromFileAtPath:[[state stateFileURL] path] completionHandler:
         ^(BOOL success, NSError *error)
         {
             if(!success)
             {
                 [self presentError:error];
                 return;
             }

             [self setEmulationPaused:NO];
         }];
    };

    if([[[_gameCoreManager plugin] bundleIdentifier] isEqualToString:[state coreIdentifier]])
    {
        loadState();
        return;
    }

    void (^runWithCore)(OECorePlugin *, NSError *) =
    ^(OECorePlugin *plugin, NSError *error)
    {
        if(plugin == nil)
        {
            [self presentError:error];
            return;
        }

        [self OE_setupGameCoreManagerUsingCorePlugin:plugin completionHandler:
         ^{
             loadState();
         }];
    };

    OEHUDAlert *alert = [OEHUDAlert alertWithMessageText:NSLocalizedString(@"This save state was created with a different core. Do you want to switch to that core now?", @"")
                                           defaultButton:NSLocalizedString(@"OK", @"")
                                         alternateButton:NSLocalizedString(@"Cancel", @"")];
    [alert showSuppressionButtonForUDKey:OEAutoSwitchCoreAlertSuppressionKey];

    if([alert runModal])
    {
        OECorePlugin *core = [OECorePlugin corePluginWithBundleIdentifier:[state coreIdentifier]];
        if(core != nil)
            runWithCore(core, nil);
        else
            [[OECoreUpdater sharedUpdater] installCoreForSaveState:state withCompletionHandler:runWithCore];
    }
    else
        [self OE_startEmulation];
}

#pragma mark - Deleting States

// delete save state expects sender or [sender representedObject] to be an OEDBSaveState object and prompts the user for confirmation
- (IBAction)deleteSaveState:(id)sender;
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

#pragma mark - OEGameViewControllerDelegate methods

- (void)gameViewController:(OEGameViewController *)sender didReceiveMouseEvent:(OEEvent *)event;
{
}

- (void)gameViewController:(OEGameViewController *)sender setDrawSquarePixels:(BOOL)drawSquarePixels
{
    [_gameCoreManager setDrawSquarePixels:drawSquarePixels];
}

@end
