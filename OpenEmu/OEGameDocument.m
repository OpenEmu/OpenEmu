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
//#define CAN_CLOSE_REPLY ((void(*)(id, SEL, NSDocument *, BOOL, void *))objc_msgSend)

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

//- (NSString *)description

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
//- (OEGameCoreManager *)_newGameCoreManagerWithCorePlugin:(OECorePlugin *)corePlugin
//- (OECorePlugin *)OE_coreForSystem:(OESystemPlugin *)system error:(NSError **)outError

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

//- (NSData *)dataOfType:(NSString *)typeName error:(NSError **)outError

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

//- (void)canCloseDocumentWithDelegate:(id)delegate shouldCloseSelector:(SEL)shouldCloseSelector contextInfo:(void *)contextInfo

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
//- (void)changeDisplayMode:(id)sender
//- (void)OE_changeDisplayModeWithDirectionReversed:(BOOL)flag
//- (IBAction)nextDisplayMode:(id)sender
//- (IBAction)lastDisplayMode:(id)sender

#pragma mark - Saving States

//- (BOOL)supportsSaveStates
//- (BOOL)OE_pauseEmulationIfNeeded
//- (void)saveState:(id)sender
//- (void)quickSave:(id)sender;
//- (void)OE_saveStateWithName:(NSString *)stateName completionHandler:(void(^)(void))handler

#pragma mark - Loading States

//- (void)loadState:(id)sender;
//- (void)quickLoad:(id)sender;
//- (void)OE_loadState:(OEDBSaveState *)state

#pragma mark - Deleting States

//- (IBAction)deleteSaveState:(id)sender;

@end
