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

#import "MednafenGameCore.h"
#include <map>
#include "types.h"
#include "git.h"
#include "trio.h"
#include "input.h"
#include "state-driver.h"

uint16_t input_buffer[2];

void BuildPortInfo(MDFNGI *gi);
@implementation MednafenGameCore

enum {
    GBA_BUTTON_A       = 0x001,
    GBA_BUTTON_B       = 0x002,
    GBA_BUTTON_SELECT  = 0x004,
    GBA_BUTTON_START   = 0x008,
    GBA_RIGHT          = 0x010,
    GBA_LEFT           = 0x020,
    GBA_UP             = 0x040,
    GBA_DOWN           = 0x080,
    GBA_BUTTON_R       = 0x100,
    GBA_BUTTON_L       = 0x200,
    GBA_BUTTON_SPEED   = 0x400, // randomly given
    GBA_BUTTON_CAPTURE = 0x800  // randomly given
};
/*
 bit 0 - cursor up
 bit 1 - cursor right
 bit 2 - cursor down
 bit 3 - cursor left
 Meaning of the bits when reading buttons:
 bit 0 - ???
 bit 1 - START
 bit 2 - A
 bit 3 - B
 */
enum {
    WS_UP           = 0x010,
    WS_RIGHT        = 0x020,
    WS_DOWN         = 0x040,
    WS_LEFT         = 0x080,
    
    WS_BUTTON_START = 0x002,
    WS_BUTTON_A     = 0x004,
    WS_BUTTON_B     = 0x008,
};

NSUInteger MednafenControlValues[] = {
    GBA_BUTTON_A,
    GBA_BUTTON_B,
    GBA_BUTTON_SELECT,
    GBA_BUTTON_START,
    GBA_RIGHT,
    GBA_LEFT,
    GBA_UP,
    GBA_DOWN,
    GBA_BUTTON_R,
    GBA_BUTTON_L,
    
    WS_UP,
    WS_LEFT,
    WS_RIGHT,
    WS_DOWN,
    WS_BUTTON_START,
    WS_BUTTON_A,
    WS_BUTTON_B ,
};

NSString *MednafenControlNames[] = {
    @"GBA@_BUTTON_A",
    @"GBA@_BUTTON_B",
    @"GBA@_BUTTON_SELECT",
    @"GBA@_BUTTON_START",
    @"GBA@_RIGHT",
    @"GBA@_LEFT",
    @"GBA@_UP",
    @"GBA@_DOWN",
    @"GBA@_BUTTON_R",
    @"GBA@_BUTTON_L",
    
    @"WS@_UP",
    @"WS@_LEFT" ,
    @"WS@_RIGHT",
    @"WS@_DOWN",
    @"WS@_BUTTON_START",
    @"WS@_BUTTON_A" ,
    @"WS@_BUTTON_B" ,
};
- (id)init
{
    self = [super init];
    if(self != nil)
    {
        driverSettings = new std::vector<MDFNSetting>();
        soundLock = [[NSLock alloc] init];
        bufLock   = [[NSLock alloc] init];
        
        NSLog(@"Inited? %d", MDFNI_Initialize((char *) [[[NSBundle bundleForClass:[self class]] resourcePath] UTF8String] , *driverSettings));
        position = 0;
        sndBuf = (uint16 *)malloc(100000 * sizeof(UInt16));
    }
    return self;
}

- (void) dealloc
{
    DLog(@"releasing/deallocating Mednafen memory");
    free(sndBuf);
    [soundLock release];
    [bufLock release];
    
    [super dealloc];
}

- (void)executeFrame
{
    //DLog(@"Executing");
    //Get a reference to the emulator
    [bufLock lock];
    EmulateSpecStruct espec;
    memset(&espec, 0, sizeof(EmulateSpecStruct));
    //NSLog(@"Executing frame");
//    espec.pixels = (uint32*)videoBuffer;
//    espec.LineWidths = lineWidths;
//    espec.SoundBuf = (int16**)(&sndBuf);
//    espec.SoundBufSize = &ssize;
//    espec.skip = 0;
//    espec.soundmultiplier = 1;
//    espec.NeedRewind = 0;
    
    //input_buffer[0] = 255;
    //input_buffer[1] = 255;
    //gameInfo->Emulate(&espec);
    //NSLog(@"Writing %d samples", ssize);
    MDFNI_Emulate(&espec);
    [[self ringBufferAtIndex:0] write:(const uint8_t*) sndBuf maxLength:ssize * [self channelCount] * 2];
    //oldrun = sms_frame(oldrun);
    [bufLock unlock];
    //NSLog(@"Unlocked");
}

