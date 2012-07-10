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
#import <OERingBuffer.h>
#import "OEGenesisSystemResponderClient.h"
#import <OpenGL/gl.h>

#include "libretro.h"

@interface GenPlusGameCore () <OEGenesisSystemResponderClient>
@end
//NSUInteger GenesisControlValues[] = { INPUT_UP, INPUT_DOWN, INPUT_LEFT, INPUT_RIGHT, INPUT_A, INPUT_B, INPUT_C, INPUT_X, INPUT_Y, INPUT_Z, INPUT_START, INPUT_MODE };
NSUInteger GenesisEmulatorValues[] = { RETRO_DEVICE_ID_JOYPAD_UP, RETRO_DEVICE_ID_JOYPAD_DOWN, RETRO_DEVICE_ID_JOYPAD_LEFT, RETRO_DEVICE_ID_JOYPAD_RIGHT, RETRO_DEVICE_ID_JOYPAD_Y, RETRO_DEVICE_ID_JOYPAD_B, RETRO_DEVICE_ID_JOYPAD_A, RETRO_DEVICE_ID_JOYPAD_L, RETRO_DEVICE_ID_JOYPAD_X, RETRO_DEVICE_ID_JOYPAD_R, RETRO_DEVICE_ID_JOYPAD_START, RETRO_DEVICE_ID_JOYPAD_SELECT };
NSString *GenesisEmulatorKeys[] = { @"Joypad@ Up", @"Joypad@ Down", @"Joypad@ Left", @"Joypad@ Right", @"Joypad@ Triangle", @"Joypad@ Circle", @"Joypad@ Cross", @"Joypad@ Square", @"Joypad@ L1", @"Joypad@ L2", @"Joypad@ L3", @"Joypad@ R1", @"Joypad@ R2", @"Joypad@ R3", @"Joypad@ Start", @"Joypad@ Select"};

GenPlusGameCore *current;
@implementation GenPlusGameCore

static void audio_callback(int16_t left, int16_t right)
{
	[[current ringBufferAtIndex:0] write:&left maxLength:2];
    [[current ringBufferAtIndex:0] write:&right maxLength:2];
}

static size_t audio_batch_callback(const int16_t *data, size_t frames){
    [[current ringBufferAtIndex:0] write:data maxLength:frames << 2];
    return frames;
}

static void video_callback(const void *data, unsigned width, unsigned height, size_t pitch)
{
    current->videoWidth  = width;
    current->videoHeight = height;
    //NSLog(@"width: %u height: %u pitch: %zu", width, height, pitch);
    
    dispatch_queue_t the_queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
    
    // TODO opencl CPU device?
    dispatch_apply(height, the_queue, ^(size_t y){
        const uint16_t *src = (uint16_t*)data + y * (pitch >> 1); //pitch is in bytes not pixels
        //uint16_t *dst = current->videoBuffer + y * current->videoWidth;
        uint16_t *dst = current->videoBuffer + y * 720;
        
        for (int x = 0; x < current->videoWidth; x++) {
            dst[x] = src[x];
        }
    });
}

static void input_poll_callback(void)
{
	//NSLog(@"poll callback");
}

static int16_t input_state_callback(unsigned port, unsigned device, unsigned index, unsigned id)
{
    //NSLog(@"polled input: port: %d device: %d id: %d", port, device, id);
    
    if (port == 0 & device == RETRO_DEVICE_JOYPAD) {
        return current->pad[0][id];
    }
    else if(port == 1 & device == RETRO_DEVICE_JOYPAD) {
        //return current->pad[1][id];
    }
    
    return 0;
}

