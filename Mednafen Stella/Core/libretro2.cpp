#ifndef _MSC_VER
#include <stdbool.h>
#endif
#include <sched.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#ifndef __CELLOS_LV2__
//#include <malloc.h>
#endif

#ifdef _MSC_VER
#define snprintf _snprintf
#pragma pack(1)
#endif

#include "libretro.h"

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

static uint16_t frame_buffer[256*160];

struct bind_conv
{
    int retro;
    int atari;
};
/*
static struct bind_conv binds[] = {
    { RETRO_DEVICE_ID_JOYPAD_B, JoystickZeroFire1 },
    { RETRO_DEVICE_ID_JOYPAD_A, JoystickZeroFire2 },
    { RETRO_DEVICE_ID_JOYPAD_X, JoystickZeroFire3 },
    { RETRO_DEVICE_ID_JOYPAD_UP, JoystickZeroUp },
    { RETRO_DEVICE_ID_JOYPAD_DOWN, JoystickZeroDown },
    { RETRO_DEVICE_ID_JOYPAD_LEFT, JoystickZeroLeft },
    { RETRO_DEVICE_ID_JOYPAD_RIGHT, JoystickZeroRight },
    { RETRO_DEVICE_ID_JOYPAD_SELECT, ConsoleSelect },
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

static retro_video_refresh_t video_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;
static retro_environment_t environ_cb;
static retro_audio_sample_t audio_cb;
static retro_audio_sample_batch_t audio_batch_cb;

void retro_set_environment(retro_environment_t cb) { environ_cb = cb; }
void retro_set_video_refresh(retro_video_refresh_t cb) { video_cb = cb; }
void retro_set_audio_sample(retro_audio_sample_t cb) { audio_cb = cb; }
void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb) { audio_batch_cb = cb; }
void retro_set_input_poll(retro_input_poll_t cb) { input_poll_cb = cb; }
void retro_set_input_state(retro_input_state_t cb) { input_state_cb = cb; }

/************************************
 * libretro implementation
 ************************************/

static struct retro_system_av_info g_av_info;

void retro_get_system_info(struct retro_system_info *info)
{
    memset(info, 0, sizeof(*info));
	info->library_name = "Stella";
	info->library_version = "3.4.1";
	info->need_fullpath = true;
	info->valid_extensions = "a26|A26";
}

void retro_get_system_av_info(struct retro_system_av_info *info)
{
    memset(info, 0, sizeof(*info));
    // Just assume NTSC for now. TODO: Verify FPS.
    info->timing.fps            = stella->GameConsole->getFramerate();
    info->timing.sample_rate    = 31400;
    info->geometry.base_width   = 160;
    info->geometry.base_height  = 210;
    info->geometry.max_width    = 160;
    info->geometry.max_height   = 256;
    info->geometry.aspect_ratio = 4.0 / 3.0;
}

void retro_set_controller_port_device(unsigned port, unsigned device)
{
   (void)port;
   (void)device;
}

size_t retro_serialize_size(void) 
{ 
	//return STATE_SIZE;
	return 0;
}

bool retro_serialize(void *data, size_t size)
{ 
   //if (size != STATE_SIZE)
   return false;
}

bool retro_unserialize(const void *data, size_t size)
{
   //if (size != STATE_SIZE)
   return false;
}

void retro_cheat_reset(void)
{}

void retro_cheat_set(unsigned index, bool enabled, const char *code)
{
   (void)index;
   (void)enabled;
   (void)code;
}

bool retro_load_game(const struct retro_game_info *info)
{
    stella = new Stella();
    const char *full_path;
    
    full_path = info->path;
    
	//Get the game properties
    string cartMD5 = MD5((const uInt8*)info->data, info->size);
    PropertiesSet propslist(0);
    Properties gameProperties;
    propslist.getMD5(cartMD5, gameProperties);
    
    //Load the cart
    string cartType = gameProperties.get(Cartridge_Type);
    string cartID = "";
    Cartridge* stellaCart = Cartridge::create((const uInt8*)info->data, info->size, cartMD5, cartType, cartID, stella->GameSettings);
    
    if(stellaCart == 0)
    {
        printf("Stella: Failed to load cartridge.");
        return false;
    }
    
    //Create the console
    stella->GameConsole = new Console(stella->Sound, stellaCart, gameProperties);
    
    //Init sound
    stella->Sound.open();
    
    //stella->Palette = 0; //NTSC

   return true;
}

bool retro_load_game_special(unsigned game_type, const struct retro_game_info *info, size_t num_info)
{
   (void)game_type;
   (void)info;
   (void)num_info;
   return false;
}

void retro_unload_game(void) 
{

}

unsigned retro_get_region(void)
{
    //stella->GameConsole->getFramerate();
    return RETRO_REGION_NTSC;
}

unsigned retro_api_version(void)
{
    return RETRO_API_VERSION;
}

void *retro_get_memory_data(unsigned id)
{
    return NULL;
}

size_t retro_get_memory_size(unsigned id)
{
    return 0;
}

void retro_init(void)
{
    unsigned level = 3;
    environ_cb(RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL, &level);
}

void retro_deinit(void)
{
    delete stella;
    stella = 0;
	//free(frame_buffer);
}

void retro_reset(void)
{
	stella->GameConsole->system().reset();
}

void retro_run(void)
{
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
            stella->GameConsole->event().set((Event::Type)(baseEvent j), inputState & 1);
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
    
    stella->GameConsole->tia().update();
    
    Int32 frameWidth = stella->GameConsole->tia().width();
    Int32 frameHeight = stella->GameConsole->tia().height();
    
    for(int i = 0; i != frameHeight; i ++)
        {
            for(int j = 0; j != frameWidth; j ++)
                {
                    Int32 pixel = stella->Palette[stella->GameConsole->tia().currentFrameBuffer()[i * frameWidth + j]];
                    uint16_t color = (pixel & 0x0000F8) >> 3; // B
                    color |= (pixel & 0x00F800) >> 6; // G
                    color |= (pixel & 0xF80000) >> 9; // R
                    frame_buffer[i * frameWidth + j] = color;
                    }
            }
    
    video_cb(frame_buffer, frameWidth, frameHeight, frameWidth * 2);

    //AUDIO
    //Get the number of samples in a frame
    uint32_t soundFrameSize = 34100.0f / stella->GameConsole->getFramerate();
    //uint32_t soundFrameSize = 44100.0f / stella->GameConsole->getFramerate();
    
    //Process one frame of audio from stella
    uint8_t samplebuffer[2048];
    stella->Sound.processFragment(samplebuffer, soundFrameSize);
    
    //Convert and stash it in the resampler...
    for(int i = 0; i != soundFrameSize; i ++)
    {
        int16_t sample = (samplebuffer[i] << 8) - 32768;
        int16_t frame[2] = {sample, sample};
        //Resampler::Fill(frame, 2);
        audio_cb(frame[0], frame[1]);
    }
    //TODO: fix
    //audio_batch_cb(NULL, NULL);
}
