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

#import "SNESGameEmu.h"
#import <OERingBuffer.h>

#include "memmap.h"
#include "pixform.h"
#include "gfx.h"
#include "display.h"
#include "ppu.h"
#include "soundux.h"
#include "apu.h"
#include "controls.h"
#include "snes9x.h"
#include "movie.h"
#include "snapshot.h"
#include "screenshot.h"
#import "OESNESSystemResponderClient.h"


#define SAMPLERATE      48000
#define SAMPLEFRAME     800
#define SIZESOUNDBUFFER SAMPLEFRAME * 4

@interface SNESGameEmu () <OESNESSystemResponderClient>
- (NSUInteger)SNES_buttonMaskForButton:(OEButton)gameButton;
@end

@implementation SNESGameEmu

NSString *SNESEmulatorKeys[] = { @"A", @"B", @"X", @"Y", @"Up", @"Down", @"Left", @"Right", @"Start", @"Select", @"L", @"R", nil };

- (void)didPushSNESButton:(OESNESButton)button forPlayer:(NSUInteger)player;
{
    S9xReportButton((player << 16) | button, true);
}

- (void)didReleaseSNESButton:(OESNESButton)button forPlayer:(NSUInteger)player;
{
    S9xReportButton((player << 16) | button, false);
}

- (void)mapButtons
{
    for(NSUInteger player = 1; player <= 8; player++)
    {
        NSUInteger playerMask = player << 16;
        
        NSString *playerString = [NSString stringWithFormat:@"Joypad%d ", player];
        
        for(NSUInteger idx = 0; idx < OESNESButtonCount; idx++)
        {
            s9xcommand_t cmd = S9xGetCommandT([[playerString stringByAppendingString:SNESEmulatorKeys[idx]] UTF8String]);
            S9xMapButton(playerMask | idx, cmd, false);
        }
    }
}

#pragma mark Exectuion

- (void)executeFrame
{
    [self executeFrameSkippingFrame:NO];
}

- (void)executeFrameSkippingFrame: (BOOL) skip
{
    IPPU.RenderThisFrame = !skip;
    S9xMainLoop();
    
    S9xMixSamples((unsigned char*)soundBuffer, SAMPLEFRAME * [self channelCount]);
    [[self ringBufferAtIndex:0] write:soundBuffer maxLength:sizeof(UInt16) * [self channelCount] * SAMPLEFRAME];
}

- (BOOL)loadFileAtPath: (NSString*) path
{
    memset(&Settings, 0, sizeof(Settings));
    Settings.ForcePAL            = false;
    Settings.ForceNTSC           = false;
    Settings.ForceHeader         = false;
    Settings.ForceNoHeader       = false;
    
    Settings.ForceSuperFX = Settings.ForceNoSuperFX = false;
    Settings.ForceDSP1    = Settings.ForceNoDSP1    = false;
    Settings.ForceSA1     = Settings.ForceNoSA1     = false;
    Settings.ForceC4      = Settings.ForceNoC4      = false;
    Settings.ForceSDD1    = Settings.ForceNoSDD1    = false;
    
    Settings.MouseMaster = true;
    Settings.SuperScopeMaster = true;
    Settings.MultiPlayer5Master = true;
    Settings.JustifierMaster = true;
    Settings.ShutdownMaster = false;
    Settings.BlockInvalidVRAMAccess = true;
    Settings.HDMATimingHack = 100;
    Settings.APUEnabled = true;
    Settings.NextAPUEnabled = true;
    Settings.SoundPlaybackRate = 48000;
    Settings.Stereo = true;
    Settings.SixteenBitSound = true;
    Settings.SoundEnvelopeHeightReading = true;
    Settings.DisableSampleCaching = false;
    Settings.DisableSoundEcho = false;
    Settings.InterpolatedSound = true;
    Settings.Transparency = true;
    Settings.SupportHiRes = true;
    Settings.SDD1Pack = true;
    GFX.InfoString = nil;
    GFX.InfoStringTimeout = 0;
    
    if(videoBuffer) 
        free(videoBuffer);
    videoBuffer = (unsigned char*)malloc(MAX_SNES_WIDTH*MAX_SNES_HEIGHT*sizeof(uint16_t));
    //GFX.PixelFormat = 3;
    
    GFX.Pitch = 512 * 2;
    //GFX.PPL = SNES_WIDTH;
    GFX.Screen = (short unsigned int*)videoBuffer;
    
    S9xUnmapAllControls();
    
    [self mapButtons];
    
    S9xSetController(0, CTL_JOYPAD,     0, 0, 0, 0);
    S9xSetController(1, CTL_JOYPAD,     1, 0, 0, 0);
    
    //S9xSetRenderPixelFormat(RGB565);
    if(!Memory.Init() || !S9xInitAPU() || !S9xGraphicsInit())
        NSLog(@"Couldn't init");
    NSLog(@"loading %@", path);
    
    Settings.NoPatch = true;
    if(Memory.LoadROM([path UTF8String]))
    {
        NSString *path = [NSString stringWithUTF8String:Memory.ROMFilename];
        NSString *extensionlessFilename = [[path lastPathComponent] stringByDeletingPathExtension];
        
        NSString *batterySavesDirectory = [self batterySavesDirectoryPath];
        
//        if((batterySavesDirectory != nil) && ![batterySavesDirectory isEqualToString:@""])
        if([batterySavesDirectory length] != 0)
        {
            [[NSFileManager defaultManager] createDirectoryAtPath:batterySavesDirectory withIntermediateDirectories:YES attributes:nil error:NULL];
        
            NSString *filePath = [batterySavesDirectory stringByAppendingPathComponent:[extensionlessFilename stringByAppendingPathExtension:@"sav"]];
        
            Memory.LoadSRAM([filePath UTF8String]);
        }
        
        S9xInitSound(1, Settings.Stereo, SIZESOUNDBUFFER);
    }
    return YES;
}

