/*
 Copyright (c) 2012, OpenEmu Team
 

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

#import "BSNESGameCore.h"
#import <OpenEmuBase/OERingBuffer.h>
#import "OESNESSystemResponderClient.h"
#import <OpenGL/gl.h>

#include "libretro.h"

@interface BSNESGameCore () <OESNESSystemResponderClient>
{
    uint32_t *videoBuffer;
    int videoWidth, videoHeight;
    int16_t pad[2][12];
    NSString *romName;
    double sampleRate;
}

@end

NSUInteger BSNESEmulatorValues[] = { RETRO_DEVICE_ID_JOYPAD_UP, RETRO_DEVICE_ID_JOYPAD_DOWN, RETRO_DEVICE_ID_JOYPAD_LEFT, RETRO_DEVICE_ID_JOYPAD_RIGHT, RETRO_DEVICE_ID_JOYPAD_A, RETRO_DEVICE_ID_JOYPAD_B, RETRO_DEVICE_ID_JOYPAD_X, RETRO_DEVICE_ID_JOYPAD_Y, RETRO_DEVICE_ID_JOYPAD_L, RETRO_DEVICE_ID_JOYPAD_R, RETRO_DEVICE_ID_JOYPAD_START, RETRO_DEVICE_ID_JOYPAD_SELECT };
NSString *BSNESEmulatorKeys[] = { @"Joypad@ Up", @"Joypad@ Down", @"Joypad@ Left", @"Joypad@ Right", @"Joypad@ A", @"Joypad@ B", @"Joypad@ X", @"Joypad@ Y", @"Joypad@ L", @"Joypad@ R", @"Joypad@ Start", @"Joypad@ Select"};

@implementation BSNESGameCore

static BSNESGameCore *current;

static void audio_callback(int16_t left, int16_t right)
{
	[[current ringBufferAtIndex:0] write:&left maxLength:2];
    [[current ringBufferAtIndex:0] write:&right maxLength:2];
}

static size_t audio_batch_callback(const int16_t *data, size_t frames)
{
    [[current ringBufferAtIndex:0] write:data maxLength:frames << 2];
    return frames;
}

static void video_callback(const void *data, unsigned width, unsigned height, size_t pitch)
{
    current->videoWidth  = width;
    current->videoHeight = height;
    
    dispatch_queue_t the_queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
    
    dispatch_apply(height, the_queue, ^(size_t y) {
        const uint16_t *src = (uint16_t*)data + y * (pitch >> 1); //pitch is in bytes not pixels
        uint32_t *dst = current->videoBuffer + y * 512;
        
        memcpy(dst, src, sizeof(uint32_t)*width);
    });
}

static void input_poll_callback(void)
{
	//NSLog(@"poll callback");
}

static int16_t input_state_callback(unsigned port, unsigned device, unsigned index, unsigned _id)
{
	if(port == 0 & device == RETRO_DEVICE_JOYPAD)
        return current->pad[0][_id];
    else if(port == 1 & device == RETRO_DEVICE_JOYPAD)
        return current->pad[1][_id];
    
    return 0;
}

static bool environment_callback(unsigned cmd, void *data)
{
    switch(cmd)
    {
        case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY :
        {
            /* Path where BSNES will look for the required coprocessor chips:
             cx4.rom (3,072 bytes)
             dsp1.rom (8,192 bytes)
             dsp1b.rom (8,192 bytes)
             dsp2.rom (8,192 bytes)
             dsp3.rom (8,192 bytes)
             dsp4.rom (8,192 bytes)
             st010.rom (53,248 bytes)
             st011.rom (53,248 bytes)
             st018.rom (163,840 bytes)
             */

            // FIXME: Build a path in a more appropriate place
            NSString *appSupportPath = [NSString pathWithComponents:@[
                                        [NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES) lastObject],
                                        @"OpenEmu", @"BIOS"]];
            
            *(const char **)data = [appSupportPath UTF8String];
            NSLog(@"Environ SYSTEM_DIRECTORY: \"%@\".\n", appSupportPath);
            break;
        }
        default :
            NSLog(@"Environ UNSUPPORTED (#%u).\n", cmd);
            return NO;
    }
    
    return YES;
}


static void loadSaveFile(const char* path, int type)
{
    FILE *file;
    
    file = fopen(path, "rb");
    if(file == NULL) return;
    
    size_t size = retro_get_memory_size(type);
    void *data = retro_get_memory_data(type);
    
    if(size == 0 || !data)
    {
        fclose(file);
        return;
    }
    
    int rc = fread(data, sizeof(uint8_t), size, file);
    if(rc != size)
        NSLog(@"Couldn't load save file: %s.", path);
    else
        NSLog(@"Loaded save file: %s", path);
    
    fclose(file);
}

static void writeSaveFile(const char* path, int type)
{
    size_t size = retro_get_memory_size(type);
    void *data = retro_get_memory_data(type);
    
    if(data != NULL && size > 0)
    {
        FILE *file = fopen(path, "wb");
        if(file != NULL)
        {
            NSLog(@"Saving state %s. Size: %d bytes.", path, (int)size);
            retro_serialize(data, size);
            if(fwrite(data, sizeof(uint8_t), size, file) != size)
                NSLog(@"Did not save state properly.");
            fclose(file);
        }
    }
}

- (oneway void)didPushSNESButton:(OESNESButton)button forPlayer:(NSUInteger)player;
{
    pad[player-1][BSNESEmulatorValues[button]] = 1;
}

- (oneway void)didReleaseSNESButton:(OESNESButton)button forPlayer:(NSUInteger)player;
{
    pad[player-1][BSNESEmulatorValues[button]] = 0;
}

