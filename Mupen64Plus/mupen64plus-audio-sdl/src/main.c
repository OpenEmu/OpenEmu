/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus-sdl-audio - main.c                                        *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2007-2009 Richard Goedeken                              *
 *   Copyright (C) 2007-2008 Ebenblues                                     *
 *   Copyright (C) 2003 JttL                                               *
 *   Copyright (C) 2002 Hactarux                                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>
#include <SDL_audio.h>

#ifdef USE_SRC
#include <samplerate.h>
#endif

#define M64P_PLUGIN_PROTOTYPES 1
#include "m64p_types.h"
#include "m64p_plugin.h"
#include "m64p_common.h"
#include "m64p_config.h"

#include "main.h"
#include "volume.h"
#include "osal_dynamiclib.h"
#include "osal_preproc.h"

/* Default start-time size of primary buffer (in equivalent output samples).
   This is the buffer where audio is loaded after it's extracted from n64's memory.
   This value must be larger than PRIMARY_BUFFER_TARGET */
#define PRIMARY_BUFFER_SIZE 16384

/* this is the buffer fullness level (in equivalent output samples) which is targeted
   for the primary audio buffer (by inserting delays) each time data is received from
   the running N64 program.  This value must be larger than the SECONDARY_BUFFER_SIZE.
   Decreasing this value will reduce audio latency but requires a faster PC to avoid
   choppiness. Increasing this will increase audio latency but reduce the chance of
   drop-outs. The default value 10240 gives a 232ms maximum A/V delay at 44.1khz */
#define PRIMARY_BUFFER_TARGET 10240

/* Size of secondary buffer, in output samples. This is the requested size of SDL's
   hardware buffer, and the size of the mix buffer for doing SDL volume control. The
   SDL documentation states that this should be a power of two between 512 and 8192. */
#define SECONDARY_BUFFER_SIZE 2048

/* This sets default frequency what is used if rom doesn't want to change it.
   Probably only game that needs this is Zelda: Ocarina Of Time Master Quest 
   *NOTICE* We should try to find out why Demos' frequencies are always wrong
   They tend to rely on a default frequency, apparently, never the same one ;)*/
#define DEFAULT_FREQUENCY 33600

/* number of bytes per sample */
#define N64_SAMPLE_BYTES 4
#define SDL_SAMPLE_BYTES 4

/* volume mixer types */
#define VOLUME_TYPE_SDL     1
#define VOLUME_TYPE_OSS     2

/* local variables */
static void (*l_DebugCallback)(void *, int, const char *) = NULL;
static void *l_DebugCallContext = NULL;
static int l_PluginInit = 0;
static int l_PausedForSync = 1; /* Audio is started in paused state after SDL initialization */
static m64p_handle l_ConfigAudio;

/* Read header for type definition */
static AUDIO_INFO AudioInfo;
/* The hardware specifications we are using */
static SDL_AudioSpec *hardware_spec;
/* Pointer to the primary audio buffer */
static unsigned char *primaryBuffer = NULL;
static unsigned int primaryBufferBytes = 0;
/* Pointer to the mixing buffer for voume control*/
static unsigned char *mixBuffer = NULL;
/* Position in buffer array where next audio chunk should be placed */
static unsigned int buffer_pos = 0;
/* Audio frequency, this is usually obtained from the game, but for compatibility we set default value */
static int GameFreq = DEFAULT_FREQUENCY;
/* timestamp for the last time that our audio callback was called */
static unsigned int last_callback_ticks = 0;
/* SpeedFactor is used to increase/decrease game playback speed */
static unsigned int speed_factor = 100;
// If this is true then left and right channels are swapped */
static int SwapChannels = 0;
// Size of Primary audio buffer in equivalent output samples
static unsigned int PrimaryBufferSize = PRIMARY_BUFFER_SIZE;
// Fullness level target for Primary audio buffer, in equivalent output samples
static unsigned int PrimaryBufferTarget = PRIMARY_BUFFER_TARGET;
// Size of Secondary audio buffer in output samples
static unsigned int SecondaryBufferSize = SECONDARY_BUFFER_SIZE;
// Resample or not
static unsigned char Resample = 1;
// volume to scale the audio by, range of 0..100
static int VolPercent = 80;
// how much percent to increment/decrement volume by
static int VolDelta = 5;
// the actual volume passed into SDL, range of 0..SDL_MIX_MAXVOLUME
static int VolSDL = SDL_MIX_MAXVOLUME;
// stores the previous volume when it is muted
static int VolMutedSave = -1;
//which type of volume control to use
static int VolumeControlType = VOLUME_TYPE_OSS;

static int OutputFreq;

// Prototype of local functions
static void my_audio_callback(void *userdata, unsigned char *stream, int len);
static void InitializeAudio(int freq);
static void ReadConfig(void);
static void InitializeSDL(void);

static int critical_failure = 0;

