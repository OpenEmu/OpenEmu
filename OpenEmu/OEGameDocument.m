/*
 Copyright (c) 2020, OpenEmu Team

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

@import OpenEmuBase;
@import OpenEmuSystem;
@import OpenEmuKit;
#import "OEGameDocument.h"

#import "OEAudioDeviceManager.h"
#import "OECoreUpdater.h"
#import "OEDBRom.h"
#import "OEDBGame.h"
#import "OEDBSaveState.h"
#import "OEDBSystem.h"
#import "OELibraryDatabase.h"
#import "OEPopoutGameWindowController.h"
#import "OEDownload.h"
#import "OEROMImporter.h"
#import "OEDBScreenshot.h"

#import <objc/message.h>

#import "OpenEmu-Swift.h"

NSString *const OEGameCoreDisplayModeKeyFormat = @"displayMode.%@";

NSString *const OEScreenshotFileFormatKey = @"screenshotFormat";
NSString *const OEScreenshotPropertiesKey = @"screenshotProperties";

NSString *const OEGameCoreManagerModePreferenceKey = @"OEGameCoreManagerModePreference";
NSString *const OEGameDocumentErrorDomain = @"OEGameDocumentErrorDomain";

#define UDDefaultCoreMappingKeyPrefix   @"defaultCore"
#define UDSystemCoreMappingKeyForSystemIdentifier(_SYSTEM_IDENTIFIER_) [NSString stringWithFormat:@"%@.%@", UDDefaultCoreMappingKeyPrefix, _SYSTEM_IDENTIFIER_]

// Helper to call a method with this signature:
// - (void)document:(NSDocument *)doc shouldClose:(BOOL)shouldClose  contextInfo:(void  *)contextInfo
#define CAN_CLOSE_REPLY ((void(*)(id, SEL, NSDocument *, BOOL, void *))objc_msgSend)

@implementation OEGameDocument

+ (void)initialize
{
    if([self class] == [OEGameDocument class])
    {
        [[NSUserDefaults standardUserDefaults] registerDefaults:@{
                                                                  OEScreenshotFileFormatKey : @(NSBitmapImageFileTypePNG),
                                                                  OEScreenshotPropertiesKey : @{},
                                                                  }];
    }
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

//- (NSString *)coreIdentifier;
//- (NSString *)systemIdentifier;

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
            
            if([[OEAlert romDownloadRequiredAlert:name] runModal] == NSAlertFirstButtonReturn)
            {
                __block NSURL   *destination;
                __block NSError *error;
                
                OEAlert *alert = [[OEAlert alloc] init];
                alert.messageText = [NSString stringWithFormat:NSLocalizedString(@"Downloading %@…", @"Downloading rom message text"), name];
                alert.defaultButtonTitle = NSLocalizedString(@"Cancel", @"");
                alert.showsProgressbar = YES;
                alert.progress = -1;
                
                [alert performBlockInModalSession:^{
                    OEDownload *download = [[OEDownload alloc] initWithURL:sourceURL];
                    [download setProgressHandler:^BOOL(CGFloat progress) {
                        alert.progress = progress;
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
                             @{
                               NSLocalizedFailureReasonErrorKey : NSLocalizedString(@"The file you selected doesn't exist", @"Inexistent file error reason."),
                               NSLocalizedRecoverySuggestionErrorKey : NSLocalizedString(@"Choose a valid file.", @"Inexistent file error recovery suggestion.")
                               }];
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
            else if([error.domain isEqual:NSCocoaErrorDomain] && error.code == NSUserCancelledError)
            {
                blockError = error;
            }
        }];
        *outError = blockError;
        return NO;
    }
    
    _gameCoreManager = [self _newGameCoreManagerWithCorePlugin:_corePlugin];
    _gameViewController = [[OEGameViewController alloc] initWithDocument:self];
    
    return _gameCoreManager != nil;
}

//- (void)OE_setupGameCoreManagerUsingCorePlugin:(OECorePlugin *)core completionHandler:(void(^)(void))completionHandler

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
    NSDictionary <NSString *, id> *lastDisplayModeInfo = ([NSUserDefaults.standardUserDefaults objectForKey:[NSString stringWithFormat:OEGameCoreDisplayModeKeyFormat, _corePlugin.bundleIdentifier]]
                                ? : nil);
     // if file is in an archive append :entryIndex to path, so the core manager can figure out which entry to load
    if([[self rom] archiveFileIndex] != nil)
        path = [path stringByAppendingFormat:@":%d",[[[self rom] archiveFileIndex] intValue]];
    
    // Never extract arcade roms and .md roms (XADMaster identifies some as LZMA archives)
    NSString *extension = path.pathExtension.lowercaseString;
    if(![_systemPlugin.systemIdentifier isEqualToString:@"openemu.system.arcade"] && ![extension isEqualToString:@"md"] && ![extension isEqualToString:@"nds"] && ![extension isEqualToString:@"iso"]) {
        path = OEDecompressFileInArchiveAtPathWithHash(path, self.rom.md5, &_romDecompressed);
        _romPath = path;
    }
    
    OEShaderModel *shader = [OEShadersModel.shared shaderForSystem:self.systemIdentifier];
    NSDictionary<NSString *, NSNumber *> *params = [shader parametersForIdentifier:_systemPlugin.systemIdentifier];

    
    OEGameStartupInfo *info = [[OEGameStartupInfo alloc] initWithROMPath:path
                                                                  romMD5:self.rom.md5
                                                               romHeader:self.rom.header
                                                               romSerial:self.rom.serial
                                                            systemRegion:OELocalizationHelper.sharedHelper.regionName
                                                         displayModeInfo:lastDisplayModeInfo
                                                                  shader:shader.url
                                                        shaderParameters:params
                                                          corePluginPath:_corePlugin.path
                                                        systemPluginPath:_systemPlugin.path];

    return [[managerClass alloc] initWithStartupInfo:info corePlugin:_corePlugin systemPlugin:_systemPlugin gameCoreOwner:self];
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
    [self oe_deinit];
}

#pragma mark - Game Window

//- (void)setGameWindowController:(NSWindowController *)value
//- (void)OE_addObserversForWindowController:(NSWindowController *)windowController
//- (void)OE_removeObserversForWindowController:(NSWindowController *)windowController
//- (void)windowDidResignMain:(NSNotification *)notification
//- (void)windowDidBecomeMain:(NSNotification *)notification

#pragma mark - Device Notifications
//- (void)OE_addDeviceNotificationObservers
//- (void)OE_removeDeviceNotificationObservers
//- (void)OE_didReceiveLowBatteryWarningNotification:(NSNotification *)notification
//- (void)OE_deviceDidDisconnectNotification:(NSNotification *)notification

//- (void)showInSeparateWindowInFullScreen:(BOOL)fullScreen;

//- (NSString *)displayName

#pragma mark - Display Sleep Handling

//- (void)enableOSSleep
//- (void)disableOSSleep

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
                         @{
                           NSLocalizedFailureReasonErrorKey : NSLocalizedString(@"The file you selected doesn't exist", @"Inexistent file error reason."),
                           NSLocalizedRecoverySuggestionErrorKey : NSLocalizedString(@"Choose a valid file.", @"Inexistent file error recovery suggestion.")
                           }];
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
            
            OEAlert *alert = [[OEAlert alloc] init];
            
            NSString *fileName    = [[absoluteURL lastPathComponent] stringByDeletingPathExtension];
            NSString *messageText = [NSString stringWithFormat:NSLocalizedString(@"The game '%@' was imported.", @""), fileName];
            
            alert.messageText = NSLocalizedString(@"Your game finished importing, do you want to play it now?", @"");
            alert.informativeText = messageText;
            alert.defaultButtonTitle = NSLocalizedString(@"Play Game", @"");
            alert.alternateButtonTitle = NSLocalizedString(@"Cancel", @"");
            
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
    if(state != nil && [[OEAlert loadAutoSaveGameAlert] runModal] == NSAlertFirstButtonReturn)
        return [self OE_setupDocumentWithSaveState:state error:outError];
    else
        return [self OE_setupDocumentWithROM:[game defaultROM] usingCorePlugin:nil error:outError];
}

#pragma mark - Control Emulation

//- (void)setupGameWithCompletionHandler:(void(^)(BOOL success, NSError *error))handler;
//- (void)OE_startEmulation
//- (BOOL)isEmulationPaused
//- (void)setEmulationPaused:(BOOL)pauseEmulation
//- (void)setHandleEvents:(BOOL)handleEvents
//- (void)setHandleKeyboardEvents:(BOOL)handleKeyboardEvents
//- (void)switchCore:(id)sender;
//- (IBAction)editControls:(id)sender
//- (void)toggleFullScreen:(id)sender
//- (void)takeScreenshot:(id)sender
//- (void)OE_writeScreenshotImageData:(NSData *)imageData
//- (NSImage *)screenshot

#pragma mark - Volume

//- (IBAction)changeAudioOutputDevice:(id)sender
//- (float)volume
//- (void)setVolume:(float)volume asDefault:(BOOL)defaultFlag
//- (IBAction)changeVolume:(id)sender;
//- (IBAction)mute:(id)sender;
//- (IBAction)unmute:(id)sender;
//- (void)volumeUp:(id)sender;
//- (void)volumeDown:(id)sender;

#pragma mark - Controlling Emulation
//- (IBAction)performClose:(id)sender
//- (IBAction)stopEmulation:(id)sender;
//- (void)toggleEmulationPaused:(id)sender;
//- (void)resetEmulation:(id)sender;
//- (BOOL)shouldTerminateEmulation

//+ (BOOL)autosavesInPlace
//- (BOOL)isDocumentEdited

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

//- (BOOL)OE_checkRequiredFiles
//- (BOOL)OE_checkGlitches
//- (BOOL)OE_checkDeprecatedCore

#pragma mark - Cheats

//- (BOOL)supportsCheats
//- (IBAction)addCheat:(id)sender;
//- (IBAction)setCheat:(id)sender;
//- (IBAction)toggleCheat:(id)sender;
//- (void)setCheat:(NSString *)cheatCode withType:(NSString *)type enabled:(BOOL)enabled;

#pragma mark - Discs

//- (BOOL)supportsMultipleDiscs
//- (IBAction)setDisc:(id)sender;

#pragma mark - File Insertion

//- (BOOL)supportsFileInsertion
//- (IBAction)insertFile:(id)sender;

#pragma mark - Display Mode

//- (BOOL)supportsDisplayModeChange

- (void)changeDisplayMode:(id)sender
{
    BOOL fromMenu;
    NSDictionary <NSString *, id> *modeDict;
    if ([sender respondsToSelector:@selector(representedObject)])
    {
        fromMenu = YES;
        modeDict = [sender representedObject];
    }
    else if ([sender isKindOfClass:[NSDictionary class]])
    {
        fromMenu = NO;
        modeDict = sender;
    }
    else
        return;

    BOOL isSelected   = [modeDict[OEGameCoreDisplayModeStateKey] boolValue];
    BOOL isToggleable = [modeDict[OEGameCoreDisplayModeAllowsToggleKey] boolValue];
    BOOL isPrefSaveDisallowed = [modeDict[OEGameCoreDisplayModeDisallowPrefSaveKey] boolValue];
    BOOL isManual     = [modeDict[OEGameCoreDisplayModeManualOnlyKey] boolValue];

    // Mutually exclusive option is already selected, do nothing
    if (isSelected && !isToggleable)
        return;

    NSString *displayModeKeyForCore = [NSString stringWithFormat:OEGameCoreDisplayModeKeyFormat, _corePlugin.bundleIdentifier];
    NSString *prefKey  = modeDict[OEGameCoreDisplayModePrefKeyNameKey];
    NSString *prefVal  = modeDict[OEGameCoreDisplayModePrefValueNameKey];
    NSString *modeName = modeDict[OEGameCoreDisplayModeNameKey];
    NSUserDefaults *defaults = NSUserDefaults.standardUserDefaults;
    NSMutableDictionary <NSString *, id> *displayModeInfo = [NSMutableDictionary dictionary];

    // Copy existing prefs
    if ([defaults.dictionaryRepresentation[displayModeKeyForCore] isKindOfClass:[NSDictionary class]])
        displayModeInfo = [defaults.dictionaryRepresentation[displayModeKeyForCore] mutableCopy];

    // Mutually exclusive option is unselected
    if (!isToggleable)
    {
        displayModeInfo[prefKey] = prefVal ?: modeName;
        if (fromMenu && !isManual)
            _lastSelectedDisplayModeOption = modeName;
    }
    // Toggleable option, swap YES/NO
    else if (isToggleable)
        displayModeInfo[prefKey] = @(!isSelected);

    if (!isPrefSaveDisallowed)
        [defaults setObject:displayModeInfo forKey:displayModeKeyForCore];

    [_gameCoreManager changeDisplayWithMode:modeName];
}

- (void)OE_changeDisplayModeWithDirectionReversed:(BOOL)flag
{
    NSMutableArray <NSDictionary <NSString *, id> *> *availableOptions = [NSMutableArray array];
    NSString *mode;
    BOOL isToggleable, isSelected, isManual;

    for (NSDictionary *optionsDict in self.gameViewController.displayModes)
    {
        mode         = optionsDict[OEGameCoreDisplayModeNameKey];
        isToggleable = [optionsDict[OEGameCoreDisplayModeAllowsToggleKey] boolValue];
        isSelected   = [optionsDict[OEGameCoreDisplayModeStateKey] boolValue];
        isManual     = [optionsDict[OEGameCoreDisplayModeManualOnlyKey] boolValue];

        if (optionsDict[OEGameCoreDisplayModeSeparatorItemKey] || optionsDict[OEGameCoreDisplayModeLabelKey] || isManual)
        {
            continue;
        }
        else if (mode && !isToggleable)
        {
            [availableOptions addObject:optionsDict];

            // There may be multiple, but just take the first selected and start from the top
            if (_lastSelectedDisplayModeOption == nil && isSelected)
                _lastSelectedDisplayModeOption = mode;
        }
        else if (optionsDict[OEGameCoreDisplayModeGroupNameKey])
        {
            // Submenu Items
            for (NSDictionary *subOptionsDict in optionsDict[OEGameCoreDisplayModeGroupItemsKey])
            {
                mode         = subOptionsDict[OEGameCoreDisplayModeNameKey];
                isToggleable = [subOptionsDict[OEGameCoreDisplayModeAllowsToggleKey] boolValue];
                isSelected   = [subOptionsDict[OEGameCoreDisplayModeStateKey] boolValue];
                isManual     = [subOptionsDict[OEGameCoreDisplayModeManualOnlyKey] boolValue];

                if (subOptionsDict[OEGameCoreDisplayModeSeparatorItemKey] || subOptionsDict[OEGameCoreDisplayModeLabelKey] || isManual)
                {
                    continue;
                }
                else if (mode && !isToggleable)
                {
                    [availableOptions addObject:subOptionsDict];

                    if (_lastSelectedDisplayModeOption == nil && isSelected)
                        _lastSelectedDisplayModeOption = mode;
                }
            }

            continue;
        }
    }

    // Reverse
    if (flag)
        availableOptions = [[availableOptions reverseObjectEnumerator].allObjects mutableCopy];
        
    // If there are multiple mutually-exclusive groups of modes we want to enumerate
    // all the combinations.
    
    // List of pref keys used by each group of mutually exclusive modes
    NSMutableArray <NSString *> *prefKeys = [NSMutableArray array];
    // Index of the currently selected mode for each group
    NSMutableDictionary <NSString *, NSNumber *> *prefKeyToSelected = [NSMutableDictionary dictionary];
    // Indexes of the modes that are part of the same group
    NSMutableDictionary <NSString *, NSMutableIndexSet *> *prefKeyToOptions = [NSMutableDictionary dictionary];
    
    NSInteger i = 0;
    for (NSDictionary *optionsDict in availableOptions)
    {
        NSString *prefKey = optionsDict[OEGameCoreDisplayModePrefKeyNameKey];
        NSString *name = optionsDict[OEGameCoreDisplayModeNameKey];
        
        if ([name isEqual:_lastSelectedDisplayModeOption])
        {
            // Put the group of the last mode manually selected by the user in front of the list
            // This way the options of this group will be cycled through first
            [prefKeys removeObject:prefKey];
            [prefKeys insertObject:prefKey atIndex:0];
        }
        else if (![prefKeys containsObject:prefKey])
        {
            // Prioritize cycling all other modes in the order that they appear
            [prefKeys addObject:prefKey];
        }
            
        if ([optionsDict[OEGameCoreDisplayModeStateKey] isEqual:@(YES)])
            prefKeyToSelected[prefKey] = @(i);
        
        NSMutableIndexSet *optionsIdxes = prefKeyToOptions[prefKey];
        if (!optionsIdxes)
        {
            optionsIdxes = [NSMutableIndexSet indexSet];
            prefKeyToOptions[prefKey] = optionsIdxes;
        }
        [optionsIdxes addIndex:i];
        
        i++;
    }
    
    for (NSString *prefKey in prefKeys)
    {
        NSInteger current = [prefKeyToSelected[prefKey] integerValue];
        NSIndexSet *options = prefKeyToOptions[prefKey];
        
        BOOL carry = NO;
        NSInteger next = [options indexGreaterThanIndex:current];
        if (next == NSNotFound)
        {
            // Finished cycling through this mode; advance to the next one
            carry = YES;
            next = [options firstIndex];
        }
        
        NSDictionary *nextMode = [availableOptions objectAtIndex:next];
        [self changeDisplayMode:nextMode];
        
        if (!carry) break;
    }
}

//- (IBAction)nextDisplayMode:(id)sender
//- (IBAction)lastDisplayMode:(id)sender

#pragma mark - Saving States

//- (BOOL)supportsSaveStates
//- (BOOL)OE_pauseEmulationIfNeeded
//- (void)saveState:(id)sender
//- (void)quickSave:(id)sender;

- (void)OE_saveStateWithName:(NSString *)stateName completionHandler:(void(^)(void))handler
{
    if(![self supportsSaveStates])
    {
        if(handler)
            handler();
        return;
    }

    if(_emulationStatus <= OEEmulationStatusStarting || [self rom] == nil)
    {
        if(handler)
            handler();
        return;
    }
    
    NSString *temporaryDirectoryPath = NSTemporaryDirectory();
    NSURL    *temporaryDirectoryURL  = [NSURL fileURLWithPath:temporaryDirectoryPath];
    NSURL    *temporaryStateFileURL  = [NSURL URLWithString:NSUUID.UUID.UUIDString relativeToURL:temporaryDirectoryURL];
    OECorePlugin *core = [_gameCoreManager plugin];
    
    temporaryStateFileURL =
    [temporaryStateFileURL uniqueURLUsingBlock:
     ^ NSURL *(NSInteger triesCount)
     {
         return [NSURL URLWithString:NSUUID.UUID.UUIDString relativeToURL:temporaryDirectoryURL];
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
         
         NSUserDefaults *standardUserDefaults = [NSUserDefaults standardUserDefaults];
         NSBitmapImageFileType type = [standardUserDefaults integerForKey:OEScreenshotFileFormatKey];
         NSDictionary *properties = [standardUserDefaults dictionaryForKey:OEScreenshotPropertiesKey];
         
         [self->_gameCoreManager captureSourceImageWithCompletionHandler:^(NSBitmapImageRep *image) {
             NSSize newSize = self->_gameViewController.defaultScreenSize;
             image = [image resized:newSize];
             __block NSData *convertedData = [image representationUsingType:type properties:properties];
             dispatch_async(dispatch_get_main_queue(), ^{
                 __autoreleasing NSError *saveError = nil;
                 if([state screenshotURL] == nil || ![convertedData writeToURL:[state screenshotURL] options:NSDataWritingAtomic error:&saveError])
                     NSLog(@"Could not create screenshot at url: %@ with error: %@", [state screenshotURL], saveError);
                 
                 if(handler != nil) handler();
             });
         }];
     }];
}

#pragma mark - Loading States

//- (void)loadState:(id)sender;
//- (void)quickLoad:(id)sender;
//- (void)OE_loadState:(OEDBSaveState *)state

#pragma mark - Deleting States

//- (IBAction)deleteSaveState:(id)sender;

@end
