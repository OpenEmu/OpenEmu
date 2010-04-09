/*
 Copyright (c) 2009, OpenEmu Team
 All rights reserved.
 
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

#import "GenPlusGameCore.h"
#import <IOKit/hid/IOHIDLib.h>
#import <GameDocument.h>
#import <OERingBuffer.h>

//#include "shared.h"
#include "system.h"
#include "genesis.h"
#include "loadrom.h"
#include "gen_input.h"
#include "state.h"


#define SAMPLERATE 44100
#define SAMPLEFRAME 735
#define SIZESOUNDBUFFER SAMPLEFRAME*4

extern void set_config_defaults(void);

@implementation GenPlusGameCore

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
		videoBuffer = malloc(720 * 576 * 4);
		
		position = 0;
		sndBuf = malloc(SIZESOUNDBUFFER * sizeof(UInt16));
		memset(sndBuf, 0, SIZESOUNDBUFFER * sizeof(UInt16));
		//ringBuffer = [self ringBufferAtIndex:0];
	}
	return self;
}

- (void) dealloc
{
	DLog(@"releasing/deallocating CrabEmu memory");
	free(sndBuf);
	[soundLock release];
	[bufLock release];
	free(videoBuffer);

	[super dealloc];
}

- (void)executeFrame
{
	system_frame(0);
	audio_update(snd.buffer_size);
	for(int i = 0 ; i < snd.buffer_size; i++)
	{
		[[self ringBufferAtIndex:0] write:&snd.buffer[0][i] maxLength:2];
		[[self ringBufferAtIndex:0] write:&snd.buffer[1][i] maxLength:2];
	}
}

void update_input()
{
	
}

- (void)setupEmulation
{
}

- (BOOL)loadFileAtPath:(NSString*)path
{
	DLog(@"Loaded File");
	
	set_config_defaults();
	
	cart_rom = malloc(MAXROMSIZE);
	if( load_rom((char*)[path UTF8String]) )
	{
		/* allocate global work bitmap */
		memset (&bitmap, 0, sizeof (bitmap));
		bitmap.width  = 720;
		bitmap.height = 576;
		bitmap.depth  = 16;
		bitmap.granularity = 2;
		bitmap.pitch = bitmap.width * bitmap.granularity;
		bitmap.viewport.w = 256;
		bitmap.viewport.h = 224;
		bitmap.viewport.x = 0;
		bitmap.viewport.y = 0;
		bitmap.data =videoBuffer;
		
		/* default system */
		input.system[0] = SYSTEM_GAMEPAD;
		input.system[1] = SYSTEM_GAMEPAD;
		
		system_init();
		audio_init(SAMPLERATE);
		system_reset();
	}
	//TODO: add choice NTSC/PAL
	//if(sms_init(SMS_VIDEO_NTSC, SMS_REGION_DOMESTIC))
	//	return NO;
	
	//if(sms_mem_load_rom([path UTF8String])) {
     //   return NO;
    //}	
    return YES;
}
- (void)resetEmulation
{
	system_reset();
}

- (void)stopEmulation
{
	//sms_write_cartram_to_file();
	[super stopEmulation];
}

- (IBAction)pauseEmulation:(id)sender
{
  //  [bufLock lock];
  //  sms_z80_nmi();
  //  [bufLock unlock];
}

- (CGRect)sourceRect
{
	return CGRectMake( bitmap.viewport.x, bitmap.viewport.y, bitmap.viewport.w, bitmap.viewport.h );
}

- (NSUInteger)bufferWidth
{
	return bitmap.width;
}

- (NSUInteger)bufferHeight
{
	return bitmap.height;
}


- (NSUInteger)screenWidth
{
	return 320;
//	if (sms_console != CONSOLE_GG)
//		return 256;
//	else
//		return 160;
}

- (NSUInteger)screenHeight
{
	return 240;
//	if (sms_console != CONSOLE_GG)
//		return smsvdp.lines;
//	else
//		return 144;
}

- (const void *)videoBuffer
{
/*	if (sms_console != CONSOLE_GG)
		return smsvdp.framebuffer;
	else
        for (int i = 0; i < 144; i++)
			//jump 24 lines, skip 48 pixels and capture for each line of the buffer 160 pixels
            // sizeof(unsigned char) is always equal to 1 by definition
			memcpy(tempBuffer + i * 160 * 4, smsvdp.framebuffer  + 24 * 256 * 1 + 48 * 1 + i * 256 * 1, 160 * 4);*/
	return videoBuffer;
}