/* definitions of pointers to Core config functions */
ptr_ConfigOpenSection      ConfigOpenSection = NULL;
ptr_ConfigDeleteSection    ConfigDeleteSection = NULL;
ptr_ConfigSaveSection      ConfigSaveSection = NULL;
ptr_ConfigSetParameter     ConfigSetParameter = NULL;
ptr_ConfigGetParameter     ConfigGetParameter = NULL;
ptr_ConfigGetParameterHelp ConfigGetParameterHelp = NULL;
ptr_ConfigSetDefaultInt    ConfigSetDefaultInt = NULL;
ptr_ConfigSetDefaultFloat  ConfigSetDefaultFloat = NULL;
ptr_ConfigSetDefaultBool   ConfigSetDefaultBool = NULL;
ptr_ConfigSetDefaultString ConfigSetDefaultString = NULL;
ptr_ConfigGetParamInt      ConfigGetParamInt = NULL;
ptr_ConfigGetParamFloat    ConfigGetParamFloat = NULL;
ptr_ConfigGetParamBool     ConfigGetParamBool = NULL;
ptr_ConfigGetParamString   ConfigGetParamString = NULL;

/* Global functions */
static void DebugMessage(int level, const char *message, ...)
{
  char msgbuf[1024];
  va_list args;

  if (l_DebugCallback == NULL)
      return;

  va_start(args, message);
  vsprintf(msgbuf, message, args);

  (*l_DebugCallback)(l_DebugCallContext, level, msgbuf);

  va_end(args);
}

