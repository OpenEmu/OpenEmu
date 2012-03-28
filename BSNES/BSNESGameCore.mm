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

#import "BSNESGameCore.h"
#import <OpenEmuBase/OERingBuffer.h>
#import "OESNESSystemResponderClient.h"
#import <OpenGL/gl.h>

#include "libsnes.hpp"

@interface BSNESGameCore () <OESNESSystemResponderClient>
@end

NSUInteger BSNESEmulatorValues[] = { SNES_DEVICE_ID_JOYPAD_A, SNES_DEVICE_ID_JOYPAD_B, SNES_DEVICE_ID_JOYPAD_X, SNES_DEVICE_ID_JOYPAD_Y, SNES_DEVICE_ID_JOYPAD_UP, SNES_DEVICE_ID_JOYPAD_DOWN, SNES_DEVICE_ID_JOYPAD_LEFT, SNES_DEVICE_ID_JOYPAD_RIGHT, SNES_DEVICE_ID_JOYPAD_START, SNES_DEVICE_ID_JOYPAD_SELECT, SNES_DEVICE_ID_JOYPAD_L, SNES_DEVICE_ID_JOYPAD_R };
NSString *BSNESEmulatorKeys[] = { @"Joypad@ A", @"Joypad@ B", @"Joypad@ X", @"Joypad@ Y", @"Joypad@ Up", @"Joypad@ Down", @"Joypad@ Left", @"Joypad@ Right", @"Joypad@ Start", @"Joypad@ Select", @"Joypad@ L", @"Joypad@ R"};

BSNESGameCore *current;
@implementation BSNESGameCore

static uint16_t conv555Rto565(uint16_t p)
{
    unsigned r, g, b;
    
    b = (p >> 10);
    g = (p >> 5) & 0x1f;
    r = p & 0x1f;
    
    // 5 to 6 bit
    g = (g << 1) + (g >> 4);
    
    return r | (g << 5) | (b << 11);
}

//BSNES callbacks
static void audio_callback(uint16_t left, uint16_t right)
{
	[[current ringBufferAtIndex:0] write:&left maxLength:2];
    [[current ringBufferAtIndex:0] write:&right maxLength:2];
}

static void video_callback(const uint16_t *data, unsigned width, unsigned height)
{
    // Normally our pitch is 2048 bytes.
    int stride = 1024;
    // If we have an interlaced mode, pitch is 1024 bytes.
    if ( height == 448 || height == 478 )
        stride = 512;

    current->videoWidth  = width;
    current->videoHeight = height;
    
    dispatch_queue_t the_queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);

    // TODO opencl CPU device?
    dispatch_apply(height, the_queue, ^(size_t y){
        const uint16_t *src = data + y * stride;
        uint16_t *dst = current->videoBuffer + y * 512;

        for (int x = 0; x < width; x++) {
            dst[x] = conv555Rto565(src[x]);
        }
    });
}

static bool environment_callback(unsigned env, void *data)
{
    switch (env) {
        case SNES_ENVIRONMENT_SET_TIMING:
        {
            snes_system_timing *t = (snes_system_timing*)data;
            current->frameInterval = t->fps;
            current->sampleRate    = t->sample_rate;
            return true;
        }
        default:
            return false; // TODO implement the other stuff?
    }
}

static void input_poll_callback(void)
{
	//NSLog(@"poll callback");
}

static int16_t input_state_callback(bool port, unsigned device, unsigned index, unsigned id)
{
    //NSLog(@"polled input: port: %d device: %d id: %d", port, device, id);
    
	if (port == SNES_PORT_1 & device == SNES_DEVICE_JOYPAD) {
        return current->pad[0][id];
    }
    else if(port == SNES_PORT_2 & device == SNES_DEVICE_JOYPAD) {
        return current->pad[1][id];
    }
    
    return 0;
}

static void loadSaveFile(const char* path, int type)
{
    FILE *file;
    
    file = fopen(path, "rb");
    if ( !file )
    {
        return;
    }
    
    size_t size = snes_get_memory_size(type);
    uint8_t *data = snes_get_memory_data(type);
    
    if (size == 0 || !data)
    {
        fclose(file);
        return;
    }
    
    int rc = fread(data, sizeof(uint8_t), size, file);
    if ( rc != size )
    {
        NSLog(@"Couldn't load save file.");
    }
    
    NSLog(@"Loaded save file: %s", path);
    
    fclose(file);
}

static void writeSaveFile(const char* path, int type)
{
    size_t size = snes_get_memory_size(type);
    uint8_t *data = snes_get_memory_data(type);
    
    if ( data && size > 0 )
    {
        FILE *file = fopen(path, "wb");
        if ( file != NULL )
        {
            NSLog(@"Saving state %s. Size: %d bytes.", path, (int)size);
            if ( fwrite(data, sizeof(uint8_t), size, file) != size )
                NSLog(@"Did not save state properly.");
            fclose(file);
        }
    }
}

