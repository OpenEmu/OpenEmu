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

#include "../mednafen.h"
#include <string.h>
#include <sys/types.h>
//#include <cdio/cdio.h>
#include <trio/trio.h>
#include "cdromif.h"
#include "cdromfile.h"
#include "../general.h"
#include "dvdisaster.h"

#include <queue>

// Read from either thread, only written to during the startup of the CD reading thread.
static CD_TOC toc;

// Only access from CD reading thread!
static CDRFile *p_cdrfile = NULL;

static MDFN_Thread *CDReadThread = NULL;

static bool LEC_Eval;

enum
{
 CDIF_MSG_INIT_DONE = 0,	// Read -> emu
 CDIF_MSG_FATAL_ERROR,		// Read -> emu

 CDIF_MSG_DIEDIEDIE,		// Emu -> read

 CDIF_MSG_READ_SECTOR,		/* Emu -> read
					args[0] = lba
				*/
};

class CDIF_Message
{
 public:

 CDIF_Message()
 {
  message = 0;

  memset(args, 0, sizeof(args));
  parg = NULL;
 }

 CDIF_Message(unsigned int _message, uint32 arg0, uint32 arg1, uint32 arg2, uint32 arg3, void *_parg)
 {
  message = _message;
  args[0] = arg0;
  args[1] = arg1;
  args[2] = arg2;
  args[3] = arg3;
  parg = _parg;
 }

 ~CDIF_Message()
 {

 }

 unsigned int message;
 uint32 args[4];
 void *parg;
};

class CDIF_Queue
{
 public:

 CDIF_Queue()
 {
  ze_mutex = MDFND_CreateMutex();
 }

 ~CDIF_Queue()
 {
  MDFND_DestroyMutex(ze_mutex);
 }

 // Returns FALSE if message not read, TRUE if it was read.  Will always return TRUE if "blocking" is set.
 bool Read(CDIF_Message *message, bool blocking = TRUE)
 {
  TryAgain:

  MDFND_LockMutex(ze_mutex);
  
  if(ze_queue.size() > 0)
  {
   *message = ze_queue.front();
   ze_queue.pop();
   MDFND_UnlockMutex(ze_mutex);
   return(TRUE);
  }
  else if(blocking)
  {
   MDFND_UnlockMutex(ze_mutex);
   MDFND_Sleep(1);
   goto TryAgain;
  }
  else
  {
   MDFND_UnlockMutex(ze_mutex);
   return(FALSE);
  }
 }

 void Write(const CDIF_Message &message)
 {
  MDFND_LockMutex(ze_mutex);

  ze_queue.push(message);

  MDFND_UnlockMutex(ze_mutex);
 }

 std::queue<CDIF_Message> ze_queue;
 MDFN_Mutex *ze_mutex;
};

// Queue for messages to the read thread.
static CDIF_Queue *ReadThreadQueue = NULL;

// Queue for messages to the emu thread.
static CDIF_Queue *EmuThreadQueue = NULL;


typedef struct
{
 bool valid;
 uint32 lba;
 uint8 data[2352 + 96];
} CDIF_Sector_Buffer;

static CDIF_Sector_Buffer *SectorBuffers = NULL;
static uint32 SBWritePos;
static const uint32 SBSize = 256;
static MDFN_Mutex *SBMutex = NULL;

static uint32 ra_lba;
static int ra_count;
static int32 last_read_lba;