bool8 S9xOpenSoundDevice (int mode, bool8 stereo, int buffer_size)
{
    NSLog(@"Open sound");
    so.buffer_size = buffer_size;
    
    so.playback_rate = Settings.SoundPlaybackRate;
    so.stereo        = Settings.Stereo;
    so.sixteen_bit   = Settings.SixteenBitSound;
    so.encoded       = false;
    
    so.samples_mixed_so_far = 0;
    
    S9xSetPlaybackRate(so.playback_rate);
    
    so.mute_sound = false;
    return true;
}

#pragma mark Video
- (const void *)videoBuffer
{
    return GFX.Screen;
}

- (OEIntRect)screenRect
{
    return OERectMake(0, 0, IPPU.RenderedScreenWidth, IPPU.RenderedScreenHeight);
}

- (OEIntSize)bufferSize
{
    return OESizeMake(MAX_SNES_WIDTH, MAX_SNES_HEIGHT);
}

- (void)setupEmulation
{
    if(soundBuffer)
        free(soundBuffer);
    soundBuffer = (UInt16*)malloc(SIZESOUNDBUFFER* sizeof(UInt16));
    memset(soundBuffer, 0, SIZESOUNDBUFFER*sizeof(UInt16));
}

- (void)resetEmulation
{
    S9xSoftReset();
}

- (void)stopEmulation
{
    NSString *path = [NSString stringWithUTF8String:Memory.ROMFilename];
    NSString *extensionlessFilename = [[path lastPathComponent] stringByDeletingPathExtension];
    
    NSString *batterySavesDirectory = [self batterySavesDirectoryPath];
    
    if([batterySavesDirectory length] != 0)
    {
        
        [[NSFileManager defaultManager] createDirectoryAtPath:batterySavesDirectory withIntermediateDirectories:YES attributes:nil error:NULL];
    
        NSLog(@"Trying to save SRAM");
    
        NSString *filePath = [batterySavesDirectory stringByAppendingPathComponent:[extensionlessFilename stringByAppendingPathExtension:@"sav"]];
    
        Memory.SaveSRAM([filePath UTF8String]);
    }
    
    [super stopEmulation];
}

- (void)dealloc
{
    free(videoBuffer);
    free(soundBuffer);
    [super dealloc];
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

- (NSTimeInterval)frameInterval
{
    if( Settings.PAL )
        return 50;
    else
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
        case OEButton_1      : button = SNES_A_MASK;      break;
        case OEButton_2      : button = SNES_B_MASK;      break;
        case OEButton_3      : button = SNES_X_MASK;      break;
        case OEButton_4      : button = SNES_Y_MASK;      break;
        case OEButton_5      : button = SNES_TL_MASK;     break;
        case OEButton_6      : button = SNES_TR_MASK;     break;
        case OEButton_Up     : button = SNES_UP_MASK;     break;
        case OEButton_Down   : button = SNES_DOWN_MASK;   break;
        case OEButton_Left   : button = SNES_LEFT_MASK;   break;
        case OEButton_Right  : button = SNES_RIGHT_MASK;  break;
        case OEButton_Start  : button = SNES_START_MASK;  break;
        case OEButton_Select : button = SNES_SELECT_MASK; break;
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

- (BOOL)saveStateToFileAtPath: (NSString *) fileName
{
    bool8 success = S9xFreezeGame([fileName UTF8String]);
    
    if(success)
        return YES;
    return NO;
}

- (BOOL)loadStateFromFileAtPath: (NSString *) fileName
{
    bool8 success = S9xUnfreezeGame([fileName UTF8String]);
    
    if(success)
        return YES;
    return NO;
}

@end
