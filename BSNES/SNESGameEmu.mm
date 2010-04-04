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

#import "SNESGameEmu.h"
#import <OERingBuffer.h>

//#include <libreader/libreader.hpp>
//#include <libreader/filereader.hpp>
//#include "utility/utility.hpp"

#define SAMPLERATE 32040
#define SAMPLEFRAME 800
#define SIZESOUNDBUFFER SAMPLEFRAME*4

@interface BSNESGameEmu ()
- (NSUInteger)SNES_buttonMaskForButton:(OEButton)gameButton;
@end

@implementation BSNESGameEmu

NSUInteger BSNESEmulatorValues[] = { SNES::Input::JoypadR, SNES::Input::JoypadL, SNES::Input::JoypadX, SNES::Input::JoypadA, SNES::Input::JoypadRight, SNES::Input::JoypadLeft, SNES::Input::JoypadDown, SNES::Input::JoypadUp, SNES::Input::JoypadStart, SNES::Input::JoypadSelect, SNES::Input::JoypadY, SNES::Input::JoypadB };
NSString *BSNESEmulatorNames[] = { @"Joypad@ R", @"Joypad@ L", @"Joypad@ X", @"Joypad@ A", @"Joypad@ Right", @"Joypad@ Left", @"Joypad@ Down", @"Joypad@ Up", @"Joypad@ Start", @"Joypad@ Select", @"Joypad@ Y", @"Joypad@ B" };

- (OEEmulatorKey)emulatorKeyForKey:(NSString *)aKey index:(NSUInteger)index player:(NSUInteger)thePlayer
{
    NSUInteger val = BSNESEmulatorValues[index];
    OEEmulatorKey ret = OEMakeEmulatorKey(thePlayer, val);
    return ret;
}

- (void)pressEmulatorKey:(OEEmulatorKey)aKey
{
	interface->pad[aKey.player-1][aKey.key] = 0xFFFF;
}

- (void)releaseEmulatorKey:(OEEmulatorKey)aKey
{
	interface->pad[aKey.player-1][aKey.key] = 0;
}
#pragma mark Exectuion

- (void)executeFrame
{
	[self executeFrameSkippingFrame:NO];
}

- (void)executeFrameSkippingFrame: (BOOL) skip
{
	SNES::system.run();
	//[self reportControlPad:0 withFlags:controlPad[0]];		
	
//	IPPU.RenderThisFrame = !skip;
//	S9xMainLoop();
//	
	//S9xMixSamples((unsigned char*)soundBuffer, SAMPLEFRAME * [self channelCount]);
    //[[self ringBufferAtIndex:0] write:soundBuffer maxLength:sizeof(UInt16) * [self channelCount] * SAMPLEFRAME];
}

bool loadCartridge(const char *filename, SNES::MappedRAM &memory) {
	if(file::exists(filename) == false) return false;
	//Reader::Type filetype = Reader::detect(filename, true);
	
	uint8_t *data;
	unsigned size;

	NSData* dataObj = [NSData dataWithContentsOfFile:[[NSString stringWithUTF8String:filename] stringByStandardizingPath]];
	if(dataObj == nil) return false;
	size = [dataObj length];
	data = (uint8_t*)[dataObj bytes];
	
	//remove copier header, if it exists
	if((size & 0x7fff) == 512) memmove(data, data + 512, size -= 512);
	
	memory.copy(data, size);
	return true;
}

- (BOOL)loadFileAtPath: (NSString*) path
{
	//system = new SNES::System();
	
	//[self mapButtons];
	

	interface = new BSNESInterface();

	memset(&interface->pad, 0, sizeof(int16_t) * 24);
	interface->video = (uint16_t*)malloc(512*480*2);
	interface->ringBuffer = [self ringBufferAtIndex:0];
	SNES::system.init(interface);
	
		
	
	SNES::MappedRAM memory;
	if(loadCartridge([path UTF8String], memory) == false) return NO;
	SNES::Cartridge::Type type = SNES::cartridge.detect_image_type(memory.data(), memory.size());
	memory.reset();
	
	if(loadCartridge([path UTF8String], SNES::memory::cartrom));
	{
		SNES::cartridge.load(SNES::Cartridge::ModeNormal); 
		SNES::system.power();
		SNES::input.port_set_device(0, SNES::Input::DeviceJoypad);
		SNES::input.port_set_device(1, SNES::Input::DeviceJoypad);
	}
	return YES;
}


#pragma mark Video
- (const void *)videoBuffer
{
	return interface->video;
}

- (CGRect)sourceRect
{
	return CGRectMake(0, 0, interface->width, interface->height);
}

- (NSUInteger)screenWidth
{
	return 512;
}

- (NSUInteger)screenHeight
{
	return 478;
}

- (NSUInteger)bufferWidth
{
	return 1024;
}

- (NSUInteger)bufferHeight
{
	return 478;
}

