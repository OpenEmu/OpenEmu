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

@import Cocoa;
@import IOKit.pwr_mgt;

extern NSString *const OEGameVolumeKey;
extern NSString *const OEGameCoreDisplayModeKeyFormat;
extern NSString *const OEBackgroundPauseKey;
extern NSString *const OEBackgroundControllerPlayKey;
extern NSString *const OETakeNativeScreenshots;

extern NSString *const OEScreenshotFileFormatKey;
extern NSString *const OEScreenshotPropertiesKey;
extern NSString *const OEScreenshotAspectRatioCorrectionDisabled;

extern NSString *const OEGameCoreManagerModePreferenceKey;
extern NSString *const OEGameDocumentErrorDomain;

typedef NS_ERROR_ENUM(OEGameDocumentErrorDomain, OEGameDocumentErrorCodes)
{
    OENoError                      =  0,
    OEFileDoesNotExistError        = -1,
    OENoCoreError                  = -2,
    OENoCoreForSystemError         = -7,
    OENoCoreForSaveStateError      = -8,
    OEImportRequiredError          = -9,
    OECouldNotLoadROMError         = -10,
    OEGameCoreCrashedError         = -11
};

@class OECorePlugin;
@class OEDBGame;
@class OEDBRom;
@class OEDBSaveState;
@class OEGameViewController;
@class OESystemPlugin;
@class OEGameCoreManager;
@protocol OEGameCoreHelper;

@interface OEGameDocument : NSDocument

- (id)initWithRom:(OEDBRom *)rom core:(OECorePlugin *)core error:(NSError **)outError;
- (id)initWithGame:(OEDBGame *)game core:(OECorePlugin *)core error:(NSError **)outError;
- (id)initWithSaveState:(OEDBSaveState *)state error:(NSError **)outError;

- (void)setupGameWithCompletionHandler:(void(^)(BOOL success, NSError *error))handler;

@property(readonly) OEDBRom *rom;
@property(readonly) NSURL   *romFileURL;
@property(readonly) OECorePlugin *corePlugin;
@property(readonly) OESystemPlugin *systemPlugin;

@property(readonly) OEGameViewController *gameViewController;

@property(nonatomic) NSWindowController *gameWindowController;

@property(nonatomic) BOOL handleEvents;
@property(nonatomic) BOOL handleKeyboardEvents;

#pragma mark - Controlling Emulation
- (IBAction)takeScreenshot:(id)sender;

#pragma mark - File Insertion
- (IBAction)insertFile:(id)sender;

#pragma mark - Display Mode
- (void)changeDisplayMode:(id)sender;

#pragma mark - Private

typedef NS_ENUM(NSUInteger, OEEmulationStatus)
{
    /// The current OEGameCoreManager has not been instantiated yet,
    /// or it has been deallocated because emulation has terminated
    OEEmulationStatusNotSetup,
    /// The OEGameCoreManager is ready, but the emulation was not started for
    /// the first time yet
    OEEmulationStatusSetup,
    /// The emulation has been requested to start
    OEEmulationStatusStarting,
    ///
    OEEmulationStatusPlaying,
    ///
    OEEmulationStatusPaused,
    /// After emulation stops, but before OEGameCoreManager is deallocated
    OEEmulationStatusTerminating,
};

- (void)OE_setupGameCoreManagerUsingCorePlugin:(OECorePlugin *)core completionHandler:(void(^)(void))completionHandler NS_SWIFT_NAME(setupGameCoreManager(using:completionHandler:));
- (void)OE_changeDisplayModeWithDirectionReversed:(BOOL)flag NS_SWIFT_NAME(changeDisplayMode(directionReversed:));
- (void)OE_saveStateWithName:(NSString *)stateName completionHandler:(void(^)(void))handler NS_SWIFT_NAME(saveState(name:completionHandler:));
- (void)OE_loadState:(OEDBSaveState *)state NS_SWIFT_NAME(loadState(state:));

@property OEEmulationStatus  emulationStatus;
@property OEGameCoreManager *gameCoreManager;
@property IOPMAssertionID    displaySleepAssertionID;
@property(nullable) NSDate  *lastPlayStartDate;
@property BOOL               isMuted;
@property BOOL               pausedByGoingToBackground;
@property BOOL               coreDidTerminateSuddenly;
@property BOOL               isUndocking;

@end