static bool environment_callback(unsigned cmd, void *data)
{
    switch (cmd)
    {
            /*case RETRO_ENVIRONMENT_GET_OVERSCAN:
             *(bool*)data = !g_settings.video.crop_overscan;
             NSLog(@"Environ GET_OVERSCAN: %u\n", (unsigned)!g_settings.video.crop_overscan);
             break;
             
             case RETRO_ENVIRONMENT_GET_CAN_DUPE:
             #ifdef HAVE_FFMPEG
             *(bool*)data = true;
             NSLog(@"Environ GET_CAN_DUPE: true\n");
             #else
             *(bool*)data = false;
             NSLog(@"Environ GET_CAN_DUPE: false\n");
             #endif
             break;*/
            
        case RETRO_ENVIRONMENT_GET_VARIABLE:
        {
            struct retro_variable *var = (struct retro_variable*)data;
            if (var->key)
            {
                // Split string has '\0' delimiters so we have to find the position in original string,
                // then pass the corresponding offset into the split string.
                const char *key = strstr(current->systemEnvironment, var->key);
                size_t key_len = strlen(var->key);
                if (key && key[key_len] == '=')
                {
                    ptrdiff_t offset = key - current->systemEnvironment;
                    var->value = current->systemEnvironmentSplit[offset + key_len + 1];
                }
                else
                    var->value = NULL;
            }
            else
                var->value = current->systemEnvironment;
            
            NSLog(@"Environ GET_VARIABLE: %s=%s\n",
                  var->key ? var->key : "null",
                  var->value ? var->value : "null");
            
            break;
        }
            
        case RETRO_ENVIRONMENT_SET_VARIABLES:
        {
            NSLog(@"Environ SET_VARIABLES:\n");
            NSLog(@"=======================\n");
            const struct retro_variable *vars = (const struct retro_variable*)data;
            while (vars->key)
            {
                NSLog(@"\t%s :: %s\n",
                      vars->key,
                      vars->value ? vars->value : "N/A");
                
                vars++;
            }
            NSLog(@"=======================\n");
            break;
        }
            
            /*case RETRO_ENVIRONMENT_SET_MESSAGE:
             {
             const struct retro_message *msg = (const struct retro_message*)data;
             NSLog(@"Environ SET_MESSAGE: %s\n", msg->msg);
             if (g_extern.msg_queue)
             msg_queue_push(g_extern.msg_queue, msg->msg, 1, msg->frames);
             break;
             }
             
             case RETRO_ENVIRONMENT_SET_ROTATION:
             {
             unsigned rotation = *(const unsigned*)data;
             NSLog(@"Environ SET_ROTATION: %u\n", rotation);
             if (!g_settings.video.allow_rotate)
             break;
             
             g_extern.system.rotation = rotation;
             
             if (driver.video && driver.video->set_rotation)
             {
             if (driver.video_data)
             video_set_rotation_func(rotation);
             }
             else
             return false;
             break;
             }*/
            
        case RETRO_ENVIRONMENT_SHUTDOWN:
            //g_extern.system.shutdown = true;
            break;
            
        case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY:
        {
            NSString *appSupportPath = [[[[NSHomeDirectory() stringByAppendingPathComponent:@"Library"]
                                          stringByAppendingPathComponent:@"Application Support"]
                                         stringByAppendingPathComponent:@"OpenEmu"]
                                        stringByAppendingPathComponent:@"BIOS"];
            
            *(const char **)data = [appSupportPath cStringUsingEncoding:NSUTF8StringEncoding] ? [appSupportPath cStringUsingEncoding:NSUTF8StringEncoding] : NULL;
            NSLog(@"Environ SYSTEM_DIRECTORY: \"%@\".\n", appSupportPath);
            break;
        }
            
        case RETRO_ENVIRONMENT_GET_SAVES_DIRECTORY:
        {
            NSString *batterySavesDirectory = current.batterySavesDirectoryPath;
            
            *(const char **)data = [batterySavesDirectory cStringUsingEncoding:NSUTF8StringEncoding];
            NSLog(@"Environ SAVES_DIRECTORY: \"%@\".\n", batterySavesDirectory);
            break;
        }
        default:
            NSLog(@"Environ UNSUPPORTED (#%u).\n", cmd);
            return false;
    }
    
    return true;
}