- (id)init
{
    if((self = [super init]))
    {
        videoBuffer = (uint32_t *)malloc(512 * 480 * sizeof(uint32_t));
    }
	
	current = self;
    
	return self;
}

#pragma mark Exectuion

- (void)executeFrame
{
    [self executeFrameSkippingFrame:NO];
}

- (void)executeFrameSkippingFrame:(BOOL)skip
{
    retro_run();
}

- (BOOL)loadFileAtPath:(NSString *)path
{
	memset(pad, 0, sizeof(int16_t) * 24);
    
    uint8_t *data;
    size_t size;
    romName = [path copy];
    
    //load cart, read bytes, get length
    NSData *dataObj = [NSData dataWithContentsOfFile:[romName stringByStandardizingPath]];

    if(dataObj == nil) return NO;

    size = [dataObj length];
    data = (uint8_t *)[dataObj bytes];
    const char *meta = NULL;
    
    //remove copier header, if it exists
    if((size & 0x7fff) == 512) memmove(data, data + 512, size -= 512);
    
    retro_set_environment(environment_callback);
	retro_init();
	
    retro_set_audio_sample(audio_callback);
    retro_set_audio_sample_batch(audio_batch_callback);
    retro_set_video_refresh(video_callback);
    retro_set_input_poll(input_poll_callback);
    retro_set_input_state(input_state_callback);
    
    const char *fullPath = [path UTF8String];

    struct retro_game_info gameInfo = {NULL};
    gameInfo.path = fullPath;
    gameInfo.data = data;
    gameInfo.size = size;
    gameInfo.meta = meta;
    
    if(retro_load_game(&gameInfo))
    {
        NSString *path = romName;
        NSString *extensionlessFilename = [[path lastPathComponent] stringByDeletingPathExtension];
        
        NSString *batterySavesDirectory = [self batterySavesDirectoryPath];
        
        if([batterySavesDirectory length] != 0)
        {
            [[NSFileManager defaultManager] createDirectoryAtPath:batterySavesDirectory withIntermediateDirectories:YES attributes:nil error:NULL];
            
            NSString *filePath = [batterySavesDirectory stringByAppendingPathComponent:[extensionlessFilename stringByAppendingPathExtension:@"sav"]];
            
            loadSaveFile([filePath UTF8String], RETRO_MEMORY_SAVE_RAM);
        }
        
        struct retro_system_av_info avInfo;
        retro_get_system_av_info(&avInfo);
        
        current->frameInterval = avInfo.timing.fps;
        current->sampleRate = avInfo.timing.sample_rate;
        
        //retro_set_controller_port_device(0, RETRO_DEVICE_JOYPAD);
        
        retro_get_region();
        
        retro_run();

        return YES;
    }
    
    return NO;
}

#pragma mark Video

- (const void *)videoBuffer
{
    return videoBuffer;
}

- (OEIntRect)screenRect
{
    return OEIntRectMake(0, 0, current->videoWidth, current->videoHeight);
}

- (OEIntSize)bufferSize
{
    return OEIntSizeMake(512, 480);
}

- (void)setupEmulation
{
}

- (void)resetEmulation
{
    retro_reset();
}

- (void)stopEmulation
{
    NSString *path = romName;
    NSString *extensionlessFilename = [[path lastPathComponent] stringByDeletingPathExtension];
    
    NSString *batterySavesDirectory = [self batterySavesDirectoryPath];
    
    if([batterySavesDirectory length] != 0)
    {
        [[NSFileManager defaultManager] createDirectoryAtPath:batterySavesDirectory withIntermediateDirectories:YES attributes:nil error:NULL];
        
        NSLog(@"Trying to save SRAM");
        
        NSString *filePath = [batterySavesDirectory stringByAppendingPathComponent:[extensionlessFilename stringByAppendingPathExtension:@"sav"]];
        
        writeSaveFile([filePath UTF8String], RETRO_MEMORY_SAVE_RAM);
    }
    
    NSLog(@"snes term");
    retro_unload_game();
    retro_deinit();

    [super stopEmulation];
}

- (void)dealloc
{
    free(videoBuffer);
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


- (double)audioSampleRate
{
    return sampleRate ? sampleRate : 32040.5;
}

- (NSTimeInterval)frameInterval
{
    return frameInterval ? frameInterval : 60;
}

- (NSUInteger)channelCount
{
    return 2;
}

- (BOOL)saveStateToFileAtPath:(NSString *)fileName
{
    int serial_size = retro_serialize_size();
    uint8_t *serial_data = (uint8_t *) malloc(serial_size);
    
    retro_serialize(serial_data, serial_size);
    
    FILE *state_file = fopen([fileName UTF8String], "wb");
    long bytes_written = fwrite(serial_data, sizeof(uint8_t), serial_size, state_file);
    
    free(serial_data);
    
    if(bytes_written != serial_size)
    {
        NSLog(@"Couldn't write state");
        return NO;
    }

    fclose(state_file);
    return YES;
}

- (BOOL)loadStateFromFileAtPath:(NSString *)fileName
{
    FILE *state_file = fopen([fileName UTF8String], "rb");
    if(state_file == NULL)
    {
        NSLog(@"Could not open state file");
        return NO;
    }
    
    int serial_size = retro_serialize_size();
    uint8_t *serial_data = (uint8_t *) malloc(serial_size);
    
    if(!fread(serial_data, sizeof(uint8_t), serial_size, state_file))
    {
        NSLog(@"Couldn't read file");
        return NO;
    }
    fclose(state_file);
    
    if(!retro_unserialize(serial_data, serial_size))
    {
        NSLog(@"Couldn't unpack state");
        return NO;
    }
    
    free(serial_data);
    
    return YES;
}

@end
