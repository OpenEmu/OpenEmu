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

#import "NGPGameEmu.h"
#include "neopop.h"
#include <sys/stat.h>
#import <OpenGL/gl.h>
#import "OENGPSystemResponderClient.h"
#import <OERingBuffer.h>

enum {
    NGPButtonUp     = 0x01,
    NGPButtonDown   = 0x02,
    NGPButtonLeft   = 0x04,
    NGPButtonRight  = 0x08,
    NGPButtonA      = 0x10,
    NGPButtonB      = 0x20,
    NGPButtonOption = 0x40
};

#define JOYPORT_ADDR    0x6F82

#define SAMPLERATE 44100
#define SAMPLEFRAME 735
#define SIZESOUNDBUFFER SAMPLEFRAME*16

_u8 system_frameskip_key;

BOOL system_rom_load(const char *filename);

@interface NGPGameEmu () <OENGPSystemResponderClient>
@end

@implementation NGPGameEmu

static NSString *gPathToFile = NULL;
int *gBlit = NULL;
uint16_t *chipBuf;
uint8_t *dacBuf;
uint8_t inputState;
static OERingBuffer *chipBuffer;
static OERingBuffer *dacBuffer;

- (void)didPushNGPButton:(OENGPButton)button;
{
    switch (button)
    {
        case OENGPButtonUp:
            inputState |= NGPButtonUp;
            break;
        case OENGPButtonDown:
            inputState |= NGPButtonDown;
            break;
        case OENGPButtonLeft:
            inputState |= NGPButtonLeft;
            break;
        case OENGPButtonRight:
            inputState |= NGPButtonRight;
            break;
        case OENGPButtonA:
            inputState |= NGPButtonA;
            break;
        case OENGPButtonB:
            inputState |= NGPButtonB;
            break;
        case OENGPButtonOption:
            inputState |= NGPButtonOption;
            break;
        default:
            break;
    }
}

- (void)didReleaseNGPButton:(OENGPButton)button;
{
    switch (button)
    {
        case OENGPButtonUp:
            inputState &= ~NGPButtonUp;
            break;
        case OENGPButtonDown:
            inputState &= ~NGPButtonDown;
            break;
        case OENGPButtonLeft:
            inputState &= ~NGPButtonLeft;
            break;
        case OENGPButtonRight:
            inputState &= ~NGPButtonRight;
            break;
        case OENGPButtonA:
            inputState &= ~NGPButtonA;
            break;
        case OENGPButtonB:
            inputState &= ~NGPButtonB;
            break;
        case OENGPButtonOption:
            inputState &= ~NGPButtonOption;
            break;
        default:
            break;
    }
}

- (void) executeFrame
{
    [bufLock lock];
    while(!blit)
    {
        emulate();
    }
    blit = 0;
    
    [bufLock unlock];
}

- (void)stopEmulation
{
    rom_unload();
    [super stopEmulation];
}

- (id)init
{
    self = [super init];
    if(self != nil)
    {
        chipBuf = calloc(SAMPLEFRAME * 2, sizeof(uint16_t));
        dacBuf = calloc(DAC_FREQUENCY / 60, sizeof(uint8_t));
        soundLock = [[NSLock alloc] init];
        bufLock = [[NSLock alloc] init];
        gPathToFile = pathToFile;
        gBlit = &blit;
        chipBuffer = [self ringBufferAtIndex:0];
        dacBuffer = [self ringBufferAtIndex:1];
    }
    return self;
}

- (void) dealloc
{
    free(chipBuf);
    free(dacBuf);
}


- (void)setupEmulation
{
    reset();
    sound_init(SAMPLERATE);
    NSLog(@"Setup done for neopop");
}

- (BOOL)loadFileAtPath:(NSString*)path
{
    pathToFile = [[path stringByDeletingLastPathComponent] copy];
    gPathToFile = pathToFile;
    
    /* auto-select colour mode */
    system_colour = COLOURMODE_AUTO;
    /* default to English as language for now */
    language_english = YES;
    /* default to sound on */
    mute = NO;
    /* show every frame */
    system_frameskip_key = 1;
    
    /* Fill BIOS buffer */
    if (!bios_install())
    {
        NSLog(@"Cannot install NGP bios");
        return NO;
    }
    
    /* Remove me later */
    mute = YES;
    
    return system_rom_load([path UTF8String]);
}

- (OEIntSize)bufferSize
{
    return OESizeMake(SCREEN_WIDTH, SCREEN_HEIGHT);
}

- (const void*)videoBuffer
{
    //NSLog(@"buffer: %x %x %x\n", cfb[540], cfb[541], cfb[542]);
    return cfb;
}

