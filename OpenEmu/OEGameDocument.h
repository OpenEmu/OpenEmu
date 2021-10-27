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

@interface OEGameDocument : NSDocument

- (id)initWithRom:(OEDBRom *)rom core:(OECorePlugin *)core error:(NSError **)outError;
- (id)initWithGame:(OEDBGame *)game core:(OECorePlugin *)core error:(NSError **)outError;
- (id)initWithSaveState:(OEDBSaveState *)state error:(NSError **)outError;

@property(/*readonly*/) OEDBRom *rom;
@property(/*readonly*/) NSURL   *romFileURL;
@property(/*readonly*/) OECorePlugin *corePlugin;
@property(/*readonly*/) OESystemPlugin *systemPlugin;

@property(/*readonly*/) OEGameViewController *gameViewController;

@property(nonatomic) NSWindowController *_gameWindowController;

@property(nonatomic) BOOL _handleEvents;
@property(nonatomic) BOOL _handleKeyboardEvents;

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

// former ivars
@property OEGameCoreManager  *gameCoreManager;

@property IOPMAssertionID     displaySleepAssertionID;

@property OEEmulationStatus   emulationStatus;
@property OEDBSaveState      *saveStateForGameStart;
@property NSDate             *lastPlayStartDate;
@property NSString           *lastSelectedDisplayModeOption;
@property BOOL                isMuted;
@property BOOL                pausedByGoingToBackground;
@property BOOL                coreDidTerminateSuddenly;
/// Indicates whether the document is currently moving from the main window into a separate popout window.
@property BOOL                isUndocking;

// track if ROM was decompressed
@property NSString           *romPath;
@property BOOL                romDecompressed;

@end
