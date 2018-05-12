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

#import "OEAudioDeviceManager.h"
#import "OEBackgroundColorView.h"
#import "OECorePlugin.h"
#import "OECoreUpdater.h"
#import "OEDBRom.h"
#import "OEDBGame.h"
#import "OEDBSaveState.h"
#import "OEDBSystem.h"
#import "OEGameCoreManager.h"
#import "OEGameView.h"
#import "OEGameViewController.h"
#import "OEHUDAlert+DefaultAlertsAdditions.h"
#import "OEHUDWindow.h"
#import "OELibraryDatabase.h"
#import "OEPopoutGameWindowController.h"
#import "OESystemPlugin.h"
#import "OEThreadGameCoreManager.h"
#import "OEXPCGameCoreManager.h"
#import "OEDownload.h"
#import "OEROMImporter.h"

// using the main window controller here is not very nice, but meh
#import "OEMainWindowController.h"

#import <objc/message.h>
#import <IOKit/pwr_mgt/IOPMLib.h>

#import "OpenEmu-Swift.h"

NSString *const OEGameCoreManagerModePreferenceKey = @"OEGameCoreManagerModePreference";
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

@interface OEGameDocument () <OEGameCoreOwner, OESystemBindingsObserver>
{
    OEGameCoreManager  *_gameCoreManager;

    IOPMAssertionID     _displaySleepAssertionID;

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
        _gameViewController = [[OEGameViewController alloc] init];
        [[self gameViewController] setDocument:self];
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

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@ %p, ROM: '%@', System: '%@', Core: '%@'>", [self class], self, [[[self rom] game] displayName], [_systemPlugin systemIdentifier], [_corePlugin bundleIdentifier]];
}

- (NSString *)coreIdentifier;
{
    return [[_gameCoreManager plugin] bundleIdentifier];
}

- (NSString *)systemIdentifier;
{
    return [_systemPlugin systemIdentifier];
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
    NSURL *fileURL = [rom URL];

    // Check if local file is available
    if(![fileURL checkResourceIsReachableAndReturnError:nil])
    {
        fileURL = nil;
        NSURL *sourceURL = [rom sourceURL];

        // try to fallback on external source
        if(sourceURL)
        {
            NSString *name   = [rom fileName] ?: [[sourceURL lastPathComponent] stringByDeletingPathExtension];
            NSString *server = [sourceURL host];

            if([[OEHUDAlert romDownloadRequiredAlert:name server:server] runModal] == NSAlertFirstButtonReturn)
            {
                __block NSURL   *destination;
                __block NSError *error;

                NSString *message = [NSString stringWithFormat:NSLocalizedString(@"Downloading %@…", @"Downloading rom message text"), name];
                OEHUDAlert *alert = [OEHUDAlert alertWithMessageText:message defaultButton:NSLocalizedString(@"Cancel",@"") alternateButton:@""];
                [alert setShowsProgressbar:YES];
                [alert setProgress:-1];

                [alert performBlockInModalSession:^{
                    OEDownload *download = [[OEDownload alloc] initWithURL:sourceURL];
                    [download setProgressHandler:^BOOL(CGFloat progress) {
                        [alert setProgress:progress];
                        return YES;
                    }];

                    [download setCompletionHandler:^(NSURL *dst, NSError *err) {
                        destination = dst;
                        error = err;
                        [alert closeWithResult:NSAlertSecondButtonReturn];
                    }];

                    [download startDownload];
                }];


                if([alert runModal] == NSAlertFirstButtonReturn || [error code] == NSUserCancelledError)
                {
                    // User canceld
                    if(outError != NULL)
                        *outError = [NSError errorWithDomain:NSCocoaErrorDomain
                                                        code:NSUserCancelledError
                                                    userInfo:nil];
                    return NO;
                }
                else
                {
                    if(error || destination == nil)
                    {
                        if(outError != NULL)
                            *outError = [error copy];
                        return NO;
                    }

                    fileURL = [destination copy];
                    // make sure that rom's fileName is set
                    if([rom fileName] == nil) {
                        [rom setFileName:[destination lastPathComponent]];
                        [rom save];
                    }
                }
            }
            else
            {
                // User canceld
                if(outError != NULL)
                    *outError = [NSError errorWithDomain:NSCocoaErrorDomain
                                                    code:NSUserCancelledError
                                                userInfo:nil];
                return NO;
            }
        }

        // check if we have recovered
        if(fileURL == nil || [fileURL checkResourceIsReachableAndReturnError:nil] == NO)
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
    }

    _rom = rom;
    _romFileURL = fileURL;
    _corePlugin = core;
    _systemPlugin = [[[[self rom] game] system] plugin];

    if(_corePlugin == nil)
        _corePlugin = [self OE_coreForSystem:_systemPlugin error:outError];
    
    if(_corePlugin == nil)
    {
        __block NSError *blockError = *outError;
        [[OECoreUpdater sharedUpdater] installCoreForGame:[[self rom] game] withCompletionHandler:
        ^(OECorePlugin *plugin, NSError *error)
        {
            if(error == nil && plugin != nil)
            {
                self->_corePlugin = plugin;
            }
            else if(error == nil)
            {
                blockError = nil;
            }
        }];
        *outError = blockError;
    }

    _gameCoreManager = [self _newGameCoreManagerWithCorePlugin:_corePlugin];

    return _gameCoreManager != nil;
}