- (GLenum)pixelFormat
{
    return GL_RGBA;
}

- (GLenum)pixelType
{
    return GL_UNSIGNED_SHORT_4_4_4_4_REV;
}

- (GLenum)internalPixelFormat
{
    return GL_RGB4;
}

- (NSUInteger)soundBufferSizeForBuffer:(NSUInteger)buffer;
{
    return SIZESOUNDBUFFER;
}

- (NSUInteger)frameSampleCountForBuffer:(NSUInteger)buffer;
{
    return buffer == 0 ? SAMPLEFRAME : DAC_FREQUENCY / 60;
}

- (NSUInteger)frameSampleRateForBuffer:(NSUInteger)buffer;
{
    return buffer == 0 ? SAMPLERATE : DAC_FREQUENCY;
}

- (NSUInteger)channelCountForBuffer:(NSUInteger)buffer;
{
    return buffer == 0 ? 2 : 1;
}

- (NSUInteger)soundBufferCount;
{
    return 2;
}

- (BOOL)saveStateToFileAtPath:(NSString *)fileName
{
    state_store([fileName UTF8String]);
    return YES;
}

- (BOOL)loadStateFromFileAtPath:(NSString *)fileName
{
    state_restore([fileName UTF8String]);
    return YES;
}

/* NeoPop callbacks and internal functions */

static BOOL rom_load(const char *filename)
{
    struct stat st;
    
    if (stat(filename, &st) == -1) {
        NSLog(@"Error finding NGP rom");
        return NO;
    }
    
    rom.length = st.st_size;
    rom.data = (unsigned char *)calloc(rom.length, 1);
    
    if (system_io_rom_read(filename, rom.data, rom.length))
    {
        NSLog(@"Rom loaded, rom data %x", rom.data[128]);
        return YES;
    }
    
    NSLog(@"Error reading NGP rom");
    free(rom.data);
    rom.data = NULL;
    return NO;
}

BOOL system_rom_load(const char *filename)
{
    const char *fn = "";
    
    /* Remove old ROM from memory */
    rom_unload();
    
    if(!rom_load(filename)) return NO;
    
    memset(rom.filename, 0, sizeof(rom.filename));
    if ((fn=strrchr(filename, '/')) == NULL)
        fn = filename;
    
    /* don't copy extension */
    strncpy(rom.filename, fn, min(sizeof(rom.filename), strlen(fn)-4));
    NSLog(@"Rom name: %s %s\n", rom.filename, rom.name);
    rom_loaded();
    
    return YES;
}


void system_VBL(void)
{
    *gBlit = 1;
    sound_update(chipBuf, SAMPLEFRAME * 4);
    [chipBuffer write:(uint8_t*)chipBuf maxLength:SAMPLEFRAME * 4];

    dac_update(dacBuf, DAC_FREQUENCY/60);
    for (int i = 0; i < DAC_FREQUENCY/60; ++i)
    {
        float floatSample = ((float)dacBuf[i]) / UINT8_MAX;
        chipBuf[i] = (int16_t)(floatSample * UINT16_MAX - INT16_MAX);
    }
    [dacBuffer write:(uint8_t*)chipBuf maxLength:(DAC_FREQUENCY/60) * 2];
    
    ram[JOYPORT_ADDR] = inputState;
}

void system_sound_chipreset(void)
{
    sound_init(SAMPLERATE);
}

void system_sound_silence(void)
{
    memset(chipBuf, 0, sizeof(uint16_t) * SAMPLEFRAME * 2);
    memset(dacBuf, 0, sizeof(uint8_t) * (DAC_FREQUENCY / 60));
}

BOOL system_comms_read(_u8* buffer)
{
    return NO;
}

void system_comms_write(_u8 data)
{
}

BOOL system_comms_poll(_u8* buffer)
{
    return NO;
}

void system_message(char *vaMessage, ...)
{
    NSLog(@"Message from Neopop core %s", vaMessage);
}

char *system_make_file_name(const char *dir, const char *ext, int writing)
{
    NSLog(@"make file: %s", dir);
    char *fname, *name, *home, *p;
    int len;
    
    name = rom.filename;
    len = strlen(dir)+strlen(name)+strlen(ext)+2;
    
    home = NULL;
    if (strncmp(dir, "~/", 2) == 0) {
        home = getenv("HOME");
        if (home == NULL)
            return NULL;
        len += strlen(home)-1;
    }
    
    if ((fname = malloc(len)) == NULL)
        return NULL;
    
    if (strncmp(dir, "~/", 2) == 0)
        sprintf(fname, "%s%s", home, dir+1);
    else
        strcpy(fname, dir);
    
    //if (writing && !validate_dir(fname))
    //return NULL;
    
    /* XXX: maybe replace all but [-_A-Za-z0-9] */
    p = fname+strlen(fname);
    sprintf(p, "/%s%s", name, ext);
    while(*(++p))
        if(*p == '/')
            *p = '_';
    
    return fname;
}

