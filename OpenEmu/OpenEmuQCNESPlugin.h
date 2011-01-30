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

#import <Quartz/Quartz.h>

// protocol
#import "OEGameCoreHelper.h"
#import "OEGameCoreManager.h"

#import "GameCore.h"

#pragma mark Nestopia specific extensions
@interface GameCore (NesAdditions)

- (BOOL)isUnlimitedSpritesEnabled;
- (int)brightness;
- (int)saturation;
- (int)contrast;
- (int)sharpness;
- (int)colorRes;
- (int)colorBleed;
- (int)colorArtifacts;
- (int)colorFringing;
- (int)hue;

- (void)applyNTSC:(id)sender;
- (BOOL)isNTSCEnabled;

- (void)toggleUnlimitedSprites:(id)sender;
- (void)enableUnlimitedSprites:(BOOL)enable;
- (void)setCode:(NSString *)code;
- (void)enableRewinder:(BOOL)rewind;
- (BOOL)isRewinderEnabled;
- (void)rewinderDirection:(NSUInteger)rewinderDirection;
- (void)enableRewinderBackwardsSound:(BOOL)rewindSound;
- (BOOL)isRewinderBackwardsSoundEnabled;

- (int)cartVRamSize;
- (int)chrRomSize;
//- (void)setRandomNmtRamByte;
//- (void)setRandomChrRamByte;

- (void)setNMTRamByTable:(NSNumber *)table array:(NSArray *)nmtValueArray;
- (void)setNmtRamBytes:(double)off value:(double)val;
- (void)setChrRamBytes:(double)off value:(double)val;

//movie methods
- (void)recordMovie:(NSString *)moviePath mode:(BOOL)append;
- (void)playMovie:(NSString *)moviePath;
- (void)stopMovie;
- (BOOL)isMovieRecording;
- (BOOL)isMoviePlaying;
- (BOOL)isMovieStopped;

@end


@interface OpenEmuQCNES : QCPlugIn
{
    OEGameCoreProcessManager *gameCoreManager;
    BOOL                      debugMode;
    
    // Controller data
    NSArray                  *persistantControllerData;
    NSArray                  *persistantNameTableData;
}
@property(readwrite, retain) NSArray *persistantControllerData;
@property(readwrite, retain) NSArray *persistantNameTableData;
@property(readwrite)         BOOL     debugMode;

#ifdef DEBUG_PRINT
@property(assign) BOOL       inputEnableDebugMode;
#endif

@property(assign) NSString   *inputRom;
@property(assign) NSArray    *inputControllerData;
@property(assign) double      inputVolume;
@property(assign) NSString   *inputSaveStatePath;
@property(assign) NSString   *inputLoadStatePath;
@property(assign) BOOL        inputPauseEmulation;
@property(assign) NSString   *inputCheatCode;

@property(assign) BOOL        inputEnableRewinder;
@property(assign) NSUInteger  inputRewinderDirection;
@property(assign) BOOL        inputEnableRewinderBackwardsSound;
//@property(assign) BOOL        inputRewinderReset;

@property(assign) BOOL        inputNmtRamCorrupt;
@property(assign) double      inputNmtRamOffset;
@property(assign) double      inputNmtRamValue;

@property(assign) BOOL        inputCorruptNameTable;
@property(assign) NSArray    *inputNameTableData;

@property(assign) BOOL        inputChrRamCorrupt;
@property(assign) double      inputChrRamOffset;
@property(assign) double      inputChrRamValue;

@property(assign) id<QCPlugInOutputImageProvider> outputImage;

@end

@interface OpenEmuQCNES (Execution)
- (void)terminateEmulation;
- (BOOL)controllerDataValidate:(NSArray *)cData;
- (void)handleControllerData;
- (BOOL)readFromURL:(NSURL *)absoluteURL;

- (BOOL)validateNameTableData:(NSArray *)nameTableData;
- (void)enableDebugMode:(BOOL)flag;
@end

