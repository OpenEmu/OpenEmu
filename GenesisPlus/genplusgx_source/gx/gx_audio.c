/****************************************************************************
 *  gx_audio.c
 *
 *  Genesis Plus GX audio support
 *
 *  Softdev (2006)
 *  Eke-Eke (2007,2008,2009)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ***************************************************************************/

#include "shared.h"

/* DMA soundbuffers (required to be 32-bytes aligned)
   Length is dimensionned for one frame of emulation (800/808 samples @60hz, 960 samples@50Hz)
   To prevent audio clashes, we use double buffering technique:
    one buffer is the active DMA buffer
    the other one is the current work buffer (updated during frame emulation)
   We do not need more since frame emulation and DMA operation are synchronized
*/
u8 soundbuffer[2][3840] ATTRIBUTE_ALIGN(32);

/* Current work soundbuffer */
u32 mixbuffer;

/* audio DMA status */
u32 audioStarted;

/* Background music */
static u8 *Bg_music_ogg = NULL;
static u32 Bg_music_ogg_size = 0;

/***************************************************************************************/
/*   Audio engine                                                                      */
/***************************************************************************************/

/* Audio DMA callback */
static void ai_callback(void)
{
  frameticker++;
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
void gx_audio_Update(void)
{
  /* retrieve audio samples */
  int size = audio_update() * 4;

  /* set next DMA soundbuffer */
  s16 *sb = (s16 *)(soundbuffer[mixbuffer]);
  DCFlushRange((void *)sb, size);
  AUDIO_InitDMA((u32) sb, size);
  mixbuffer ^= 1;

  /* Start Audio DMA */
  /* this is called once to kick-off DMA from external memory to audio interface        */
  /* DMA operation is automatically restarted when all samples have been sent.          */
  /* If DMA settings are not updated at that time, previous sound buffer will be used.  */
  /* Therefore we need to make sure frame emulation is completed before current DMA is  */
  /* completed, either by synchronizing frame emulation with DMA start or by syncing it */
  /* with Vertical Interrupt and outputing a suitable number of samples per frame.      */
  /*                                                                                    */
  /* In both cases, audio DMA need to be synchronized with VSYNC and therefore need to  */
  /* be resynchronized (restarted) every time video settings are changed (hopefully,    */
  /* this generally happens while no music is played.                                   */                    
  if (!audioStarted)
  {
    /* restart audio DMA */
    AUDIO_StopDMA();
    AUDIO_StartDMA();
    audioStarted = 1;

    /* resynchronize emulation */
    frameticker = 1;
  }
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
  AUDIO_StopDMA();
  AUDIO_RegisterDMACallback(NULL);
  DSP_Halt();

  /* when not using 60hz mode, frame emulation is synchronized with Audio Interface DMA */
  if (gc_pal | vdp_pal)
  {
    AUDIO_RegisterDMACallback(ai_callback);
  }

  /* reset emulation audio processing */
  memset(soundbuffer, 0, 2 * 3840);
  audioStarted = 0;
  mixbuffer = 0;
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