- (void)setupEmulation
{
}

- (BOOL)loadFileAtPath:(NSString*)path
{
    DLog(@"Loaded File");
    //TODO: add choice NTSC/PAL
    
    //MDFNI_Initialize(<#char * dir#>, <#const std * #>);
    
//    if(gameInfo = MDFNI_LoadGame([path UTF8String]))
//    {
//        MDFNI_SetPixelFormat(8,16,24,0);
//        NSLog(@"Really loaded file");
//        videoBuffer = (unsigned char *)malloc(gameInfo->pitch * 256);
//        tempBuffer = (unsigned char *)malloc(gameInfo->pitch * 256);
//        
//        MDFNI_SetSoundVolume(100);
//        
//        MDFNI_Sound( gameInfo->soundrate ? gameInfo->soundrate : 48000 );
//        NSLog(@"Alloced vid");
//        lineWidths = (MDFN_Rect *)calloc(256, sizeof(MDFN_Rect));
//        
//        BuildPortInfo(gameInfo);
//        return YES;
//    }
    
    return NO;
}
- (void)resetEmulation
{
    MDFNI_Reset();
}

- (void)stopEmulation
{
    [super stopEmulation];
}

- (IBAction)pauseEmulation:(id)sender
{
    [bufLock lock];
    paused = YES;
    [bufLock unlock];
}

- (OEIntRect)screenRect
{
    return (OEIntRect){0,0};//OERectMake(gameInfo->DisplayRect.x, gameInfo->DisplayRect.y, gameInfo->DisplayRect.w, gameInfo->DisplayRect.h);
}

- (OEIntSize)bufferSize
{
    return (OEIntSize){0,0};//OESizeMake(gameInfo->pitch / 4, gameInfo->height);
}

- (const void *)videoBuffer
{
    //NSLog(@"Buffer request");
    return videoBuffer;
}

- (GLenum)pixelFormat
{
    return GL_BGRA;
}

- (GLenum)pixelType
{
    return GL_UNSIGNED_INT_8_8_8_8;
}

- (GLenum)internalPixelFormat
{
    return GL_RGB8;
}

- (NSUInteger)frameSampleRate
{
    return 0;//FSettings.SndRate;
}

- (NSUInteger)channelCount
{
    return gameInfo->soundchan;
}

//- (OEEmulatorKey)emulatorKeyForKeyIndex:(NSUInteger)index player:(NSUInteger)thePlayer
//{
//    return 0;//OEMakeEmulatorKey(thePlayer, MednafenControlValues[index]);
//}
//
//- (void)pressEmulatorKey:(OEEmulatorKey)aKey
//{
//    //input_buffer[aKey.player - 1] |= aKey.key;
//}
//
//- (void)releaseEmulatorKey:(OEEmulatorKey)aKey
//{
//   // input_buffer[aKey.player - 1] &= ~aKey.key;
//}

void MDFND_Message(const char *s)
{
    NSLog(@"Message: %s", s);
}

void MDFND_PrintError(const char *s)
{
    NSLog(@"Error: %s", s);
}

void MDFND_SetMovieStatus(StateStatusStruct*)
{
    
}

uint32 MDFND_GetTime()
{
    NSLog(@"Asked for tiem");
    return 1;
}

bool MDFND_ExitBlockingLoop()
{
    return YES;
}

void BuildPortInfo(MDFNGI *gi)
{
    InputInfoStruct *info = gi->InputInfo;
    const InputPortInfoStruct *portInfo = info->Types;
    for( int i = 0; i < portInfo->NumTypes; i++ )
    {
        InputDeviceInfoStruct* idii = &portInfo->DeviceInfo[i];
        for( int j = 0; j < idii->NumInputs; j++ )
        {
            const InputDeviceInputInfoStruct *inputDevice = &idii->IDII[j];
            
            NSLog(@"Button %s on device %s", inputDevice->Name, idii->FullName);
        }
    }
    
    MDFNI_SetInput(0, portInfo->DeviceInfo[0].ShortName,  input_buffer, 2);//void MDFNI_SetInput(int port, const char *type, void *ptr, uint32 ptr_len_thingy)
}

void MDFND_DispMessage(unsigned char*msg)
{
    NSLog(@"Message: %s", msg);
}
void MDFND_SetStateStatus(StateStatusStruct*)
{
    
}

void MDFN_WriteWaveData(int16 * buffer, int count)
{
    
}

@end
