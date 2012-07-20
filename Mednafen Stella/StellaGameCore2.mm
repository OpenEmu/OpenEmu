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

#import "StellaGameCore.h"
#import <OERingBuffer.h>
#import "OEVBSystemResponderClient.h"
#import <OpenGL/gl.h>

//#include <mednafen/mednafen.h>
//#include <mednafen/mempatcher.h>
//#include <mednafen/git.h>
//#include "libretro.h"

#include "Settings.hxx"
#include "TIA.hxx"
#include "Props.hxx"
#include "PropsSet.hxx"
#include "Cart.hxx"
#include "Console.hxx"
#include "Serializer.hxx"
#include "Event.hxx"
#include "Switches.hxx"
#include "MD5.hxx"
#include "SoundSDL.hxx"

//extern MDFNGI EmulatedStella;

@interface StellaGameCore () <OEVBSystemResponderClient>
@end

struct					Stella
{
	Console*			GameConsole;
	Settings			GameSettings;
    
	const uInt32*		Palette;
    
	SoundSDL			Sound;
    
	Stella() 	{GameConsole = 0; Palette = 0;}
	~Stella()	{delete GameConsole;}
};
Stella*				stella;

NSUInteger VBEmulatorValues[] = { };
NSString *VBEmulatorKeys[] = { @"Joypad@ Up", @"Joypad@ Down", @"Joypad@ Left", @"Joypad@ Right", @"Joypad@ 1", @"Joypad@ 2", @"Joypad@ Run", @"Joypad@ Select"};

StellaGameCore *current;
@implementation StellaGameCore
/*
static MDFNSetting ModuleSettings[] =
{
    {"stella.ramrandom",	MDFNSF_NOFLAGS,		"Use random values to initialize RAM",					NULL, MDFNST_BOOL,		"1"},
    {"stella.fastscbios",	MDFNSF_NOFLAGS,		"Use quick method to init SuperCharger BIOS",			NULL, MDFNST_BOOL,		"0"},
    {"stella.colorloss",	MDFNSF_NOFLAGS,		"Emulate Color loss",									NULL, MDFNST_BOOL,		"0"},
    {"stella.tiadriven",	MDFNSF_NOFLAGS,		"Randomly toggle unused TIA pins",						NULL, MDFNST_BOOL,		"0"},
    {"stella.palette",		MDFNSF_NOFLAGS,		"Color Palette to use",									NULL, MDFNST_STRING,	"standard"},
    {"stella.framerate",	MDFNSF_NOFLAGS,		"Unused",												NULL, MDFNST_UINT,		"60", "30", "70"},
    {NULL}
};

MDFNGI EmulatedStella =
{
    "stella",
    "Atari 2600 (Stella)",
    NULL,
    MODPRIO_INTERNAL_HIGH,
#ifdef WANT_DEBUGGER
    NULL,
#else
    NULL,
#endif
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
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    MDFN_MASTERCLOCK_FIXED(6000), //3072000
    0,
    TRUE, // Multires possible?
    
    160,   // lcm_width //160
    5250,   // lcm_height //5250
    NULL,  // Dummy
    
    320,	// Nominal width //320
    250,	// Nominal height //250
    
    640,	// Framebuffer width //640
    480,	// Framebuffer height //480
    
    2,     // Number of output sound channels
};
*/

//Set the palette for the current stella instance
void stellaMDFNSetPalette (const uInt32* palette)
{
	if(stella)
	{
		stella->Palette = palette;
	}
}

//Get the settings from the current stella instance
Settings& stellaMDFNSettings ()
{
	if(stella)
	{
		return stella->GameSettings;
	}
    
	//HACK
	abort();
}

- (oneway void)didPushVBButton:(OEVBButton)button forPlayer:(NSUInteger)player;
{
    pad[player-1][VBEmulatorValues[button]] = 1;
}

- (oneway void)didReleaseVBButton:(OEVBButton)button forPlayer:(NSUInteger)player;
{
    pad[player-1][VBEmulatorValues[button]] = 0;
}

