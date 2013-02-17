/****************************************************************************
 *  gx_audio.c
 *
 *  Genesis Plus GX audio support
 *
 *  Copyright Eke-Eke (2007-2012), based on original work from Softdev (2006)
 *
 *  Redistribution and use of this code or any derivative works are permitted
 *  provided that the following conditions are met:
 *
 *   - Redistributions may not be sold, nor may they be used in a commercial
 *     product or activity.
 *
 *   - Redistributions that are modified from the original source must include the
 *     complete source code, including the source code for all components used by a
 *     binary built from the modified sources. However, as a special exception, the
 *     source code distributed need not include anything that is normally distributed
 *     (in either source or binary form) with the major components (compiler, kernel,
 *     and so on) of the operating system on which the executable runs, unless that
 *     component itself accompanies the executable.
 *
 *   - Redistributions must reproduce the above copyright notice, this list of
 *     conditions and the following disclaimer in the documentation and/or other
 *     materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************************/

#include "shared.h"

/* Length is dimensionned for at least one frame of emulation */
#define SOUND_BUFFER_LEN 4096

/* Number of sound buffers */
#define SOUND_BUFFER_NUM 3

/* audio DMA status */
u32 audioStarted;

/* DMA soundbuffers (required to be 32-bytes aligned) */
static u8 soundbuffer[SOUND_BUFFER_NUM][SOUND_BUFFER_LEN] ATTRIBUTE_ALIGN(32);

/* Current work soundbuffer */
static u8 mixbuffer;

/* Background music */
static u8 *Bg_music_ogg = NULL;
static u32 Bg_music_ogg_size = 0;

/* Frame Sync */
static u8 audio_sync;

/***************************************************************************************/
/*   Audio engine                                                                      */
/***************************************************************************************/

/* Audio DMA callback */
static void ai_callback(void)
{
#ifdef LOG_TIMING
  u64 current = gettime();
  if (prevtime)
  {
    delta_time[frame_cnt] = diff_nsec(prevtime, current);
    frame_cnt = (frame_cnt + 1) % LOGSIZE;
  }
  prevtime = current;
#endif

  audio_sync = 1;
}

/* AUDIO engine initialization */
void gx_audio_Init(void)
{
  /* Initialize AUDIO processing library (ASNDLIB) */
  /* AUDIO & DSP hardware are initialized */
  /* Default samplerate is set to 48kHz */
  ASND_Init();

  /* Load background music from FAT device */
  char fname[MAXPATHLEN];
  sprintf(fname,"%s/Bg_music.ogg",DEFAULT_PATH);
  FILE *f = fopen(fname,"rb");
  if (f)
  {
    struct stat filestat;
    stat(fname, &filestat);
    Bg_music_ogg_size = filestat.st_size;
    Bg_music_ogg = memalign(32,Bg_music_ogg_size);
    if (Bg_music_ogg)
    {
      fread(Bg_music_ogg,1,Bg_music_ogg_size,f);
    }
    fclose(f);
  }
}

/* AUDIO engine shutdown */
void gx_audio_Shutdown(void)
{
  PauseOgg(1);
  StopOgg();
  ASND_Pause(1);
  ASND_End();
  if (Bg_music_ogg)
  {
    free(Bg_music_ogg);
  }
}

/*** 
      gx_audio_Update

     This function retrieves samples for the frame then set the next DMA parameters 
     Parameters will be taken in account only when current DMA operation is over
 ***/
int gx_audio_Update(void)
{
  if (audio_sync)
  {
    /* Current available soundbuffer */
    s16 *sb = (s16 *)(soundbuffer[mixbuffer]);

    /* Retrieve audio samples (size must be multiple of 32 bytes) */
    int size = audio_update(sb) * 4;

  #ifdef LOG_TIMING
    if (prevtime && (frame_cnt < LOGSIZE - 1))
    {
      delta_samp[frame_cnt + 1] = size;
    }
    else
    {
      delta_samp[0] = size;
    }
  #endif

    /* Update DMA settings */
    DCFlushRange((void *)sb, size);
    AUDIO_InitDMA((u32) sb, size);
    mixbuffer = (mixbuffer + 1) % SOUND_BUFFER_NUM;
    audio_sync = 0;

    /* Start Audio DMA */
    /* this is called once to kick-off DMA from external memory to audio interface        */
    /* DMA operation is automatically restarted when all samples have been sent.          */
    /* If DMA settings are not updated at that time, previous sound buffer will be used.  */
    /* Therefore we need to make sure frame emulation is completed before current DMA is  */
    /* completed, by synchronizing frame emulation with DMA start and also by syncing it  */
    /* with Video Interrupt and outputing a suitable number of samples per frame.         */
    if (!audioStarted)
    {
      /* restart audio DMA */
      AUDIO_StopDMA();
      AUDIO_StartDMA();
      audioStarted = 1;
    }

    return SYNC_AUDIO;
  }

  return NO_SYNC;
}

/*** 
      gx_audio_Start

     This function restart the audio engine
     This is called when coming back from Main Menu
 ***/
void gx_audio_Start(void)
{
  /* shutdown background music */
  PauseOgg(1);	
  StopOgg();	
  
  /* shutdown menu audio processing */
  ASND_Pause(1);
  ASND_End();
  AUDIO_StopDMA();
  AUDIO_RegisterDMACallback(NULL);
  DSP_Halt();

  /* DMA Interrupt callback */
  AUDIO_RegisterDMACallback(ai_callback);

  /* reset emulation audio processing */
  memset(soundbuffer, 0, 3 * SOUND_BUFFER_LEN);
  audioStarted = 0;
  mixbuffer = 0;
  audio_sync = 1;
}

/***
      gx_audio_Stop

     This function stops current Audio DMA process
     This is called when going back to Main Menu
     DMA need to be restarted when going back to the game (see above)
 ***/
void gx_audio_Stop(void)
{
  /* restart menu audio processing */
  DSP_Unhalt();
  ASND_Init();
  ASND_Pause(0);
	
  /* play background music */
  if (Bg_music_ogg && !Shutdown)
  {
    PauseOgg(0);	
    PlayOgg((char *)Bg_music_ogg, Bg_music_ogg_size, 0, OGG_INFINITE_TIME);	
    SetVolumeOgg(((int)config.bgm_volume * 255) / 100);	
  }
}