static int ReadThreadStart(void *arg)
{
 char *device_name = (char *)arg;
 bool Running = TRUE;

 MDFN_printf(_("Loading %s...\n\n"), device_name ? device_name : _("PHYSICAL CDROM DISC"));
 MDFN_indent(1);

 if(!(p_cdrfile = cdrfile_open(device_name)))
 {
  MDFN_indent(-1);
  EmuThreadQueue->Write(CDIF_Message(CDIF_MSG_INIT_DONE, FALSE, 0, 0, 0, NULL));
  return(0);
 }

 if(!cdrfile_read_toc(p_cdrfile, &toc))
 {
  puts("Error reading TOC");
  MDFN_indent(-1);
  EmuThreadQueue->Write(CDIF_Message(CDIF_MSG_INIT_DONE, FALSE, 0, 0, 0, NULL));
  return(0);
 }

 if(toc.first_track < 1 || toc.last_track > 99 || toc.first_track > toc.last_track)
 {
  puts("First/Last track numbers bad");
  MDFN_indent(-1);
  EmuThreadQueue->Write(CDIF_Message(CDIF_MSG_INIT_DONE, FALSE, 0, 0, 0, NULL));
  return(0);
 }

 for(int32 track = toc.first_track; track <= toc.last_track; track++)
 {
  MDFN_printf(_("Track %2d, LBA: %6d  %s\n"), track, toc.tracks[track].lba, (toc.tracks[track].control & 0x4) ? "DATA" : "AUDIO");
 }

 MDFN_printf("Leadout: %6d\n", toc.tracks[100].lba);
 MDFN_indent(-1);

 EmuThreadQueue->Write(CDIF_Message(CDIF_MSG_INIT_DONE, TRUE, 0, 0, 0, NULL));

 while(Running)
 {
  CDIF_Message msg;

  // Only do a blocking-wait for a message if we don't have any sectors to read-ahead.
  if(ReadThreadQueue->Read(&msg, ra_count ? FALSE : TRUE))
  {
   switch(msg.message)
   {
    case CDIF_MSG_DIEDIEDIE: Running = FALSE;
 		 	 break;

    case CDIF_MSG_READ_SECTOR:
			 {
			  uint32 new_lba = msg.args[0];

			  if(new_lba == (last_read_lba + 1))
			  {
			   //if(ra_count < 8)
			   // ra_count += 2;
			   //else
			    ra_count++;
			  }
			  else
			  {
                           ra_lba = new_lba;
			   ra_count = 4;
			  }
			  last_read_lba = new_lba;
			 }
			 break;
   }
  }

  // Don't read >= the "end" of the disc, silly snake.  Slither.
  if(ra_count && ra_lba == toc.tracks[100].lba)
  {
   ra_count = 0;
   //printf("Ephemeral scarabs: %d!\n", ra_lba);
  }

  if(ra_count)
  {
   uint8 tmpbuf[2352 + 96];

   if(!cdrfile_read_raw_sector(p_cdrfile, tmpbuf, ra_lba))
   {
    printf("Sector %d read error!  Abandon ship!", ra_lba);
    memset(tmpbuf, 0, sizeof(tmpbuf));
   }
   MDFND_LockMutex(SBMutex);

   SectorBuffers[SBWritePos].lba = ra_lba;
   memcpy(SectorBuffers[SBWritePos].data, tmpbuf, 2352 + 96);
   SectorBuffers[SBWritePos].valid = TRUE;
   SBWritePos = (SBWritePos + 1) % SBSize;

   MDFND_UnlockMutex(SBMutex);

   ra_lba++;
   ra_count--;
  }
 }

 if(p_cdrfile)
 {
  cdrfile_destroy(p_cdrfile);
  p_cdrfile = NULL;
 }

 return(1);
}

bool CDIF_Open(const char *device_name)
{
 CDIF_Message msg;

 ReadThreadQueue = new CDIF_Queue();
 EmuThreadQueue = new CDIF_Queue();
 
 SBMutex = MDFND_CreateMutex();
 SectorBuffers = (CDIF_Sector_Buffer *)calloc(SBSize, sizeof(CDIF_Sector_Buffer));

 SBWritePos = 0;
 ra_lba = 0;
 ra_count = 0;
 last_read_lba = -1;

 CDReadThread = MDFND_CreateThread(ReadThreadStart, device_name ? strdup(device_name) : NULL);

 EmuThreadQueue->Read(&msg);

 if(!msg.args[0])
 {
  MDFND_WaitThread(CDReadThread, NULL);
  delete ReadThreadQueue;
  delete EmuThreadQueue;

  ReadThreadQueue = NULL;
  EmuThreadQueue = NULL;

  return(FALSE);
 }

 LEC_Eval = MDFN_GetSettingB("cdrom.lec_eval");
 if(LEC_Eval)
 {
  Init_LEC_Correct();
 }

 MDFN_printf(_("Raw rip data correction using L-EC: %s\n\n"), LEC_Eval ? _("Enabled") : _("Disabled"));

 return(TRUE);
}

bool CDIF_ValidateRawSector(uint8 *buf)
{
 int mode = buf[12 + 3];

 if(mode != 0x1 && mode != 0x2)
  return(false);

 if(LEC_Eval || cdrfile_is_physical(p_cdrfile))
 {
  if(!ValidateRawSector(buf, mode == 2))
   return(false);
 }
 return(true);
}

bool CDIF_Close(void)
{
 ReadThreadQueue->Write(CDIF_Message(CDIF_MSG_DIEDIEDIE, 0, 0, 0, 0, NULL));
 MDFND_WaitThread(CDReadThread, NULL);

 if(SectorBuffers)
 {
  free(SectorBuffers);
  SectorBuffers = NULL;
 }

 if(ReadThreadQueue)
 {
  delete ReadThreadQueue;
  ReadThreadQueue = NULL;
 }

 if(EmuThreadQueue)
 {
  delete EmuThreadQueue;
  EmuThreadQueue = NULL;
 }

 if(SBMutex)
 {
  MDFND_DestroyMutex(SBMutex);
  SBMutex = NULL;
 }

 return(1);
}

