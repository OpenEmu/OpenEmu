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

#import "GenesisGameCore.h"
//#import "GENGameController.h"
#import <OERingBuffer.h>
#import <OpenGL/gl.h>
#import "OEHIDEvent.h"
#include "g_main.hpp"
#include "g_md.hpp"
#include "g_palette.h"
#include "ggenie.h"
#include "rom.hpp"
#include "g_update.hpp"
#include "audio_osx.hpp"
#include "options.hpp"
#include "v_draw_osx.hpp"
#include "input_osx.hpp"
#include "io.h"
#include "save.hpp"
#include <map>
#include <vector>
#import "OEGenesisSystemResponderClient.h"

#define SAMPLERATE 44100

@interface GenesisGameCore () <OEGenesisSystemResponderClient>
//- (NSUInteger)GEN_buttonMaskForButton:(OEGenesisButton)gameButton;
@end

NSString *GENControlNames[] = { @"CONTROLLER@_UP", @"CONTROLLER@_DOWN", @"CONTROLLER@_LEFT", @"CONTROLLER@_RIGHT", @"CONTROLLER@_START", @"CONTROLLER@_A", @"CONTROLLER@_B", @"CONTROLLER@_C", @"CONTROLLER@_MODE", @"CONTROLLER@_X", @"CONTROLLER@_Y", @"CONTROLLER@_Z" };
NSUInteger GENControlValues[] = { CONTROLLER_UP, CONTROLLER_DOWN, CONTROLLER_LEFT, CONTROLLER_RIGHT, CONTROLLER_START, CONTROLLER_A, CONTROLLER_B, CONTROLLER_C, CONTROLLER_MODE, CONTROLLER_X, CONTROLLER_Y, CONTROLLER_Z };

extern unsigned short MD_Screen[336 * 240];
extern unsigned short MD_Palette[256];
extern unsigned short _32X_Palette_16B[0x10000];


@implementation GenesisGameCore

//UInt16 audiobuf[735 * 2 * sizeof(UInt16)];
//unsigned short videoBuffer[320 *224];

- (oneway void)didPushGenesisButton:(OEGenesisButton)button forPlayer:(NSUInteger)player;
{
    //pad[player-1][GENControlValues[button]] = 1; //1 or 0xFFFF
    //Input_OSX *osx_in = (Input_OSX *)input;
    //osx_in->pressKey(player - 1, [self GEN_buttonMaskForButton:gameButton]);
}

- (oneway void)didReleaseGenesisButton:(OEGenesisButton)button forPlayer:(NSUInteger)player;
{
    //pad[player-1][GENControlValues[button]] = 0;
}

//- (id)initWithDocument:(GameDocument*) aDocument
- (id)init
{
	self = [super init];
	if(self)
	{
		audio = new Audio_OSX([self ringBufferAtIndex:0]);
		draw = new VDraw_OSX();
		input = new Input_OSX();
		NSString *appSupportPath = [[[[NSHomeDirectory() stringByAppendingPathComponent:@"Library"] 
									 stringByAppendingPathComponent:@"Application Support"] 
									stringByAppendingPathComponent:@"OpenEmu"]
									stringByAppendingPathComponent:@"BIOS"];
		
		
		
		strcpy(BIOS_Filenames._32X_MC68000, [[appSupportPath stringByAppendingPathComponent:@"32X_G_BIOS.BIN"] UTF8String]);
		strcpy(BIOS_Filenames._32X_MSH2, [[appSupportPath stringByAppendingPathComponent:@"32X_M_BIOS.BIN"] UTF8String]);
		strcpy(BIOS_Filenames._32X_SSH2, [[appSupportPath stringByAppendingPathComponent:@"32X_S_BIOS.BIN"] UTF8String]);
		strcpy(BIOS_Filenames.MegaCD_JP, [[appSupportPath stringByAppendingPathComponent:@"jp_mcd1_9112.bin"] UTF8String]);
		strcpy(BIOS_Filenames.SegaCD_US, [[appSupportPath stringByAppendingPathComponent:@"us_scd1_9210.bin"] UTF8String]);
		strcpy(BIOS_Filenames.MegaCD_EU, [[appSupportPath stringByAppendingPathComponent:@"eu_mcd1_9210.bin"] UTF8String]);
		Init_Settings();
		Init_GameGenie();
		
		Brightness_Level = 100;
		Contrast_Level = 100;
		
		Options::setSoundEnable(true);
		Recalculate_Palettes();
		Init_Genesis_Bios();
		
		if(!Init())
        {
            [self release];
            return nil;
        }
    }
	return self;
}

