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

@class GameDocument;

@interface GameCore : NSObject
{
	NSThread *emulationThread;
	GameDocument *document;
	NSTimeInterval frameInterval;
}

+ (NSTimeInterval)defaultTimeInterval;
+ (void)setDefaultTimeInterval:(NSTimeInterval)aTimeInterval;

@property(assign) GameDocument *document;

- (id)initWithDocument:(GameDocument *)document;

#pragma mark Execution
- (void)frameRefreshThread:(id)anArgument;
- (void)setPauseEmulation:(BOOL)flag;
- (void)setupEmulation;
- (void)stopEmulation;
- (void)startEmulation;
- (void)resetEmulation;
- (void)frameRefreshThread:(id)anArgument;
- (void)executeFrame;
- (void)refreshFrame;

- (BOOL)loadFileAtPath:(NSString*)path;

@property NSTimeInterval frameInterval;

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

@end

#pragma mark Optional
@interface GameCore (OptionalMethods)
- (void)saveStateToFileAtPath:(NSString *)fileName;
- (void)loadStateFromFileAtPath:(NSString *)fileName;

- (void) requestAudio: (int) frames inBuffer: (void*)buf;
- (NSSize) outputSize;
- (void) setRandomByte;
@end