- (void)setupEmulation
{
	soundBuffer = (UInt16*)malloc(SIZESOUNDBUFFER* sizeof(UInt16));
	memset(soundBuffer, 0, SIZESOUNDBUFFER*sizeof(UInt16));
}

- (void)resetEmulation
{
	//S9xSoftReset();
}

- (void)stopEmulation
{
	/*
	NSString* path = [NSString stringWithUTF8String:Memory.ROMFilename];
	NSString *extensionlessFilename = [[path lastPathComponent] stringByDeletingPathExtension];	
	NSString *appSupportPath = [[[NSHomeDirectory() stringByAppendingPathComponent:@"Library"] stringByAppendingPathComponent:@"Application Support"] stringByAppendingPathComponent:@"Snes9x"];
	if(![[NSFileManager defaultManager] fileExistsAtPath:appSupportPath])
		[[NSFileManager defaultManager] createDirectoryAtPath:appSupportPath attributes:nil];

	
	NSLog(@"Trying to save SRAM");
	
	NSString *batterySavesDirectory = [appSupportPath stringByAppendingPathComponent:@"Battery Saves"];
	if(![[NSFileManager defaultManager] fileExistsAtPath:batterySavesDirectory])
		[[NSFileManager defaultManager] createDirectoryAtPath:batterySavesDirectory attributes:nil];
	NSString *filePath = [batterySavesDirectory stringByAppendingPathComponent:[extensionlessFilename stringByAppendingPathExtension:@"sav"]];
	
	Memory.SaveSRAM( [filePath UTF8String] );
	*/
	[super stopEmulation];
}

- (GLenum)pixelFormat
{
	return GL_RGBA;
}

- (GLenum)pixelType
{
	return GL_UNSIGNED_SHORT_1_5_5_5_REV;
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

- (NSTimeInterval)frameInterval
{
	/*if( Settings.PAL )
		return 50;
	else
		return 60;*/
	return 60;
}

- (NSUInteger)channelCount
{
	return 2;
}

- (NSUInteger)SNES_buttonMaskForButton:(OEButton)gameButton
{
    NSUInteger button = 0;
    switch (gameButton)
    {
        case OEButton_1      : button = SNES::Input::JoypadA;      break;
        case OEButton_2      : button = SNES::Input::JoypadB;      break;
        case OEButton_3      : button = SNES::Input::JoypadX;      break;
        case OEButton_4      : button = SNES::Input::JoypadY;      break;
        case OEButton_5      : button = SNES::Input::JoypadL;     break;
        case OEButton_6      : button = SNES::Input::JoypadR;     break;
        case OEButton_Up     : button = SNES::Input::JoypadUp;     break;
        case OEButton_Down   : button = SNES::Input::JoypadDown;   break;
        case OEButton_Left   : button = SNES::Input::JoypadLeft;   break;
        case OEButton_Right  : button = SNES::Input::JoypadRight;  break;
        case OEButton_Start  : button = SNES::Input::JoypadStart;  break;
        case OEButton_Select : button = SNES::Input::JoypadSelect; break;
    }
    return button;
}

- (void)player:(NSUInteger)thePlayer didPressButton:(OEButton)gameButton;
{
    controlPad[thePlayer - 1] |= [self SNES_buttonMaskForButton:gameButton];
}

- (void)player:(NSUInteger)thePlayer didReleaseButton:(OEButton)gameButton;
{
    controlPad[thePlayer - 1] &= ~[self SNES_buttonMaskForButton:gameButton];
}

- (BOOL) saveStateToFileAtPath: (NSString *) fileName
{
	SNES::system.runtosave();
	serializer state = SNES::system.serialize();
	FILE* state_file = fopen([fileName UTF8String], "w+b");
	long bytes_written = fwrite(state.data(), sizeof(uint8_t), state.size(), state_file);
	if( bytes_written != state.size() )
	{
		NSLog(@"Couldn't write state");
		return NO;
	}
	fclose( state_file );
	return YES;
}

- (BOOL) loadStateFromFileAtPath: (NSString *) fileName
{
	FILE* state_file = fopen([fileName UTF8String], "rb");
	if( !state_file )
	{
		NSLog(@"Could not open state file");
		return NO;
	}
	long file_size;
	
	fseek (state_file , 0 , SEEK_END);
	file_size = ftell (state_file);
	rewind (state_file);
	
	uint8_t* state_buffer = (uint8_t*) malloc (sizeof(uint8_t)*file_size);
	long read_bytes =fread(state_buffer, sizeof(uint8_t), file_size, state_file);
	if( read_bytes != file_size ) 
	{
		NSLog(@"Couldn't read file");
		return NO;
	}

	serializer state(state_buffer,sizeof(uint8_t)*file_size);
	bool loaded = SNES::system.unserialize(state);	
	if( ! loaded )
	{
		NSLog(@"Couldn't unpack state");
		return NO;
	}
	fclose(state_file);
	free(state_buffer);
	return YES;
}


@end
