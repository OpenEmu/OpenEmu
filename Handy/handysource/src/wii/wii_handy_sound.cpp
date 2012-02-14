/****************************************************************************
* FCE Ultra 0.98.12
* Nintendo Wii/Gamecube Port
*
* Tantric September 2008
* eke-eke October 2008
*
* gcaudio.c
*
* Audio driver
****************************************************************************/

#include <gccore.h>
#include <string.h>
#include <sdl.h>

#include "system.h"

#define SAMPLERATE 48000  

#define MIXBUFSIZE_BYTES 64000 //16000
#define MIXBUFSIZE_SHORT (MIXBUFSIZE_BYTES / 2)
#define MIXBUFSIZE_WORDS (MIXBUFSIZE_BYTES / 4)

#define SOUNDBUFSIZE 4096 //2048

static u8 soundbuffer[2][SOUNDBUFSIZE] ATTRIBUTE_ALIGN(32);
static u8 mixbuffer[MIXBUFSIZE_BYTES];
static int mixhead = 0;
static int mixtail = 0;
static int whichab = 0;
static int IsPlaying = 0;

/****************************************************************************
* MixerCollect
*
* Collects sound samples from mixbuffer and puts them into outbuffer
* Makes sure to align them to 32 bytes for AUDIO_InitDMA
***************************************************************************/
static int MixerCollect( u8 *outbuffer, int len )
{
  u32 *dst = (u32 *)outbuffer;
  u32 *src = (u32 *)mixbuffer;
  int done = 0;

  // Always clear output buffer
  memset(outbuffer, 0, len);

  while ( ( mixtail != mixhead ) && ( done < len ) )
  {
    *dst++ = src[mixtail++];
    if (mixtail == MIXBUFSIZE_WORDS) mixtail = 0;
    done += 4;
  }

  // Realign to 32 bytes for DMA
  mixtail -= ((done&0x1f) >> 2);
  if (mixtail < 0)
    mixtail += MIXBUFSIZE_WORDS;
  done &= ~0x1f;

  return done;
}

/****************************************************************************
* AudioSwitchBuffers
*
* Manages which buffer is played next
***************************************************************************/
static void AudioSwitchBuffers()
{
  int len = MixerCollect( soundbuffer[whichab], SOUNDBUFSIZE );
  if (len == 0) 
    return;

  DCFlushRange(soundbuffer[whichab], len);
  AUDIO_InitDMA((u32)soundbuffer[whichab], len);
  AUDIO_StartDMA();
  whichab ^= 1;  
  IsPlaying = 1;   
}

/****************************************************************************
* InitialiseAudio
*
* Initializes sound system on first load of emulator
***************************************************************************/
void InitialiseAudio()
{
  AUDIO_Init(NULL); // Start audio subsystem
  AUDIO_SetDSPSampleRate(AI_SAMPLERATE_48KHZ);
  AUDIO_RegisterDMACallback( AudioSwitchBuffers );    
  memset(soundbuffer, 0, SOUNDBUFSIZE*2);
  memset(mixbuffer, 0, MIXBUFSIZE_BYTES);
}

/****************************************************************************
* StopAudio
*
* Pause audio output when returning to menu
***************************************************************************/
void StopAudio()
{
  AUDIO_StopDMA();
  IsPlaying = 0;
}

/****************************************************************************
* ResetAudio
*
* Reset audio output when loading a new game
***************************************************************************/
void ResetAudio()
{
  memset(soundbuffer, 0, SOUNDBUFSIZE*2);
  memset(mixbuffer, 0, MIXBUFSIZE_BYTES);
  mixhead = mixtail = 0;
}

int mixoverlap_count = 0;

/****************************************************************************
* PlaySound
*
* Puts incoming mono samples into mixbuffer
* Splits mono samples into two channels (stereo)
****************************************************************************/
static void PlaySound32( u32 *Buffer, int count )
{
  u32 *dst = (u32 *)mixbuffer;
  int i;

  for( i = 0; i < count; i++ )
  {
    dst[mixhead++] = Buffer[i];

    if(mixhead == mixtail)
      mixoverlap_count++;

    if (mixhead == MIXBUFSIZE_WORDS)
      mixhead = 0;
  }

  // Restart Sound Processing if stopped
  if (IsPlaying == 0)
  {
    AudioSwitchBuffers ();
  }
}

// Buffer for conversion
u8 playbuffer[HANDY_AUDIO_BUFFER_SIZE<<2];

/*
 * Plays the specified 8-bit audio sample
 *
 * sample The audio sample
 * length The length of the audio sample
 */
void PlaySound( u8* sample, int length )
{
  memcpy( playbuffer, sample, length );

  // Convert from 8-bit mono to stero 16-bit signed (MSB)
  SDL_AudioCVT audio_convert;
  SDL_BuildAudioCVT(
    &audio_convert,
    AUDIO_U8,
    1,
    48000,
    AUDIO_S16MSB, 
    2,
    48000
    );
  audio_convert.buf = playbuffer;
  audio_convert.len = length;
  SDL_ConvertAudio( &audio_convert );

  // Play the converted buffer
  PlaySound32( (u32*)playbuffer, ( audio_convert.len_cvt>>2 ) );        
}