/* Mupen64Plus plugin functions */
EXPORT m64p_error CALL PluginStartup(m64p_dynlib_handle CoreLibHandle, void *Context,
                                   void (*DebugCallback)(void *, int, const char *))
{
    ptr_CoreGetAPIVersions CoreAPIVersionFunc;
    
    int ConfigAPIVersion, DebugAPIVersion, VidextAPIVersion, bSaveConfig;
    float fConfigParamsVersion = 0.0f;
    
    if (l_PluginInit)
        return M64ERR_ALREADY_INIT;

    /* first thing is to set the callback function for debug info */
    l_DebugCallback = DebugCallback;
    l_DebugCallContext = Context;

    /* attach and call the CoreGetAPIVersions function, check Config API version for compatibility */
    CoreAPIVersionFunc = (ptr_CoreGetAPIVersions) osal_dynlib_getproc(CoreLibHandle, "CoreGetAPIVersions");
    if (CoreAPIVersionFunc == NULL)
    {
        DebugMessage(M64MSG_ERROR, "Core emulator broken; no CoreAPIVersionFunc() function found.");
        return M64ERR_INCOMPATIBLE;
    }
    
    (*CoreAPIVersionFunc)(&ConfigAPIVersion, &DebugAPIVersion, &VidextAPIVersion, NULL);
    if ((ConfigAPIVersion & 0xffff0000) != (CONFIG_API_VERSION & 0xffff0000))
    {
        DebugMessage(M64MSG_ERROR, "Emulator core Config API (v%i.%i.%i) incompatible with plugin (v%i.%i.%i)",
                VERSION_PRINTF_SPLIT(ConfigAPIVersion), VERSION_PRINTF_SPLIT(CONFIG_API_VERSION));
        return M64ERR_INCOMPATIBLE;
    }

    /* Get the core config function pointers from the library handle */
    ConfigOpenSection = (ptr_ConfigOpenSection) osal_dynlib_getproc(CoreLibHandle, "ConfigOpenSection");
    ConfigDeleteSection = (ptr_ConfigDeleteSection) osal_dynlib_getproc(CoreLibHandle, "ConfigDeleteSection");
    ConfigSaveSection = (ptr_ConfigSaveSection) osal_dynlib_getproc(CoreLibHandle, "ConfigSaveSection");
    ConfigSetParameter = (ptr_ConfigSetParameter) osal_dynlib_getproc(CoreLibHandle, "ConfigSetParameter");
    ConfigGetParameter = (ptr_ConfigGetParameter) osal_dynlib_getproc(CoreLibHandle, "ConfigGetParameter");
    ConfigSetDefaultInt = (ptr_ConfigSetDefaultInt) osal_dynlib_getproc(CoreLibHandle, "ConfigSetDefaultInt");
    ConfigSetDefaultFloat = (ptr_ConfigSetDefaultFloat) osal_dynlib_getproc(CoreLibHandle, "ConfigSetDefaultFloat");
    ConfigSetDefaultBool = (ptr_ConfigSetDefaultBool) osal_dynlib_getproc(CoreLibHandle, "ConfigSetDefaultBool");
    ConfigSetDefaultString = (ptr_ConfigSetDefaultString) osal_dynlib_getproc(CoreLibHandle, "ConfigSetDefaultString");
    ConfigGetParamInt = (ptr_ConfigGetParamInt) osal_dynlib_getproc(CoreLibHandle, "ConfigGetParamInt");
    ConfigGetParamFloat = (ptr_ConfigGetParamFloat) osal_dynlib_getproc(CoreLibHandle, "ConfigGetParamFloat");
    ConfigGetParamBool = (ptr_ConfigGetParamBool) osal_dynlib_getproc(CoreLibHandle, "ConfigGetParamBool");
    ConfigGetParamString = (ptr_ConfigGetParamString) osal_dynlib_getproc(CoreLibHandle, "ConfigGetParamString");

    if (!ConfigOpenSection || !ConfigDeleteSection || !ConfigSetParameter || !ConfigGetParameter ||
        !ConfigSetDefaultInt || !ConfigSetDefaultFloat || !ConfigSetDefaultBool || !ConfigSetDefaultString ||
        !ConfigGetParamInt   || !ConfigGetParamFloat   || !ConfigGetParamBool   || !ConfigGetParamString)
        return M64ERR_INCOMPATIBLE;

    /* ConfigSaveSection was added in Config API v2.1.0 */
    if (ConfigAPIVersion >= 0x020100 && !ConfigSaveSection)
        return M64ERR_INCOMPATIBLE;

    /* get a configuration section handle */
    if (ConfigOpenSection("Audio-SDL", &l_ConfigAudio) != M64ERR_SUCCESS)
    {
        DebugMessage(M64MSG_ERROR, "Couldn't open config section 'Audio-SDL'");
        return M64ERR_INPUT_NOT_FOUND;
    }

    /* check the section version number */
    bSaveConfig = 0;
    if (ConfigGetParameter(l_ConfigAudio, "Version", M64TYPE_FLOAT, &fConfigParamsVersion, sizeof(float)) != M64ERR_SUCCESS)
    {
        DebugMessage(M64MSG_WARNING, "No version number in 'Audio-SDL' config section. Setting defaults.");
        ConfigDeleteSection("Audio-SDL");
        ConfigOpenSection("Audio-SDL", &l_ConfigAudio);
        bSaveConfig = 1;
    }
    else if (((int) fConfigParamsVersion) != ((int) CONFIG_PARAM_VERSION))
    {
        DebugMessage(M64MSG_WARNING, "Incompatible version %.2f in 'Audio-SDL' config section: current is %.2f. Setting defaults.", fConfigParamsVersion, (float) CONFIG_PARAM_VERSION);
        ConfigDeleteSection("Audio-SDL");
        ConfigOpenSection("Audio-SDL", &l_ConfigAudio);
        bSaveConfig = 1;
    }
    else if ((CONFIG_PARAM_VERSION - fConfigParamsVersion) >= 0.0001f)
    {
        /* handle upgrades */
        float fVersion = CONFIG_PARAM_VERSION;
        ConfigSetParameter(l_ConfigAudio, "Version", M64TYPE_FLOAT, &fVersion);
        DebugMessage(M64MSG_INFO, "Updating parameter set version in 'Audio-SDL' config section to %.2f", fVersion);
        bSaveConfig = 1;
    }

    /* set the default values for this plugin */
    ConfigSetDefaultFloat(l_ConfigAudio, "Version",             CONFIG_PARAM_VERSION,  "Mupen64Plus SDL Audio Plugin config parameter version number");
    ConfigSetDefaultInt(l_ConfigAudio, "DEFAULT_FREQUENCY",     DEFAULT_FREQUENCY,     "Frequency which is used if rom doesn't want to change it");
    ConfigSetDefaultBool(l_ConfigAudio, "SWAP_CHANNELS",        0,                     "Swaps left and right channels");
    ConfigSetDefaultInt(l_ConfigAudio, "PRIMARY_BUFFER_SIZE",   PRIMARY_BUFFER_SIZE,   "Size of primary buffer in output samples. This is where audio is loaded after it's extracted from n64's memory.");
    ConfigSetDefaultInt(l_ConfigAudio, "PRIMARY_BUFFER_TARGET", PRIMARY_BUFFER_TARGET, "Fullness level target for Primary audio buffer, in equivalent output samples");
    ConfigSetDefaultInt(l_ConfigAudio, "SECONDARY_BUFFER_SIZE", SECONDARY_BUFFER_SIZE, "Size of secondary buffer in output samples. This is SDL's hardware buffer.");
    ConfigSetDefaultInt(l_ConfigAudio, "RESAMPLE",              1,                     "Audio resampling algorithm.  1 = unfiltered, 2 = SINC resampling (Best Quality, requires libsamplerate)");
    ConfigSetDefaultInt(l_ConfigAudio, "VOLUME_CONTROL_TYPE",   VOLUME_TYPE_OSS,       "Volume control type: 1 = SDL (only affects Mupen64Plus output)  2 = OSS mixer (adjusts master PC volume)");
    ConfigSetDefaultInt(l_ConfigAudio, "VOLUME_ADJUST",         5,                     "Percentage change each time the volume is increased or decreased");
    ConfigSetDefaultInt(l_ConfigAudio, "VOLUME_DEFAULT",        80,                    "Default volume when a game is started.  Only used if VOLUME_CONTROL_TYPE is 1");

    if (bSaveConfig && ConfigAPIVersion >= 0x020100)
        ConfigSaveSection("Audio-SDL");

    l_PluginInit = 1;
    return M64ERR_SUCCESS;
}

