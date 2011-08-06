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

#import "NESGameEmu.h"
#import "NESGameController.h"
#import <OERingBuffer.h>
#include <sys/time.h>
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
#include <map>
#import "OENESSystemResponderClient.h"

#define SAMPLERATE 48000

@interface NESGameEmu () <OENESSystemResponderClient>
- (NSUInteger)NES_buttonMaskForButton:(OEButton)gameButton;
@end

NSUInteger NESControlValues[] = { Nes::Api::Input::Controllers::Pad::A, Nes::Api::Input::Controllers::Pad::B, Nes::Api::Input::Controllers::Pad::UP, Nes::Api::Input::Controllers::Pad::DOWN, Nes::Api::Input::Controllers::Pad::LEFT, Nes::Api::Input::Controllers::Pad::RIGHT, Nes::Api::Input::Controllers::Pad::START, Nes::Api::Input::Controllers::Pad::SELECT
};

@implementation NESGameEmu

@synthesize romPath;

UInt32 bufInPos, bufOutPos, bufUsed;

static bool NST_CALLBACK VideoLock(void* userData, Nes::Api::Video::Output& video)
{
    DLog(@"Locking: %@", userData);
    return [(NESGameEmu*)userData lockVideo:&video];
}

static void NST_CALLBACK VideoUnlock(void* userData, Nes::Api::Video::Output& video)
{
    [(NESGameEmu*)userData unlockVideo:&video];
}

static bool NST_CALLBACK SoundLock(void* userData,Nes::Api::Sound::Output& sound)
{
    return [(NESGameEmu*)userData lockSound];
}

static void NST_CALLBACK SoundUnlock(void* userData,Nes::Api::Sound::Output& sound)
{
    [(NESGameEmu *)userData unlockSound];
}

- (id)init;
{
    self = [super init];
    if(self)
    {
        _nesSound = new Nes::Api::Sound::Output();
        _nesVideo = new Nes::Api::Video::Output();
        _controls = new Nes::Api::Input::Controllers();
        _emu = new Nes::Api::Emulator();
        soundLock = [[NSLock alloc] init];
        videoLock = [[NSLock alloc] init];
    }
    return self;
}