static void loadSaveFile(const char* path, int type)
{
    FILE *file;
    
    file = fopen(path, "rb");
    if ( !file )
    {
        return;
    }
    
    size_t size = retro_get_memory_size(type);
    void *data = retro_get_memory_data(type);
    
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
    size_t size = retro_get_memory_size(type);
    void *data = retro_get_memory_data(type);
    
    if ( data && size > 0 )
    {
        FILE *file = fopen(path, "wb");
        if ( file != NULL )
        {
            NSLog(@"Saving state %s. Size: %d bytes.", path, (int)size);
            retro_serialize(data, size);
            if ( fwrite(data, sizeof(uint8_t), size, file) != size )
                NSLog(@"Did not save state properly.");
            fclose(file);
        }
    }
}

-(BOOL)rendersToOpenGL;
{
    return NO;
}

- (oneway void)didPushGenesisButton:(OEGenesisButton)button forPlayer:(NSUInteger)player;
{
    pad[player-1][GenesisEmulatorValues[button]] = 1;
}

- (oneway void)didReleaseGenesisButton:(OEGenesisButton)button forPlayer:(NSUInteger)player;
{
    pad[player-1][GenesisEmulatorValues[button]] = 0;
}

- (id)init
{
	self = [super init];
    if(self != nil)
    {
        if(videoBuffer) 
            free(videoBuffer);
        videoBuffer = (uint16_t*)malloc(720 * 576 * 2);
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
    retro_run();
}

- (BOOL)loadFileAtPath: (NSString*) path
{
	memset(pad, 0, sizeof(int16_t) * 10);
    
    const void *data;
    size_t size;
    romName = [path copy];
    
    //load cart, read bytes, get length
    NSData* dataObj = [NSData dataWithContentsOfFile:[romName stringByStandardizingPath]];
    if(dataObj == nil) return false;
    size = [dataObj length];
    data = (uint8_t*)[dataObj bytes];
    const char *meta = NULL;
    
    //memory.copy(data, size);
    retro_set_environment(environment_callback);
	retro_init();
	
    retro_set_audio_sample(audio_callback);
    retro_set_audio_sample_batch(audio_batch_callback);
    retro_set_video_refresh(video_callback);
    retro_set_input_poll(input_poll_callback);
    retro_set_input_state(input_state_callback);
    
    
    const char *fullPath = [path UTF8String];
    //*(const char**)data = [current->romName cStringUsingEncoding:NSUTF8StringEncoding];
    struct retro_game_info info = {NULL};
    //info.path = *(const char**)path;
    info.path = fullPath;
    info.data = data;
    info.size = size;
    info.meta = meta;
    
    if(retro_load_game(&info))
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
        
        struct retro_system_av_info info;
        retro_get_system_av_info(&info);
        
        //retro_set_controller_port_device(SNES_PORT_1, RETRO_DEVICE_JOYPAD);
        
        retro_get_region();
        
        retro_run();
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
    return OERectMake(0, 0, current->videoWidth, current->videoHeight);
}

- (OEIntSize)bufferSize
{
    return OESizeMake(720, 576);
    //return OESizeMake(current->videoWidth, current->videoHeight);
}
/*
 - (void)setupEmulation
 {
 if(soundBuffer)
 free(soundBuffer);
 soundBuffer = (UInt16*)malloc(SIZESOUNDBUFFER* sizeof(UInt16));
 memset(soundBuffer, 0, SIZESOUNDBUFFER*sizeof(UInt16));
 }
 */
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
    //[super dealloc];
}

- (GLenum)pixelFormat
{
    //return GL_RGB;
    return GL_BGRA;
}

- (GLenum)pixelType
{
    //return GL_UNSIGNED_SHORT_5_6_5;
    return GL_UNSIGNED_SHORT_1_5_5_5_REV;
}

- (GLenum)internalPixelFormat
{
    return GL_RGB5;
}

- (double)audioSampleRate
{
    return sampleRate ? sampleRate : 48000;
}