EXPORT m64p_error CALL PluginShutdown(void)
{
    if (!l_PluginInit)
        return M64ERR_NOT_INIT;

    /* reset some local variables */
    l_DebugCallback = NULL;
    l_DebugCallContext = NULL;

    /* make sure our buffer is freed */
    if (mixBuffer != NULL)
    {
        free(mixBuffer);
        mixBuffer = NULL;
    }

    l_PluginInit = 0;
    return M64ERR_SUCCESS;
}

EXPORT m64p_error CALL PluginGetVersion(m64p_plugin_type *PluginType, int *PluginVersion, int *APIVersion, const char **PluginNamePtr, int *Capabilities)
{
    /* set version info */
    if (PluginType != NULL)
        *PluginType = M64PLUGIN_AUDIO;

    if (PluginVersion != NULL)
        *PluginVersion = SDL_AUDIO_PLUGIN_VERSION;

    if (APIVersion != NULL)
        *APIVersion = AUDIO_PLUGIN_API_VERSION;
    
    if (PluginNamePtr != NULL)
        *PluginNamePtr = "Mupen64Plus SDL Audio Plugin";

    if (Capabilities != NULL)
    {
        *Capabilities = 0;
    }
                    
    return M64ERR_SUCCESS;
}

/* ----------- Audio Functions ------------- */
EXPORT void CALL AiDacrateChanged( int SystemType )
{
    int f = GameFreq;

    if (!l_PluginInit)
        return;

    switch (SystemType)
    {
        case SYSTEM_NTSC:
            f = 48681812 / (*AudioInfo.AI_DACRATE_REG + 1);
            break;
        case SYSTEM_PAL:
            f = 49656530 / (*AudioInfo.AI_DACRATE_REG + 1);
            break;
        case SYSTEM_MPAL:
            f = 48628316 / (*AudioInfo.AI_DACRATE_REG + 1);
            break;
    }
    InitializeAudio(f);
}


EXPORT void CALL AiLenChanged( void )
{
    unsigned int LenReg;
    unsigned char *p;
    unsigned int CurrLevel, CurrTime, ExpectedLevel, ExpectedTime;

    if (critical_failure == 1)
        return;
    if (!l_PluginInit)
        return;

    LenReg = *AudioInfo.AI_LEN_REG;
    p = (unsigned char*)(AudioInfo.RDRAM + (*AudioInfo.AI_DRAM_ADDR_REG & 0xFFFFFF));

    if (buffer_pos + LenReg < primaryBufferBytes)
    {
        unsigned int i;

        SDL_LockAudio();
        for ( i = 0 ; i < LenReg ; i += 4 )
        {

            if(SwapChannels == 0)
            {
                // Left channel
                primaryBuffer[ buffer_pos + i ] = p[ i + 2 ];
                primaryBuffer[ buffer_pos + i + 1 ] = p[ i + 3 ];

                // Right channel
                primaryBuffer[ buffer_pos + i + 2 ] = p[ i ];
                primaryBuffer[ buffer_pos + i + 3 ] = p[ i + 1 ];
            } else {
                // Left channel
                primaryBuffer[ buffer_pos + i ] = p[ i ];
                primaryBuffer[ buffer_pos + i + 1 ] = p[ i + 1 ];

                // Right channel
                primaryBuffer[ buffer_pos + i + 2 ] = p[ i + 2];
                primaryBuffer[ buffer_pos + i + 3 ] = p[ i + 3 ];
            }
        }
        buffer_pos += i;
        SDL_UnlockAudio();
    }
    else
    {
        DebugMessage(M64MSG_WARNING, "AiLenChanged(): Audio buffer overflow.");
    }

    /* Now we need to handle synchronization, by inserting time delay to keep the emulator running at the correct speed */
    /* Start by calculating the current Primary buffer fullness in terms of output samples */
    CurrLevel = (unsigned int) (((long long) (buffer_pos/N64_SAMPLE_BYTES) * OutputFreq * 100) / (GameFreq * speed_factor));
    /* Next, extrapolate to the buffer level at the expected time of the next audio callback, assuming that the
       buffer is filled at the same rate as the output frequency */
    CurrTime = SDL_GetTicks();
    ExpectedTime = last_callback_ticks + ((1000 * SecondaryBufferSize) / OutputFreq);
    ExpectedLevel = CurrLevel;
    if (CurrTime < ExpectedTime)
        ExpectedLevel += (ExpectedTime - CurrTime) * OutputFreq / 1000;
    /* If the expected value of the Primary Buffer Fullness at the time of the next audio callback is more than 10
       milliseconds ahead of our target buffer fullness level, then insert a delay now */
    DebugMessage(M64MSG_VERBOSE, "%03i New audio bytes: %i  Time to next callback: %i  Current/Expected buffer level: %i/%i",
                 CurrTime % 1000, LenReg, (int) (ExpectedTime - CurrTime), CurrLevel, ExpectedLevel);
    if (ExpectedLevel >= PrimaryBufferTarget + OutputFreq / 100)
    {
        unsigned int WaitTime = (ExpectedLevel - PrimaryBufferTarget) * 1000 / OutputFreq;
        DebugMessage(M64MSG_VERBOSE, "    AiLenChanged(): Waiting %ims", WaitTime);
        if (l_PausedForSync)
            SDL_PauseAudio(0);
        l_PausedForSync = 0;
        SDL_Delay(WaitTime);
    }
    /* Or if the expected level of the primary buffer is less than the secondary buffer size
       (ie, predicting an underflow), then pause the audio to let the emulator catch up to speed */
    else if (ExpectedLevel < SecondaryBufferSize)
    {
        DebugMessage(M64MSG_VERBOSE, "    AiLenChanged(): Possible underflow at next audio callback; pausing playback");
        if (!l_PausedForSync)
            SDL_PauseAudio(1);
        l_PausedForSync = 1;
    }
    /* otherwise the predicted buffer level is within our tolerance, so everything is okay */
    else
    {
        if (l_PausedForSync)
            SDL_PauseAudio(0);
        l_PausedForSync = 0;
    }
}