- (void)OE_setupGameCoreManagerUsingCorePlugin:(OECorePlugin *)core completionHandler:(void(^)(void))completionHandler
{
    NSAssert(core != [_gameCoreManager plugin], @"Do not attempt to run a new core using the same plug-in as the current one.");

    _emulationStatus = OEEmulationStatusNotSetup;
    [_gameCoreManager stopEmulationWithCompletionHandler:^{
        [[[OEBindingsController defaultBindingsController] systemBindingsForSystemController:self->_systemPlugin.controller] removeBindingsObserver:self];

        self->_gameCoreManager = [self _newGameCoreManagerWithCorePlugin:core];
        [self setupGameWithCompletionHandler:^(BOOL success, NSError *error) {
            if(!success) {
                [self presentError:error];
                return;
            }

            completionHandler();
        }];
    }];
}

- (OEGameCoreManager *)_newGameCoreManagerWithCorePlugin:(OECorePlugin *)corePlugin
{
    if(corePlugin == nil)
        return nil;
    
    NSString *managerClassName = [[NSUserDefaults standardUserDefaults] objectForKey:OEGameCoreManagerModePreferenceKey];

    Class managerClass = NSClassFromString(managerClassName);
    if(managerClass != [OEThreadGameCoreManager class])
        managerClass = [OEXPCGameCoreManager class];

    _corePlugin = corePlugin;

    NSString *path = [[self romFileURL] path];
     // if file is in an archive append :entryIndex to path, so the core manager can figure out which entry to load
    if([[self rom] archiveFileIndex])
        path = [path stringByAppendingFormat:@":%d",[[[self rom] archiveFileIndex] intValue]];

    return [[managerClass alloc] initWithROMPath:path romCRC32:[[self rom] crc32] romMD5:[[self rom] md5] romHeader:[[self rom] header] romSerial:[[self rom] serial] systemRegion:[[OELocalizationHelper sharedHelper] regionName] corePlugin:_corePlugin systemPlugin:_systemPlugin gameCoreOwner:self];
}

- (OECorePlugin *)OE_coreForSystem:(OESystemPlugin *)system error:(NSError **)outError
{
    OECorePlugin *chosenCore = nil;
    NSArray *validPlugins = [OECorePlugin corePluginsForSystemIdentifier:[self systemIdentifier]];

    if([validPlugins count] == 0 && outError != nil)
    {
            *outError = [NSError errorWithDomain:OEGameDocumentErrorDomain
                                            code:OENoCoreError
                                        userInfo: @{
                                                    NSLocalizedFailureReasonErrorKey : NSLocalizedString(@"OpenEmu could not find a Core to launch the game", @"No Core error reason."),
                                                    NSLocalizedRecoverySuggestionErrorKey : NSLocalizedString(@"Make sure your internet connection is active and download a suitable core.", @"No Core error recovery suggestion."),
                                                    }];
        chosenCore = nil;
    }
    else if([validPlugins count] == 1)
        chosenCore = [validPlugins lastObject];
    else
    {
        NSUserDefaults *standardUserDefaults = [NSUserDefaults standardUserDefaults];
        NSString *coreIdentifier = [standardUserDefaults valueForKey:UDSystemCoreMappingKeyForSystemIdentifier([self systemIdentifier])];
        chosenCore = [OECorePlugin corePluginWithBundleIdentifier:coreIdentifier];
        if(chosenCore == nil)
        {
            validPlugins = [validPlugins sortedArrayUsingComparator:
                            ^ NSComparisonResult (id obj1, id obj2)
                            {
                                return [[obj1 displayName] caseInsensitiveCompare:[obj2 displayName]];
                            }];

            chosenCore = [validPlugins objectAtIndex:0];
        }
    }

    return chosenCore;
}

