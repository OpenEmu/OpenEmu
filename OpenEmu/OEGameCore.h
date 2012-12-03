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

#define OE_EXPORTED_CLASS __attribute__((visibility("default")))

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

static inline NSString *NSStringFromOEIntPoint(OEIntPoint p)
{
    return [NSString stringWithFormat:@"{ %d, %d }", p.x, p.y];
}

static inline NSString *NSStringFromOEIntSize(OEIntSize s)
{
    return [NSString stringWithFormat:@"{ %d, %d }", s.width, s.height];
}

static inline NSString *NSStringFromOEIntRect(OEIntRect r)
{
    return [NSString stringWithFormat:@"{ %@, %@ }", NSStringFromOEIntPoint(r.origin), NSStringFromOEIntSize(r.size)];
}

@class OEHIDEvent, OERingBuffer;

#pragma mark -

@interface OEGameCore : NSResponder <OESystemResponderClient, OESettingObserver>
{    
    NSThread               *emulationThread;
    NSTimeInterval          frameInterval;
    OERingBuffer __strong **ringBuffers;
    NSUInteger              frameSkip;
    NSUInteger              frameCounter;
    NSUInteger              tenFrameCounter;
    NSUInteger              autoFrameSkipLastTime;
    NSUInteger              frameskipadjust;
    
    // for lightgun/pointer support.
    NSPoint                 mousePosition;
    
    BOOL                    frameFinished;
    BOOL                    willSkipFrame;
    
    BOOL                    isRunning;
    BOOL                    shouldStop;
}

@property(weak)     id<OERenderDelegate>  renderDelegate;

@property(weak)     OEGameCoreController *owner;
@property(readonly) NSString             *pluginName;

@property(readonly) NSString             *supportDirectoryPath;
@property(readonly) NSString             *batterySavesDirectoryPath;

@property(readonly) NSTimeInterval        frameInterval;
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

// ============================================================================
// Abstract methods: Those methods should be overridden by subclasses
// ============================================================================
- (void)resetEmulation;
- (void)executeFrame;
- (void)executeFrameSkippingFrame:(BOOL) skip;

- (BOOL)loadFileAtPath:(NSString *)path;

#pragma mark -
#pragma mark Video
// The full size of the internal video buffer used by the core
// This is typically the largest size possible.
@property(readonly) OEIntSize   bufferSize;

// The size of the current portion of the buffer that is needs to be displayed as "active" to the user
// Note that this rect may not be have the same aspect ratio as what the end user sees. 
@property(readonly) OEIntRect   screenRect;

// The *USER INTERFACE* aspect of the actual final displayed video on screen.
@property(readonly) OEIntSize   aspectSize;

@property(readonly) const void *videoBuffer;
@property(readonly) GLenum      pixelFormat;
@property(readonly) GLenum      pixelType;
@property(readonly) GLenum      internalPixelFormat;

#pragma mark -
#pragma mark Audio
@property(readonly) NSUInteger  audioBufferCount; // overriding it is optional, should be constant

// used when audioBufferCount == 1
@property(readonly) NSUInteger  channelCount;
@property(readonly) double      audioSampleRate;

// used when more than 1 buffer
- (NSUInteger)channelCountForBuffer:(NSUInteger)buffer;
- (NSUInteger)audioBufferSizeForBuffer:(NSUInteger)buffer;
- (double)audioSampleRateForBuffer:(NSUInteger)buffer;

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

- (void)poke:(uint32_t)address value:(uint32_t)value;
@end
