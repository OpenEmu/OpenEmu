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
#define SAMPLEFRAME (SAMPLERATE * 4389/262144)
#define SIZESOUNDBUFFER SAMPLEFRAME*4
#define NEWSIZESOUNDBUFFER 80000

#import <OERingBuffer.h>
#import "GBGameEmu.h"
#include "gambatte.h"
#include "CocoaBlitter.h"
#include "InputGetter.h"
#include "adaptivesleep.h"
#include "resamplerinfo.h"
#include <sys/time.h>

@interface GBGameEmu ()
- (void)GB_setInputForButton:(GBButtons)gameButton isPressed:(BOOL)isPressed;
@end


@implementation GBGameEmu


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
    timespec tspec = { tv_sec: 0,
        tv_nsec: usecs * 1000 };
    
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
    resampler->resample((short int*)tmpBuf, (short int*)sndBuf, 35112);
    [[self ringBufferAtIndex:0] write:tmpBuf maxLength:SAMPLEFRAME * sizeof(UInt16) * 2];
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

- (void)GB_setInputForButton:(GBButtons)gameButton isPressed:(BOOL)isPressed
{
    switch(gameButton)
    {
        case GB_A      : input.setButA(isPressed);   break;
        case GB_B      : input.setButB(isPressed);   break;
        case GB_Up     : input.setUp(isPressed);     break;
        case GB_Down   : input.setDown(isPressed);   break;
        case GB_Left   : input.setLeft(isPressed);   break;
        case GB_Right  : input.setRight(isPressed);  break;
        case GB_Start  : input.setStart(isPressed);  break;
        case GB_Select : input.setSelect(isPressed); break;
    }
}


NSString *GBButtonNameTable[] = { @"GB_PAD_UP", @"GB_PAD_DOWN", @"GB_PAD_LEFT", @"GB_PAD_RIGHT", @"GB_PAD_A", @"GB_PAD_B",  @"GB_START", @"GB_SELECT" };

- (BOOL)shouldPauseForButton:(NSInteger)button
{
    return NO;
}

- (OEEmulatorKey)emulatorKeyForKeyIndex:(NSUInteger)index player:(NSUInteger)thePlayer
{
    return OEMakeEmulatorKey(0, index);
}


- (void)player:(NSUInteger)thePlayer didPressButton:(OEButton)gameButton
{
    switch (gameButton) {
        case OEButton_Up:
            [self GB_setInputForButton:GB_Up isPressed:YES];
            break;
        case OEButton_Down:
            [self GB_setInputForButton:GB_Down isPressed:YES];
            break;
        case OEButton_Left:
            [self GB_setInputForButton:GB_Left isPressed:YES];
            break;
        case OEButton_Right:
            [self GB_setInputForButton:GB_Right isPressed:YES];
            break;
        case OEButton_Start:
            [self GB_setInputForButton:GB_Start isPressed:YES];
            break;
        case OEButton_Select:
            [self GB_setInputForButton:GB_Select isPressed:YES];
            break;
        case OEButton_1:
            [self GB_setInputForButton:GB_A isPressed:YES];
            break;
        case OEButton_2:
            [self GB_setInputForButton:GB_B isPressed:YES];
            break;
    }
}

- (void)player:(NSUInteger)thePlayer didReleaseButton:(OEButton)gameButton
{
    switch (gameButton) {
        case OEButton_Up:
            [self GB_setInputForButton:GB_Up isPressed:NO];
            break;
        case OEButton_Down:
            [self GB_setInputForButton:GB_Down isPressed:NO];
            break;
        case OEButton_Left:
            [self GB_setInputForButton:GB_Left isPressed:NO];
            break;
        case OEButton_Right:
            [self GB_setInputForButton:GB_Right isPressed:NO];
            break;
        case OEButton_Start:
            [self GB_setInputForButton:GB_Start isPressed:NO];
            break;
        case OEButton_Select:
            [self GB_setInputForButton:GB_Select isPressed:NO];
            break;
        case OEButton_1:
            [self GB_setInputForButton:GB_A isPressed:NO];
            break;
        case OEButton_2:
            [self GB_setInputForButton:GB_B isPressed:NO];
            break;
    }
}

- (void)pressEmulatorKey:(OEEmulatorKey)aKey
{
    unsigned button = aKey.key;
    [self GB_setInputForButton:(GBButtons)button isPressed:YES];
}

- (void)releaseEmulatorKey:(OEEmulatorKey)aKey
{
    unsigned button = aKey.key;
    [self GB_setInputForButton:(GBButtons)button isPressed:NO];
}

- (NSUInteger)channelCount
{
    return 2;
}

- (NSUInteger)soundBufferSize
{
    return SIZESOUNDBUFFER;
}

- (NSUInteger)frameSampleCount
{
    return SAMPLEFRAME;
}
- (NSUInteger)frameSampleRate
{
    return SAMPLERATE;
}

- (void) dealloc
{
    free(sndBuf);
    [super dealloc];
}
@end