#pragma mark Exectuion

- (void)executeFrame
{
    [self executeFrameSkippingFrame:NO];
}

- (void)executeFrameSkippingFrame: (BOOL) skip
{
    Update_Emulation();
}

- (BOOL)loadFileAtPath:(NSString*)path
{
	Country_Order[0] = 0;
	Country_Order[1] = 1;
	Country_Order[2] = 2;
    NSLog(@"%s: %s", __func__, [path fileSystemRepresentation]);
    ROM::openROM([path fileSystemRepresentation]);
	//return (ROM::openROM([path fileSystemRepresentation]) >= 0);
    return YES;
}

#pragma mark Video
- (const void *)videoBuffer
{
	int x,y;
	for(x = 0; x < 320; x++)
		for(y = 0; y < 224; y++)
			videoBuffer[x + (y * 320)] = MD_Palette[MD_Screen[(x + 8) + (y * 336)] & 0xFF];
    
	//return &(videoBuf);
    return videoBuffer;
}

- (OEIntRect)screenRect
{
    return OERectMake(0, 0, 320, 224);
}

- (OEIntSize)bufferSize
{
    return OESizeMake(320, 224);
}

- (void)setupEmulation
{
}

- (void)resetEmulation
{
}

- (void)stopEmulation
{
    [super stopEmulation];
}

- (void) dealloc
{
    [super dealloc];
    //free(videoBuffer);
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

- (NSTimeInterval)frameInterval
{
    return 60;
}

#pragma mark Audio

- (NSUInteger)channelCount
{
	return 2;
}

//- (NSUInteger)frameSampleCount
//{
//	return 735;
//}


//- (NSUInteger)soundBufferSize
//{
//	return (735 * [self channelCount] * sizeof(UInt16));
//}

- (double)audioSampleRate
{
    return SAMPLERATE;
}
/*
#define PLAYER_MASK(player) (1 << (15 + (player)))

#pragma mark Input
- (NSUInteger)GEN_buttonMaskForButton:(OEGenesisButton)gameButton
{
    switch (gameButton) {
        case OEButton_1      : return CONTROLLER_A;
        case OEButton_2      : return CONTROLLER_B;
        case OEButton_3      : return CONTROLLER_C;
        case OEButton_4      : return CONTROLLER_X;
        case OEButton_5      : return CONTROLLER_Y;
        case OEButton_6      : return CONTROLLER_Z;
        case OEButton_Up     : return CONTROLLER_UP;
        case OEButton_Down   : return CONTROLLER_DOWN;
        case OEButton_Left   : return CONTROLLER_LEFT;
        case OEButton_Right  : return CONTROLLER_RIGHT;
        case OEButton_Start  : return CONTROLLER_START;
        case OEButton_Select : return CONTROLLER_MODE;
        default              : return 0;
    }
}

- (void)player:(NSUInteger)thePlayer didPressButton:(OEButton)gameButton
{
	Input_OSX *osx_in = (Input_OSX *)input;
    osx_in->pressKey(thePlayer - 1, [self GEN_buttonMaskForButton:gameButton]);
}

- (void)player:(NSUInteger)thePlayer didReleaseButton:(OEButton)gameButton
{
	Input_OSX *osx_in = (Input_OSX *)input;
    osx_in->releaseKey(thePlayer - 1, [self GEN_buttonMaskForButton:gameButton]);
}

- (OEEmulatorKey)emulatorKeyForKeyIndex:(NSUInteger)index player:(NSUInteger)thePlayer
{
    return OEMakeEmulatorKey(thePlayer - 1, GENControlValues[index]);
}

- (void)pressEmulatorKey:(OEEmulatorKey)aKey
{
	Input_OSX* osx_in = (Input_OSX*)input;
    osx_in->pressKey(aKey.player, aKey.key);
}

- (void)releaseEmulatorKey:(OEEmulatorKey)aKey
{
	Input_OSX* osx_in = (Input_OSX*)input;
    osx_in->releaseKey(aKey.player, aKey.key);
}*/

//- (void)loadStateFromFileAtPath:(NSString *)fileName
- (BOOL)loadStateFromFileAtPath:(NSString *)fileName
{
	Savestate::loadState([fileName UTF8String]);
    return YES;
}

//- (void)saveStateToFileAtPath:(NSString *)fileName
- (BOOL)saveStateToFileAtPath:(NSString *)fileName
{
	Savestate::saveState([fileName UTF8String]);
    return YES;
}

@end
