//
//  OpenEmuQCPlugIn.h
//  OpenEmuQC
//
//  Started by Dan Winckler on 4/11/08 with soooooooooooooooo much help from vade and daemoncollector.
//

#import <Quartz/Quartz.h>
#import <OpenGL/OpenGL.h>
#import <OpenGL/gl.h>
#import <Cocoa/Cocoa.h>
#import <AudioToolbox/AudioToolbox.h>

@class GameCore;
@class OEGameCoreController;
@class GameAudio, GameBuffer;

@interface OpenEmuQC : QCPlugIn
{
	// init stuff
	NSArray              *bundles;
	NSArray              *validExtensions;
	OEGameCoreController *gameCoreController;
	GameCore             *gameCore;
	GameAudio            *gameAudio;
	GameBuffer           *gameBuffer;

	BOOL loadedRom, audioPaused;
	NSMutableArray* persistantControllerData;
	NSRecursiveLock * gameLock;
}

/*
Declare here the Obj-C 2.0 properties to be used as input and output ports for the plug-in e.g.
You can access their values in the appropriate plug-in methods using self.inputFoo or self.inputBar
*/

@property(assign) NSString* inputRom;
@property (assign) NSArray * inputControllerData;
@property (assign) double inputVolume;
@property (assign) BOOL inputPauseEmulation;
@property (assign) NSString* inputSaveStatePath;
@property (assign) NSString* inputLoadStatePath;
@property (assign) id <QCPlugInOutputImageProvider>outputImage;

@end

@interface OpenEmuQC (Execution)
- (BOOL) controllerDataValidate:(NSArray*) cData;
- (void) handleControllerData;
- (void) refresh;
- (NSBundle*)bundleForType:(NSString*) type;
- (BOOL) loadRom: (NSString*) romPath;
- (void) saveState: (NSString *) fileName;
- (BOOL) loadState: (NSString *) fileName;

@end