EXPORT int CALL InitiateAudio( AUDIO_INFO Audio_Info )
{
    if (!l_PluginInit)
        return 0;

    AudioInfo = Audio_Info;
    return 1;
}

static int underrun_count = 0;

#ifdef USE_SRC
static float *_src = NULL;
static unsigned int _src_len = 0;
static float *_dest = NULL;
static unsigned int _dest_len = 0;
static int error;
static SRC_STATE *src_state;
static SRC_DATA src_data;
#endif

static int resample(unsigned char *input, int input_avail, int oldsamplerate, unsigned char *output, int output_needed, int newsamplerate)
{
    int *psrc = (int*)input;
    int *pdest = (int*)output;
    int i = 0, j = 0;
#ifdef USE_SRC
    if(Resample == 2)
    {
        // the high quality resampler needs more input than the samplerate ratio would indicate to work properly
        if (input_avail > output_needed * 3 / 2)
            input_avail = output_needed * 3 / 2; // just to avoid too much short-float-short conversion time
        if (_src_len < input_avail*2 && input_avail > 0)
        {
            if(_src) free(_src);
            _src_len = input_avail*2;
            _src = malloc(_src_len);
        }
        if (_dest_len < output_needed*2 && output_needed > 0)
        {
            if(_dest) free(_dest);
            _dest_len = output_needed*2;
            _dest = malloc(_dest_len);
        }
        memset(_src,0,_src_len);
        memset(_dest,0,_dest_len);
        if(src_state == NULL)
        {
            src_state = src_new (SRC_SINC_BEST_QUALITY, 2, &error);
            if(src_state == NULL)
            {
                memset(output, 0, output_needed);
                return 0;
            }
        }
        src_short_to_float_array ((short *) input, _src, input_avail/2);
        src_data.end_of_input = 0;
        src_data.data_in = _src;
        src_data.input_frames = input_avail/4;
        src_data.src_ratio = (float) newsamplerate / oldsamplerate;
        src_data.data_out = _dest;
        src_data.output_frames = output_needed/4;
        if ((error = src_process (src_state, &src_data)))
        {
            memset(output, 0, output_needed);
            return input_avail;  // number of bytes consumed
        }
        src_float_to_short_array (_dest, (short *) output, output_needed/2);
        return src_data.input_frames_used * 4;
    }
#endif
    // RESAMPLE == 1
    if (newsamplerate >= oldsamplerate)
    {
        int sldf = oldsamplerate;
        int const2 = 2*sldf;
        int dldf = newsamplerate;
        int const1 = const2 - 2*dldf;
        int criteria = const2 - dldf;
        for (i = 0; i < output_needed/4; i++)
        {
            pdest[i] = psrc[j];
            if(criteria >= 0)
            {
                ++j;
                criteria += const1;
            }
            else criteria += const2;
        }
        return j * 4; //number of bytes consumed
    }
    // newsamplerate < oldsamplerate, this only happens when speed_factor > 1
    for (i = 0; i < output_needed/4; i++)
    {
        j = i * oldsamplerate / newsamplerate;
        pdest[i] = psrc[j];
    }
    return j * 4; //number of bytes consumed
}

