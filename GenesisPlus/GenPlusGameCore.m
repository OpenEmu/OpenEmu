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

#import "GenPlusGameCore.h"
#import <IOKit/hid/IOHIDLib.h>
#import <OEGameDocument.h>
#import <OERingBuffer.h>
#import <OpenGL/gl.h>
#import "OEGenesisSystemResponderClient.h"

#include "shared.h"

#define SAMPLERATE 48000

extern void set_config_defaults(void);

void openemu_input_UpdateEmu(void)
{
}

@implementation GenPlusGameCore

@synthesize romPath;

/*
 OpenEmu Core internal functions
 */
- (id)init
{
    self = [super init];
    if(self != nil)
    {
        bufLock = [[NSLock alloc] init];
        videoBuffer = malloc(720 * 576 * 2);
        
        position = 0;
    }
    return self;
}

- (void) dealloc
{
    DLog(@"releasing/deallocating memory");
    free(videoBuffer);    
}

- (void)executeFrame
{
    system_frame(0);
    int size = audio_update();
    for(int i = 0 ; i < size; i++)
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
    
    /* allocate cart.rom here (10 MBytes) */
    cart.rom = malloc(MAXROMSIZE);
    
    if (!cart.rom)
        DLog(@"error allocating");
    
    if( load_rom((char*)[path UTF8String]) )
    {
        /* allocate global work bitmap */
        memset (&bitmap, 0, sizeof (bitmap));
        bitmap.width  = 720;
        bitmap.height = 576;
        bitmap.depth  = 16;
        bitmap.granularity = 2;
        bitmap.pitch = bitmap.width * bitmap.granularity;
        //bitmap.viewport.w = 256;
        //bitmap.viewport.h = 224;
        //bitmap.viewport.x = 0;
        //bitmap.viewport.y = 0;
        bitmap.data = videoBuffer;
        
        /* default system */
        input.system[0] = SYSTEM_MD_GAMEPAD;
        input.system[1] = SYSTEM_MD_GAMEPAD;
        
        float framerate = vdp_pal ? 50.0 : 60.0;
		audio_init(SAMPLERATE, framerate);
        system_init();
        system_reset();
        
        [self setRomPath:path];
		[self loadSram];
    }
    
    return YES;
}

- (NSTimeInterval)frameInterval
{
	return vdp_pal ? 50 : 60;
}

- (void)resetEmulation
{
    system_reset();
}

- (void)stopEmulation
{
    [self saveSram];
    [super stopEmulation];
}

- (IBAction)pauseEmulation:(id)sender
{
}

- (OEIntRect)screenRect
{
    return OERectMake(bitmap.viewport.x, bitmap.viewport.y, bitmap.viewport.w, bitmap.viewport.h);
}

- (OEIntSize)bufferSize
{
    return OESizeMake(bitmap.width, bitmap.height);
}

- (const void *)videoBuffer
{
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

- (double)frameSampleRate
{
    return SAMPLERATE;
}

- (NSUInteger)channelCount
{
    return 2;
}

NSUInteger GenesisControlValues[] = { INPUT_A, INPUT_B, INPUT_C, INPUT_X, INPUT_Y, INPUT_Z, INPUT_UP, INPUT_DOWN, INPUT_LEFT, INPUT_RIGHT, INPUT_START, INPUT_MODE };

- (BOOL)shouldPauseForButton:(NSInteger)button
{
    return NO;
}

- (void)didPushGenesisButton:(OEGenesisButton)button forPlayer:(NSUInteger)player;
{
    input.pad[player - 1] |=  GenesisControlValues[button];
}

- (void)didReleaseGenesisButton:(OEGenesisButton)button forPlayer:(NSUInteger)player;
{
    input.pad[player - 1] &= ~GenesisControlValues[button];
}

- (BOOL)saveStateToFileAtPath:(NSString *)fileName
{
    NSMutableData *data = [[NSMutableData alloc] initWithLength:STATE_SIZE];
    
    NSInteger bytesCount = state_save([data mutableBytes]);
    
    return bytesCount > 0 && [[[NSData alloc] initWithBytesNoCopy:[data mutableBytes] length:bytesCount freeWhenDone:NO] writeToFile:fileName atomically:NO];
}

- (BOOL)loadStateFromFileAtPath:(NSString *)fileName
{
    NSData *loaded = [[NSData alloc] initWithContentsOfFile:fileName];
    
    return loaded != nil && state_load([loaded bytes], [loaded length]) == 1;
}

- (void)saveSram
{
	if (sram.on)
	{
		NSString *path = self->romPath;
		
		NSString *extensionlessFilename = [[path lastPathComponent] stringByDeletingPathExtension];
		NSString *batterySavesDirectory = [self batterySavesDirectoryPath];
		[[NSFileManager defaultManager] createDirectoryAtPath:batterySavesDirectory withIntermediateDirectories:YES attributes:nil error:NULL];
		
		NSData* theData;
		NSString* filePath;
		
		filePath = [batterySavesDirectory stringByAppendingPathComponent:[extensionlessFilename stringByAppendingPathExtension:@"sav"]];
		theData = [NSData dataWithBytes:sram.sram length:0x10000];
		[theData writeToFile:filePath atomically:YES];
	}
}

- (void)loadSram
{
	NSFileManager *fileManager = [NSFileManager defaultManager];
    
    NSString *extensionlessFilename = [[self->romPath lastPathComponent] stringByDeletingPathExtension];
    NSString *batterySavesDirectory = [self batterySavesDirectoryPath];
    [[NSFileManager defaultManager] createDirectoryAtPath:batterySavesDirectory withIntermediateDirectories:YES attributes:nil error:NULL];
    
    NSData* theData;
    NSString* filePath;
	
	filePath = [batterySavesDirectory stringByAppendingPathComponent:[extensionlessFilename stringByAppendingPathExtension:@"sav"]];
	if([fileManager fileExistsAtPath:filePath])
	{
		NSLog(@"SRAM found");
		theData = [NSData dataWithContentsOfFile:filePath];
		memcpy(sram.sram, [theData bytes], 0x10000);
		sram.crc = crc32(0, sram.sram, 0x10000);
	}
}

@end
