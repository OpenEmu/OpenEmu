//
//  OpenEmuQCNESPlugIn.h
//  OpenEmuQCNES
//
//  A NES-only QC plugin for teh glitchy insanity.  Started by Dan Winckler on 11/16/08.
//
#import <Cocoa/Cocoa.h>

@protocol GameCore;
@class NESGameEmu, GameAudio, GameBuffer;

@interface OpenEmuQCNES : QCPlugIn
{
	// init stuff
	NSBundle* bundle;
	id <GameCore> gameCore;
	NESGameEmu* nesEmu;
	GameAudio*  gameAudio;
	GameBuffer* gameBuffer;
	BOOL loadedRom;
	BOOL romFinishedLoading;
	BOOL hasNmtRam;
	BOOL hasChrRam;
	NSMutableArray* persistantControllerData;
	NSRecursiveLock * gameLock;
}

/*
Declare here the Obj-C 2.0 properties to be used as input and output ports for the plug-in e.g.
You can access their values in the appropriate plug-in methods using self.inputFoo or self.inputBar
*/

@property (assign) NSString* inputRom;
@property (assign) NSArray * inputControllerData;
@property (assign) double inputVolume;
@property (assign) NSString* inputSaveStatePath;
@property (assign) NSString* inputLoadStatePath;
@property (assign) BOOL inputPauseEmulation;
@property (assign) NSString* inputCheatCode;

@property (assign) BOOL inputEnableRewinder;
@property (assign) NSUInteger inputRewinderDirection;
@property (assign) BOOL inputEnableRewinderBackwardsSound;
@property (assign) BOOL inputRewinderReset;

@property (assign) BOOL inputNmtRamCorrupt;
@property (assign) double inputNmtRamOffset;
@property (assign) double inputNmtRamValue;

@property (assign) BOOL inputChrRamCorrupt;
@property (assign) double inputChrRamOffset;
@property (assign) double inputChrRamValue;

@property (assign) id <QCPlugInOutputImageProvider>outputImage;

@end

@interface OpenEmuQCNES (Execution)
- (BOOL) controllerDataValidate:(NSArray*) cData;
- (void) handleControllerData;
- (void) refresh;
- (void) loadRom: (NSString*) romPath;
- (void) saveState: (NSString *) fileName;
- (BOOL) loadState: (NSString *) fileName;
- (void) setCode:(NSString*)cheatCode;
- (void) enableRewinder:(BOOL) rewind;

@end