static void my_audio_callback(void *userdata, unsigned char *stream, int len)
{
    int oldsamplerate, newsamplerate;

    if (!l_PluginInit)
        return;

    /* mark the time, for synchronization on the input side */
    last_callback_ticks = SDL_GetTicks();

    newsamplerate = OutputFreq * 100 / speed_factor;
    oldsamplerate = GameFreq;

    if (buffer_pos > (unsigned int) (len * oldsamplerate) / newsamplerate)
    {
        int input_used;
#if defined(HAS_OSS_SUPPORT)
        if (VolumeControlType == VOLUME_TYPE_OSS)
        {
            input_used = resample(primaryBuffer, buffer_pos, oldsamplerate, stream, len, newsamplerate);
        }
        else
#endif
        {
            input_used = resample(primaryBuffer, buffer_pos, oldsamplerate, mixBuffer, len, newsamplerate);
            SDL_MixAudio(stream, mixBuffer, len, VolSDL);
        }
        memmove(primaryBuffer, &primaryBuffer[input_used], buffer_pos - input_used);
        buffer_pos -= input_used;
        DebugMessage(M64MSG_VERBOSE, "%03i my_audio_callback: used %i samples",
                     last_callback_ticks % 1000, len / SDL_SAMPLE_BYTES);
    }
    else
    {
        unsigned int SamplesNeeded = (len * oldsamplerate) / (newsamplerate * SDL_SAMPLE_BYTES);
        unsigned int SamplesPresent = buffer_pos / N64_SAMPLE_BYTES;
        underrun_count++;
        DebugMessage(M64MSG_VERBOSE, "%03i Buffer underflow (%i).  %i samples present, %i needed",
                     last_callback_ticks % 1000, underrun_count, SamplesPresent, SamplesNeeded);
        memset(stream , 0, len);
    }
}
EXPORT int CALL RomOpen(void)
{
    if (!l_PluginInit)
        return 0;

    ReadConfig();
    InitializeAudio(GameFreq);
    return 1;
}

static void InitializeSDL(void)
{
    DebugMessage(M64MSG_INFO, "Initializing SDL audio subsystem...");

    if(SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0)
    {
        DebugMessage(M64MSG_ERROR, "Failed to initialize SDL audio subsystem; forcing exit.\n");
        critical_failure = 1;
        return;
    }
    critical_failure = 0;

}

static void CreatePrimaryBuffer(void)
{
    unsigned int newPrimaryBytes = (unsigned int) ((long long) PrimaryBufferSize * GameFreq * speed_factor /
                                                   (OutputFreq * 100)) * N64_SAMPLE_BYTES;
    if (primaryBuffer == NULL)
    {
        DebugMessage(M64MSG_VERBOSE, "Allocating memory for audio buffer: %i bytes.", newPrimaryBytes);
        primaryBuffer = (unsigned char*) malloc(newPrimaryBytes);
        memset(primaryBuffer, 0, newPrimaryBytes);
        primaryBufferBytes = newPrimaryBytes;
    }
    else if (newPrimaryBytes > primaryBufferBytes) /* primary buffer only grows; there's no point in shrinking it */
    {
        unsigned char *newPrimaryBuffer = (unsigned char*) malloc(newPrimaryBytes);
        unsigned char *oldPrimaryBuffer = primaryBuffer;
        SDL_LockAudio();
        memcpy(newPrimaryBuffer, oldPrimaryBuffer, primaryBufferBytes);
        memset(newPrimaryBuffer + primaryBufferBytes, 0, newPrimaryBytes - primaryBufferBytes);
        primaryBuffer = newPrimaryBuffer;
        primaryBufferBytes = newPrimaryBytes;
        SDL_UnlockAudio();
        free(oldPrimaryBuffer);
    }
}