uint32 CDIF_GetTrackStartPositionLBA(int32 track)
{
 if(track == (toc.last_track + 1)) // Leadout track.
  track = 100;
 else if(track < toc.first_track || track > toc.last_track)
 {
  assert(0);
  return(0);
 }

 return(toc.tracks[track].lba);
}

int CDIF_FindTrackByLBA(uint32 LBA)
{
 for(int32 track = toc.first_track; track <= (toc.last_track + 1); track++)
 {
  if(track == (toc.last_track + 1))
  {
   if(LBA < toc.tracks[100].lba)
    return(track - 1);
  }
  else
  {
   if(LBA < toc.tracks[track].lba)
    return(track - 1);
  }
 }
 return(0);
}

bool CDIF_ReadRawSector(uint8 *buf, uint32 lba)
{
 bool found = FALSE;

 // This shouldn't happen, the emulated-system-specific CDROM emulation code should make sure the emulated program doesn't try
 // to read past the last "real" sector of the disc.
 if(lba >= toc.tracks[100].lba)
 {
  printf("Attempt to read LBA %d, >= LBA %d\n", lba, toc.tracks[100].lba);
  return(FALSE);
 }

 ReadThreadQueue->Write(CDIF_Message(CDIF_MSG_READ_SECTOR, lba, 0, 0, 0, NULL));

 do
 {
  MDFND_LockMutex(SBMutex);

  for(int i = 0; i < SBSize; i++)
  {
   if(SectorBuffers[i].valid && SectorBuffers[i].lba == lba)
   {
    memcpy(buf, SectorBuffers[i].data, 2352 + 96);
    found = TRUE;
   }
  }

  MDFND_UnlockMutex(SBMutex);

  if(!found)
   MDFND_Sleep(1);
 } while(!found);

 return(TRUE);
}

bool CDIF_HintReadSector(uint32 lba)
{
 ReadThreadQueue->Write(CDIF_Message(CDIF_MSG_READ_SECTOR, lba, 0, 0, 0, NULL));

 return(1);
}

bool CDIF_ReadSector(uint8* pBuf, uint32 lba, uint32 nSectors)
{
 while(nSectors--)
 {
  uint8 tmpbuf[2352 + 96];

  if(!CDIF_ReadRawSector(tmpbuf, lba))
  {
   puts("CDIF Raw Read error");
   return(FALSE);
  }

  if(!CDIF_ValidateRawSector(tmpbuf))
  {
   MDFN_DispMessage(_("Uncorrectable data at sector %d"), lba);
   MDFN_PrintError(_("Uncorrectable data at sector %d"), lba);
   return(false);
  }

  const int mode = tmpbuf[12 + 3];

  if(mode == 1)
  {
   memcpy(pBuf, &tmpbuf[12 + 4], 2048);
  }
  else if(mode == 2)
  {
   memcpy(pBuf, &tmpbuf[12 + 4 + 8], 2048);
  }
  else
  {
   printf("CDIF_ReadSector() invalid sector type at LBA=%u\n", (unsigned int)lba);
   return(false);
  }

  pBuf += 2048;
  lba++;
 }

 return(true);
}

uint32 CDIF_GetTrackSectorCount(int32 track)
{
 return(cdrfile_get_track_sec_count(p_cdrfile, track));
}

bool CDIF_CheckSubQChecksum(uint8 *SubQBuf)
{
 return(cdrfile_check_subq_checksum(SubQBuf));
}

bool CDIF_ReadTOC(CD_TOC *read_target)
{
 *read_target = toc;

 return(TRUE);
}


bool CDIF_DumpCD(const char *fn)
{
 FILE *fp;

 if(!(fp = fopen(fn, "wb")))
 {
  ErrnoHolder ene(errno);

  printf("File open error: %s\n", ene.StrError());
  return(0);
 }

 for(long long i = 0; i < toc.tracks[100].lba; i++)
 {
  uint8 buf[2352 + 96];

  CDIF_ReadRawSector(buf, i);

  if(fwrite(buf, 1, 2352 + 96, fp) != 2352 + 96)
  {
   ErrnoHolder ene(errno);

   printf("File write error: %s\n", ene.StrError());
  }
 }

 if(fclose(fp))
 {
  ErrnoHolder ene(errno);

  printf("fclose error: %s\n", ene.StrError());
 }

 return(1);
}