// for various file operations, usually called during image file load, power on/off and reset
void NST_CALLBACK doFileIO(void *userData, Nes::Api::User::File& file)
{
    NESGameEmu *self = (NESGameEmu *)userData;
    
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSString *path = self->romPath;
    
    NSString *extensionlessFilename = [[path lastPathComponent] stringByDeletingPathExtension];
    NSString *batterySavesDirectory = [self batterySavesDirectoryPath];
    [[NSFileManager defaultManager] createDirectoryAtPath:batterySavesDirectory withIntermediateDirectories:YES attributes:nil error:NULL];
    
    NSData* theData;
    NSString* filePath;
    
    DLog(@"Doing file IO");
    switch (file.GetAction())
    {
        case Nes::Api::User::File::LOAD_SAMPLE:
        {
            /*
             XADArchive* romArchive = (XADArchive*)userData;
             const wchar_t* romInZip = file.GetName();
             NSString *romName = 
             (NSString *) CFStringCreateWithBytes(kCFAllocatorDefault, 
             (const UInt8 *) romInZip,
             wcslen(romInZip) * sizeof(wchar_t),
             kCFStringEncodingUTF32LE, false);
             DLog(romName);
             int fileIndex = -1;
             for(int i = 0; i < [romArchive numberOfEntries]; i++)
             {
             if([[romArchive nameOfEntry:i] isEqualToString:romName] )
             {
             fileIndex = i;
             break;
             }
             }
             theData = [romArchive contentsOfEntry:fileIndex];
             file.SetSampleContent([theData bytes] , [theData length], false, 32, 44100);
             */
            break;
        }
        case Nes::Api::User::File::LOAD_ROM:
        {
            /*
             XADArchive* romArchive = (XADArchive*)userData;
             const wchar_t* romInZip = file.GetName();
             NSString *romName = 
             (NSString *) CFStringCreateWithBytes(kCFAllocatorDefault, 
             (const UInt8 *) romInZip,
             wcslen(romInZip) * sizeof(wchar_t),
             kCFStringEncodingUTF32LE, false);
             DLog(romName);
             int fileIndex = -1;
             for(int i = 0; i < [romArchive numberOfEntries]; i++)
             {
             if([[romArchive nameOfEntry:i] isEqualToString:romName] )
             {
             fileIndex = i;
             break;
             }
             }
             theData = [romArchive contentsOfEntry:fileIndex];
             file.SetContent([theData bytes] , [theData length]);
             */
            break;
        }
            
        case Nes::Api::User::File::LOAD_BATTERY: // load in battery data from a file
        case Nes::Api::User::File::LOAD_EEPROM: // used by some Bandai games, can be treated the same as battery files
        {
            DLog(@"Trying to load EEPROM");
            filePath = [batterySavesDirectory stringByAppendingPathComponent:[extensionlessFilename stringByAppendingPathExtension:@"sav"]];
            DLog(@"%@",filePath);
            if(![fileManager fileExistsAtPath:filePath])
            {
                NSLog(@"Couldn't find save");
                return;
            }
            theData = [NSData dataWithContentsOfFile:filePath];
            file.SetContent([theData bytes], [theData length]);
            break;
        }
        case Nes::Api::User::File::SAVE_BATTERY: // save battery data to a file
        case Nes::Api::User::File::SAVE_EEPROM: // can be treated the same as battery files
        {
            
            DLog(@"Trying to save EEPROM");
            const void* savedata;
            unsigned long savedatasize;
            file.GetContent( savedata, savedatasize );
            filePath = [batterySavesDirectory stringByAppendingPathComponent:[extensionlessFilename stringByAppendingPathExtension:@"sav"]];
            theData = [NSData dataWithBytes:savedata length:savedatasize];
            [theData writeToFile:filePath atomically:YES];
            break;
        }
            
        case Nes::Api::User::File::SAVE_FDS: // for saving modified Famicom Disk System files
        {
            /*
             char fdsname[512];
             
             sprintf(fdsname, "%s.fds", savename);
             
             std::ofstream fdsFile( fdsname, std::ifstream::out|std::ifstream::binary );
             
             if (fdsFile.is_open())
             fdsFile.write( (const char*) &data.front(), data.size() );
             
             break;
             */
        }
            
        case Nes::Api::User::File::LOAD_TAPE: // for loading Famicom cassette tapes
            DLog(@"Loading tape");
            break;
        case Nes::Api::User::File::SAVE_TAPE: // for saving Famicom cassette tapes
        case Nes::Api::User::File::LOAD_TURBOFILE: // for loading turbofile data
        case Nes::Api::User::File::SAVE_TURBOFILE: // for saving turbofile data
            break;
        case Nes::Api::User::File::LOAD_SAMPLE_MOERO_PRO_YAKYUU:
            
            DLog(@"Asked for sample Moreo");
            break;
        case Nes::Api::User::File::LOAD_SAMPLE_MOERO_PRO_YAKYUU_88:
            DLog(@"Asked for sample Moreo 88");
            break;
        case Nes::Api::User::File::LOAD_SAMPLE_MOERO_PRO_TENNIS:
            DLog(@"Asked for sample Moreo Tennis");
            break;
        case Nes::Api::User::File::LOAD_SAMPLE_TERAO_NO_DOSUKOI_OOZUMOU:
        case Nes::Api::User::File::LOAD_SAMPLE_AEROBICS_STUDIO:
            break;
    }
}


Nes::Api::User::Answer NST_CALLBACK doQuestion(void* userData, Nes::Api::User::Question question)
{
    switch(question)
    {
        case Nes::Api::User::QUESTION_NST_PRG_CRC_FAIL_CONTINUE:
            break;
        case Nes::Api::User::QUESTION_NSV_PRG_CRC_FAIL_CONTINUE:
            break;
    }
    
    NSLog(@"CRC Failed");
    return Nes::Api::User::ANSWER_DEFAULT;
    
}

