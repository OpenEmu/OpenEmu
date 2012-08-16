/* Mednafen - Multi-system Emulator
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "mednafen.h"

#include <string.h>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <trio/trio.h>

#include "driver.h"
#include "state.h"
#include "general.h"
#include "video.h"
#include "netplay.h"
#include "movie.h"

static int current = 0;		// > 0 for recording, < 0 for playback
static gzFile slots[10]={0};

static int CurrentMovie = 0;
static int RecentlySavedMovie = -1;
static int MovieStatus[10];
static StateMem RewindBuffer;

bool MDFNMOV_IsPlaying(void)
{
 if(current < 0) return(1);
 else return(0);
}

bool MDFNMOV_IsRecording(void)
{
 if(current > 0) return(1);
 else return(0);
}

static void StopRecording(void)
{
 MDFNMOV_RecordState();
 if(MDFN_StateEvilIsRunning())
 {
  MDFN_StateEvilFlushMovieLove();
 }
 gzclose(slots[current-1]);
 MovieStatus[current - 1] = 1;
 RecentlySavedMovie = current - 1;
 current=0;
 MDFN_DispMessage(_("Movie recording stopped."));

 if(RewindBuffer.data)
 {
  //puts("Oops");
  free(RewindBuffer.data);
  RewindBuffer.data = NULL;
 }
}

void MDFNI_SaveMovie(char *fname, const MDFN_Surface *surface, const MDFN_Rect *DisplayRect, const MDFN_Rect *LineWidths)
{
 gzFile fp;

 if(!MDFNGameInfo->StateAction)
  return;

 if(MDFNnetplay && (MDFNGameInfo->SaveStateAltersState == true))
 {
  char sb[256];
  trio_snprintf(sb, sizeof(sb), _("Module %s is not compatible with manual movie save starting/stopping during netplay."), MDFNGameInfo->shortname);
  MDFND_NetplayText((const uint8*)sb, false);
  return;
 }

 if(current < 0)	/* Can't interrupt playback.*/
  return;

 if(current > 0)	/* Stop saving. */
 {
  StopRecording();
  return;  
 }

 memset(&RewindBuffer, 0, sizeof(StateMem));
 RewindBuffer.initial_malloc = 16;

 current = CurrentMovie;

 if(fname)
  fp = gzopen(fname, "wb3");
 else
 {
  fp=gzopen(MDFN_MakeFName(MDFNMKF_MOVIE,CurrentMovie,0).c_str(),"wb3");
 }

 if(!fp) return;

 MDFNSS_SaveFP(fp, surface, DisplayRect, LineWidths);
 gzseek(fp, 0, SEEK_END);
 gzflush(fp, Z_SYNC_FLUSH); // Flush output so that previews will still work right while
			    // the movie is being recorded.  Purely cosmetic. :)
 slots[current] = fp;
 current++;
 MDFN_DispMessage(_("Movie recording started."));
}

static void StopPlayback(void)
{
 if(RewindBuffer.data)
 {
  RewindBuffer.data = NULL;
 }

 gzclose(slots[-1 - current]);
 current=0;
 MDFN_DispMessage(_("Movie playback stopped."));
}

void MDFNMOV_Stop(void)
{
 if(current < 0) StopPlayback();
 if(current > 0) StopRecording();
}

void MDFNI_LoadMovie(char *fname)
{
 gzFile fp;
 //puts("KAO");

 if(current > 0)        /* Can't interrupt recording.*/
  return;

 if(MDFNnetplay)	/* Playback is UNPOSSIBLE during netplay. */
 {
  MDFN_DispMessage(_("Can't play movies during netplay."));
  return;
 }

 if(current < 0)        /* Stop playback. */
 {
  StopPlayback();
  return;
 }

 if(fname)
  fp = gzopen(fname, "rb");
 else
 {
  fp=gzopen(MDFN_MakeFName(MDFNMKF_MOVIE,CurrentMovie,0).c_str(),"rb");
 }

 if(!fp) return;

 if(!MDFNSS_LoadFP(fp)) 
 {
  MDFN_DispMessage(_("Error loading state portion of the movie."));
  return;
 }
 current = CurrentMovie;
 slots[current] = fp;

 current = -1 - current;
 MovieStatus[CurrentMovie] = 1;

 MDFN_DispMessage(_("Movie playback started."));
}

