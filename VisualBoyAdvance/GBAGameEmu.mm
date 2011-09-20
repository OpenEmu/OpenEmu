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

#import "GBAGameEmu.h"
#include <sys/time.h>
#import <OEGameDocument.h>
#import <OERingBuffer.h>
#import <OEHIDEvent.h>
#import "GBAGameController.h"
#import "OEGBASystemResponderClient.h"

#include "Core/gba/GBA.h"
#include "Core/gba/agbprint.h"
#include "Core/gba/Flash.h"
#include "Core/common/Port.h"
#include "Core/gba/RTC.h"
#include "core/gba/Sound.h"
//#include "core/Text.h"
//#include "core/unzip.h"
#include "core/Util.h"
#include "core/gb/GB.h"
#include "core/gb/gbGlobals.h"
#include "CASoundDriver.h"
#include "core/gba/Sound.h"

#define GBA_WIDTH   240
#define GBA_HEIGHT  160
#define SAMPLERATE 44100
#define SAMPLEFRAME (SAMPLERATE/60)
#define SIZESOUNDBUFFER SAMPLEFRAME*4

int systemFrameSkip = 0;

@implementation GBAGameEmu

char filename[2048];
char ipsname[2048];
char biosFileName[2048];

/*
 Visual Boy Advance properties 
 */

bool systemSoundOn;
u16 systemColorMap16[0x10000];
u32 systemColorMap32[0x10000];
u16 systemGbPalette[24];
int systemRedShift = 0;
int systemBlueShift = 0;
int systemGreenShift = 0;
int systemColorDepth = 0;
int systemDebug = 0;
int systemVerbose = 0;

int systemSaveUpdateCounter = SYSTEM_SAVE_NOT_UPDATED;
int emulating = 0;
int RGB_LOW_BITS_MASK = 0x010101;
u32 autoFrameSkipLastTime = 0;

struct EmulatedSystem emulator = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    false,
    0
};

static NSUInteger _GBAButtons[4] = { 0 };
static CASoundDriver *driver;
enum {
    KEY_BUTTON_A       = 0x001,
    KEY_BUTTON_B       = 0x002,
    KEY_BUTTON_SELECT  = 0x004,
    KEY_BUTTON_START   = 0x008,
    KEY_RIGHT          = 0x010,
    KEY_LEFT           = 0x020,
    KEY_UP             = 0x040,
    KEY_DOWN           = 0x080,
    KEY_BUTTON_R       = 0x100,
    KEY_BUTTON_L       = 0x200,
    KEY_BUTTON_SPEED   = 0x400, // randomly given
    KEY_BUTTON_CAPTURE = 0x800  // randomly given
};

NSString *GBAControlNames[] = { @"KEY@_BUTTON_A", @"KEY@_BUTTON_B", @"KEY@_BUTTON_SELECT", @"KEY@_BUTTON_START", @"KEY@_RIGHT", @"KEY@_LEFT", @"KEY@_UP", @"KEY@_DOWN", @"KEY@_BUTTON_R", @"KEY@_BUTTON_L" };
NSUInteger GBAControlValues[] = { KEY_BUTTON_A, KEY_BUTTON_B, KEY_BUTTON_SELECT, KEY_BUTTON_START, KEY_RIGHT, KEY_LEFT, KEY_UP, KEY_DOWN, KEY_BUTTON_R, KEY_BUTTON_L };

void readBattery();
void writeBattery();

/*
 GameCore functions 
 */