void NST_CALLBACK doLog(void* userData, const char* text,unsigned long length)
{
    NSLog(@"%@",[NSString stringWithUTF8String:text]);
}

void NST_CALLBACK doEvent(void* userData, Nes::Api::Machine::Event event,Nes::Result result)
{
    switch(event)
    {
        case Nes::Api::Machine::EVENT_LOAD:
            NSLog(@"Load returned : %d", result);
            break;
        case Nes::Api::Machine::EVENT_UNLOAD:
            NSLog(@"Unload returned : %d", result);
            break;
        case Nes::Api::Machine::EVENT_POWER_ON:
            NSLog(@"Power on returned : %d", result);
            break;
        case Nes::Api::Machine::EVENT_POWER_OFF:
            NSLog(@"Power off returned : %d", result);
            break;
        case Nes::Api::Machine::EVENT_RESET_SOFT:
        case Nes::Api::Machine::EVENT_RESET_HARD:
        case Nes::Api::Machine::EVENT_MODE_NTSC:
        case Nes::Api::Machine::EVENT_MODE_PAL:
            break;
            
    }
    
}

- (const void *)videoBuffer
{
    return videoBuffer;
}

- (BOOL)lockVideo:(void *)_video
{
    Nes::Api::Video::Output *video = (Nes::Api::Video::Output *)_video;
    [videoLock lock];
    video->pixels = (void*)videoBuffer;
    video->pitch = width*4;
    return true;
}

- (void)unlockVideo:(void *)_video
{
    Nes::Api::Video::Output *video = (Nes::Api::Video::Output *)_video;
    [videoLock unlock];
    video->pitch = NULL;
}

- (BOOL)lockSound
{
    if([soundLock tryLock])
    {
        return YES;
    }
    return NO;
}

