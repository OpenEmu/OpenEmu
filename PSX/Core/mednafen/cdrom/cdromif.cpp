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
#include <trio/trio.h>
#include "cdromif.h"
#include "CDAccess.h"
#include "../general.h"

#include <algorithm>

using namespace CDUtility;

enum
{
 // Status/Error messages
 CDIF_MSG_DONE = 0,		// Read -> emu. args: No args.
 CDIF_MSG_INFO,			// Read -> emu. args: str_message
 CDIF_MSG_FATAL_ERROR,		// Read -> emu. args: *TODO ARGS*

 //
 // Command messages.
 //
 CDIF_MSG_DIEDIEDIE,		// Emu -> read

 CDIF_MSG_READ_SECTOR,		/* Emu -> read
					args[0] = lba
				*/

 CDIF_MSG_EJECT,		// Emu -> read, args[0]; 0=insert, 1=eject
};

class CDIF_Message
{
 public:

 CDIF_Message();
 CDIF_Message(unsigned int message_, uint32 arg0 = 0, uint32 arg1 = 0, uint32 arg2 = 0, uint32 arg3 = 0);
 CDIF_Message(unsigned int message_, const std::string &str);
 ~CDIF_Message();

 unsigned int message;
 uint32 args[4];
 void *parg;
 std::string str_message;
};

class CDIF_Queue
{
 public:

 CDIF_Queue();
 ~CDIF_Queue();

 bool Read(CDIF_Message *message, bool blocking = TRUE);

 void Write(const CDIF_Message &message);

 private:
 std::queue<CDIF_Message> ze_queue;
 MDFN_Mutex *ze_mutex;
};


typedef struct
{
 bool valid;
 bool error;
 uint32 lba;
 uint8 data[2352 + 96];
} CDIF_Sector_Buffer;

// TODO: prohibit copy constructor
class CDIF_MT : public CDIF
{
 public:

 CDIF_MT(CDAccess *cda);
 virtual ~CDIF_MT();

 virtual void HintReadSector(uint32 lba);
 virtual bool ReadRawSector(uint8 *buf, uint32 lba);

 // Return true if operation succeeded or it was a NOP(either due to not being implemented, or the current status matches eject_status).
 // Returns false on failure(usually drive error of some kind; not completely fatal, can try again).
 virtual bool Eject(bool eject_status);

 // FIXME: Semi-private:
 int ReadThreadStart(void);

 private:

 CDAccess *disc_cdaccess;

 MDFN_Thread *CDReadThread;

 // Queue for messages to the read thread.
 CDIF_Queue ReadThreadQueue;

 // Queue for messages to the emu thread.
 CDIF_Queue EmuThreadQueue;


 enum { SBSize = 256 };
 CDIF_Sector_Buffer SectorBuffers[SBSize];

 uint32 SBWritePos;
 
 MDFN_Mutex *SBMutex;


 //
 // Read-thread-only:
 //
 void RT_EjectDisc(bool eject_status, bool skip_actual_eject = false);

 uint32 ra_lba;
 int ra_count;
 uint32 last_read_lba;
};


// TODO: prohibit copy constructor
class CDIF_ST : public CDIF
{
 public:

 CDIF_ST(CDAccess *cda);
 virtual ~CDIF_ST();

 virtual void HintReadSector(uint32 lba);
 virtual bool ReadRawSector(uint8 *buf, uint32 lba);
 virtual bool Eject(bool eject_status);

 private:
 CDAccess *disc_cdaccess;
};

CDIF::CDIF() : UnrecoverableError(false), is_phys_cache(false), DiscEjected(false)
{

}

CDIF::~CDIF()
{

}


CDIF_Message::CDIF_Message()
{
 message = 0;

 memset(args, 0, sizeof(args));
}

CDIF_Message::CDIF_Message(unsigned int message_, uint32 arg0, uint32 arg1, uint32 arg2, uint32 arg3)
{
 message = message_;
 args[0] = arg0;
 args[1] = arg1;
 args[2] = arg2;
 args[3] = arg3;
}

CDIF_Message::CDIF_Message(unsigned int message_, const std::string &str)
{
 message = message_;
 str_message = str;
}

CDIF_Message::~CDIF_Message()
{

}

