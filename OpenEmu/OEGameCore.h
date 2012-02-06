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

#import <Cocoa/Cocoa.h>
#import "OEGameCoreController.h"
#import "OESystemResponderClient.h"

#ifndef DLog

#ifdef DEBUG_PRINT
#define DLog(format, ...) NSLog(@"%s: " format, __FUNCTION__, ##__VA_ARGS__)
#else
#define DLog(format, ...) do {} while(0)
#endif

#endif

#pragma mark -
@protocol OERenderDelegate

@required
- (void) willExecute;
- (void) didExecute;
@end

#pragma mark -

typedef struct OEIntPoint {
    int x;
    int y;
} OEIntPoint;

typedef struct OEIntSize {
    int width;
    int height;
} OEIntSize;

typedef struct OEIntRect {
    OEIntPoint origin;
    OEIntSize size;
} OEIntRect;

static inline OEIntSize OESizeMake(int width, int height)
{
    return (OEIntSize){width, height};
}

static inline OEIntRect OERectMake(int x, int y, int width, int height)
{
    return (OEIntRect){(OEIntPoint){x,y}, (OEIntSize){width, height}};
}

static inline NSSize NSSizeFromOEIntSize(OEIntSize size)
{
    return NSMakeSize(size.width, size.height);
}
@class OEHIDEvent, OERingBuffer;

#pragma mark -

@interface OEGameCore : NSResponder <OESystemResponderClient, OESettingObserver>
{
    id<OERenderDelegate> renderDelegate;
    
    NSThread              *emulationThread;
    NSTimeInterval         frameInterval;
    OEGameCoreController  *owner;
    __strong OERingBuffer         **ringBuffers;
    //OEMapRef               keyMap;
    NSUInteger             frameSkip;
    NSUInteger             frameCounter;
    NSUInteger             tenFrameCounter;
    NSUInteger             autoFrameSkipLastTime;
    NSUInteger             frameskipadjust;
    
    // for lightgun/pointer support.
    NSPoint                mousePosition;
    
    BOOL                   frameFinished;
    BOOL                   willSkipFrame;
    
    BOOL                   isRunning;
    BOOL                   shouldStop;
}

+ (NSTimeInterval)defaultTimeInterval;
+ (void)setDefaultTimeInterval:(NSTimeInterval)aTimeInterval;

@property (readwrite, retain) id<OERenderDelegate> renderDelegate;

@property(assign)   OEGameCoreController *owner;
@property(readonly) NSString             *pluginName;
@property(readonly) NSString             *gameSystemName DEPRECATED_ATTRIBUTE;

@property(readonly) NSString             *supportDirectoryPath;
@property(readonly) NSString             *batterySavesDirectoryPath;

@property           NSTimeInterval        frameInterval;
@property           BOOL                  frameFinished;

- (void)getAudioBuffer:(void *)buffer frameCount:(NSUInteger)frameCount bufferIndex:(NSUInteger)index;
- (OERingBuffer *)ringBufferAtIndex:(NSUInteger)index;

- (void)calculateFrameSkip:(NSUInteger)rate;

#pragma mark -
#pragma mark Execution
@property(getter=isEmulationPaused) BOOL pauseEmulation;
- (BOOL)rendersToOpenGL;
- (void)frameRefreshThread:(id)anArgument;
- (void)setupEmulation;
- (void)stopEmulation;
- (void)startEmulation;

#pragma mark -
#pragma mark Tracking preference changes
- (void)settingWasSet:(id)aValue forKey:(NSString *)keyName;
/*
- (void)setEventValue:(NSInteger)appKey forEmulatorKey:(OEEmulatorKey)emulKey;
- (void)unsetEventForKey:(NSString *)keyName withValueMask:(NSUInteger)keyMask;
- (void)keyboardEventWasSet:(id)theEvent forKey:(NSString *)keyName;
- (void)keyboardEventWasRemovedForKey:(NSString *)keyName;

- (void)HIDEventWasSet:(id)theEvent forKey:(NSString *)keyName;
- (void)HIDEventWasRemovedForKey:(NSString *)keyName;
 */
// ============================================================================
// Abstract methods: Those methods should be overridden by subclasses
// ============================================================================
- (void)resetEmulation;
- (void)executeFrame;
- (void)executeFrameSkippingFrame:(BOOL) skip;

- (BOOL)loadFileAtPath:(NSString *)path;

#pragma mark -
#pragma mark Video
// current subrect in the videoBuffer which is being updated
// the size of the game window is set to screenRect.size of the first frame
// it is assumed that this size has the same aspect ratio as bufferSize
// if screenRect.size changes, the image is resized to the original aspect ratio
// this must not be larger than bufferSize
// ...all these semantics are designed to match SNES hi-res mode
@property(readonly) OEIntRect   screenRect;
// maximum size of output video
@property(readonly) OEIntSize   bufferSize;

@property(readonly) const void *videoBuffer;
@property(readonly) GLenum      pixelFormat;
@property(readonly) GLenum      pixelType;
@property(readonly) GLenum      internalPixelFormat;

#pragma mark -
#pragma mark Audio
@property(readonly) NSUInteger  soundBufferCount; // overriding it is optional, should be constant

// used when soundBufferCount == 1
@property(readonly) NSUInteger  channelCount;
@property(readonly) NSUInteger  frameSampleCount;
@property(readonly) NSUInteger  soundBufferSize;
@property(readonly) NSUInteger  frameSampleRate;

// used when more than 1 buffer
- (NSUInteger)channelCountForBuffer:(NSUInteger)buffer;
- (NSUInteger)frameSampleCountForBuffer:(NSUInteger)buffer;
- (NSUInteger)soundBufferSizeForBuffer:(NSUInteger)buffer;
- (NSUInteger)frameSampleRateForBuffer:(NSUInteger)buffer;

#pragma mark -
#pragma mark Lightgun/Pointer Support
@property(readwrite) NSPoint mousePosition DEPRECATED_ATTRIBUTE;

/*
#pragma mark Input Settings & Parsing
- (OEEmulatorKey)emulatorKeyForKey:(NSString *)aKey index:(NSUInteger)index player:(NSUInteger)thePlayer;
- (OEEmulatorKey)emulatorKeyForKeyIndex:(NSUInteger)index player:(NSUInteger)thePlayer;
- (void)pressEmulatorKey:(OEEmulatorKey)aKey;
- (void)releaseEmulatorKey:(OEEmulatorKey)aKey;
 */
#pragma mark Input
//- (void)player:(NSUInteger)thePlayer didPressButton:(OEButton)gameButton;
//- (void)player:(NSUInteger)thePlayer didReleaseButton:(OEButton)gameButton;

#pragma mark -
#pragma mark Save state - Optional
- (BOOL)saveStateToFileAtPath:(NSString *)fileName;
- (BOOL)loadStateFromFileAtPath:(NSString *)fileName;

// ============================================================================
// End Abstract methods.
// ============================================================================

@end

#pragma mark -
#pragma mark Optional
@interface OEGameCore (OptionalMethods)
- (IBAction)pauseEmulation:(id)sender;

- (NSTrackingAreaOptions)mouseTrackingOptions;

- (NSSize) outputSize;
- (void) setRandomByte;
@end