- (void)unlockSound
{
    [soundLock unlock];
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

- (NSTimeInterval) frameInterval
{
    Nes::Api::Machine machine(*emu);
    
    if ( machine.GetMode() == Nes::Api::Machine::NTSC )
    {
        return 60;
    }
    else
    {
        return 50;
    }
}

- (BOOL)loadFileAtPath: (NSString*) path
{
    Nes::Result result; 
    
    Nes::Api::Machine machine(*emu);
    
    
    Nes::Api::Cartridge::Database database(*emu);
    
    if(!database.IsLoaded())
    {
        
        NSString *databasePath = [[NSBundle bundleForClass:[self class]] pathForResource:@"NstDatabase" ofType:@"xml"];
        if(databasePath != nil)
        {
            DLog(@"Loading database");
            std::ifstream databaseStream([databasePath cStringUsingEncoding:NSUTF8StringEncoding], std::ifstream::in | std::ifstream::binary);
            database.Load(databaseStream);
            database.Enable(true);
            databaseStream.close();
        }
    }
    
    [self setRomPath:path];
    
    Nes::Api::User::fileIoCallback.Set(doFileIO, self);
    Nes::Api::User::logCallback.Set(doLog, self);
    Nes::Api::Machine::eventCallback.Set(doEvent, self);
    Nes::Api::User::questionCallback.Set(doQuestion, self);
    
    std::ifstream romFile([path cStringUsingEncoding:NSUTF8StringEncoding], std::ios::in | std::ios::binary);
    result = machine.Load(romFile, Nes::Api::Machine::FAVORED_NES_PAL, Nes::Api::Machine::ASK_PROFILE);
    
    
    if(NES_FAILED(result)) {
        NSString *errorDescription = nil;
        switch(result) {
            case Nes::RESULT_ERR_INVALID_FILE:
                errorDescription = NSLocalizedString(@"Invalid file.",@"Invalid file.");
                break;
            case Nes::RESULT_ERR_OUT_OF_MEMORY:
                errorDescription = NSLocalizedString(@"Out of memory.",@"Out of memory.");
                break;
            case Nes::RESULT_ERR_CORRUPT_FILE:
                errorDescription = NSLocalizedString(@"Corrupt file.",@"Corrupt file.");
                break;
            case Nes::RESULT_ERR_UNSUPPORTED_MAPPER:
                errorDescription = NSLocalizedString(@"Unsupported mapper.",@"Unsupported mapper.");
                break;
            case Nes::RESULT_ERR_MISSING_BIOS:
                errorDescription = NSLocalizedString(@"Can't find fdisksys.rom for FDS game.",@"Can't find fdisksys.rom for FDS game.");
                break;
            default:
                errorDescription = [NSString stringWithFormat:NSLocalizedString(@"Unknown nestopia error #%d.",@"Unknown nestopia error #%d."),result];
                break;
        }
        NSLog(@"%@",errorDescription); 
        
        return NO;
    }
    machine.Power(true);
    
    return YES;
}


- (void)setupAudio:(Nes::Api::Emulator*)emulator
{
    Nes::Api::Sound sound( *emulator );
    //Nes::Api::Machine machine( *emulator );
    sound.SetSampleBits( 16 );
    sound.SetSampleRate( SAMPLERATE );
    sound.SetVolume(Nes::Api::Sound::ALL_CHANNELS, 100);
    sound.SetSpeaker( Nes::Api::Sound::SPEAKER_STEREO );
    sound.SetSpeed( [self frameInterval] );
    
    bufFrameSize = (SAMPLERATE / [self frameInterval]);
    
    soundBuffer = new UInt16[bufFrameSize * [self channelCount]];
    [[self ringBufferAtIndex:0] setLength:(sizeof(UInt16) * bufFrameSize * [self channelCount] * 5)];
    
    memset(soundBuffer, 0, bufFrameSize * [self channelCount] * sizeof(UInt16));
    nesSound->samples[0] = soundBuffer;
    nesSound->length[0] = bufFrameSize;
    nesSound->samples[1] = NULL;
    nesSound->length[1] = 0;
}

static Nes::Api::Video::RenderState::Filter filters[2] = 
{ 
    Nes::Api::Video::RenderState::FILTER_NONE, 
    Nes::Api::Video::RenderState::FILTER_NTSC, 
    //Nes::Api::Video::RenderState::FILTER_SCALE2X, 
    //Nes::Api::Video::RenderState::FILTER_SCALE3X, 
    //Nes::Api::Video::RenderState::FILTER_HQ2X, 
    //Nes::Api::Video::RenderState::FILTER_HQ3X, 
    //Nes::Api::Video::RenderState::FILTER_HQ4X 
};
static int Widths[2] =
{
    Nes::Api::Video::Output::WIDTH,
    Nes::Api::Video::Output::NTSC_WIDTH,
    //Nes::Api::Video::Output::WIDTH*2,
    //Nes::Api::Video::Output::WIDTH*3,
    //Nes::Api::Video::Output::WIDTH*2,
    //Nes::Api::Video::Output::WIDTH*3,
    //Nes::Api::Video::Output::WIDTH*4,
};
static int Heights[2] =
{
    Nes::Api::Video::Output::HEIGHT,
    Nes::Api::Video::Output::HEIGHT,
    //Nes::Api::Video::Output::HEIGHT*2,
    //Nes::Api::Video::Output::HEIGHT*3,
    //Nes::Api::Video::Output::HEIGHT*2,
    //Nes::Api::Video::Output::HEIGHT*3,
    //Nes::Api::Video::Output::HEIGHT*4,
};

- (void)setupVideo:(void *)_emulator withFilter:(int)filter
{
    Nes::Api::Emulator *emulator = (Nes::Api::Emulator *)_emulator;
    // renderstate structure
    Nes::Api::Video::RenderState* renderState = new Nes::Api::Video::RenderState();
    
    Nes::Api::Machine machine( *emulator );
    Nes::Api::Cartridge::Database database( *emulator );
    
    
    //machine.SetMode(Nes::Api::Machine::NTSC);
    
    width =Widths[filter]; 
    height = Heights[filter];
    DLog(@"buffer dim width: %d, height: %d\n", width, height);
    [videoLock lock];
    if(videoBuffer)
        delete[] videoBuffer;
    videoBuffer = new unsigned char[width * height * 4];
    [videoLock unlock];
    
    renderState->bits.count = 32;
    renderState->bits.mask.r = 0xFF0000;
    renderState->bits.mask.g = 0x00FF00;
    renderState->bits.mask.b = 0x0000FF;
    
    renderState->filter = filters[filter];
    renderState->width = Widths[filter];
    renderState->height = Heights[filter];
    
    Nes::Api::Video video( *emulator );
    
    [self toggleUnlimitedSprites:nil];
    
    // set up the NTSC type
    /*
     switch (0)
     {
     
     case 0:    // composite
     video.SetSharpness(Nes::Api::Video::DEFAULT_SHARPNESS_COMP);
     video.SetColorResolution(Nes::Api::Video::DEFAULT_COLOR_RESOLUTION_COMP);
     video.SetColorBleed(Nes::Api::Video::DEFAULT_COLOR_BLEED_COMP);
     video.SetColorArtifacts(Nes::Api::Video::DEFAULT_COLOR_ARTIFACTS_COMP);
     video.SetColorFringing(Nes::Api::Video::DEFAULT_COLOR_FRINGING_COMP);
     break;
     
     case 1:    // S-Video
     video.SetSharpness(Nes::Api::Video::DEFAULT_SHARPNESS_SVIDEO);
     video.SetColorResolution(Nes::Api::Video::DEFAULT_COLOR_RESOLUTION_SVIDEO);
     video.SetColorBleed(Nes::Api::Video::DEFAULT_COLOR_BLEED_SVIDEO);
     video.SetColorArtifacts(Nes::Api::Video::DEFAULT_COLOR_ARTIFACTS_SVIDEO);
     video.SetColorFringing(Nes::Api::Video::DEFAULT_COLOR_FRINGING_SVIDEO);
     break;
     
     case 2:    // RGB
     video.SetSharpness(Nes::Api::Video::DEFAULT_SHARPNESS_RGB);
     video.SetColorResolution(Nes::Api::Video::DEFAULT_COLOR_RESOLUTION_RGB);
     video.SetColorBleed(Nes::Api::Video::DEFAULT_COLOR_BLEED_RGB);
     video.SetColorArtifacts(Nes::Api::Video::DEFAULT_COLOR_ARTIFACTS_RGB);
     video.SetColorFringing(Nes::Api::Video::DEFAULT_COLOR_FRINGING_RGB);
     break;
     }*/
    /*
     video.SetSharpness([self sharpness]);
     video.SetColorResolution([self colorRes]);
     video.SetColorBleed([self colorBleed]);
     video.SetBrightness([self brightness]);
     video.SetContrast([self contrast]);
     video.SetColorArtifacts([self colorArtifacts]);
     video.SetHue([self hue]);
     video.SetColorFringing([self colorFringing]);
     video.SetSaturation([self saturation]);
     */
    // set the render state, make use of the NES_FAILED macro, expands to: "if (function(...) < Nes::RESULT_OK)"
    if (NES_FAILED(video.SetRenderState( *renderState )))
    {
        printf("NEStopia core rejected render state\n");
        exit(0);
    }
    DLog(@"Loaded video");
    
    nesVideo->pixels = (void*)videoBuffer;
    nesVideo->pitch = width*4;
}

- (void)settingWasSet:(id)aValue forKey:(NSString *)keyName
{
    Nes::Api::Video video( *emu );
    NSInteger value = [aValue integerValue];
    if([NESNTSC isEqualToString:keyName])
    {
        if(value)
            [self setupVideo:emu withFilter:1];
        else
            [self setupVideo:emu withFilter:0];
    }
    else if([NESBrightness isEqualToString:keyName])
        video.SetBrightness(value);
    else if([NESSaturation isEqualToString:keyName])
        video.SetSaturation(value);
    else if([NESContrast isEqualToString:keyName])
        video.SetContrast(value);
    else if([NESSharpness isEqualToString:keyName])
        video.SetSharpness(value);
    else if([NESColorRes isEqualToString:keyName])
        video.SetColorResolution(value);
    else if([NESColorBleed isEqualToString:keyName])
        video.SetColorBleed(value);
    else if([NESColorArtifacts isEqualToString:keyName])
        video.SetColorArtifacts(value);
    else if([NESColorFringing isEqualToString:keyName])
        video.SetColorFringing(value);
    else if([NESHue isEqualToString:keyName])
        video.SetHue(value);
}

- (void)setupEmulation
{
    //soundLock = [[NSLock alloc] init];
    // Lets set up the database!
    
    Nes::Api::Cartridge::Database database(*emu);
    
    if(!database.IsLoaded())
    {
        
        NSString *databasePath = [[NSBundle bundleForClass:[self class]] pathForResource:@"NstDatabase" ofType:@"xml"];
        if(databasePath != nil)
        {
            DLog(@"Loading database");
            std::ifstream databaseStream([databasePath cStringUsingEncoding:NSUTF8StringEncoding], std::ifstream::in | std::ifstream::binary);
            database.Load(databaseStream);
            database.Enable(true);
            databaseStream.close();
        }
    }
    
    if(database.IsLoaded())
    {
        DLog(@"Database loaded");
        Nes::Api::Input(*emu).AutoSelectControllers();
    }
    else {
        Nes::Api::Input(*emu).ConnectController(0,Nes::Api::Input::PAD1);
    }
    
    
    Nes::Api::Machine machine(*emu);
    machine.SetMode(machine.GetDesiredMode());
    
    //nesControls = new Nes::Api::Input::Controllers;
    //[inputController setupNesController:nesControls];
    if([self isNTSCEnabled])
        [self setupVideo:emu withFilter:1];//[[[OpenNestopiaPreferencesController sharedPreferencesController:self] filter] intValue]];
    else
        [self setupVideo:emu withFilter:0];
    [self setupAudio:emu];
    
    DLog(@"Setup");
}

# pragma mark -

- (void)executeFrame
{
    //Get a reference to the emulator
    [videoLock lock];
    [soundLock lock];
    emu->Execute(nesVideo, nesSound, controls);
    [[self ringBufferAtIndex:0] write:soundBuffer maxLength:[self channelCount] * bufFrameSize * sizeof(UInt16)];
    //DLog(@"Wrote %d frames", frames);
    [videoLock unlock];
    [soundLock unlock];
}

# pragma mark -

- (void)resetEmulation
{
    DLog(@"Resetting NES");
    Nes::Api::Machine machine(*emu);
    machine.Reset(true);
}

- (void)dealloc
{
    delete[] soundBuffer;
    delete[] videoBuffer;
    delete emu;
    delete nesSound;
    delete nesVideo;
    delete controls;
    [romPath release];
    [videoLock release];
    [soundLock release];
    [super dealloc];
}

- (void)didPushNESButton:(OENESButton)button forPlayer:(NSUInteger)player;
{
    controls->pad[player - 1].buttons |=  NESControlValues[button];
}

- (void)didReleaseNESButton:(OENESButton)button forPlayer:(NSUInteger)player;
{
    controls->pad[player - 1].buttons &= ~NESControlValues[button];
}

- (NSUInteger)NES_buttonMaskForButton:(OEButton)gameButton
{
    NSUInteger button = 0;
    switch (gameButton)
    {
        case OEButton_1      : button = Nes::Api::Input::Controllers::Pad::A;      break;
        case OEButton_2      : button = Nes::Api::Input::Controllers::Pad::B;      break;
        case OEButton_Up     : button = Nes::Api::Input::Controllers::Pad::UP;     break;
        case OEButton_Down   : button = Nes::Api::Input::Controllers::Pad::DOWN;   break;
        case OEButton_Left   : button = Nes::Api::Input::Controllers::Pad::LEFT;   break;
        case OEButton_Right  : button = Nes::Api::Input::Controllers::Pad::RIGHT;  break;
        case OEButton_Start  : button = Nes::Api::Input::Controllers::Pad::START;  break;
        case OEButton_Select : button = Nes::Api::Input::Controllers::Pad::SELECT; break;
    }
    
    return button;
}

- (void)player:(NSUInteger)thePlayer didPressButton:(OEButton)gameButton;
{
    controls->pad[thePlayer].buttons |= [self NES_buttonMaskForButton:gameButton];
}

- (void)player:(NSUInteger)thePlayer didReleaseButton:(OEButton)gameButton
{
    controls->pad[thePlayer].buttons &= ~[self NES_buttonMaskForButton:gameButton];
}

- (NSUInteger)soundBufferSize
{
    return bufFrameSize * 4;
}

- (NSUInteger)frameSampleCount
{
    return bufFrameSize;
}

- (NSUInteger)frameSampleRate
{
    return SAMPLERATE;
}

- (NSUInteger)channelCount
{
    return 2;
}

- (BOOL)saveStateToFileAtPath:(NSString *)fileName
{
    const char* filename = [fileName UTF8String];
    
    Nes::Result result; 
    
    Nes::Api::Machine machine(*emu);
    std::ofstream stateFile( filename, std::ifstream::out|std::ifstream::binary );
    
    if (stateFile.is_open())
        result = machine.SaveState(stateFile, Nes::Api::Machine::NO_COMPRESSION );
    
    if(NES_FAILED(result)) {
        NSString *errorDescription = nil;
        switch(result) {
            case Nes::RESULT_ERR_NOT_READY:
                errorDescription = NSLocalizedString(@"Not ready to save state.",@"Not ready to save state.");
                break;
            case Nes::RESULT_ERR_OUT_OF_MEMORY:
                errorDescription = NSLocalizedString(@"Out of memory.",@"Out of memory.");
                break;
            default:
                errorDescription = [NSString stringWithFormat:NSLocalizedString(@"Unknown nestopia error #%d.",@"Unknown nestopia error #%d."),result];
                break;
        }
        NSLog(@"%@",errorDescription); 
        
        return NO;
    }
    
    stateFile.close();
    
    return YES;
}

- (BOOL)loadStateFromFileAtPath:(NSString *)fileName
{
    Nes::Result result; 
    
    Nes::Api::Machine machine(*emu);
    std::ifstream stateFile( [fileName UTF8String], std::ifstream::in|std::ifstream::binary );
    
    if (stateFile.is_open())
        result = machine.LoadState(stateFile);
    
    if(NES_FAILED(result)) {
        NSString *errorDescription = nil;
        switch(result) {
            case Nes::RESULT_ERR_NOT_READY:
                errorDescription = NSLocalizedString(@"Not ready to save state.",@"Not ready to save state.");
                break;
            case Nes::RESULT_ERR_INVALID_CRC:
                errorDescription = NSLocalizedString(@"Invalid CRC checksum.",@"Invalid CRC checksum.");
                break;
            case Nes::RESULT_ERR_OUT_OF_MEMORY:
                errorDescription = NSLocalizedString(@"Out of memory.",@"Out of memory.");
                break;
            default:
                errorDescription = [NSString stringWithFormat:NSLocalizedString(@"Unknown nestopia error #%d.",@"Unknown nestopia error #%d."),result];
                break;
        }
        NSLog(@"%@",errorDescription); 
        
        return NO;
    }
    
    return YES;
}

- (OEIntSize)bufferSize
{
    return [self isNTSCEnabled] ? OESizeMake(Widths[1], Heights[1]*2)
                                : OESizeMake(Widths[0], Heights[0]);
}

@end