- (const void *)videoBuffer
{
    //get rid of the first line and the last row
    for (int i = 0; i < GBA_HEIGHT; i++)
        memcpy(tempBuffer + i* GBA_WIDTH * 4* sizeof(unsigned char), pix + (i+1)*(GBA_WIDTH+1) * 4* sizeof(unsigned char), GBA_WIDTH * 4 * sizeof(unsigned char));
    
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

- (void)resetEmulation
{
    CPUReset();
}

- (void)stopEmulation
{
    writeBattery();
    [super stopEmulation];
}

OERingBuffer *ringBuffer = nil;

- (id)init
{
    self = [super init];
    if(self != nil)
    {
        soundLock = [[NSLock alloc] init];
        bufLock = [[NSLock alloc] init];
        sndBuf = new UInt16[SIZESOUNDBUFFER];
        ringBuffer = [self ringBufferAtIndex:0];
        memset(sndBuf, 0, SIZESOUNDBUFFER * 2);
        tempBuffer = (unsigned char*) malloc(240 * 160 * 4 * sizeof(unsigned char));
    }
    return self;
}

- (void)dealloc
{
    delete[] sndBuf;
    free(tempBuffer);
    [soundLock lock];
    [bufLock lock];
    [super dealloc];
}


- (void)executeFrame
{
    //    [bufLock lock];
    // 54 cycles per frame
    emulator.emuMain(emulator.emuCount);
    //    [bufLock unlock];
}

- (void)setupEmulation
{
    emulator = GBASystem;
    
    NSString *appSupportPath = [[[[NSHomeDirectory() stringByAppendingPathComponent:@"Library"] 
                                  stringByAppendingPathComponent:@"Application Support"] 
                                 stringByAppendingPathComponent:@"OpenEmu"]
                                stringByAppendingPathComponent:@"BIOS"];
    
    strcpy(biosFileName, [[appSupportPath stringByAppendingPathComponent:@"GBA.BIOS"] UTF8String]);
    
    //no bios by default for the moment, add support later
    CPUInit(biosFileName, YES);
    
    
    
    int size = 0x2000000;
    //    utilApplyIPS(ipsname, &rom, &size);
    if(size != 0x2000000) {
        CPUReset();
    }
    
    readBattery();
    
    //soundFiltering = 0.0f;
    //soundInterpolation = false;
    
    flashSetSize(0x20000);
    //soundSampleRate = SAMPLERATE;
    systemColorDepth = 32;
    systemRedShift = 19;
    systemGreenShift = 11;
    systemBlueShift = 3;
    
    for(int i = 0; i < 0x10000; i++) {
        systemColorMap32[i] = ((i & 0x1f) << systemRedShift) |
        (((i & 0x3e0) >> 5) << systemGreenShift) |
        (((i & 0x7c00) >> 10) << systemBlueShift);
    }
    
    if(!soundInit())
        NSLog(@"Couldn't init sound");
    
    CPUReset();
    //systemFrameSkip = 2;
    //autoFrameSkipLastTime = systemGetClock();
}

- (BOOL)loadFileAtPath:(NSString*)path
{
    if (utilIsGBAImage([path UTF8String]))
    {
        int size = CPULoadRom([path UTF8String]);
        if (size != 0)
        {
            strcpy(filename, [path UTF8String]);
            //utilGetBaseName([path UTF8String], filename);
            //sprintf(ipsname, "%s.ips", filename);
            emulating = true;
            return YES;
        }
    }
    return NO;
}

- (OEIntSize)bufferSize
{
    return OESizeMake(GBA_WIDTH, GBA_HEIGHT);
}

- (NSUInteger)channelCount
{
    return 2;
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

- (void)didPushGBAButton:(OEGBAButton)button forPlayer:(NSUInteger)player;
{
    _GBAButtons[player - 1] |=  (1 << button);
}

- (void)didReleaseGBAButton:(OEGBAButton)button forPlayer:(NSUInteger)player;
{
    _GBAButtons[player - 1] &= ~(1 << button);
}

- (BOOL)saveStateToFileAtPath:(NSString *)fileName
{
    if(emulator.emuWriteState)
    {
        emulator.emuWriteState([fileName UTF8String]);
        systemScreenMessage("Wrote state");
        return YES;
    }
    return NO;
}

- (BOOL)loadStateFromFileAtPath:(NSString *)fileName
{
    if(emulator.emuReadState)
    {
        emulator.emuReadState([fileName UTF8String]);
        systemScreenMessage("Loaded state");
        return YES;
    }
    return NO;
}

/*
 Battery functions
 */

void writeBattery()
{
    char buffer[1048];
    
    sprintf(buffer, "%s.sav", filename);
    emulator.emuWriteBattery(buffer);
    NSLog(@"wrote to %s\n", buffer);
    
    systemScreenMessage("Wrote battery");
}

void readBattery()
{
    char buffer[1048];
    
    sprintf(buffer, "%s.sav", filename);
    
    if(emulator.emuReadBattery(buffer))
        systemScreenMessage("Loaded battery");
}

/*
 Visual Boy Advance callbacks
 */

void winlog(const char *,...)
{
    
}

void log(const char * str,...)
{
    va_list args;
    va_start (args, str);
    vprintf (str, args);
    va_end (args);
}

bool systemPauseOnFrame()
{
    return NO;
}

void systemGbPrint(u8 *,int,int,int,int)
{
    
}

void systemScreenCapture(int)
{
    
}

void systemDrawScreen()
{
}

bool systemReadJoypads()
{
    return true;
}

u32 systemReadJoypad(int which)
{
    u32 res = 0;
    
    which %= 4;
    if(which == -1) 
        which = 0;
    res = _GBAButtons[which];
    
    // disallow L+R or U+D of being pressed at the same time
    if((res & (KEY_RIGHT | KEY_LEFT)) == (KEY_RIGHT | KEY_LEFT)) res &= ~ KEY_RIGHT;
    if((res & (KEY_UP    | KEY_DOWN)) == (KEY_UP    | KEY_DOWN)) res &= ~ KEY_UP;
    
    /*
     if(autoFire) {
     res &= (~autoFire);
     if(autoFireToggle)
     res |= autoFire;
     autoFireToggle = !autoFireToggle;
     }
     */
    
    return res;
}

u32 systemGetClock()
{
    struct timeval tv;
    
    gettimeofday(&tv, NULL); 
    return tv.tv_sec*1000;
}

void systemMessage(int, const char * message, ...)
{
    va_list args;
    va_start(args, message);
    NSLogv([@"message: " stringByAppendingString:[NSString stringWithUTF8String:message]], args);
    va_end(args);
}

void systemSetTitle(const char * title)
{
    NSLog(@"title: %s", title);
}

void systemSoundShutdown()
{
    
}

void systemSoundPause()
{
    
}

void systemSoundResume()
{
    
}

void systemSoundReset()
{
    
}

void systemOnWriteDataToSoundBuffer(const u16 * finalWave, int length)
{
    [ringBuffer write:(const uint8_t*)finalWave maxLength:length];
}

SoundDriver* systemSoundInit()
{
    driver = new CASoundDriver();
    return driver;
}

void systemScreenMessage(const char * message)
{
    NSLog(@"message: %s", message);
}

void systemUpdateMotionSensor()
{
    
}

int  systemGetSensorX()
{
    return 0;
}

int  systemGetSensorY()
{
    return 0;
}

bool systemCanChangeSoundQuality()
{
    return false;
}

void systemShowSpeed(int speedInt)
{
    NSLog(@"speed: %d %%", speedInt);
}


int frameskipadjust = 0;
void system10Frames(int rate)
{
    u32 time = systemGetClock();
    if( NO ) {
        u32 diff = time - autoFrameSkipLastTime;
        int speed = 100;
        
        if(diff)
            speed = (1000000/rate)/diff;
        
        if(speed >= 98) {
            frameskipadjust++;
            
            if(frameskipadjust >= 3) {
                frameskipadjust=0;
                if(systemFrameSkip > 0)
                    systemFrameSkip--;
            }
        } else {
            if(speed  < 80)
                frameskipadjust -= (90 - speed)/5;
            else if(systemFrameSkip < 9)
                frameskipadjust--;
            
            if(frameskipadjust <= -2) {
                frameskipadjust += 2;
                if(systemFrameSkip < 9)
                    systemFrameSkip++;
            }
        }
    }
    
    
    if(systemSaveUpdateCounter) {
        if(--systemSaveUpdateCounter <= SYSTEM_SAVE_NOT_UPDATED) {
            writeBattery();
            systemSaveUpdateCounter = SYSTEM_SAVE_NOT_UPDATED;
        }
    }
    
    autoFrameSkipLastTime = time;
}

void systemFrame()
{
    
}

void systemGbBorderOn()
{
    
}

void systemOnSoundShutdown()
{
    
}

void debuggerOutput(const char * stringDebug, u32)
{
    NSLog(@"Debug: %s", stringDebug);
}

void (*dbgOutput)(const char *, u32) = debuggerOutput;

@end