- (void)dealloc
{
    NSURL *url = [self romFileURL];
    if([url isNotEqualTo:[[self rom] URL]])
    {
        [[NSFileManager defaultManager] removeItemAtURL:url error:nil];
    }
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

- (void)windowDidResignMain:(NSNotification *)notification
{
    BOOL backgroundPause = [[NSUserDefaults standardUserDefaults] boolForKey:OEBackgroundPauseKey];
    if(backgroundPause && _emulationStatus == OEEmulationStatusPlaying)
    {
        [self setEmulationPaused:YES];
        _pausedByGoingToBackground = YES;
    }
}

- (void)windowDidBecomeMain:(NSNotification *)notification
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
                                           defaultButton:NSLocalizedString(@"Resume", @"Resume game after battery warning button label")
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

#pragma mark - Display Sleep Handling

- (void)enableOSSleep
{
    if(_displaySleepAssertionID == kIOPMNullAssertionID) return;

    IOPMAssertionRelease(_displaySleepAssertionID);
    _displaySleepAssertionID = kIOPMNullAssertionID;
}

- (void)disableOSSleep
{
    if(_displaySleepAssertionID != kIOPMNullAssertionID) return;

    IOPMAssertionCreateWithName(kIOPMAssertionTypePreventUserIdleDisplaySleep,
                                kIOPMAssertionLevelOn, CFSTR("OpenEmu playing game"), &_displaySleepAssertionID);
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
    DLog(@"%@", typeName);
    if([typeName isEqualToString:@"org.openemu.savestate"])
    {
        NSManagedObjectContext *context = [[OELibraryDatabase defaultDatabase] mainThreadContext];
        OEDBSaveState *state = [OEDBSaveState updateOrCreateStateWithURL:absoluteURL inContext:context];
        if(state && [self OE_setupDocumentWithSaveState:state error:outError])
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
    if(![absoluteURL isFileURL])
    {
        DLog(@"URLs that are not file urls are currently not supported!");
        // TODO: Handle URLS, by downloading to temp folder
    }

    OEDBGame *game = [OEDBGame gameWithURL:absoluteURL inDatabase:[OELibraryDatabase defaultDatabase] error:outError];
    if(game == nil)
    {
        // Could not find game in database. Try to import the file
        OEROMImporter *importer = [[OELibraryDatabase defaultDatabase] importer];
        OEImportItemCompletionBlock completion =
        ^(NSManagedObjectID *romID){
            
            // import probably failed
            if(!romID) return;
            
            OEHUDAlert *alert = [[OEHUDAlert alloc] init];

            NSString *fileName    = [[absoluteURL lastPathComponent] stringByDeletingPathExtension];
            NSString *messageText = [NSString stringWithFormat:NSLocalizedString(@"The game '%@' was imported.", @""), fileName];
            
            alert.headlineText = NSLocalizedString(@"Your game finished importing, do you want to play it now?", @"");
            alert.messageText = messageText;
            alert.defaultButtonTitle = NSLocalizedString(@"Yes", @"");
            alert.alternateButtonTitle = NSLocalizedString(@"No", @"");

            if([alert runModal] == NSAlertFirstButtonReturn)
            {
                NSManagedObjectContext *context = [[OELibraryDatabase defaultDatabase] mainThreadContext];
                OEDBRom *rom = [OEDBRom objectWithID:romID inContext:context];

                // Ugly hack to start imported games in main window
                OEMainWindowController *mainWindowController = [(OEApplicationDelegate*)[NSApp delegate] mainWindowController];
                if([mainWindowController mainWindowRunsGame] == NO)
                {
                    [mainWindowController startGame:[rom game]];
                }
                else
                {
                    [[NSDocumentController sharedDocumentController] openDocumentWithContentsOfURL:[rom URL] display:NO completionHandler:^(NSDocument * _Nullable document, BOOL documentWasAlreadyOpen, NSError * _Nullable error) {
                        ;
                    }];
                }
            }
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
    if(state != nil && [[OEHUDAlert loadAutoSaveGameAlert] runModal] == NSAlertFirstButtonReturn)
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
        NSInteger slot = [menuItem representedObject] ? [[menuItem representedObject] integerValue] : [menuItem tag];
        return [[self rom] quickSaveStateInSlot:slot]!=nil;
    }
    else if(action == @selector(quickSave:))
    {
        if(![self supportsSaveStates])
            return NO;
    }
    else if(action == @selector(toggleEmulationPaused:))
    {
        if(_emulationStatus == OEEmulationStatusPaused)
        {
            [menuItem setTitle:NSLocalizedString(@"Resume Emulation", @"")];
            return YES;
        }

        [menuItem setTitle:NSLocalizedString(@"Pause Emulation", @"")];
        return _emulationStatus == OEEmulationStatusPlaying;
    }

    return YES;
}

#pragma mark - Control Emulation

- (void)setupGameWithCompletionHandler:(void(^)(BOOL success, NSError *error))handler;
{
    if(![self OE_checkRequiredFiles]){
        handler(NO, nil);
        return;
    }
    
    [self OE_checkGlitches];
    
    if(_emulationStatus != OEEmulationStatusNotSetup) {
        handler(NO, nil);
        return;
    }

    [_gameCoreManager loadROMWithCompletionHandler:^{
        [self->_gameCoreManager setupEmulationWithCompletionHandler:^(IOSurfaceID surfaceID, OEIntSize screenSize, OEIntSize aspectSize) {
            NSLog(@"SETUP DONE.");
            [self->_gameViewController setScreenSize:screenSize aspectSize:aspectSize withIOSurfaceID:surfaceID];

            self->_emulationStatus = OEEmulationStatusSetup;

            // TODO: #567 and #568 need to be fixed first
            //[self OE_addDeviceNotificationObservers];

            [self disableOSSleep];
            [[self rom] incrementPlayCount];
            [[self rom] markAsPlayedNow];
            self->_lastPlayStartDate = [NSDate date];

            if(self->_saveStateForGameStart)
            {
                [self OE_loadState:self->_saveStateForGameStart];
                self->_saveStateForGameStart = nil;
            }

            // set initial volume
            [self setVolume:[self volume] asDefault:NO];

            [[[OEBindingsController defaultBindingsController] systemBindingsForSystemController:self->_systemPlugin.controller] addBindingsObserver:self];

            self->_gameCoreManager.handleEvents = self->_handleEvents;
            self->_gameCoreManager.handleKeyboardEvents = self->_handleKeyboardEvents;

            handler(YES, nil);
        }];
    } errorHandler:^(NSError *error) {
        [[[OEBindingsController defaultBindingsController] systemBindingsForSystemController:self->_systemPlugin.controller] removeBindingsObserver:self];
        self->_gameCoreManager = nil;
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
         self->_emulationStatus = OEEmulationStatusPlaying;
     }];
    
    [[self gameViewController] reflectEmulationPaused:NO];
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

- (void)setHandleEvents:(BOOL)handleEvents
{
    if (_handleEvents == handleEvents)
        return;

    _handleEvents = handleEvents;
    _gameCoreManager.handleEvents = handleEvents;
}

- (void)setHandleKeyboardEvents:(BOOL)handleKeyboardEvents
{
    if (_handleKeyboardEvents == handleKeyboardEvents)
        return;

    _handleKeyboardEvents = handleKeyboardEvents;
    _gameCoreManager.handleKeyboardEvents = handleKeyboardEvents;
}

// switchCore:: expects sender or [sender representedObject] to be an OECorePlugin object and prompts the user for confirmation
- (void)switchCore:(id)sender;
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

    [self setEmulationPaused:YES];

    OEHUDAlert *alert = [OEHUDAlert alertWithMessageText:NSLocalizedString(@"If you change the core you current progress will be lost and save states will not work anymore.", @"")
                                           defaultButton:NSLocalizedString(@"Change Core", @"")
                                         alternateButton:NSLocalizedString(@"Cancel", @"")];
    [alert showSuppressionButtonForUDKey:OEAutoSwitchCoreAlertSuppressionKey];

    [alert setCallbackHandler:
     ^(OEHUDAlert *alert, NSModalResponse result)
     {
         if(result != NSAlertFirstButtonReturn)
             return;

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
        [OEPreferencesWindowController userInfoPanelNameKey] : @"Controls",
        [OEPreferencesWindowController userInfoSystemIdentifierKey] : self.systemIdentifier,
    };

    [[NSNotificationCenter defaultCenter] postNotificationName:[OEPreferencesWindowController openPaneNotificationName] object:nil userInfo:userInfo];
}

- (void)toggleFullScreen:(id)sender
{
    [[[self gameWindowController] window] toggleFullScreen:sender];
}

- (void)takeScreenshot:(id)sender
{
    [[self gameViewController] takeScreenshot:sender];
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

- (void)toggleAudioMute
{
    if(_isMuted)
        [self unmute:self];
    else
        [self mute:self];
}

- (IBAction)mute:(id)sender;
{
    _isMuted = YES;
    [self setVolume:0.0 asDefault:NO];
}

- (IBAction)unmute:(id)sender;
{
    _isMuted = NO;
    [self setVolume:[self volume] asDefault:NO];
}

- (void)volumeUp:(id)sender;
{
    CGFloat volume = [self volume];
    volume += 0.1;
    if(volume > 1.0) volume = 1.0;
    [self setVolume:volume asDefault:YES];
}

- (void)volumeDown:(id)sender;
{
    CGFloat volume = [self volume];
    volume -= 0.1;
    if(volume < 0.0) volume = 0.0;
    [self setVolume:volume asDefault:YES];
}

#pragma mark - Controlling Emulation
- (IBAction)performClose:(id)sender
{
    [self stopEmulation:sender];
}

- (IBAction)stopEmulation:(id)sender;
{
    // we can't just close the document here because proper shutdown is implemented in
    // method -canCloseDocumentWithDelegate:shouldCloseSelector:contextInfo:
    [[[self windowControllers] valueForKey:@"window"] makeObjectsPerformSelector:@selector(performClose:)
                                                                      withObject:sender];
}

- (void)toggleEmulationPaused:(id)sender;
{
    [self setEmulationPaused:![self isEmulationPaused]];
}

- (void)resetEmulation:(id)sender;
{
    if([[OEHUDAlert resetSystemAlert] runModal] == NSAlertFirstButtonReturn)
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

    if([[OEHUDAlert stopEmulationAlert] runModal] != NSAlertFirstButtonReturn)
    {
        //[[self controlsWindow] setCanShow:YES];
        [self disableOSSleep];
        [self setEmulationPaused:NO];
        return NO;
    }

    return YES;
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

    [self OE_saveStateWithName:OESaveStateAutosaveName completionHandler:^{
        self->_emulationStatus = OEEmulationStatusTerminating;
        // TODO: #567 and #568 need to be fixed first
        //[self OE_removeDeviceNotificationObservers];

        [self->_gameCoreManager stopEmulationWithCompletionHandler:^{
            DLog(@"Emulation stopped");
            [[[OEBindingsController defaultBindingsController] systemBindingsForSystemController:self->_systemPlugin.controller] removeBindingsObserver:self];

            self->_emulationStatus = OEEmulationStatusNotSetup;

            self->_gameCoreManager = nil;

            [[self rom] addTimeIntervalToPlayTime:ABS([self->_lastPlayStartDate timeIntervalSinceNow])];
            self->_lastPlayStartDate = nil;

            [super canCloseDocumentWithDelegate:delegate shouldCloseSelector:shouldCloseSelector contextInfo:contextInfo];
        }];
    }];
}

- (BOOL)OE_checkRequiredFiles
{
    // Check current system plugin for OERequiredFiles and core plugin for OEGameCoreRequiresFiles opt-in
    if (![[[_gameCoreManager plugin] controller] requiresFilesForSystemIdentifier:[_systemPlugin systemIdentifier]])
        return YES;

    NSArray *validRequiredFiles = [[[_gameCoreManager plugin] controller] requiredFilesForSystemIdentifier:[_systemPlugin systemIdentifier]];
    if (validRequiredFiles == nil)
        return YES;

    return [[[OEBIOSFile alloc] init] allRequiredFilesAvailableForSystemIdentifier:validRequiredFiles];
}

- (BOOL)OE_checkGlitches
{
    NSString *OEGameCoreGlitchesKey       = OEGameCoreGlitchesSuppressionKey;
    NSString *OEGameCoreGlitchesKeyFormat = @"%@.%@";
    NSString *coreName                    = [[[_gameCoreManager plugin] controller] pluginName];
    NSString *systemIdentifier            = [_systemPlugin systemIdentifier];
    NSString *systemKey                   = [NSString stringWithFormat:OEGameCoreGlitchesKeyFormat, coreName, systemIdentifier];
    NSUserDefaults *userDefaults          = [NSUserDefaults standardUserDefaults];
    
    NSDictionary *glitchInfo              = [userDefaults objectForKey:OEGameCoreGlitchesKey];
    BOOL showAlert                        = ![[glitchInfo valueForKey:systemKey] boolValue];
    
    if([[[_gameCoreManager plugin] controller] hasGlitchesForSystemIdentifier:[_systemPlugin systemIdentifier]] && showAlert)
    {
        NSString *message = [NSString stringWithFormat:NSLocalizedString(@"The %@ core has compatibility issues and some games may contain glitches or not play at all.\n\nPlease do not report problems as we are not responsible for the development of %@.", @""), coreName, coreName];
        OEHUDAlert *alert = [OEHUDAlert alertWithMessageText:message
                                               defaultButton:NSLocalizedString(@"OK", @"")
                                             alternateButton:nil];
        [alert setHeadlineText:NSLocalizedString(@"Warning", @"")];
        [alert setShowsSuppressionButton:YES];
        [alert setSuppressionLabelText:NSLocalizedString(@"Do not show me again", @"Alert suppression label")];
        
        if([alert runModal] == NSAlertFirstButtonReturn && [[alert suppressionButton] state] == NSOnState)
        {
            NSMutableDictionary *systemKeyGlitchInfo = [NSMutableDictionary dictionary];
            [systemKeyGlitchInfo addEntriesFromDictionary:glitchInfo];
            [systemKeyGlitchInfo setValue:@YES forKey:systemKey];
            
            [userDefaults setObject:systemKeyGlitchInfo forKey:OEGameCoreGlitchesKey];
        }
        
        return YES;
    }
    return NO;
}

#pragma mark - Cheats

- (BOOL)supportsCheats
{
    return [[[_gameCoreManager plugin] controller] supportsCheatCodeForSystemIdentifier:[_systemPlugin systemIdentifier]];
}

- (IBAction)addCheat:(id)sender;
{
    OEHUDAlert *alert = [[OEHUDAlert alloc] init];

    [alert setOtherInputLabelText:NSLocalizedString(@"Title:", @"")];
    [alert setShowsOtherInputField:YES];
    [alert setOtherStringValue:NSLocalizedString(@"Cheat Description", @"")];

    [alert setInputLabelText:NSLocalizedString(@"Code:", @"")];
    [alert setShowsInputField:YES];
    [alert setStringValue:NSLocalizedString(@"Join multi-line cheats with '+' e.g. 000-000+111-111", @"")];

    [alert setDefaultButtonTitle:NSLocalizedString(@"Add Cheat", @"")];
    [alert setAlternateButtonTitle:NSLocalizedString(@"Cancel", @"")];
    
    [alert setShowsSuppressionButton:YES];
    [alert setSuppressionLabelText:NSLocalizedString(@"Enable now", @"Cheats button label")];

    [alert setInputLimit:1000];

    if([alert runModal] == NSAlertFirstButtonReturn)
    {
        NSNumber *enabled;
        if ([[alert suppressionButton] state] == NSOnState)
        {
            enabled = @YES;
            [self setCheat:[alert stringValue] withType:@"Unknown" enabled:[enabled boolValue]];
        }
        else
        {
            enabled = @NO;
        }
        
        TODO("decide how to handle setting a cheat type from the modal and save added cheats to file");
        [[sender representedObject] addObject:[@{
             @"code" : [alert stringValue],
             @"type" : @"Unknown",
             @"description" : [alert otherStringValue],
             @"enabled" : enabled,
         } mutableCopy]];
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
        [[sender representedObject] setObject:@NO forKey:@"enabled"];
        enabled = NO;
    }
    else {
        [[sender representedObject] setObject:@YES forKey:@"enabled"];
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

#pragma mark - Discs

- (BOOL)supportsMultipleDiscs
{
    return [[[_gameCoreManager plugin] controller] supportsMultipleDiscsForSystemIdentifier:[_systemPlugin systemIdentifier]];
}

- (IBAction)setDisc:(id)sender;
{
    [_gameCoreManager setDisc:[[sender representedObject] unsignedIntegerValue]];
}

#pragma mark - Saving States

- (BOOL)supportsSaveStates
{
    return ![[[_gameCoreManager plugin] controller] saveStatesNotSupportedForSystemIdentifier:[_systemPlugin systemIdentifier]];
}

- (BOOL)OE_pauseEmulationIfNeeded
{
    BOOL pauseNeeded = _emulationStatus == OEEmulationStatusPlaying;

    if(pauseNeeded) [self setEmulationPaused:YES];

    return pauseNeeded;
}

- (void)saveState
{
    [self saveState:nil];
}

- (void)saveState:(id)sender
{
    if(![self supportsSaveStates])
        return;
    
    BOOL didPauseEmulation = [self OE_pauseEmulationIfNeeded];

    NSInteger   saveGameNo    = [[self rom] saveStateCount] + 1;
    NSDate *date = [NSDate date];
	NSDateFormatter *formatter = [[NSDateFormatter alloc] init];
	formatter.timeZone = [NSTimeZone localTimeZone];
	formatter.dateFormat = @"yyyy-MM-dd HH:mm:ss ZZZ";
	
    NSString *format = NSLocalizedString(@"Save-Game-%ld %@", @"default save game name");
    NSString    *proposedName = [NSString stringWithFormat:format, saveGameNo, [formatter stringFromDate:date]];
    OEHUDAlert  *alert        = [OEHUDAlert saveGameAlertWithProposedName:proposedName];

    [alert setWindow:[[[self gameViewController] view] window]];

    if ([alert runModal] == NSAlertFirstButtonReturn) {
        [self OE_saveStateWithName:[alert stringValue] completionHandler:nil];
    }
    
    if(didPauseEmulation) [self setEmulationPaused:NO];
}

- (void)quickSave
{
    [self quickSave:nil];
}

- (void)quickSave:(id)sender;
{
    NSInteger slot = 0;
    if([sender respondsToSelector:@selector(representedObject)] && [[sender representedObject] respondsToSelector:@selector(intValue)])
        slot = [[sender representedObject] integerValue];
    else if([sender respondsToSelector:@selector(tag)])
        slot = [sender tag];

    NSString *name = [OEDBSaveState nameOfQuickSaveInSlot:slot];
    BOOL didPauseEmulation = [self OE_pauseEmulationIfNeeded];

    [self OE_saveStateWithName:name completionHandler:
     ^{
         if(didPauseEmulation) [self setEmulationPaused:NO];
         [[[self gameViewController] gameView] showQuickSaveNotification];
    }];
}

- (void)OE_saveStateWithName:(NSString *)stateName completionHandler:(void(^)(void))handler
{
    if(_emulationStatus <= OEEmulationStatusStarting || [self rom] == nil)
    {
        if(handler)
            handler();
        return;
    }

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

             NSString *coreIdentifier = [core bundleIdentifier];
             NSString *coreVersion = [core version];
             [state setCoreIdentifier:coreIdentifier];
             [state setCoreVersion:coreVersion];
         }

         if(state == nil)
         {
             NSManagedObjectContext *context = [[OELibraryDatabase defaultDatabase] mainThreadContext];
             state = [OEDBSaveState createSaveStateNamed:stateName forRom:[self rom] core:core withFile:temporaryStateFileURL inContext:context];
         }
         else
         {
             [state replaceStateFileWithFile:temporaryStateFileURL];
             [state setTimestamp:[NSDate date]];
         }

         [state save];
         NSManagedObjectContext *mainContext = [state managedObjectContext];
         [mainContext performBlock:^{
             [mainContext save:nil];
         }];

         NSData *TIFFData = [[[self gameViewController] takeNativeScreenshot] TIFFRepresentation];
         NSBitmapImageRep *bitmapImageRep = [NSBitmapImageRep imageRepWithData:TIFFData];

         NSUserDefaults *standardUserDefaults = [NSUserDefaults standardUserDefaults];
         NSBitmapImageFileType type = [standardUserDefaults integerForKey:OEScreenshotFileFormatKey];
         NSDictionary *properties = [standardUserDefaults dictionaryForKey:OEScreenshotPropertiesKey];
         NSData *convertedData = [bitmapImageRep representationUsingType:type properties:properties];

         __autoreleasing NSError *saveError = nil;
         if([state screenshotURL] == nil || ![convertedData writeToURL:[state screenshotURL] options:NSDataWritingAtomic error:&saveError])
             NSLog(@"Could not create screenshot at url: %@ with error: %@", [state screenshotURL], saveError);

         if(handler != nil) handler();
     }];
}

#pragma mark - Loading States

- (void)loadState
{
    FIXME("This replaces a call from OESystemResponder which used to pass self, but passing OESystemResponder would yield the same result in -loadState: so I do not know whether this ever worked in this case.");
    [self loadState:nil];
}

- (void)loadState:(id)sender;
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

- (void)quickLoad
{
    [self quickLoad:nil];
}

- (void)quickLoad:(id)sender;
{
    NSInteger slot = 0;
    if([sender respondsToSelector:@selector(representedObject)] && [[sender representedObject] respondsToSelector:@selector(intValue)])
        slot = [[sender representedObject] integerValue];
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
        [self->_gameCoreManager loadStateFromFileAtPath:[[state dataFileURL] path] completionHandler:
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
            if(error != nil)
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

    if([alert runModal] == NSAlertFirstButtonReturn)
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

    if([alert runModal] == NSAlertFirstButtonReturn) [state deleteAndRemoveFiles];
}

#pragma mark - OEGameViewControllerDelegate methods

- (void)gameViewController:(OEGameViewController *)sender didReceiveMouseEvent:(OEEvent *)event;
{
    [_gameCoreManager handleMouseEvent:event];
}

#pragma mark - OESystemBindingsObserver

- (void)systemBindings:(OESystemBindings *)sender didSetEvent:(OEHIDEvent *)event forBinding:(__kindof OEBindingDescription *)bindingDescription playerNumber:(NSUInteger)playerNumber
{
    [_gameCoreManager systemBindingsDidSetEvent:event forBinding:bindingDescription playerNumber:playerNumber];
}

- (void)systemBindings:(OESystemBindings *)sender didUnsetEvent:(OEHIDEvent *)event forBinding:(__kindof OEBindingDescription *)bindingDescription playerNumber:(NSUInteger)playerNumber
{
    [_gameCoreManager systemBindingsDidUnsetEvent:event forBinding:bindingDescription playerNumber:playerNumber];
}

#pragma mark - OEGameCoreOwner

- (void)toggleFullScreen
{
    [self toggleFullScreen:self];
}

- (void)takeScreenshot
{
    [self takeScreenshot:self];
}

- (void)volumeUp
{
    [self volumeUp:self];
}

- (void)volumeDown
{
    [self volumeDown:self];
}

- (void)stopEmulation
{
    [self stopEmulation:self];
}

- (void)resetEmulation
{
    [self resetEmulation:self];
}

- (void)toggleEmulationPaused
{
    [self toggleEmulationPaused:self];
}

- (void)fastForwardGameplay:(BOOL)enable
{
    if(_emulationStatus != OEEmulationStatusPlaying) return;

    [[[self gameViewController] gameView] showFastForwardNotification:enable];
}

- (void)rewindGameplay:(BOOL)enable
{
    if(_emulationStatus != OEEmulationStatusPlaying) return;

    [[[self gameViewController] gameView] showRewindNotification:enable];
}

- (void)stepGameplayFrameForward
{
    if(_emulationStatus == OEEmulationStatusPlaying)
        [self toggleEmulationPaused:self];

    if(_emulationStatus == OEEmulationStatusPaused)
        [[[self gameViewController] gameView] showStepForwardNotification];
}

- (void)stepGameplayFrameBackward
{
    if(_emulationStatus == OEEmulationStatusPlaying)
        [self toggleEmulationPaused:self];

    if(_emulationStatus == OEEmulationStatusPaused)
        [[[self gameViewController] gameView] showStepBackwardNotification];
}

- (void)setEnableVSync:(BOOL)enable
{
    [_gameViewController setEnableVSync:enable];
}

- (void)setAspectSize:(OEIntSize)newAspectSize
{
    [_gameViewController setAspectSize:newAspectSize];
}

- (void)setScreenSize:(OEIntSize)newScreenSize withIOSurfaceID:(IOSurfaceID)newSurfaceID
{
    [_gameViewController setScreenSize:newScreenSize withIOSurfaceID:newSurfaceID];
}

- (void)setScreenSize:(OEIntSize)newScreenSize aspectSize:(OEIntSize)newAspectSize withIOSurfaceID:(IOSurfaceID)newSurfaceID
{
    [_gameViewController setScreenSize:newScreenSize aspectSize:newAspectSize withIOSurfaceID:newSurfaceID];
}

- (void)setDiscCount:(NSUInteger)discCount
{
    [_gameViewController setDiscCount:discCount];
}

@end