static void InitializeAudio(int freq)
{
    SDL_AudioSpec *desired, *obtained;
    
    if(SDL_WasInit(SDL_INIT_AUDIO|SDL_INIT_TIMER) == (SDL_INIT_AUDIO|SDL_INIT_TIMER) ) 
    {
        DebugMessage(M64MSG_VERBOSE, "InitializeAudio(): SDL Audio sub-system already initialized.");
        SDL_PauseAudio(1);
        SDL_CloseAudio();
    }
    else 
    {
        DebugMessage(M64MSG_VERBOSE, "InitializeAudio(): Initializing SDL Audio");
        DebugMessage(M64MSG_VERBOSE, "Primary buffer: %i output samples.", PrimaryBufferSize);
        DebugMessage(M64MSG_VERBOSE, "Primary target fullness: %i output samples.", PrimaryBufferTarget);
        DebugMessage(M64MSG_VERBOSE, "Secondary buffer: %i output samples.", SecondaryBufferSize);
        InitializeSDL();
    }
    if (critical_failure == 1)
        return;
    GameFreq = freq; // This is important for the sync
    if(hardware_spec != NULL) free(hardware_spec);

    // Allocate space for SDL_AudioSpec
    desired = malloc(sizeof(SDL_AudioSpec));
    obtained = malloc(sizeof(SDL_AudioSpec));

    if(freq < 11025) OutputFreq = 11025;
    else if(freq < 22050) OutputFreq = 22050;
    else OutputFreq = 44100;
    
    desired->freq = OutputFreq;
    
    DebugMessage(M64MSG_VERBOSE, "Requesting frequency: %iHz.", desired->freq);
    /* 16-bit signed audio */
    desired->format=AUDIO_S16SYS;
    DebugMessage(M64MSG_VERBOSE, "Requesting format: %i.", desired->format);
    /* Stereo */
    desired->channels=2;
    /* reload these because they gets re-assigned from SDL data below, and InitializeAudio can be called more than once */
    PrimaryBufferSize = ConfigGetParamInt(l_ConfigAudio, "PRIMARY_BUFFER_SIZE");
    PrimaryBufferTarget = ConfigGetParamInt(l_ConfigAudio, "PRIMARY_BUFFER_TARGET");
    SecondaryBufferSize = ConfigGetParamInt(l_ConfigAudio, "SECONDARY_BUFFER_SIZE");
    desired->samples = SecondaryBufferSize;
    /* Our callback function */
    desired->callback = my_audio_callback;
    desired->userdata = NULL;

    /* Open the audio device */
    l_PausedForSync = 1;
    if (SDL_OpenAudio(desired, obtained) < 0)
    {
        DebugMessage(M64MSG_ERROR, "Couldn't open audio: %s", SDL_GetError());
        critical_failure = 1;
        return;
    }
    if (desired->format != obtained->format)
    {
        DebugMessage(M64MSG_WARNING, "Obtained audio format differs from requested.");
    }
    if (desired->freq != obtained->freq)
    {
        DebugMessage(M64MSG_WARNING, "Obtained frequency differs from requested.");
    }

    /* desired spec is no longer needed */
    free(desired);
    hardware_spec=obtained;

    /* allocate memory for audio buffers */
    OutputFreq = hardware_spec->freq;
    SecondaryBufferSize = hardware_spec->samples;
    if (PrimaryBufferTarget < SecondaryBufferSize)
        PrimaryBufferTarget = SecondaryBufferSize;
    if (PrimaryBufferSize < PrimaryBufferTarget)
        PrimaryBufferSize = PrimaryBufferTarget;
    if (PrimaryBufferSize < SecondaryBufferSize * 2)
        PrimaryBufferSize = SecondaryBufferSize * 2;
    CreatePrimaryBuffer();
    if (mixBuffer != NULL)
        free(mixBuffer);
    mixBuffer = (unsigned char*) malloc(SecondaryBufferSize * SDL_SAMPLE_BYTES);

    /* preset the last callback time */
    if (last_callback_ticks == 0)
        last_callback_ticks = SDL_GetTicks();

    DebugMessage(M64MSG_VERBOSE, "Frequency: %i", hardware_spec->freq);
    DebugMessage(M64MSG_VERBOSE, "Format: %i", hardware_spec->format);
    DebugMessage(M64MSG_VERBOSE, "Channels: %i", hardware_spec->channels);
    DebugMessage(M64MSG_VERBOSE, "Silence: %i", hardware_spec->silence);
    DebugMessage(M64MSG_VERBOSE, "Samples: %i", hardware_spec->samples);
    DebugMessage(M64MSG_VERBOSE, "Size: %i", hardware_spec->size);

    /* set playback volume */
#if defined(HAS_OSS_SUPPORT)
    if (VolumeControlType == VOLUME_TYPE_OSS)
    {
        VolPercent = volGet();
    }
    else
#endif
    {
        VolSDL = SDL_MIX_MAXVOLUME * VolPercent / 100;
    }

}
EXPORT void CALL RomClosed( void )
{
    if (!l_PluginInit)
        return;
   if (critical_failure == 1)
       return;
    DebugMessage(M64MSG_VERBOSE, "Cleaning up SDL sound plugin...");
    
    // Shut down SDL Audio output
    SDL_PauseAudio(1);
    SDL_CloseAudio();

    // Delete the buffer, as we are done producing sound
    if (primaryBuffer != NULL)
    {
        primaryBufferBytes = 0;
        free(primaryBuffer);
        primaryBuffer = NULL;
    }
    if (mixBuffer != NULL)
    {
        free(mixBuffer);
        mixBuffer = NULL;
    }

    // Delete the hardware spec struct
    if(hardware_spec != NULL) free(hardware_spec);
    hardware_spec = NULL;

    // Shutdown the respective subsystems
    if(SDL_WasInit(SDL_INIT_AUDIO) != 0) SDL_QuitSubSystem(SDL_INIT_AUDIO);
    if(SDL_WasInit(SDL_INIT_TIMER) != 0) SDL_QuitSubSystem(SDL_INIT_TIMER);
}

EXPORT void CALL ProcessAList(void)
{
}

