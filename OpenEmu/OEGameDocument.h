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
#import "OEGameViewController.h"

extern NSString * const OEScreenshotAspectRatioCorrectionDisabled;

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
@class OEEvent;

@interface OEGameDocument : NSDocument <OEGameViewControllerDelegate>

- (id)initWithRom:(OEDBRom *)rom core:(OECorePlugin *)core error:(NSError **)outError;
- (id)initWithGame:(OEDBGame *)game core:(OECorePlugin *)core error:(NSError **)outError;
- (id)initWithSaveState:(OEDBSaveState *)state error:(NSError **)outError;

- (void)setupGameWithCompletionHandler:(void(^)(BOOL success, NSError *error))handler;
- (void)showInSeparateWindowInFullScreen:(BOOL)fullScreen;

@property(readonly) OEDBRom *rom;
@property(readonly) NSURL   *romFileURL;
@property(readonly) OECorePlugin *corePlugin;
@property(readonly) OESystemPlugin *systemPlugin;

@property(readonly) OEGameViewController *gameViewController;

@property(readonly) NSString *coreIdentifier;
@property(readonly) NSString *systemIdentifier;

@property(nonatomic) NSWindowController *gameWindowController;

@property(getter=isEmulationPaused) BOOL emulationPaused;

@property(nonatomic) BOOL handleEvents;
@property(nonatomic) BOOL handleKeyboardEvents;

#pragma mark - Actions
- (IBAction)editControls:(id)sender;

#pragma mark - Volume
@property (readonly) float volume;
- (IBAction)changeAudioOutputDevice:(id)sender;
- (IBAction)changeVolume:(id)sender;
- (IBAction)mute:(id)sender;
- (IBAction)unmute:(id)sender;
- (void)volumeDown:(id)sender;
- (void)volumeUp:(id)sender;

#pragma mark - Controlling Emulation
- (void)switchCore:(id)sender;
- (void)toggleEmulationPaused:(id)sender;
- (void)resetEmulation:(id)sender;
- (IBAction)stopEmulation:(id)sender;
- (IBAction)takeScreenshot:(id)sender;

/*! Return a filtered screenshot of the currently running core */
- (NSImage *)screenshot;

#pragma mark - Cheats
- (IBAction)addCheat:(id)sender;
- (IBAction)setCheat:(id)sender;
- (IBAction)toggleCheat:(id)sender;
- (BOOL)supportsCheats;
- (void)setCheat:(NSString *)cheatCode withType:(NSString *)type enabled:(BOOL)enabled;

#pragma mark - Discs
- (IBAction)setDisc:(id)sender;
- (BOOL)supportsMultipleDiscs;

#pragma mark - File Insertion
- (IBAction)insertFile:(id)sender;
- (BOOL)supportsFileInsertion;

#pragma mark - Display Mode
- (void)changeDisplayMode:(id)sender;
- (BOOL)supportsDisplayModeChange;
- (IBAction)nextDisplayMode:(id)sender;
- (IBAction)lastDisplayMode:(id)sender;

#pragma mark - Saving States
- (BOOL)supportsSaveStates;
- (void)quickSave:(id)sender;
- (void)quickLoad:(id)sender;

#pragma mark - Deleting States
- (IBAction)deleteSaveState:(id)sender;

#pragma mark - Full Screen
- (void)toggleFullScreen:(id)sender;

#pragma mark - OEGameViewController Methods
- (void)setOutputBounds:(NSRect)bounds;
- (void)gameViewController:(OEGameViewController *)sender didReceiveMouseEvent:(OEEvent *)event;
- (void)gameViewController:(OEGameViewController *)sender updateBounds:(CGRect)newBounds;
- (void)gameViewController:(OEGameViewController *)sender setShaderURL:(NSURL *)url completionHandler:(void (^)(BOOL success, NSError *error))block;
- (void)gameViewController:(OEGameViewController *)sender shaderParamGroupsWithCompletionHandler:(void (^)(NSArray<OEShaderParamGroupValue *> *))handler;
- (void)gameViewController:(OEGameViewController *)sender setShaderParameterValue:(CGFloat)value atIndex:(NSUInteger)index atGroupIndex:(NSUInteger)group;

@end
