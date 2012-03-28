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

#define SAMPLERATE 48000
#define NEWSIZESOUNDBUFFER 80000

#import <OpenEmuBase/OERingBuffer.h>
#import <OpenGL/gl.h>
#import "OEGBSystemResponderClient.h"
#import "GBGameCore.h"
#include "gambatte.h"
#include "CocoaBlitter.h"
#include "InputGetter.h"
#include "adaptivesleep.h"
#include "resamplerinfo.h"
#include <sys/time.h>

@interface GBGameCore () <OEGBSystemResponderClient>
- (void)GB_setInputForButton:(OEGBButton)gameButton isPressed:(BOOL)isPressed;
@end


@implementation GBGameCore


Gambatte::GB gambatte;
CocoaBlitter blitter;
Gambatte::InputGetter input;

NSLock* soundLock;
NSLock* bufLock;
UInt16* sndBuf;

std::auto_ptr<Resampler> resampler;

bool ** tabInput;

usec_t getusecs() {
    timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec * usec_t(1000000) + t.tv_usec;
}

void usecsleep(const usec_t usecs) {
    timespec tspec =
    {
        .tv_sec  = 0,
        .tv_nsec = usecs * 1000
    };
    
    nanosleep(&tspec, NULL);
}

/*
 OpenEmu Core internal functions
 */
- (id)init
{
    self = [super init];
    if(self != nil)
    {
        NSLog(@"setup");
        tmpBuf = new UInt16[NEWSIZESOUNDBUFFER];
        sndBuf = new UInt16[NEWSIZESOUNDBUFFER];
        memset(sndBuf, 0, NEWSIZESOUNDBUFFER*sizeof(UInt16));
        memset(tmpBuf, 0, (NEWSIZESOUNDBUFFER*sizeof(UInt16)));
        
        soundLock = [NSLock new];
        bufLock = [NSLock new];
        resampler.reset(ResamplerInfo::get(1).create(2097152, SAMPLERATE, 35112));
        
        gambatte.setVideoFilter(0);
        
        blitter.setCore(self);
        gambatte.setVideoBlitter(&blitter);
        
        gambatte.setInputStateGetter(&input);
        //tabInput =(bool **) malloc(8*sizeof(bool *));
        //input.FillTabInput(tabInput);
    }
    return self;
}

- (void)executeFrame
{
    static int samples = 0;
    [bufLock lock];
    [soundLock lock];
        
    samples += gambatte.runFor(reinterpret_cast<Gambatte::uint_least32_t*>(static_cast<UInt16*>(&sndBuf[0])) + samples, 35112 - samples);
    samples -= 35112;
    int size = resampler->resample((short int*)tmpBuf, (short int*)sndBuf, 35112);
    [[self ringBufferAtIndex:0] write:tmpBuf maxLength:size * sizeof(UInt16) * 2];
    //memmove(sndBuf, sndBuf + 35112 * 2, samples * sizeof(UInt16) * 2);
    [soundLock unlock];
    [bufLock unlock];
}

- (BOOL)loadFileAtPath:(NSString*)path
{
    NSLog(@"loading");
    return !(gambatte.load([path UTF8String]));
}

- (OEIntSize)bufferSize
{
    return OESizeMake(gambatte.videoWidth(), gambatte.videoHeight());
}

- (void)setVideoBuffer:(const void *)buffer
{
    videoBuffer = buffer;
}

- (const void *)videoBuffer
{
    return videoBuffer;
}

- (void)resetEmulation
{
    gambatte.reset();
}

- (BOOL)saveStateToFileAtPath: (NSString*) fileName
{
    gambatte.saveState([fileName UTF8String]);
    return YES;
}

- (BOOL)loadStateFromFileAtPath: (NSString*) fileName
{
    gambatte.loadState([fileName UTF8String]);
    return YES;
}

- (void)setupEmulation
{
    
}

- (GLenum)pixelFormat
{
    return GL_BGRA;
}

- (GLenum)pixelType
{
    return GL_UNSIGNED_INT_8_8_8_8_REV;
}

- (GLenum)internalPixelFormat
{
    return GL_RGB8;
}

- (oneway void)didPushGBButton:(OEGBButton)button;
{
    [self GB_setInputForButton:(OEGBButton)button isPressed:YES];
}

- (oneway void)didReleaseGBButton:(OEGBButton)button;
{
    [self GB_setInputForButton:(OEGBButton)button isPressed:NO];
}

- (void)GB_setInputForButton:(OEGBButton)gameButton isPressed:(BOOL)isPressed
{
    switch(gameButton)
    {
        case OEGBButtonA      : input.setButA(isPressed);   break;
        case OEGBButtonB      : input.setButB(isPressed);   break;
        case OEGBButtonUp     : input.setUp(isPressed);     break;
        case OEGBButtonDown   : input.setDown(isPressed);   break;
        case OEGBButtonLeft   : input.setLeft(isPressed);   break;
        case OEGBButtonRight  : input.setRight(isPressed);  break;
        case OEGBButtonStart  : input.setStart(isPressed);  break;
        case OEGBButtonSelect : input.setSelect(isPressed); break;
        default : break;
    }
}


NSString *GBButtonNameTable[] = { @"GB_PAD_UP", @"GB_PAD_DOWN", @"GB_PAD_LEFT", @"GB_PAD_RIGHT", @"GB_PAD_A", @"GB_PAD_B",  @"GB_START", @"GB_SELECT" };

- (BOOL)shouldPauseForButton:(NSInteger)button
{
    return NO;
}

- (NSUInteger)channelCount
{
    return 2;
}

- (double)audioSampleRate
{
    return SAMPLERATE;
}

- (NSTimeInterval)frameInterval
{
    return 2097152./35112.; // 59.7
}

- (void) dealloc
{
    free(sndBuf);
}
@end
