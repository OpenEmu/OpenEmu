/*
 *  GameCore.h
 *  OpenEmu
 *
 *  Created by Josh Weinberg on 9/1/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#define DEBUG_PRINT

#ifdef DEBUG_PRINT
#define DLog(format, ...) NSLog((format), ##__VA_ARGS__)
#else
#define DLog(format, ...) 1
#endif

#import <Cocoa/Cocoa.h>

#import "OEGameCoreController.h"

@class GameDocument, OEHIDEvent;

@interface GameCore : NSResponder
{
	NSThread *emulationThread;
	GameDocument *document;
	NSTimeInterval frameInterval;
    OEGameCoreController *owner;
}

+ (NSTimeInterval)defaultTimeInterval;
+ (void)setDefaultTimeInterval:(NSTimeInterval)aTimeInterval;

@property(assign) OEGameCoreController *owner;
@property(assign) GameDocument *document;
@property NSTimeInterval frameInterval;

- (id)initWithDocument:(GameDocument *)document;

#pragma mark Execution
- (void)frameRefreshThread:(id)anArgument;
- (void)setPauseEmulation:(BOOL)flag;
- (void)setupEmulation;
- (void)stopEmulation;
- (void)startEmulation;

// ============================================================================
// Abstract methods: Those methods should be overridden by subclasses
// ============================================================================
- (void)resetEmulation;
- (void)executeFrame;
- (void)refreshFrame;

- (BOOL)loadFileAtPath:(NSString *)path;

#pragma mark Video
@property(readonly) NSInteger width;
@property(readonly) NSInteger height;
@property(readonly) const unsigned char *videoBuffer;
@property(readonly) GLenum pixelFormat;
@property(readonly) GLenum pixelType;
@property(readonly) GLenum internalPixelFormat;

#pragma mark Audio
@property(readonly) const UInt16 *soundBuffer;
@property(readonly) NSInteger channelCount;
@property(readonly) NSInteger frameSampleCount;
@property(readonly) NSInteger soundBufferSize;
@property(readonly) NSInteger frameSampleRate;

#pragma mark Input
- (void)player:(NSInteger)thePlayer didPressButton:(NSInteger)gameButton;
- (void)player:(NSInteger)thePlayer didReleaseButton:(NSInteger)gameButton;

#pragma mark Keyboard events
- (void)keyDown:(NSEvent *)theEvent;
- (void)keyUp:(NSEvent *)theEvent;

// FIXME: Find a better way.
#pragma mark Tracking preference changes
- (void)globalEventWasSet:(id)theEvent forKey:(NSString *)keyName;
- (void)settingWasSet:(id)aValue forKey:(NSString *)keyName;
- (void)eventWasSet:(id)theEvent forKey:(NSString *)keyName inNamespace:(OEEventNamespace)aNamespace;
- (void)eventWasRemovedForKey:(NSString *)keyName inNamespace:(OEEventNamespace)aNamespace;
- (void)removeFromGameController;

// ============================================================================
// End Abstract methods.
// ============================================================================

@end

#pragma mark Optional
@interface GameCore (OptionalMethods)
- (void)saveStateToFileAtPath:(NSString *)fileName;
- (void)loadStateFromFileAtPath:(NSString *)fileName;

- (IBAction)pauseEmulation:(id)sender;

- (NSTrackingAreaOptions)mouseTrackingOptions;

- (void) requestAudio: (int) frames inBuffer: (void*)buf;
- (NSSize) outputSize;
- (void) setRandomByte;
@end

#pragma mark Gamepad events
@interface GameCore (OEEventHandler)

- (void)handleHIDEvent:(OEHIDEvent *)anEvent;

@end