- (id)init
{
	self = [super init];
    if(self != nil)
    {
        //if(videoBuffer)
        //    free(videoBuffer);
        //videoBuffer = (uint16_t*)malloc(160 * 210 * 2);
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
{/*
    Int32 frameWidth = stella->GameConsole->tia().width();
    Int32 frameHeight = stella->GameConsole->tia().height();
    
    for(int i = 0; i != frameHeight; i ++)
    {
        for(int j = 0; j != frameWidth; j ++)
        {
            espec->surface->pixels[i * espec->surface->pitchinpix + j] = stella->Palette[stella->GameConsole->tia().currentFrameBuffer()[i * frameWidth + j] & 0xFF];
        }
    }*/
    
    stella->GameConsole->tia().update();
}

- (BOOL)loadFileAtPath: (NSString*) path
{
    stella = new Stella();
	memset(pad, 0, sizeof(int16_t) * 10);
    
    uInt8 *data;
    size_t size;
    romName = [path copy];
    
    //load cart, read bytes, get length
    NSData* dataObj = [NSData dataWithContentsOfFile:[romName stringByStandardizingPath]];
    if(dataObj == nil) return false;
    size = [dataObj length];
    data = (uint8_t*)[dataObj bytes];

    //const char *fullPath = [path UTF8String];
    //*(const char**)data = [current->romName cStringUsingEncoding:NSUTF8StringEncoding];
    
    //Get the game properties
    //string MD5(const uInt8* buffer, uInt32 length)
    string cartMD5 = MD5(data, size);
    PropertiesSet propslist(0);
    Properties gameProperties;
    propslist.getMD5(cartMD5, gameProperties);
    
    //Load the cart
    string cartType = gameProperties.get(Cartridge_Type);
    string cartID = "";
    //static Cartridge* create(const uInt8* image, uInt32 size, string& md5,
    //                         string& dtype, string& id, Settings& settings);
    Cartridge* stellaCart = Cartridge::create(data, size, cartMD5, cartType, cartID, stella->GameSettings);
    
    if(stellaCart == 0)
    {
        NSLog(@"Stella: Failed to load cartridge.");
        return NO;
    }
    
    //Create the console
    stella->GameConsole = new Console(stella->Sound, stellaCart, gameProperties);
    
    //Init sound
    stella->Sound.open();
    
    //frame rate
    stella->GameConsole->getFramerate();
    
    //stella->Palette = 0;
    
    /*
    //INPUT
    //Update stella's event structure
    for(int i = 0; i != 2; i ++)
    {
        //Get the base event id for this port
        Event::Type baseEvent = (i == 0) ? Event::JoystickZeroUp : Event::JoystickOneUp;
        
        //Get the input data for this port and stuff it in the event structure
        uint32_t inputState = Input::GetPort<3>(i);
        for(int j = 0; j != 19; j ++, inputState >>= 1)
        {
            stella->GameConsole->event().set((Event::Type)(baseEvent + j), inputState & 1);
        }
    }
    
    //Update the reset and select events
    uint32_t inputState = Input::GetPort<0, 3>() >> 19;
    stella->GameConsole->event().set(Event::ConsoleSelect, inputState & 1);
    stella->GameConsole->event().set(Event::ConsoleReset, inputState & 2);
    */
    //Tell all input devices to read their state from the event structure
    stella->GameConsole->switches().update();
    stella->GameConsole->controller(Controller::Left).update();
    stella->GameConsole->controller(Controller::Right).update();
    
    return YES;
}

#pragma mark Video
- (const void *)videoBuffer
{
    //videoBuffer = (uint16_t*)malloc(320 * 240 * 2);
    
    //Int32 frameWidth = stella->GameConsole->tia().width();
    //Int32 frameHeight = stella->GameConsole->tia().height();
    
    //for(int i = 0; i != frameHeight; i ++)
    //{
    //    for(int j = 0; j != frameWidth; j ++)
    //    {
            //espec->surface->pixels[i * espec->surface->pitchinpix + j] = stella->Palette[stella->GameConsole->tia().currentFrameBuffer()[i * frameWidth + j] & 0xFF];
            
            //stella->Palette[stella->GameConsole->tia().currentFrameBuffer()[i * frameWidth + j] & 0xFF];
            //memcpy(videoBuffer, stella->Palette[stella->GameConsole->tia().currentFrameBuffer()[i * frameWidth + j] & 0xFF], sizeof(uInt8) * stella->GameConsole->tia().width() * stella->GameConsole->tia().height());
    //    }
    //}
    
    //memcpy(videoBuffer, stella->GameConsole->tia().currentFrameBuffer(), sizeof(uInt8) * stella->GameConsole->tia().width() * stella->GameConsole->tia().height());

    return stella->GameConsole->tia().currentFrameBuffer();
    //return (const void*)stella->Palette[stella->GameConsole->tia().currentFrameBuffer()[120 * 160 + 120] & 0xFF];
}

- (OEIntRect)screenRect
{
    return OERectMake(0, 0, 160, 210); //320x192 or 320x210?
    //return OERectMake(0, 0, stella->GameConsole->tia().width(), stella->GameConsole->tia().height());
}

- (OEIntSize)bufferSize
{
    return OESizeMake(160, 210); //160x210 ?
    //return OESizeMake(stella->GameConsole->tia().width(), stella->GameConsole->tia().height());
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
    stella->GameConsole->system().reset();
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
        
        //writeSaveFile([filePath UTF8String], RETRO_MEMORY_SAVE_RAM);
    }
    
    NSLog(@"snes term");
    delete stella;
    stella = 0;
    [super stopEmulation];
}

- (void)dealloc
{
    free(videoBuffer);
    [super dealloc];
}

- (GLenum)pixelFormat
{
    return GL_RGB;
    //return GL_BGRA;
}

- (GLenum)pixelType
{
    return GL_UNSIGNED_SHORT_5_6_5;
    //return GL_UNSIGNED_SHORT_1_5_5_5_REV;
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
    return frameInterval ? frameInterval : stella->GameConsole->getFramerate();
}

- (NSUInteger)channelCount
{
    return 2;
}

- (BOOL)saveStateToFileAtPath:(NSString *)fileName
{   /*
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
     */
    return YES;
}

- (BOOL)loadStateFromFileAtPath:(NSString *)fileName
{   /*
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
    */
    return YES;
}

@end
