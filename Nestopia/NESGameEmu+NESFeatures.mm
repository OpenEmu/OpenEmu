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

#import "GameCore.h"
#import "NESGameEmu.h"

#include <NstBase.hpp>
#include <NstApiEmulator.hpp>
#include <NstApiMachine.hpp>
#include <NstApiCartridge.hpp>
#include <NstApiVideo.hpp>
#include <NstApiSound.hpp>
#include <NstApiUser.hpp>
#include <NstApiCheats.hpp>
#include <NstApiRewinder.hpp>
#include <NstApiRam.h>
#include <NstApiMovie.hpp>
#include <iostream>
#include <fstream>


#pragma mark --NES-specific features--
@implementation NESGameEmu (NesAdditions)
NSString * const NESNTSC = @"NESNTSC";
NSString * const NESBrightness = @"NESBrightness";
NSString * const NESSaturation = @"NESSaturation";
NSString * const NESContrast = @"NESContrast";
NSString * const NESSharpness = @"NESSharpness";
NSString * const NESColorRes = @"NESColorRes";
NSString * const NESColorBleed = @"NESColorBleed";
NSString * const NESColorArtifacts = @"NESColorArtifacts";
NSString * const NESColorFringing = @"NESColorFringing";
NSString * const NESHue = @"NESHue";
NSString * const NESUnlimitedSprites = @"NESUnlimitedSprites";


- (BOOL)isUnlimitedSpritesEnabled
{
    return [[NSUserDefaults standardUserDefaults] boolForKey:NESUnlimitedSprites];
}

- (BOOL)isNTSCEnabled
{
    return [[NSUserDefaults standardUserDefaults] boolForKey:NESNTSC];
}

- (int)brightness
{
    return [[NSUserDefaults standardUserDefaults] integerForKey:NESBrightness];
}

- (int)saturation
{
    return [[NSUserDefaults standardUserDefaults] integerForKey:NESSaturation];
}

- (int)contrast
{
    return [[NSUserDefaults standardUserDefaults] integerForKey:NESContrast];
}

- (int)sharpness
{
    return [[NSUserDefaults standardUserDefaults] integerForKey:NESSharpness];
}

- (int)colorRes
{
    return [[NSUserDefaults standardUserDefaults] integerForKey:NESColorRes];
}

- (int)colorBleed
{
    return [[NSUserDefaults standardUserDefaults] integerForKey:NESColorBleed];
}

- (int)colorArtifacts
{
    return [[NSUserDefaults standardUserDefaults] integerForKey:NESColorArtifacts];
}

- (int)colorFringing
{
    return [[NSUserDefaults standardUserDefaults] integerForKey:NESColorFringing];
}

- (int)hue
{
    return [[NSUserDefaults standardUserDefaults] integerForKey:NESHue];
}


- (void)toggleNTSC:(id)sender
{
    if([self isNTSCEnabled])
        [self setupVideo:emu withFilter:1];//[[[OpenNestopiaPreferencesController sharedPreferencesController:self] filter] intValue]];
    else
        [self setupVideo:emu withFilter:0];
    
}

- (void)applyNTSC:(id)sender
{
    NSLog(@"Filters!");
    Nes::Api::Video video( *emu );
    
    video.SetSharpness([self sharpness]);
    video.SetColorResolution([self colorRes]);
    video.SetColorBleed([self colorBleed]);
    video.SetBrightness([self brightness]);
    video.SetContrast([self contrast]);
    video.SetColorArtifacts([self colorArtifacts]);
    video.SetHue([self hue]);
    video.SetColorFringing([self colorFringing]);
    video.SetSaturation([self saturation]);
    //    [self setupVideo:emu withFilter: 0];
}

- (void)toggleUnlimitedSprites:(id)sender
{
    // FIXME: the value returned by -isUnlimitedSpritesEnabled never changes because the value is never saved in the user defaults.
    //[self enableUnlimitedSprites:[self isUnlimitedSpritesEnabled]];
}

- (void)enableUnlimitedSprites:(BOOL)enable 
{
    Nes::Api::Video video( *emu );
    
    video.EnableUnlimSprites(enable ? true : false);
}

-(void)setCode:(NSString*)code
{
    if(![code isEqualToString:@""])
    {
        char cCode[9];
        [code cStringUsingEncoding:NSUTF8StringEncoding];
        Nes::Api::Cheats cheater(*emu);
        Nes::Api::Cheats::Code ggCode;
        Nes::Api::Cheats::GameGenieDecode(cCode, ggCode); //FIXME: something's going awry here --dmw
        cheater.SetCode(ggCode);
    }
    else
    {
        NSLog(@"Null code");
    }
}

- (void)enableRewinder:(BOOL)rewind 
{
    Nes::Api::Rewinder rewinder(*emu);
    rewinder.Enable(rewind);
}

- (BOOL)isRewinderEnabled
{
    Nes::Api::Rewinder rewinder(*emu);
    return rewinder.IsEnabled();
}

- (void)rewinderDirection: (NSUInteger) rewinderDirection
{
    Nes::Api::Rewinder rewinder(*emu);
    
    if(rewinderDirection == 1) {
        rewinder.SetDirection(Nes::Api::Rewinder::FORWARD);
        DLog(@"rewinder direction is forward");
    }
    else {
        rewinder.SetDirection(Nes::Api::Rewinder::BACKWARD);
        DLog(@"rewinder direction is backward");
    }
}

- (void)enableRewinderBackwardsSound: (BOOL) rewindSound
{
    Nes::Api::Rewinder rewinder(*emu);
    if(rewindSound) 
    {
        rewinder.EnableSound(YES);
    } 
    else 
    {
        rewinder.EnableSound(NO);
    }
}

