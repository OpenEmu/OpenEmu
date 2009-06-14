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
#import "OEMap.h"
#import "OEGameCoreController.h"

enum {
    OEButton_Up = 1,
    OEButton_Down,
    OEButton_Left,
    OEButton_Right,
    OEButton_Start,
    OEButton_Select,
    OEButton_1 = 11,
    OEButton_2,
    OEButton_3,
    OEButton_4,
    OEButton_5,
    OEButton_6,
    OEButton_7,
    OEButton_8,
    OEButton_9,
    OEButton_10,
    OEButton_11,
    OEButton_12,
    OEButton_13,
    OEButton_14,
    OEButton_15,
    OEButton_16,
    OEButton_17,
    OEButton_18,
    OEButton_19,
    OEButton_20 // = 30
};
typedef NSInteger OEButton;

@class GameDocument, OEHIDEvent, OERingBuffer;

@interface GameCore : NSResponder
{
	NSThread              *emulationThread;
	GameDocument          *document;
	NSTimeInterval         frameInterval;
    OEGameCoreController  *owner;
    OEMapRef               keyMap;
    OERingBuffer         **ringBuffers;
}

+ (NSTimeInterval)defaultTimeInterval;
+ (void)setDefaultTimeInterval:(NSTimeInterval)aTimeInterval;

@property(assign) OEGameCoreController *owner;
@property(assign) GameDocument *document;
@property NSTimeInterval frameInterval;

- (void)getAudioBuffer:(void *)buffer frameCount:(NSUInteger)frameCount bufferIndex:(NSUInteger)index;
- (OERingBuffer *)ringBufferAtIndex:(NSUInteger)index;

- (id)initWithDocument:(GameDocument *)document;
- (void)removeFromGameController;

#pragma mark Execution
- (void)frameRefreshThread:(id)anArgument;
- (void)setPauseEmulation:(BOOL)flag;
- (void)setupEmulation;
- (void)stopEmulation;
- (void)startEmulation;

#pragma mark Tracking preference changes
- (void)setEventValue:(NSInteger)appKey forEmulatorKey:(OEEmulatorKey)emulKey;
- (void)unsetEventForKey:(NSString *)keyName withValueMask:(NSUInteger)keyMask;
- (void)settingWasSet:(id)aValue forKey:(NSString *)keyName;
- (void)keyboardEventWasSet:(id)theEvent forKey:(NSString *)keyName;
- (void)keyboardEventWasRemovedForKey:(NSString *)keyName;

- (void)HIDEventWasSet:(id)theEvent forKey:(NSString *)keyName;
- (void)HIDEventWasRemovedForKey:(NSString *)keyName;

// ============================================================================
// Abstract methods: Those methods should be overridden by subclasses
// ============================================================================
- (void)resetEmulation;
- (void)executeFrame;
- (void)refreshFrame;

- (BOOL)loadFileAtPath:(NSString *)path;

#pragma mark Video
@property(readonly) NSUInteger  width;
@property(readonly) NSUInteger  height;
@property(readonly) const void *videoBuffer;
@property(readonly) GLenum      pixelFormat;
@property(readonly) GLenum      pixelType;
@property(readonly) GLenum      internalPixelFormat;

#pragma mark Audio
@property(readonly) NSUInteger  soundBufferCount; // overriding it is optional, should be constant
@property(readonly) const void *soundBuffer;
@property(readonly) NSUInteger  channelCount;
@property(readonly) NSUInteger  frameSampleCount;
@property(readonly) NSUInteger  soundBufferSize;
@property(readonly) NSUInteger  frameSampleRate;

#pragma mark Input Settings & Parsing
- (OEEmulatorKey)emulatorKeyForKey:(NSString *)aKey index:(NSUInteger)index player:(NSUInteger)thePlayer;
- (OEEmulatorKey)emulatorKeyForKeyIndex:(NSUInteger)index player:(NSUInteger)thePlayer;
- (void)pressEmulatorKey:(OEEmulatorKey)aKey;
- (void)releaseEmulatorKey:(OEEmulatorKey)aKey;

#pragma mark Input
- (void)player:(NSUInteger)thePlayer didPressButton:(OEButton)gameButton;
- (void)player:(NSUInteger)thePlayer didReleaseButton:(OEButton)gameButton;

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

- (NSSize) outputSize;
- (void) setRandomByte;
@end