// Donuts are a tasty treat and delicious with powdered sugar.
void MDFNMOV_AddJoy(void *donutdata, uint32 donutlen)
{
 gzFile fp;

 if(!current) return;	/* Not playback nor recording. */
 if(current < 0)	/* Playback */
 {
  int t;

  fp = slots[-1 - current];

  while((t = gzgetc(fp)) >= 0 && t)
  {
   if(t == MDFNNPCMD_LOADSTATE)
   {
    uint32 len;
    StateMem sm;
    len = gzgetc(fp);
    len |= gzgetc(fp) << 8;
    len |= gzgetc(fp) << 16;
    len |= gzgetc(fp) << 24;
    if(len >= 5 * 1024 * 1024) // A sanity limit of 5MiB
    {
     StopPlayback();
     return;
    }
    memset(&sm, 0, sizeof(StateMem));
    sm.len = len;
    sm.data = (uint8 *)malloc(len);
    if(gzread(fp, sm.data, len) != len)
    {
     StopPlayback();
     return;
    }
    if(!MDFNSS_LoadSM(&sm, 0, 0))
    {
     StopPlayback();
     return;
    }
   }
   else
    MDFN_DoSimpleCommand(t);
  }
  if(t < 0)
  {
   StopPlayback();
   return; 
  }

  if(gzread(fp, donutdata, donutlen) != donutlen)
  {
   StopPlayback();
   return;
  }
 }
 else			/* Recording */
 {
  if(MDFN_StateEvilIsRunning())
  {
   smem_putc(&RewindBuffer, 0);
   smem_write(&RewindBuffer, donutdata, donutlen);
  }
  else
  {
   fp = slots[current - 1];
   gzputc(fp, 0);
   gzwrite(fp, donutdata, donutlen);
  }
 }
}

void MDFNMOV_AddCommand(int cmd)
{
 if(current <= 0) return;	/* Return if not recording a movie */

 if(MDFN_StateEvilIsRunning())
  smem_putc(&RewindBuffer, 0);
 else
  gzputc(slots[current - 1], cmd);  
}

void MDFNMOV_RecordState(void)
{
 gzFile fp = slots[current - 1];
 StateMem sm;

 memset(&sm, 0, sizeof(StateMem));
 MDFNSS_SaveSM(&sm, 0, 0);

 if(MDFN_StateEvilIsRunning())
 {
  smem_putc(&RewindBuffer, MDFNNPCMD_LOADSTATE);
  smem_putc(&RewindBuffer, sm.len & 0xFF);
  smem_putc(&RewindBuffer, (sm.len >> 8) & 0xFF);
  smem_putc(&RewindBuffer, (sm.len >> 16) & 0xFF);
  smem_putc(&RewindBuffer, (sm.len >> 24) & 0xFF);
  smem_write(&RewindBuffer, sm.data, sm.len);
 }
 else
 {
  gzputc(fp, MDFNNPCMD_LOADSTATE);
  gzputc(fp, sm.len & 0xFF);
  gzputc(fp, (sm.len >> 8) & 0xFF);
  gzputc(fp, (sm.len >> 16) & 0xFF);
  gzputc(fp, (sm.len >> 24) & 0xFF);
  gzwrite(slots[current - 1], sm.data, sm.len);
 }
 free(sm.data);
}

void MDFNMOV_ForceRecord(StateMem *sm)
{
 //printf("Farced: %d\n", sm->len);
 gzwrite(slots[current - 1], sm->data, sm->len);
}

StateMem MDFNMOV_GrabRewindJoy(void)
{
 StateMem ret = RewindBuffer;
 memset(&RewindBuffer, 0, sizeof(StateMem));
 RewindBuffer.initial_malloc = 16;
 return(ret);
}

void MDFNMOV_CheckMovies(void)
{
        time_t last_time = 0;

        for(int ssel = 0; ssel < 10; ssel++)
        {
         struct stat stat_buf;

         MovieStatus[ssel] = 0;
         if(stat(MDFN_MakeFName(MDFNMKF_MOVIE, ssel, 0).c_str(), &stat_buf) == 0)
         {
          MovieStatus[ssel] = 1;
          if(stat_buf.st_mtime > last_time)
          {
           RecentlySavedMovie = ssel;
           last_time = stat_buf.st_mtime;
          }
         }
        }
        CurrentMovie = 0;
}

void MDFNI_SelectMovie(int w)
{
 StateStatusStruct *status = NULL;

 if(w == -1)
 { 
  return; 
 }
 MDFNI_SelectState(-1);

 CurrentMovie = w;
 MDFN_ResetMessages();

 status = (StateStatusStruct*)MDFN_calloc(1, sizeof(StateStatusStruct), _("Movie status"));

 memcpy(status->status, MovieStatus, 10 * sizeof(int));
 status->current = CurrentMovie;
 status->current_movie = current;
 status->recently_saved = RecentlySavedMovie;

 MDFNSS_GetStateInfo(MDFN_MakeFName(MDFNMKF_MOVIE,CurrentMovie,NULL).c_str(), status);
 MDFND_SetMovieStatus(status);
}