CDIF_Queue::CDIF_Queue()
{
 ze_mutex = MDFND_CreateMutex();
}

CDIF_Queue::~CDIF_Queue()
{
 MDFND_DestroyMutex(ze_mutex);
}

// Returns FALSE if message not read, TRUE if it was read.  Will always return TRUE if "blocking" is set.
// Will throw MDFN_Error if the read message code is CDIF_MSG_FATAL_ERROR
bool CDIF_Queue::Read(CDIF_Message *message, bool blocking)
{
  TryAgain:

  MDFND_LockMutex(ze_mutex);
  
  if(ze_queue.size() > 0)
  {
   *message = ze_queue.front();
   ze_queue.pop();
   MDFND_UnlockMutex(ze_mutex);

   if(message->message == CDIF_MSG_FATAL_ERROR)
    throw MDFN_Error(0, "%s", message->str_message.c_str());

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

void CDIF_Queue::Write(const CDIF_Message &message)
{
 MDFND_LockMutex(ze_mutex);

 ze_queue.push(message);

 MDFND_UnlockMutex(ze_mutex);
}


void CDIF_MT::RT_EjectDisc(bool eject_status, bool skip_actual_eject)
{
 int32 old_de = DiscEjected;

 DiscEjected = eject_status;

 if(old_de != DiscEjected)
 {
  if(!skip_actual_eject)
   disc_cdaccess->Eject(eject_status);

  if(!eject_status)	// Re-read the TOC
  {
   disc_cdaccess->Read_TOC(&disc_toc);

   if(disc_toc.first_track < 1 || disc_toc.last_track > 99 || disc_toc.first_track > disc_toc.last_track)
   {
    throw(MDFN_Error(0, _("TOC first(%d)/last(%d) track numbers bad."), disc_toc.first_track, disc_toc.last_track));
   }
  }

  SBWritePos = 0;
  ra_lba = 0;
  ra_count = 0;
  last_read_lba = ~0U;
  memset(SectorBuffers, 0, SBSize * sizeof(CDIF_Sector_Buffer));
 }
}

struct RTS_Args
{
 CDIF_MT *cdif_ptr;
};

static int ReadThreadStart_C(void *v_arg)
{
 RTS_Args *args = (RTS_Args *)v_arg;

 return args->cdif_ptr->ReadThreadStart();
}

int CDIF_MT::ReadThreadStart()
{
 bool Running = TRUE;

 DiscEjected = true;
 SBWritePos = 0;
 ra_lba = 0;
 ra_count = 0;
 last_read_lba = ~0U;

 try
 {
  RT_EjectDisc(false, true);
 }
 catch(std::exception &e)
 {
  EmuThreadQueue.Write(CDIF_Message(CDIF_MSG_FATAL_ERROR, std::string(e.what())));
  return(0);
 }

 is_phys_cache = disc_cdaccess->Is_Physical();

 EmuThreadQueue.Write(CDIF_Message(CDIF_MSG_DONE));

 while(Running)
 {
  CDIF_Message msg;

  // Only do a blocking-wait for a message if we don't have any sectors to read-ahead.
  // MDFN_DispMessage("%d %d %d\n", last_read_lba, ra_lba, ra_count);
  if(ReadThreadQueue.Read(&msg, ra_count ? FALSE : TRUE))
  {
   switch(msg.message)
   {
    case CDIF_MSG_DIEDIEDIE:
			 Running = FALSE;
 		 	 break;

    case CDIF_MSG_EJECT:
			try
			{
			 RT_EjectDisc(msg.args[0]);
			 EmuThreadQueue.Write(CDIF_Message(CDIF_MSG_DONE));
			}
			catch(std::exception &e)
			{
			 EmuThreadQueue.Write(CDIF_Message(CDIF_MSG_FATAL_ERROR, std::string(e.what())));
			}
			break;

    case CDIF_MSG_READ_SECTOR:
			 {
                          static const int max_ra = 16;
			  static const int initial_ra = 1;
			  static const int speedmult_ra = 2;
			  uint32 new_lba = msg.args[0];

			  assert((unsigned int)max_ra < (SBSize / 4));

			  if(last_read_lba != ~0U && new_lba == (last_read_lba + 1))
			  {
			   int how_far_ahead = ra_lba - new_lba;

			   if(how_far_ahead <= max_ra)
			    ra_count = std::min(speedmult_ra, 1 + max_ra - how_far_ahead);
			   else
			    ra_count++;
			  }
			  else if(new_lba != last_read_lba)
			  {
                           ra_lba = new_lba;
			   ra_count = initial_ra;
			  }

			  last_read_lba = new_lba;
			 }
			 break;
   }
  }

  // Don't read >= the "end" of the disc, silly snake.  Slither.
  if(ra_count && ra_lba == disc_toc.tracks[100].lba)
  {
   ra_count = 0;
   //printf("Ephemeral scarabs: %d!\n", ra_lba);
  }

  if(ra_count)
  {
   uint8 tmpbuf[2352 + 96];
   bool error_condition = false;

   try
   {
    disc_cdaccess->Read_Raw_Sector(tmpbuf, ra_lba);
   }
   catch(std::exception &e)
   {
    MDFN_PrintError(_("Sector %u read error: %s"), ra_lba, e.what());
    memset(tmpbuf, 0, sizeof(tmpbuf));
    error_condition = true;
   }
   
   MDFND_LockMutex(SBMutex);

   SectorBuffers[SBWritePos].lba = ra_lba;
   memcpy(SectorBuffers[SBWritePos].data, tmpbuf, 2352 + 96);
   SectorBuffers[SBWritePos].valid = TRUE;
   SectorBuffers[SBWritePos].error = error_condition;
   SBWritePos = (SBWritePos + 1) % SBSize;

   MDFND_UnlockMutex(SBMutex);

   ra_lba++;
   ra_count--;
  }
 }

 return(1);
}

CDIF_MT::CDIF_MT(CDAccess *cda) : disc_cdaccess(cda), CDReadThread(NULL), SBMutex(NULL)
{
 try
 {
  CDIF_Message msg;
  RTS_Args s;

  SBMutex = MDFND_CreateMutex();
  UnrecoverableError = false;

  s.cdif_ptr = this;

  CDReadThread = MDFND_CreateThread(ReadThreadStart_C, &s);
  EmuThreadQueue.Read(&msg);
 }
 catch(...)
 {
  if(CDReadThread)
  {
   MDFND_WaitThread(CDReadThread, NULL);
   CDReadThread = NULL;
  }

  if(SBMutex)
  {
   MDFND_DestroyMutex(SBMutex);
   SBMutex = NULL;
  }

  if(disc_cdaccess)
  {
   delete disc_cdaccess;
   disc_cdaccess = NULL;
  }

  throw;
 }
}


CDIF_MT::~CDIF_MT()
{
 bool thread_murdered_with_kitchen_knife = false;

 try
 {
  ReadThreadQueue.Write(CDIF_Message(CDIF_MSG_DIEDIEDIE));
 }
 catch(std::exception &e)
 {
  MDFND_PrintError(e.what());
  MDFND_KillThread(CDReadThread);
  thread_murdered_with_kitchen_knife = true;
 }

 if(!thread_murdered_with_kitchen_knife)
  MDFND_WaitThread(CDReadThread, NULL);

 if(SBMutex)
 {
  MDFND_DestroyMutex(SBMutex);
  SBMutex = NULL;
 }

 if(disc_cdaccess)
 {
  delete disc_cdaccess;
  disc_cdaccess = NULL;
 }
}

bool CDIF::ValidateRawSector(uint8 *buf)
{
 int mode = buf[12 + 3];

 if(mode != 0x1 && mode != 0x2)
  return(false);

 if(!edc_lec_check_correct(buf, mode == 2))
  return(false);

 return(true);
}

bool CDIF_MT::ReadRawSector(uint8 *buf, uint32 lba)
{
 bool found = FALSE;
 bool error_condition = false;

 if(UnrecoverableError)
 {
  memset(buf, 0, 2352 + 96);
  return(false);
 }

 // This shouldn't happen, the emulated-system-specific CDROM emulation code should make sure the emulated program doesn't try
 // to read past the last "real" sector of the disc.
 if(lba >= disc_toc.tracks[100].lba)
 {
  printf("Attempt to read LBA %d, >= LBA %d\n", lba, disc_toc.tracks[100].lba);
  return(FALSE);
 }

 ReadThreadQueue.Write(CDIF_Message(CDIF_MSG_READ_SECTOR, lba));

 do
 {
  MDFND_LockMutex(SBMutex);

  for(int i = 0; i < SBSize; i++)
  {
   if(SectorBuffers[i].valid && SectorBuffers[i].lba == lba)
   {
    error_condition = SectorBuffers[i].error;
    memcpy(buf, SectorBuffers[i].data, 2352 + 96);
    found = TRUE;
   }
  }

  MDFND_UnlockMutex(SBMutex);

  if(!found)
   MDFND_Sleep(1);
 } while(!found);

 return(!error_condition);
}

void CDIF_MT::HintReadSector(uint32 lba)
{
 if(UnrecoverableError)
  return;

 ReadThreadQueue.Write(CDIF_Message(CDIF_MSG_READ_SECTOR, lba));
}

int CDIF::ReadSector(uint8* pBuf, uint32 lba, uint32 nSectors)
{
 int ret = 0;

 if(UnrecoverableError)
  return(false);

 while(nSectors--)
 {
  uint8 tmpbuf[2352 + 96];

  if(!ReadRawSector(tmpbuf, lba))
  {
   puts("CDIF Raw Read error");
   return(FALSE);
  }

  if(!ValidateRawSector(tmpbuf))
  {
   MDFN_DispMessage(_("Uncorrectable data at sector %d"), lba);
   MDFN_PrintError(_("Uncorrectable data at sector %d"), lba);
   return(false);
  }

  const int mode = tmpbuf[12 + 3];

  if(!ret)
   ret = mode;

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

 return(ret);
}

bool CDIF_MT::Eject(bool eject_status)
{
 if(UnrecoverableError)
  return(false);

 try
 {
  CDIF_Message msg;

  ReadThreadQueue.Write(CDIF_Message(CDIF_MSG_EJECT, eject_status));
  EmuThreadQueue.Read(&msg);
 }
 catch(std::exception &e)
 {
  MDFN_PrintError(_("Error on eject/insert attempt: %s"), e.what());
  return(false);
 }

 return(true);
}

//
//
// Single-threaded implementation follows.
//
//

CDIF_ST::CDIF_ST(CDAccess *cda) : disc_cdaccess(cda)
{
 //puts("***WARNING USING SINGLE-THREADED CD READER***");

 is_phys_cache = disc_cdaccess->Is_Physical();
 UnrecoverableError = false;
 DiscEjected = false;

 disc_cdaccess->Read_TOC(&disc_toc);

 if(disc_toc.first_track < 1 || disc_toc.last_track > 99 || disc_toc.first_track > disc_toc.last_track)
 {
  throw(MDFN_Error(0, _("TOC first(%d)/last(%d) track numbers bad."), disc_toc.first_track, disc_toc.last_track));
 }
}

CDIF_ST::~CDIF_ST()
{
 if(disc_cdaccess)
 {
  delete disc_cdaccess;
  disc_cdaccess = NULL;
 }
}

void CDIF_ST::HintReadSector(uint32 lba)
{
 // TODO: disc_cdaccess seek hint? (probably not, would require asynchronousitycamel)
}

bool CDIF_ST::ReadRawSector(uint8 *buf, uint32 lba)
{
 if(UnrecoverableError)
 {
  memset(buf, 0, 2352 + 96);
  return(false);
 }

 try
 {
  disc_cdaccess->Read_Raw_Sector(buf, lba);
 }
 catch(std::exception &e)
 {
  MDFN_PrintError(_("Sector %u read error: %s"), lba, e.what());
  memset(buf, 0, 2352 + 96);
  return(false);
 }

 return(true);
}

bool CDIF_ST::Eject(bool eject_status)
{
 if(UnrecoverableError)
  return(false);

 try
 {
  int32 old_de = DiscEjected;

  DiscEjected = eject_status;

  if(old_de != DiscEjected)
  {
   disc_cdaccess->Eject(eject_status);

   if(!eject_status)     // Re-read the TOC
   {
    disc_cdaccess->Read_TOC(&disc_toc);

    if(disc_toc.first_track < 1 || disc_toc.last_track > 99 || disc_toc.first_track > disc_toc.last_track)
    {
     throw(MDFN_Error(0, _("TOC first(%d)/last(%d) track numbers bad."), disc_toc.first_track, disc_toc.last_track));
    }
   }
  }
 }
 catch(std::exception &e)
 {
  MDFN_PrintError("%s", e.what());
  return(false);
 }

 return(true);
}


class CDIF_Stream_Thing : public Stream
{
 public:

 CDIF_Stream_Thing(CDIF *cdintf_arg, uint32 lba_arg, uint32 sector_count_arg);
 ~CDIF_Stream_Thing();

 virtual uint64 attributes(void);
 virtual uint8 *map(void);
 virtual void unmap(void);
  
 virtual uint64 read(void *data, uint64 count, bool error_on_eos = true);
 virtual void write(const void *data, uint64 count);

 virtual void seek(int64 offset, int whence);
 virtual int64 tell(void);
 virtual int64 size(void);
 virtual void close(void);

 private:
 CDIF *cdintf;
 const uint32 start_lba;
 const uint32 sector_count;
 int64 position;
};

CDIF_Stream_Thing::CDIF_Stream_Thing(CDIF *cdintf_arg, uint32 start_lba_arg, uint32 sector_count_arg) : cdintf(cdintf_arg), start_lba(start_lba_arg), sector_count(sector_count_arg)
{

}

CDIF_Stream_Thing::~CDIF_Stream_Thing()
{

}

uint64 CDIF_Stream_Thing::attributes(void)
{
 return(ATTRIBUTE_READABLE | ATTRIBUTE_SEEKABLE);
}

uint8 *CDIF_Stream_Thing::map(void)
{
 return NULL;
}

void CDIF_Stream_Thing::unmap(void)
{

}
  
uint64 CDIF_Stream_Thing::read(void *data, uint64 count, bool error_on_eos)
{
 if(count > (((uint64)sector_count * 2048) - position))
 {
  if(error_on_eos)
  {
   throw MDFN_Error(0, "EOF");
  }

  count = ((uint64)sector_count * 2048) - position;
 }

 if(!count)
  return(0);

 for(uint64 rp = position; rp < (position + count); rp = (rp &~ 2047) + 2048)
 {
  uint8 buf[2048];  

  if(!cdintf->ReadSector(buf, start_lba + (rp / 2048), 1))
  {
   throw MDFN_Error(ErrnoHolder(EIO));
  }
  
  //::printf("Meow: %08llx -- %08llx\n", count, (rp - position) + std::min<uint64>(2048 - (rp & 2047), count - (rp - position)));
  memcpy((uint8*)data + (rp - position), buf + (rp & 2047), std::min<uint64>(2048 - (rp & 2047), count - (rp - position)));
 }

 position += count;

 return count;
}

void CDIF_Stream_Thing::write(const void *data, uint64 count)
{
 throw MDFN_Error(ErrnoHolder(EBADF));
}

void CDIF_Stream_Thing::seek(int64 offset, int whence)
{
 int64 new_position;

 switch(whence)
 {
  default:
	throw MDFN_Error(ErrnoHolder(EINVAL));
	break;

  case SEEK_SET:
	new_position = offset;
	break;

  case SEEK_CUR:
	new_position = position + offset;
	break;

  case SEEK_END:
	new_position = ((int64)sector_count * 2048) + offset;
	break;
 }

 if(new_position < 0 || new_position > ((int64)sector_count * 2048))
  throw MDFN_Error(ErrnoHolder(EINVAL));

 position = new_position;
}

int64 CDIF_Stream_Thing::tell(void)
{
 return position;
}

int64 CDIF_Stream_Thing::size(void)
{
 return(sector_count * 2048);
}

void CDIF_Stream_Thing::close(void)
{

}


Stream *CDIF::MakeStream(uint32 lba, uint32 sector_count)
{
 return new CDIF_Stream_Thing(this, lba, sector_count);
}


CDIF *CDIF_Open(const char *path, bool image_memcache)
{
 CDAccess *cda = cdaccess_open(path, image_memcache);

 if(cda->Is_Physical() || !image_memcache)
  return new CDIF_MT(cda);
 else
  return new CDIF_ST(cda); 
}