- (NSTimeInterval)frameInterval
{
    return frameInterval ? frameInterval : 60;
    //frameInterval = vdp_pal ? 53203424./896040. : 53693175./896040.;
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




/*
#import "GenPlusGameCore.h"
#import <IOKit/hid/IOHIDLib.h>
#import <OpenEmuBase/OERingBuffer.h>
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
    //system_frame(0);
    system_frame_gen(0);
    int size = audio_update(soundbuffer);
    for(int i = 0 ; i < size; i++)
    {
        //[[self ringBufferAtIndex:0] write:&snd.buffer[0][i] maxLength:2];
        //[[self ringBufferAtIndex:0] write:&snd.pcm.buffer[1][i] maxLength:2];
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
    
    // allocate cart.rom here (10 MBytes)
    //cart.rom = malloc(MAXROMSIZE);
    
    if (!cart.rom)
        DLog(@"error allocating");
    
    if( load_rom((char*)[path UTF8String]) )
    {
        // allocate global work bitmap
        memset (&bitmap, 0, sizeof (bitmap));
        bitmap.width  = 720;
        bitmap.height = 576;
        //bitmap.depth  = 16;
        //bitmap.granularity = 2;
        //bitmap.pitch = bitmap.width * bitmap.granularity;
        bitmap.pitch = bitmap.width * sizeof(uint16_t);
        //bitmap.viewport.w = 256;
        //bitmap.viewport.h = 224;
        //bitmap.viewport.x = 0;
        //bitmap.viewport.y = 0;
        bitmap.data = videoBuffer;
        
        // default system
        input.system[0] = SYSTEM_MD_GAMEPAD;
        input.system[1] = SYSTEM_MD_GAMEPAD;
        
        frameInterval = vdp_pal ? 53203424./896040. : 53693175./896040.; // from sound_init()
		audio_init(SAMPLERATE, frameInterval);
        system_init();
        system_reset();
        
        [self setRomPath:path];
		[self loadSram];
    }
    
    return YES;
}

- (NSTimeInterval)frameInterval
{
	return frameInterval ? frameInterval : 60;
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

- (double)audioSampleRate
{
    return SAMPLERATE;
}

- (NSUInteger)channelCount
{
    return 2;
}

NSUInteger GenesisControlValues[] = { INPUT_UP, INPUT_DOWN, INPUT_LEFT, INPUT_RIGHT, INPUT_A, INPUT_B, INPUT_C, INPUT_X, INPUT_Y, INPUT_Z, INPUT_START, INPUT_MODE };

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
    
    //return loaded != nil && state_load([loaded bytes], [loaded length]) == 1;
    return;
}

- (void)saveSram
{
	if(sram.on)
	{
		NSString *extensionlessFilename = [[romPath lastPathComponent] stringByDeletingPathExtension];
		NSString *batterySavesDirectory = [self batterySavesDirectoryPath];
		[[NSFileManager defaultManager] createDirectoryAtPath:batterySavesDirectory withIntermediateDirectories:YES attributes:nil error:NULL];
		
		NSString *filePath = [batterySavesDirectory stringByAppendingPathComponent:[extensionlessFilename stringByAppendingPathExtension:@"sav"]];
		NSData   *theData  = [NSData dataWithBytes:sram.sram length:0x10000];
		[theData writeToFile:filePath atomically:YES];
	}
}

- (void)loadSram
{
	NSFileManager *fileManager = [NSFileManager defaultManager];
    
    NSString *extensionlessFilename = [[romPath lastPathComponent] stringByDeletingPathExtension];
    NSString *batterySavesDirectory = [self batterySavesDirectoryPath];
    [[NSFileManager defaultManager] createDirectoryAtPath:batterySavesDirectory withIntermediateDirectories:YES attributes:nil error:NULL];
    
	NSString *filePath = [batterySavesDirectory stringByAppendingPathComponent:[extensionlessFilename stringByAppendingPathExtension:@"sav"]];
	if([fileManager fileExistsAtPath:filePath])
	{
		NSLog(@"SRAM found");
		NSData *data = [NSData dataWithContentsOfFile:filePath];
		memcpy(sram.sram, [data bytes], 0x10000);
		sram.crc = crc32(0, sram.sram, 0x10000);
	}
}

@end
*/