EXPORT void CALL SetSpeedFactor(int percentage)
{
    if (!l_PluginInit)
        return;
    if (percentage >= 10 && percentage <= 300)
        speed_factor = percentage;
    // we need a different size primary buffer to store the N64 samples when the speed changes
    CreatePrimaryBuffer();
}

static void ReadConfig(void)
{
    /* read the configuration values into our static variables */
    GameFreq = ConfigGetParamInt(l_ConfigAudio, "DEFAULT_FREQUENCY");
    SwapChannels = ConfigGetParamBool(l_ConfigAudio, "SWAP_CHANNELS");
    PrimaryBufferSize = ConfigGetParamInt(l_ConfigAudio, "PRIMARY_BUFFER_SIZE");
    PrimaryBufferTarget = ConfigGetParamInt(l_ConfigAudio, "PRIMARY_BUFFER_TARGET");
    SecondaryBufferSize = ConfigGetParamInt(l_ConfigAudio, "SECONDARY_BUFFER_SIZE");
    Resample = ConfigGetParamInt(l_ConfigAudio, "RESAMPLE");
    VolumeControlType = ConfigGetParamInt(l_ConfigAudio, "VOLUME_CONTROL_TYPE");
    VolDelta = ConfigGetParamInt(l_ConfigAudio, "VOLUME_ADJUST");
    VolPercent = ConfigGetParamInt(l_ConfigAudio, "VOLUME_DEFAULT");
}

EXPORT void CALL VolumeMute(void)
{
    if (!l_PluginInit)
        return;

    if (VolMutedSave > -1)
    {
        //unmute
        VolPercent = VolMutedSave;
        VolMutedSave = -1;
#if defined(HAS_OSS_SUPPORT)
        if (VolumeControlType == VOLUME_TYPE_OSS)
        {
            //OSS mixer volume
            volSet(VolPercent);
        }
        else
#endif
        {
            VolSDL = SDL_MIX_MAXVOLUME * VolPercent / 100;
        }
    } 
    else
    {
        //mute
        VolMutedSave = VolPercent;
        VolPercent = 0;
#if defined(HAS_OSS_SUPPORT)
        if (VolumeControlType == VOLUME_TYPE_OSS)
        {
            //OSS mixer volume
            volSet(0);
        }
        else
#endif
        {
            VolSDL = 0;
        }
    }
}

EXPORT void CALL VolumeUp(void)
{
    if (!l_PluginInit)
        return;

    //if muted, unmute first
    if (VolMutedSave > -1)
        VolumeMute();

#if defined(HAS_OSS_SUPPORT)
    // reload volume if we're using OSS
    if (VolumeControlType == VOLUME_TYPE_OSS)
    {
        VolPercent = volGet();
    }
#endif

    // adjust volume variable
    VolPercent += VolDelta;
    if (VolPercent > 100)
        VolPercent = 100;

#if defined(HAS_OSS_SUPPORT)
    if (VolumeControlType == VOLUME_TYPE_OSS)
    {
        //OSS mixer volume
        volSet(VolPercent);
    }
    else
#endif
    {
        VolSDL = SDL_MIX_MAXVOLUME * VolPercent / 100;
    }
}

EXPORT void CALL VolumeDown(void)
{
    if (!l_PluginInit)
        return;

    //if muted, unmute first
    if (VolMutedSave > -1)
        VolumeMute();

#if defined(HAS_OSS_SUPPORT)
    // reload volume if we're using OSS
    if (VolumeControlType == VOLUME_TYPE_OSS)
    {
        VolPercent = volGet();
    }
#endif

    // adjust volume variable
    VolPercent -= VolDelta;
    if (VolPercent < 0)
        VolPercent = 0;

#if defined(HAS_OSS_SUPPORT)
    if (VolumeControlType == VOLUME_TYPE_OSS)
    {
        //OSS mixer volume
        volSet(VolPercent);
    }
    else
#endif
    {
        VolSDL = SDL_MIX_MAXVOLUME * VolPercent / 100;
    }
}

EXPORT int CALL VolumeGetLevel(void)
{
    return VolPercent;
}

EXPORT void CALL VolumeSetLevel(int level)
{
    if (!l_PluginInit)
        return;

    // adjust volume 
    VolPercent = level;
    if (VolPercent < 0)
        VolPercent = 0;
    else if (VolPercent > 100)
        VolPercent = 100;

#if defined(HAS_OSS_SUPPORT)
    if (VolumeControlType == VOLUME_TYPE_OSS)
    {
        //OSS mixer volume
        volSet(VolPercent);
    }
    else
#endif
    {
        VolSDL = SDL_MIX_MAXVOLUME * VolPercent / 100;
    }
}

static char VolumeString[32];

EXPORT const char * CALL VolumeGetString(void)
{
    if (VolMutedSave > -1)
    {
        strcpy(VolumeString, "Mute");
    }
    else
    {
        sprintf(VolumeString, "%i%%", VolPercent);
    }

    return VolumeString;
}