- (GLenum)pixelFormat
{
	return GL_RGB;
}

- (GLenum)pixelType
{
	return GL_UNSIGNED_SHORT_5_6_5;
}

- (GLenum)internalPixelFormat
{
	return GL_RGB5;
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
/*
static const NSUInteger buttonDirections[2][5] = {
    { 0, SMS_PAD1_UP, SMS_PAD1_DOWN, SMS_PAD1_LEFT, SMS_PAD1_RIGHT },
    { 0, SMS_PAD2_UP, SMS_PAD2_DOWN, SMS_PAD2_LEFT, SMS_PAD2_RIGHT }
};
static const NSUInteger buttonActions[2][2] = {
    { SMS_PAD1_A, SMS_PAD1_B },
    { SMS_PAD2_A, SMS_PAD2_B }
};
*/
- (void)player:(NSUInteger)thePlayer didChangeButtonState:(OEButton)gameButton toPressed:(BOOL)flag
{
  /*  if(thePlayer > 1) return;
    
	void (*sms_button)(int) = (flag ? sms_button_pressed : sms_button_released);
    NSUInteger button = 0;
    
    if(OEButton_Up <= gameButton && gameButton <= OEButton_Right)
        button = buttonDirections[thePlayer][gameButton];
    else if(gameButton == OEButton_Start)
        button = GG_START;
    else if(OEButton_1 <= gameButton)
        button = buttonActions[thePlayer][(gameButton - OEButton_1) % 2];

	if(button != 0) sms_button(button);*/
    
}

- (void)player:(NSUInteger)thePlayer didPressButton:(OEButton)gameButton
{
	[self player:thePlayer didChangeButtonState:gameButton toPressed:YES];
}

- (void)player:(NSUInteger)thePlayer didReleaseButton:(OEButton)gameButton
{
	[self player:thePlayer didChangeButtonState:gameButton toPressed:NO];
}


/*
 CrabEmu callbacks
 */

NSString *GenesisButtonNameTable[] = { @"CONTROLLER@_UP", @"CONTROLLER@_DOWN", @"CONTROLLER@_LEFT", @"CONTROLLER@_RIGHT", @"CONTROLLER@_START", @"CONTROLLER@_A", @"CONTROLLER@_B", @"CONTROLLER@_C", @"CONTROLLER@_MODE", @"CONTROLLER@_X", @"CONTROLLER@_Y", @"CONTROLLER@_Z" };
NSUInteger GenesisControlValues[] = { INPUT_UP, INPUT_DOWN, INPUT_LEFT, INPUT_RIGHT, INPUT_START, INPUT_A, INPUT_B, INPUT_C, INPUT_MODE, INPUT_X, INPUT_Y, INPUT_Z };

- (BOOL)shouldPauseForButton:(NSInteger)button
{
  //  if(button == GG_START && sms_console != CONSOLE_GG)
    //{
      //  [self pauseEmulation:self];
    //    return YES;
    //}
    return NO;
}

- (OEEmulatorKey)emulatorKeyForKeyIndex:(NSUInteger)index player:(NSUInteger)thePlayer
{
    return OEMakeEmulatorKey(thePlayer - 1, GenesisControlValues[index]);
}

- (void)pressEmulatorKey:(OEEmulatorKey)aKey
{
	input.pad[aKey.player] |= aKey.key;
}

- (void)releaseEmulatorKey:(OEEmulatorKey)aKey
{
	input.pad[aKey.player] &= ~aKey.key;
}

- (BOOL)saveStateToFileAtPath:(NSString *)fileName
{
	FILE* f = fopen([fileName UTF8String],"w+b");
	if (f)
	{
		unsigned char buffer[STATE_SIZE];
		state_save(buffer);
		fwrite(&buffer, STATE_SIZE, 1, f);
		fclose(f);
		return YES;
	}
	return NO;
}

- (BOOL)loadStateFromFileAtPath:(NSString *)fileName
{
	FILE* f = fopen([fileName UTF8String],"r+b");
	if (f)
	{
		unsigned char buffer[STATE_SIZE];
		fread(&buffer, STATE_SIZE, 1, f);
		state_load(buffer);
		fclose(f);
		return YES;
	}
	return NO;
}

@end