- (BOOL)isRewinderBackwardsSoundEnabled
{
    Nes::Api::Rewinder rewinder(*emu);
    return rewinder.IsSoundEnabled();
}

- (void)setRamBytes:(double)off value:(double)val
{
    Nes::Api::Ram ram(*emu);
    
    int prgRomOffset = (off * 0x2000) + 0x8000;
    int prgRomValue = (int) (val * 256);
    
    ram.SetRAM(prgRomOffset, prgRomValue);
}

- (int)cartVRamSize 
{
    // note: some cartridges had VRAM some, some did not.  If they didn't, they used the PPU's 2K of RAM alone.
    Nes::Api::Cartridge cart(*emu);
    const Nes::Api::Cartridge::Profile* profile = cart.GetProfile();
    int vRamSize = profile->board.GetVram();
    
    //    DLog(@"vRamSize is %U",vRamSize);
    if(vRamSize == 0)    //in other words, if the cartridge has no VRAM, set the size to 2K
        return vRamSize = 2048;
    else
        return vRamSize;
}

// incoming doubles are 0.0 - > 1.0 range. We un-normalize
- (void)setNmtRamBytes:(double)off value:(double)val
{
    int nameTableSize = 960;    // each nametable (there are 4) is 1024bytes, but the last 64 bytes of each 
                                // nametable is an attribute table, which controls the palette of the preceding nametable's tiles.
                                // note: depending on the cartridge's mirroring mode, there can be one, two or four nametables (functionally speaking), while the others are just mirrors (copies).
    
    Nes::Api::Machine machine(*emu);
    
    int numPages = 3;
    
    int unnormalizedOffset = off * nameTableSize;            // subtract 64bytes to stay out of the attribute table
    int unnormalizedVal = (int) (val * 256);                // Nametable values are addresses in the pattern tables, which are at $0000 and $1000 in the PPU's RAM and $1000bytes each.  
                                                            // Here's a really good, short explanation of all the tables: http://nesdev.parodius.com/NESTechFAQ.htm#namattpat
    
    for(int i = 0; i < numPages; i++)
    {
        machine.PokeNmt((i * 0x400) + 0x2000 + unnormalizedOffset, unnormalizedVal); //note: 0x400 = 1024bytes
    }
}

- (void)setNMTRamByTable:(NSNumber*)table array:(NSArray*)nmtValueArray
{
    Nes::Api::Machine machine(*emu);
    
    int startOfNameTable = [table unsignedIntValue] * 0x400;
    
    for (int i = 0; i < 960; i++)
    {
        machine.PokeNmt(i + startOfNameTable + 0x2000, [[nmtValueArray objectAtIndex:i] intValue]);
    }
}

- (int)chrRomSize //TODO: OK, sometimes games have CHR ROM, sometimes CHR RAM, sometimes both (or none). maybe just cut the ROM and call it chrSize?  that's what GetChr() seems to be returning anyway.  or maybe we'd be better served just replacing this with sprite-only-gltiching methods.
{
    Nes::Api::Cartridge cart(*emu);
    const Nes::Api::Cartridge::Profile* profile = cart.GetProfile();
    int romSize = profile->board.GetChr();
    //    DLog(@"called chrRomSize. result: %U", romSize);
    return romSize;
}

// incoming doubles are 0.0 - > 1.0 range. We un-normalize
- (void)setChrRamBytes:(double)off value:(double)val
{
    // FIXME: this method needs to be rethunk.  and i shall do the rethunking!  --dan
    
    int pageSize = 2048;    // this should be 1024 (according to Josh), but it seems
                            // using 2K makes glitches across all screens, whereas 1K
                            // occasionally would leave entire screen widths unaltered...
    
    Nes::Api::Ram ram(*emu);
    
    int romSize = [self chrRomSize];    // should be 4096  --ORLY? :P TODO: prove it. 
    int numPages = romSize / pageSize;
    
    int unnormalizedOffset = (int) (off * pageSize);
    int unnormalizedVal = ((int) (val * pageSize)) % 64;    // this % may fix a crash in glitching chr ram when looking up a palette above max val.
    
    for(int i = 0; i < numPages ; i++)
    {
        ram.SetCHR(0, unnormalizedOffset , unnormalizedVal);
    }
}

- (void)recordMovie:(NSString*) moviePath mode:(BOOL)append
{
    Nes::Api::Movie movie(*emu);
    Nes::Result result; 
    
    std::fstream movieFile([moviePath cStringUsingEncoding:NSUTF8StringEncoding], std::ios::in | std::ios::binary);
    
    if(append) 
        result = movie.Record(movieFile,Nes::Api::Movie::APPEND);
    else 
        result = movie.Record(movieFile, Nes::Api::Movie::CLEAN);
}

- (void)playMovie:(NSString*) moviePath
{
    Nes::Api::Movie movie(*emu);
    Nes::Result result; 
    
    std::ifstream movieFile([moviePath cStringUsingEncoding:NSUTF8StringEncoding], std::ios::in | std::ios::binary);
    
    result = movie.Play(movieFile);
}

- (void)stopMovie
{
    Nes::Api::Movie movie(*emu);
    movie.Stop();
}

- (BOOL)isMovieRecording
{
    Nes::Api::Movie movie(*emu);
    return movie.IsRecording();
}

- (BOOL)isMoviePlaying
{
    Nes::Api::Movie movie(*emu);
    return movie.IsPlaying();
}

- (BOOL)isMovieStopped
{
    Nes::Api::Movie movie(*emu);
    return movie.IsStopped();
}

@end