- (oneway void)didPushSNESButton:(OESNESButton)button forPlayer:(NSUInteger)player;
{
    pad[player-1][BSNESEmulatorValues[button]] = 0xFFFF;
}

- (oneway void)didReleaseSNESButton:(OESNESButton)button forPlayer:(NSUInteger)player;
{
    pad[player-1][BSNESEmulatorValues[button]] = 0;
}

- (id)init
{
	self = [super init];
    if(self != nil)
    {
        if(videoBuffer) 
            free(videoBuffer);
        videoBuffer = (uint16_t*)malloc(512 * 478 * 2);
    }
	
	current = self;
    
	return self;
}

#pragma mark Exectuion

- (void)executeFrame
{
    [self executeFrameSkippingFrame:NO];
}

- (void)executeFrameSkippingFrame: (BOOL) skip
{
    snes_run();
}

- (BOOL)loadFileAtPath: (NSString*) path
{
	memset(pad, 0, sizeof(int16_t) * 24);

    uint8_t *data;
    unsigned size;
    romName = [path copy];
    
    //load cart, read bytes, get length
    NSData* dataObj = [NSData dataWithContentsOfFile:[romName stringByStandardizingPath]];
    if(dataObj == nil) return false;
    size = [dataObj length];
    data = (uint8_t*)[dataObj bytes];
    
    //remove copier header, if it exists
    if((size & 0x7fff) == 512) memmove(data, data + 512, size -= 512);
    
    //memory.copy(data, size);
    
	snes_init();
	
    snes_set_audio_sample(audio_callback);
    snes_set_video_refresh(video_callback);
    snes_set_input_poll(input_poll_callback);
    snes_set_input_state(input_state_callback);
	snes_set_environment(environment_callback);
    
    if(snes_load_cartridge_normal(NULL, data, size))
    {
        NSString *path = romName;
        NSString *extensionlessFilename = [[path lastPathComponent] stringByDeletingPathExtension];
        
        NSString *batterySavesDirectory = [self batterySavesDirectoryPath];
        
        //        if((batterySavesDirectory != nil) && ![batterySavesDirectory isEqualToString:@""])
        if([batterySavesDirectory length] != 0)
        {
            [[NSFileManager defaultManager] createDirectoryAtPath:batterySavesDirectory withIntermediateDirectories:YES attributes:nil error:NULL];
            
            NSString *filePath = [batterySavesDirectory stringByAppendingPathComponent:[extensionlessFilename stringByAppendingPathExtension:@"sav"]];
            
            loadSaveFile([filePath UTF8String], SNES_MEMORY_CARTRIDGE_RAM);
        }
        
        snes_set_controller_port_device(SNES_PORT_1, SNES_DEVICE_JOYPAD);
        snes_set_controller_port_device(SNES_PORT_2, SNES_DEVICE_JOYPAD);
        
        snes_get_region();
            
        snes_run();
    }

    return YES;
}

#pragma mark Video
- (const void *)videoBuffer
{
    return videoBuffer;
}

- (OEIntRect)screenRect
{
    // hope this handles hires :/
    return OERectMake(0, 0, videoWidth, videoHeight);
}

- (OEIntSize)bufferSize
{
    return OESizeMake(512, 478);
}

- (void)setupEmulation
{
}

- (void)resetEmulation
{
    snes_reset();
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
        
        writeSaveFile([filePath UTF8String], SNES_MEMORY_CARTRIDGE_RAM);
    }
    
    NSLog(@"snes term");
    //snes_unload_cartridge();
    snes_term();
    [super stopEmulation];
}

- (void)dealloc
{
    free(videoBuffer);
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
    int serial_size = snes_serialize_size();
    uint8_t *serial_data = (uint8_t *) malloc(serial_size);
    
    snes_serialize(serial_data, serial_size);

    FILE *state_file = fopen([fileName UTF8String], "wb");
    long bytes_written = fwrite(serial_data, sizeof(uint8_t), serial_size, state_file);
    
    free(serial_data);
    
    if( bytes_written != serial_size )
    {
        NSLog(@"Couldn't write state");
        return NO;
    }
    fclose( state_file );
    return YES;
}

- (BOOL)loadStateFromFileAtPath:(NSString *)fileName
{
    FILE *state_file = fopen([fileName UTF8String], "rb");
    if( !state_file )
    {
        NSLog(@"Could not open state file");
        return NO;
    }
    
    int serial_size = snes_serialize_size();
    uint8_t *serial_data = (uint8_t *) malloc(serial_size);
    
    if(!fread(serial_data, sizeof(uint8_t), serial_size, state_file))
    {
        NSLog(@"Couldn't read file");
        return NO;
    }
    fclose(state_file);
    
    if(!snes_unserialize(serial_data, serial_size))
    {
        NSLog(@"Couldn't unpack state");
        return NO;
    }
    
    free(serial_data);

    return YES;
}

@end
