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

#import "SMSGameCore.h"
#import <IOKit/hid/IOHIDLib.h>
#import <GameDocument.h>
#import <OERingBuffer.h>

#define _UINT32

#include "sms.h"
#include "smsmem.h"
#include "sound.h"
#include "smsvdp.h"
#include "smsz80.h"


#define SAMPLERATE 44100
#define SAMPLEFRAME 735
#define SIZESOUNDBUFFER SAMPLEFRAME*4

@implementation SMSGameCore

extern int sms_initialized;
extern int sms_console;

// Global variables because the callbacks need to access them...
static OERingBuffer *ringBuffer;
/*
 OpenEmu Core internal functions
 */
- (id)init
{
    self = [super init];
    if(self != nil)
    {
        soundLock = [[NSLock alloc] init];
        bufLock = [[NSLock alloc] init];
        tempBuffer = malloc(256 * 256 * 4);
        
        position = 0;
        sndBuf = malloc(SIZESOUNDBUFFER * sizeof(UInt16));
        memset(sndBuf, 0, SIZESOUNDBUFFER * sizeof(UInt16));
        ringBuffer = [self ringBufferAtIndex:0];
    }
    return self;
}

- (void)dealloc
{
    DLog(@"releasing/deallocating CrabEmu memory");
    free(sndBuf);
    [soundLock release];
    [bufLock release];
    free(tempBuffer);
    
    sms_initialized = 0;
    [super dealloc];
}

- (void)executeFrame
{
    //DLog(@"Executing");
    //Get a reference to the emulator
    [bufLock lock];
    oldrun = sms_frame(oldrun);
    [bufLock unlock];
}

- (void)setupEmulation
{
}

- (BOOL)loadFileAtPath:(NSString*)path
{
    DLog(@"Loaded File");
    //TODO: add choice NTSC/PAL
    if(sms_init(SMS_VIDEO_NTSC, SMS_REGION_DOMESTIC))
        return NO;
    
    if(sms_mem_load_rom([path UTF8String])) {
        return NO;
    }
    return YES;
}
- (void)resetEmulation
{
    sms_soft_reset();
}

- (void)stopEmulation
{
    sms_write_cartram_to_file();
    [super stopEmulation];
}

- (IBAction)pauseEmulation:(id)sender
{
    [bufLock lock];
    sms_z80_nmi();
    [bufLock unlock];
}

- (NSUInteger)screenWidth
{
    if (sms_console != CONSOLE_GG)
        return 256;
    else
        return 160;
}

- (NSUInteger)screenHeight
{
    if (sms_console != CONSOLE_GG)
        return smsvdp.lines;
    else
        return 144;
}

- (const void *)videoBuffer
{
    if (sms_console != CONSOLE_GG)
        return smsvdp.framebuffer;
    else
        for (int i = 0; i < 144; i++)
            //jump 24 lines, skip 48 pixels and capture for each line of the buffer 160 pixels
            // sizeof(unsigned char) is always equal to 1 by definition
            memcpy(tempBuffer + i * 160 * 4, smsvdp.framebuffer  + 24 * 256 * 1 + 48 * 1 + i * 256 * 1, 160 * 4);
    return tempBuffer;
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

- (NSUInteger)channelCount
{
    return 2;
}

static const NSUInteger buttonDirections[2][5] = {
    { 0, SMS_PAD1_UP, SMS_PAD1_DOWN, SMS_PAD1_LEFT, SMS_PAD1_RIGHT },
    { 0, SMS_PAD2_UP, SMS_PAD2_DOWN, SMS_PAD2_LEFT, SMS_PAD2_RIGHT }
};
static const NSUInteger buttonActions[2][2] = {
    { SMS_PAD1_A, SMS_PAD1_B },
    { SMS_PAD2_A, SMS_PAD2_B }
};

- (void)player:(NSUInteger)thePlayer didChangeButtonState:(OEButton)gameButton toPressed:(BOOL)flag
{
    if(thePlayer > 1) return;
    
    void (*sms_button)(int) = (flag ? sms_button_pressed : sms_button_released);
    NSUInteger button = 0;
    
    if(OEButton_Up <= gameButton && gameButton <= OEButton_Right)
        button = buttonDirections[thePlayer][gameButton];
    else if(gameButton == OEButton_Start)
        button = GG_START;
    else if(OEButton_1 <= gameButton)
        button = buttonActions[thePlayer][(gameButton - OEButton_1) % 2];
    
    if(button != 0) sms_button(button);
}

- (void)player:(NSUInteger)thePlayer didPressButton:(OEButton)gameButton
{
    [self player:thePlayer didChangeButtonState:gameButton toPressed:YES];
}

- (void)player:(NSUInteger)thePlayer didReleaseButton:(OEButton)gameButton
{
    [self player:thePlayer didChangeButtonState:gameButton toPressed:NO];
}

- (BOOL)saveStateToFileAtPath: (NSString*) fileName
{
    int success = sms_save_state([fileName UTF8String]);
    if(success == 0)
        return YES;
    return NO;
}

- (BOOL)loadStateFromFileAtPath: (NSString*) fileName
{
    int success = sms_load_state([fileName UTF8String]);
    if(success == 0)
        return YES;
    return NO;
}


/*
 CrabEmu callbacks
 */

void sound_update_buffer(signed short *buf, int length)
{
    //NSLog(@"%s %p", __FUNCTION__, ringBuffer);
    [ringBuffer write:buf maxLength:length];
}

int sound_init(void)
{
    return 0;
}

void sound_shutdown(void)
{
    
}

void sound_reset_buffer(void)
{
    
}

void gui_set_viewport(int w, int h)
{
    //NSLog(@"viewport, width: %d, height: %d", w, h);
}

void gui_set_aspect(float x, float y)
{
    //NSLog(@"set_aspect, x: %f, y: %f", x, y);
}

void gui_set_title(const char *str)
{
    //NSLog(@"set_title%s", str);
}

unsigned SMSButtonTable[] = {
    SMS_PAD1_UP, SMS_PAD1_DOWN, SMS_PAD1_LEFT, SMS_PAD1_RIGHT, SMS_PAD1_A, SMS_PAD1_B, SMS_RESET, GG_START,
    SMS_PAD2_UP, SMS_PAD2_DOWN, SMS_PAD2_LEFT, SMS_PAD2_RIGHT, SMS_PAD2_A, SMS_PAD2_B, SMS_RESET, GG_START
};

NSString *SMSButtonNameTable[] = { @"SMS_PAD@_UP", @"SMS_PAD@_DOWN", @"SMS_PAD@_LEFT", @"SMS_PAD@_RIGHT", @"SMS_PAD@_A", @"SMS_PAD@_B", @"SMS_RESET", @"GG_START" };

- (BOOL)shouldPauseForButton:(NSInteger)button
{
    if(button == GG_START && sms_console != CONSOLE_GG)
    {
        [self pauseEmulation:self];
        return YES;
    }
    return NO;
}

- (OEEmulatorKey)emulatorKeyForKeyIndex:(NSUInteger)index player:(NSUInteger)thePlayer
{
    if(thePlayer > 0) thePlayer--;
    index += 8 * thePlayer;
    return OEMakeEmulatorKey(0, SMSButtonTable[index]);
}

- (void)pressEmulatorKey:(OEEmulatorKey)aKey
{
    unsigned button = aKey.key;
    if(![self shouldPauseForButton:button])
        sms_button_pressed(button);
}

- (void)releaseEmulatorKey:(OEEmulatorKey)aKey
{
    unsigned button = aKey.key;
    if(button != GG_START)
        sms_button_released(button);
}

@end
