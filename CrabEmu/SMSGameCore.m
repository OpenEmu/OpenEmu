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
#import <OEGameDocument.h>
#import <OERingBuffer.h>
#import <OpenGL/gl.h>
#import "OESMSSystemResponderClient.h"
#import "OEGGSystemResponderClient.h"

#define _UINT32

#include "sms.h"
#include "smsmem.h"
#include "sound.h"
#include "smsvdp.h"
#include "smsz80.h"

#define SAMPLERATE 44100

@interface SMSGameCore () <OESMSSystemResponderClient, OEGGSystemResponderClient>
- (int)crabButtonForButton:(OESMSButton)button player:(NSUInteger)player;
@end

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
        bufLock = [[NSLock alloc] init];
        tempBuffer = malloc(256 * 256 * 4);
        
        position = 0;
        ringBuffer = [self ringBufferAtIndex:0];
    }
    return self;
}

- (void)dealloc
{
    DLog(@"releasing/deallocating CrabEmu memory");
    free(tempBuffer);
    
    sms_initialized = 0;
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

- (OEIntSize)bufferSize
{
    return OESizeMake(sms_console == CONSOLE_GG ? 160 : 256, sms_console == CONSOLE_GG ? 144 : smsvdp.lines);
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

- (double)frameSampleRate
{
    return SAMPLERATE;
}

- (NSUInteger)channelCount
{
    return 2;
}

- (BOOL)saveStateToFileAtPath:(NSString *)fileName
{
    return sms_save_state([fileName UTF8String]) == 0;
}

- (BOOL)loadStateFromFileAtPath:(NSString *)fileName
{
    return sms_load_state([fileName UTF8String]) == 0;
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

- (int)crabButtonForButton:(OESMSButton)button player:(NSUInteger)player;
{
    int btn = 0;
    switch(button)
    {
        case OESMSButtonUp    : btn = (player == 1 ? SMS_PAD1_UP    : SMS_PAD2_UP);    break;
        case OESMSButtonDown  : btn = (player == 1 ? SMS_PAD1_DOWN  : SMS_PAD2_DOWN);  break;
        case OESMSButtonLeft  : btn = (player == 1 ? SMS_PAD1_LEFT  : SMS_PAD2_LEFT);  break;
        case OESMSButtonRight : btn = (player == 1 ? SMS_PAD1_RIGHT : SMS_PAD2_RIGHT); break;
        case OESMSButtonA     : btn = (player == 1 ? SMS_PAD1_A     : SMS_PAD2_A);     break;
        case OESMSButtonB     : btn = (player == 1 ? SMS_PAD1_B     : SMS_PAD2_B);     break;
        default : break;
    }
    
    return btn;
}

- (int)crabButtonForButton:(OEGGButton)button;
{
    int btn = 0;
    switch(button)
    {
        case OEGGButtonUp:    btn = SMS_PAD1_UP;     break;
        case OEGGButtonDown:  btn = SMS_PAD1_DOWN;   break;
        case OEGGButtonLeft:  btn = SMS_PAD1_LEFT;   break;
        case OEGGButtonRight: btn = SMS_PAD1_RIGHT;  break;
        case OEGGButtonA:     btn = SMS_PAD1_A;      break;
        case OEGGButtonB:     btn = SMS_PAD1_B;      break;
        case OEGGButtonStart: btn = GG_START;        break;
        default : break;
    }
    
    return btn;
}

- (void)didPushGGButton:(OEGGButton)button;
{
    int btn = [self crabButtonForButton:button];
    if(btn > 0) sms_button_pressed(btn);
}

- (void)didReleaseGGButton:(OEGGButton)button;
{
    int btn = [self crabButtonForButton:button];
    if(btn > 0) sms_button_released(btn);
}

- (void)didPushSMSButton:(OESMSButton)button forPlayer:(NSUInteger)player;
{
    int btn = [self crabButtonForButton:button player:player];
    
    if(btn > 0) sms_button_pressed(btn);
}

- (void)didReleaseSMSButton:(OESMSButton)button forPlayer:(NSUInteger)player;
{
    int btn = [self crabButtonForButton:button player:player];
    
    if(btn > 0) sms_button_released(btn);
}

- (void)didPushSMSStartButton;
{
    if(sms_console != CONSOLE_GG)
        [self pauseEmulation:self];
    else
        sms_button_pressed(GG_START);
}

- (void)didReleaseSMSStartButton;
{
    
}

- (void)didPushSMSResetButton;
{
    sms_button_pressed(SMS_RESET);
}

- (void)didReleaseSMSResetButton;
{
    sms_button_released(SMS_RESET);
}

@end