static BOOL read_file_to_buffer(const char *filename, _u8 *buffer, _u32 len)
{
    FILE *fp;
    _u32 got;
    
    if ((fp = fopen(filename, "rb")) == NULL)
        return NO;
    
    while ((got=fread(buffer, 1, len, fp)) < len)
    {
        if (feof(fp) || ferror(fp) && errno != EINTR)
        {
            fclose(fp);
            return NO;
        }
        
        len -= got;
        buffer += got;
    }
    
    if (fclose(fp) != 0)
        return NO;
    
    return YES;
}

static BOOL write_file_from_buffer(char *filename, _u8 *buffer, _u32 len)
{
    FILE *fp;
    _u32 written;
    
    if ((fp = fopen(filename, "wb")) == NULL)
        return NO;
    
    while ((written = fwrite(buffer, 1, len, fp)) < len)
    {
        if (feof(fp) || ferror(fp) && errno != EINTR)
        {
            fclose(fp);
            return NO;
        }
        
        len -= written;
        buffer += written;
    }
    
    if (fclose(fp) != 0)
        return NO;
    
    return YES;
}

BOOL system_io_flash_read(_u8* buffer, _u32 len)
{
    NSLog(@"Flash read");
    char *fn;
    int ret;
    
    if ((fn = system_make_file_name([gPathToFile UTF8String], ".ngf", NO)) == NULL)
        return NO;
    ret = read_file_to_buffer(fn, buffer, len);
    free(fn);
    return ret;
}

BOOL system_io_flash_write(_u8* buffer, _u32 len)
{
    NSLog(@"Flash write");
    char *fn;
    int ret;
    
    if ((fn = system_make_file_name([gPathToFile UTF8String], ".ngf", YES)) == NULL)
        return NO;
    ret = write_file_from_buffer(fn, buffer, len);
    free(fn);
    return ret;
}

BOOL system_io_rom_read(const char *filename, _u8 *buffer, _u32 len)
{
    return read_file_to_buffer(filename, buffer, len);
}

BOOL system_io_state_read(const char *filename, _u8 *buffer, _u32 len)
{
    return read_file_to_buffer(filename, buffer, len);
}

BOOL system_io_state_write(const char *filename, _u8 *buffer, _u32 len)
{
    return write_file_from_buffer(filename, buffer, len);
}

/* copied from Win32/system_language.c */
typedef struct {
    char label[9];
    char string[256];
} STRING_TAG;

static STRING_TAG string_tags[]={
    { "SDEFAULT",     "Are you sure you want to revert to the default control setup?" }, 
    { "ROMFILT",      "Rom Files (*.ngp,*.ngc,*.npc,*.zip)\0*.ngp;*.ngc;*.npc;*.zip\0\0" }, 
    { "STAFILT",      "State Files (*.ngs)\0*.ngs\0\0" },
    { "FLAFILT",      "Flash Memory Files (*.ngf)\0*.ngf\0\0" },
    { "BADFLASH",     "The flash data for this rom is from a different version of NeoPop, it will be destroyed soon." },
    { "POWER",        "The system has been signalled to power down. You must reset or load a new rom." },
    { "BADSTATE",     "State is from an unsupported version of NeoPop." },
    { "ERROR1",       "An error has occured creating the application window" },
    { "ERROR2",       "An error has occured initialising DirectDraw" },
    { "ERROR3",       "An error has occured initialising DirectInput" },
    { "TIMER",        "This system does not have a high resolution timer." },
    { "WRONGROM",     "This state is from a different rom, Ignoring." },
    { "EROMFIND",     "Cannot find ROM file" },
    { "EROMOPEN",     "Cannot open ROM file" },
    { "EZIPNONE",     "No roms found" } ,
    { "EZIPBAD",      "Corrupted ZIP file" },
    { "EZIPFIND",     "Cannot find ZIP file" },
    
    { "ABORT",        "Abort" },
    { "DISCON",       "Disconnect" },
    { "CONNEC",       "Connected" }
};

char *system_get_string(STRINGS string_id)
{
    if (string_id >= STRINGS_MAX)
        return "Unknown String";
    
    return string_tags[string_id].string;
}

@end
